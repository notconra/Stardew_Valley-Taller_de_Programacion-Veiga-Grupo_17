#include "juego/tierra/tierra_tratada.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "configuracion/configuracion_servidor.h"
#include "juego/base/recursos/recursos_de_juego.h"
#include "juego/mundo.h"

// Estas pruebas leen config/servidor.toml y los mapas de Assets/, así que corren desde la raíz del
// repositorio (el add_test del CMakeLists usa ese directorio de trabajo).

namespace {

// Casilla de la granja que es tierra cultivable (pasto de la parcela grande).
const Casilla CULTIVABLE{13, 12};
// Casilla de la granja ocupada por la casa (Buildings): bloqueada.
const Casilla DE_LA_CASA{60, 12};

const RecursosDeJuego& recursos() {
    static const ConfiguracionServidor configuracion("config/servidor.toml");
    static const RecursosDeJuego recursos_de_juego(
        configuracion.mapas(),
        "granja",
        configuracion.juego(),
        ConfiguracionMazmorraServidor{});  // sin mazmorra: estas pruebas son de la granja
    return recursos_de_juego;
}

size_t casillas_tratadas(const EstadoTierra& estado) {
    size_t total = 0;
    for (const TierraDeUnMapa& tierra : estado.mapas) {
        total += tierra.casillas.size();
    }
    return total;
}

// Un jugador recién aparecido en la granja, con la pala y el pico en la barra.
class TratarTierraEnElMundo : public ::testing::Test {
protected:
    Mundo mundo{recursos(), 40};
    uint8_t pala = 0;
    uint8_t pico = 0;

    void SetUp() override {
        mundo.sincronizar_jugadores({1});
        const InventarioJugador inventario = mundo.obtener_inventario(1).inventario;
        for (size_t i = 0; i < inventario.ranuras.size(); ++i) {
            if (inventario.ranuras[i].tipo == TipoItem::PALA) {
                pala = static_cast<uint8_t>(i);
            } else if (inventario.ranuras[i].tipo == TipoItem::PICO) {
                pico = static_cast<uint8_t>(i);
            }
        }
    }

    // La casilla de los pies del jugador, calculada como el servidor con el hitbox por defecto
    // (centro de los pies a 8 píxeles del borde izquierdo y 28 del de arriba del sprite).
    Casilla pies() const {
        const EstadoJugador jugador = mundo.estado().jugadores.at(0);
        return Casilla{(jugador.x + 8) / 16, (jugador.y + 28) / 16};
    }

    // Usa el ítem sobre la casilla corrida (columnas, filas) desde los pies; por defecto, la de
    // abajo.
    CambiosPorComando usar(uint8_t ranura, int columnas = 0, int filas = 1) {
        mundo.atender(1, ComandoCliente{SeleccionarRanuraDeLaBarra{ranura}});
        const Casilla objetivo{pies().columna + columnas, pies().fila + filas};
        return mundo.atender(
            1,
            ComandoCliente{UsarItem{
                INDICE_RANURA_ACTIVA,
                static_cast<uint16_t>(objetivo.columna),
                static_cast<uint16_t>(objetivo.fila)}});
    }

    // Pregunta si la herramienta de la ranura tendría efecto en la casilla corrida desde los pies,
    // como hace el cliente para pintar el rectángulo verde.
    bool consultar(uint8_t ranura, int columnas = 0, int filas = 1) {
        mundo.atender(1, ComandoCliente{SeleccionarRanuraDeLaBarra{ranura}});
        const Casilla objetivo{pies().columna + columnas, pies().fila + filas};
        const CambiosPorComando cambios = mundo.atender(
            1,
            ComandoCliente{ConsultarCasilla{
                static_cast<uint16_t>(objetivo.columna),
                static_cast<uint16_t>(objetivo.fila)}});
        EXPECT_FALSE(cambios.tierra);
        EXPECT_FALSE(cambios.inventario);
        EXPECT_TRUE(cambios.consulta.has_value());
        if (!cambios.consulta.has_value()) {
            return false;
        }
        EXPECT_EQ(cambios.consulta->columna, objetivo.columna);
        EXPECT_EQ(cambios.consulta->fila, objetivo.fila);
        return cambios.consulta->tendria_efecto;
    }
};

}  // namespace

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

TEST(TierraTratada, EnUnMapaSinTierraCultivableNoSeTrataNada) {
    const Mapa& casa = recursos().mapas.at("FarmHouse");
    TierraTratada tierra(casa);

    for (int columna = 0; columna < casa.cantidad_de_columnas(); ++columna) {
        for (int fila = 0; fila < casa.cantidad_de_filas(); ++fila) {
            EXPECT_FALSE(tierra.tratar(Casilla{columna, fila}));
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

TEST(TierraTratada, ElEstadoTraeLasCasillasDelMapa) {
    TierraTratada tierra(recursos().mapas.at("granja"));
    EstadoTierra vacio;
    tierra.agregar_a(vacio, "granja");
    EXPECT_TRUE(vacio.mapas.empty());

    tierra.tratar(CULTIVABLE);
    EstadoTierra estado;
    tierra.agregar_a(estado, "granja");

    ASSERT_EQ(estado.mapas.size(), 1u);
    EXPECT_EQ(estado.mapas[0].mapa, "granja");
    ASSERT_EQ(estado.mapas[0].casillas.size(), 1u);
    EXPECT_EQ(estado.mapas[0].casillas[0], (CasillaTratada{13, 12}));
}

TEST_F(TratarTierraEnElMundo, LaPalaTrataLaCasillaElegida) {
    EXPECT_TRUE(mundo.estado_tierra().mapas.empty());

    const CambiosPorComando cambios = usar(pala);

    EXPECT_TRUE(cambios.tierra);
    EXPECT_FALSE(cambios.inventario);
    EXPECT_EQ(casillas_tratadas(mundo.estado_tierra()), 1u);
}

TEST_F(TratarTierraEnElMundo, ElPicoVuelveLaMismaCasillaATierraComun) {
    usar(pala);

    EXPECT_TRUE(usar(pico).tierra);
    EXPECT_TRUE(mundo.estado_tierra().mapas.empty());
}

TEST_F(TratarTierraEnElMundo, RepetirLaHerramientaNoCambiaNada) {
    EXPECT_TRUE(usar(pala).tierra);
    EXPECT_FALSE(usar(pala).tierra);
    EXPECT_TRUE(usar(pico).tierra);
    EXPECT_FALSE(usar(pico).tierra);
}

TEST_F(TratarTierraEnElMundo, OtroItemNoTocaLaTierra) {
    const InventarioJugador inventario = mundo.obtener_inventario(1).inventario;
    for (size_t i = 0; i < inventario.ranuras.size(); ++i) {
        const TipoItem tipo = inventario.ranuras[i].tipo;
        if (tipo != TipoItem::PALA && tipo != TipoItem::PICO) {
            EXPECT_FALSE(usar(static_cast<uint8_t>(i)).tierra);
        }
    }
    EXPECT_TRUE(mundo.estado_tierra().mapas.empty());
}

TEST_F(TratarTierraEnElMundo, SePuedeUsarEnCualquieraDeLasOchoCasillasDeAlrededor) {
    int tratadas = 0;
    for (int columnas = -1; columnas <= 1; ++columnas) {
        for (int filas = -1; filas <= 1; ++filas) {
            if ((columnas != 0 || filas != 0) && usar(pala, columnas, filas).tierra) {
                ++tratadas;
            }
        }
    }
    EXPECT_EQ(tratadas, 8);
}

TEST_F(TratarTierraEnElMundo, NoSePuedeUsarEnLaPropiaCasillaNiEnUnaLejana) {
    EXPECT_FALSE(usar(pala, 0, 0).tierra);
    EXPECT_FALSE(usar(pala, 2, 0).tierra);
    EXPECT_FALSE(usar(pala, 0, -3).tierra);
    EXPECT_TRUE(mundo.estado_tierra().mapas.empty());
}

TEST_F(TratarTierraEnElMundo, LaConsultaDiceSiLaHerramientaTendriaEfecto) {
    EXPECT_TRUE(consultar(pala));
    EXPECT_FALSE(consultar(pico));

    usar(pala);
    EXPECT_FALSE(consultar(pala));
    EXPECT_TRUE(consultar(pico));
}

TEST_F(TratarTierraEnElMundo, LaConsultaNoCambiaLaTierra) {
    consultar(pala);
    consultar(pico);
    EXPECT_TRUE(mundo.estado_tierra().mapas.empty());
}

TEST_F(TratarTierraEnElMundo, LaConsultaRespetaElAlcance) {
    EXPECT_TRUE(consultar(pala, 1, -1));
    EXPECT_FALSE(consultar(pala, 0, 0));
    EXPECT_FALSE(consultar(pala, 2, 0));
}

TEST_F(TratarTierraEnElMundo, ConOtroItemLaConsultaDaQueNo) {
    const InventarioJugador inventario = mundo.obtener_inventario(1).inventario;
    for (size_t i = 0; i < inventario.ranuras.size(); ++i) {
        const TipoItem tipo = inventario.ranuras[i].tipo;
        if (tipo != TipoItem::PALA && tipo != TipoItem::PICO) {
            EXPECT_FALSE(consultar(static_cast<uint8_t>(i)));
        }
    }
}
