#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

#include "../common/modo_uso.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/acciones/accion_usar_transicion.h"
#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/mazmorra/minerales.h"
#include "../server/juego/mundo.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/sector/sector.h"
#include "../server/juego/tierra/tierra_tratada.h"

// Estas pruebas leen config/servidor.toml y los mapas de Assets/, así que corren desde la raíz del
// repositorio (el target test del Makefile ejecuta ./build/tests desde ahí).

namespace {

// Casilla de la granja que es tierra cultivable (pasto de la parcela grande).
const Casilla CULTIVABLE{13, 12};
// Casilla de la granja ocupada por la casa (Buildings): bloqueada.
const Casilla DE_LA_CASA{60, 12};
// Columna y filas de la puerta de la casa en la granja (transición a FarmHouse).
const int COLUMNA_DE_LA_PUERTA = 65;
const int PRIMERA_FILA_DE_LA_PUERTA = 12;
const int ULTIMA_FILA_DE_LA_PUERTA = 14;

const RecursosDeJuego& recursos() {
    static const RecursosDeJuego recursos_de_juego = [] {
        const ConfiguracionServidor configuracion("config/servidor.toml");
        std::vector<ConfiguracionMapaServidor> mapas;
        for (const ConfiguracionMapaServidor& mapa : configuracion.mapas()) {
            if (mapa.id == "granja") {
                mapas.push_back(mapa);
            }
        }
        return RecursosDeJuego(
            mapas,
            "granja",
            configuracion.juego(),
            ConfiguracionMazmorraServidor{});  // sin mazmorra: estas pruebas son de la granja
    }();
    return recursos_de_juego;
}

// Guarda todo lo que el mundo manda para poder revisarlo. También se queda con el último snapshot
// y el último inventario, que es lo que el cliente usaría para dibujar.
class NotificadorDePrueba : public Notificador {
public:
    std::vector<EventoServidor> eventos;
    EstadoJuego estado;
    bool tiene_estado = false;
    std::optional<InventarioJugador> inventario;

    void a_jugador(uint32_t, const EventoServidor& evento) override {
        registrar(evento);
    }

    void a_todos(const EventoServidor& evento) override {
        registrar(evento);
    }

    void limpiar() {
        eventos.clear();
    }

    bool hubo_cambio_de_tierra(const CasillaTratada& casilla, bool tratada) const {
        for (const EventoServidor& evento : eventos) {
            const TierraCambiada* cambio = std::get_if<TierraCambiada>(&evento);
            if (cambio != nullptr && cambio->casilla == casilla && cambio->tratada == tratada) {
                return true;
            }
        }
        return false;
    }

private:
    void registrar(const EventoServidor& evento) {
        eventos.push_back(evento);
        if (const EstadoJuego* snapshot = std::get_if<EstadoJuego>(&evento)) {
            estado = *snapshot;
            tiene_estado = true;
        } else if (
            const ActualizacionInventario* actualizacion =
                std::get_if<ActualizacionInventario>(&evento)) {
            inventario = actualizacion->inventario;
        }
    }
};

// Cuántas veces aparece cada tipo de ítem en la barra.
int cantidad_de_ranuras_con(const InventarioJugador& inventario, TipoItem tipo) {
    return static_cast<int>(std::count_if(
        inventario.ranuras.begin(),
        inventario.ranuras.end(),
        [tipo](const RanuraInventario& ranura) { return ranura.tipo == tipo; }));
}

// El peso de un tipo de objeto en la lista, o -1 si el tipo no está.
double peso_de(const std::vector<PesoDeObjeto>& pesos, TipoObjeto tipo) {
    for (const PesoDeObjeto& peso : pesos) {
        if (peso.tipo == tipo) {
            return peso.peso;
        }
    }
    return -1.0;
}

// Un jugador recién aparecido en la granja, con la pala y el pico en la barra.
class TratarTierraEnElMundo : public ::testing::Test {
protected:
    NotificadorDePrueba notificador;
    Mundo mundo{recursos(), notificador, 40, {}, {}};
    uint8_t pala = 0;
    uint8_t pico = 0;

    void SetUp() override {
        AparienciaPersonaje apariencia;
        mundo.conectar(1, apariencia);
        ASSERT_TRUE(notificador.inventario.has_value());
        const std::vector<RanuraInventario>& ranuras = notificador.inventario->ranuras;
        for (std::size_t i = 0; i < ranuras.size(); ++i) {
            if (ranuras[i].tipo == TipoItem::PALA) {
                pala = static_cast<uint8_t>(i);
            } else if (ranuras[i].tipo == TipoItem::PICO) {
                pico = static_cast<uint8_t>(i);
            }
        }
    }

    // La casilla de los pies del jugador, calculada como el servidor con el hitbox por defecto
    // (centro de los pies a 8 píxeles del borde izquierdo y 28 del de arriba del sprite).
    Casilla pies() const {
        const EstadoJugador& jugador = notificador.estado.jugadores.at(0);
        return Casilla{
            static_cast<int>((jugador.x + 8) / 16),
            static_cast<int>((jugador.y + 28) / 16)};
    }

    Casilla objetivo(int columnas = 0, int filas = 1) const {
        return Casilla{pies().columna + columnas, pies().fila + filas};
    }

    void seleccionar(uint8_t ranura) {
        notificador.limpiar();
        mundo.atender(1, ComandoCliente{SeleccionarRanuraDeLaBarra{ranura}});
    }

    // Deja pasar los ticks de la acción para que el personaje quede libre otra vez. La duración
    // sale de la configuración, así el test no se rompe si se cambia en el TOML.
    void terminar_la_accion() {
        const ConfiguracionAccionesServidor& acciones = recursos().juego.acciones;
        const int ticks =
            std::max(acciones.duracion_picar_en_ticks, acciones.duracion_tratar_tierra_en_ticks) +
            1;
        for (int tick = 0; tick < ticks; ++tick) {
            mundo.actualizar();
        }
        notificador.limpiar();
    }

    void usar(uint8_t ranura, const Casilla& casilla) {
        seleccionar(ranura);
        notificador.limpiar();
        mundo.atender(
            1,
            ComandoCliente{UsarItem{
                ModoUso::PRINCIPAL,
                INDICE_RANURA_ACTIVA,
                true,
                static_cast<uint32_t>(casilla.columna),
                static_cast<uint32_t>(casilla.fila)}});
    }
};

// La acción de usar una transición probada sobre un sector real de la granja, con un resolvedor
// que solo anota a dónde lo mandaron.
class UsarTransicionEnLaGranja : public ::testing::Test {
protected:
    NotificadorDePrueba notificador;
    Sector sector{"granja", recursos(), 40};
    Personaje personaje{
        1,
        "granja",
        recursos().catalogo_de_items,
        recursos().juego.inventario.cantidad_ranuras_barra};
    std::optional<Transicion> resuelta;
    ResolverDeTransicion resolver = [this](Personaje&, const Transicion& transicion) {
        resuelta = transicion;
    };
    AccionUsarTransicion accion;

    ContextoDeAccion contexto(const Casilla& objetivo, ModoUso modo) {
        return ContextoDeAccion{
            1,
            RanuraInventario{},
            INDICE_RANURA_ACTIVA,
            objetivo,
            sector,
            personaje,
            recursos(),
            notificador,
            modo,
            resolver};
    }
};

}  // namespace

// TierraTratada (sin Mundo)

TEST(TierraTratada, LaPalaTrataUnaCasillaCultivable) {
    TierraTratada tierra(recursos().mapas.at("granja"));

    EXPECT_TRUE(tierra.tratar(CULTIVABLE));
    EXPECT_TRUE(tierra.esta_tratada(CULTIVABLE));
}

TEST(TierraTratada, NoSeTrataDosVecesLaMismaCasilla) {
    TierraTratada tierra(recursos().mapas.at("granja"));
    tierra.tratar(CULTIVABLE);

    EXPECT_FALSE(tierra.tratar(CULTIVABLE));
}

TEST(TierraTratada, VuelveATierraComun) {
    TierraTratada tierra(recursos().mapas.at("granja"));
    tierra.tratar(CULTIVABLE);

    EXPECT_TRUE(tierra.volver_a_comun(CULTIVABLE));
    EXPECT_FALSE(tierra.esta_tratada(CULTIVABLE));
    EXPECT_FALSE(tierra.volver_a_comun(CULTIVABLE));
}

TEST(TierraTratada, NoSeTrataUnaCasillaBloqueadaNiFueraDelMapa) {
    TierraTratada tierra(recursos().mapas.at("granja"));

    EXPECT_FALSE(tierra.tratar(DE_LA_CASA));
    EXPECT_FALSE(tierra.tratar(Casilla{-1, 5}));
    EXPECT_FALSE(tierra.tratar(Casilla{5, 1000}));
}

TEST(TierraTratada, NoSeTrataNingunaCasillaBloqueada) {
    const Mapa& granja = recursos().mapas.at("granja");
    TierraTratada tierra(granja);

    for (int columna = 0; columna < granja.cantidad_de_columnas(); ++columna) {
        for (int fila = 0; fila < granja.cantidad_de_filas(); ++fila) {
            if (granja.tiene(columna, fila, PropiedadDeCasilla::BLOQUEA)) {
                EXPECT_FALSE(tierra.tratar(Casilla{columna, fila}));
            }
        }
    }
}

TEST(TierraTratada, SePuedeTratarSoloLaTierraCultivableSinTratar) {
    TierraTratada tierra(recursos().mapas.at("granja"));

    EXPECT_TRUE(tierra.se_puede_tratar(CULTIVABLE));
    EXPECT_FALSE(tierra.se_puede_tratar(DE_LA_CASA));
    tierra.tratar(CULTIVABLE);
    EXPECT_FALSE(tierra.se_puede_tratar(CULTIVABLE));
}

TEST(TierraTratada, ElSnapshotTraeLasCasillasTratadas) {
    TierraTratada tierra(recursos().mapas.at("granja"));
    std::vector<CasillaTratada> vacio;
    tierra.agregar_a(vacio);
    EXPECT_TRUE(vacio.empty());

    tierra.tratar(CULTIVABLE);
    std::vector<CasillaTratada> casillas;
    tierra.agregar_a(casillas);

    ASSERT_EQ(casillas.size(), 1u);
    EXPECT_EQ(casillas[0], (CasillaTratada{13, 12}));
}

// Tierra tratada a través de Mundo

TEST_F(TratarTierraEnElMundo, LaPalaTrataLaCasillaElegida) {
    usar(pala, CULTIVABLE);

    EXPECT_TRUE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 12}, true));
}

TEST_F(TratarTierraEnElMundo, ElPicoVuelveLaMismaCasillaATierraComun) {
    usar(pala, CULTIVABLE);
    terminar_la_accion();

    usar(pico, CULTIVABLE);

    EXPECT_TRUE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 12}, false));
}

TEST_F(TratarTierraEnElMundo, RepetirLaHerramientaNoCambiaNada) {
    usar(pala, CULTIVABLE);
    terminar_la_accion();

    usar(pala, CULTIVABLE);

    EXPECT_FALSE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 12}, true));
    EXPECT_FALSE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 12}, false));
}

TEST_F(TratarTierraEnElMundo, MientrasElPersonajeEstaOcupadoNoSeRepiteLaAccion) {
    usar(pala, CULTIVABLE);
    ASSERT_TRUE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 12}, true));

    // Sin dejar pasar los ticks, el pico es ignorado.
    usar(pico, CULTIVABLE);

    EXPECT_FALSE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 12}, false));
}

TEST_F(TratarTierraEnElMundo, SePuedeUsarEnLasNueveCasillasDelAlcance) {
    int tratadas = 0;
    for (int columnas = -1; columnas <= 1; ++columnas) {
        for (int filas = -1; filas <= 1; ++filas) {
            const Casilla casilla = objetivo(columnas, filas);
            usar(pala, casilla);
            if (notificador.hubo_cambio_de_tierra(
                    CasillaTratada{
                        static_cast<uint32_t>(casilla.columna),
                        static_cast<uint32_t>(casilla.fila)},
                    true)) {
                ++tratadas;
            }
            terminar_la_accion();
        }
    }
    EXPECT_EQ(tratadas, 9);
}

TEST_F(TratarTierraEnElMundo, NoSePuedeUsarEnUnaCasillaLejana) {
    usar(pala, objetivo(2, 0));
    EXPECT_FALSE(notificador.hubo_cambio_de_tierra(CasillaTratada{15, 11}, true));
    terminar_la_accion();

    usar(pala, objetivo(0, -3));
    EXPECT_FALSE(notificador.hubo_cambio_de_tierra(CasillaTratada{13, 8}, true));
}

TEST_F(TratarTierraEnElMundo, ElSnapshotAlVolverAEntrarTraeLaTierraTratada) {
    usar(pala, CULTIVABLE);
    terminar_la_accion();

    mundo.desconectar(1);
    notificador.limpiar();
    AparienciaPersonaje apariencia;
    mundo.conectar(1, apariencia);

    ASSERT_TRUE(notificador.tiene_estado);
    ASSERT_EQ(notificador.estado.tierra.size(), 1u);
    EXPECT_EQ(notificador.estado.tierra[0], (CasillaTratada{13, 12}));
}
