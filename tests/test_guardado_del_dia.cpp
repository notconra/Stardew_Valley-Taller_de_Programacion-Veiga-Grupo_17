#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <sstream>
#include <vector>

#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/geometria.h"
#include "../server/juego/muebles/muebles.h"
#include "../server/juego/mundo.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/persistencia/datos_partida_guardada.h"
#include "../server/persistencia/serializador_partida.h"

// Lo que se guarda al terminar el día: el día, la vida, la energía y la última cama de cada
// jugador, y los muebles que colocaron. Estas pruebas leen config/servidor.toml y los mapas de
// Assets/, así que corren desde la raíz del repositorio.

namespace {
const uint32_t ID_JUGADOR = 1;

const RecursosDeJuego& recursos() {
    static const RecursosDeJuego recursos_de_juego = [] {
        const ConfiguracionServidor configuracion("config/servidor.toml");
        std::vector<ConfiguracionMapaServidor> mapas;
        for (const ConfiguracionMapaServidor& mapa : configuracion.mapas()) {
            if (mapa.id == "granja" || mapa.id == MAPA_DE_LA_CASA) {
                mapas.push_back(mapa);
            }
        }
        return RecursosDeJuego(
            mapas,
            "granja",
            configuracion.juego(),
            ConfiguracionMazmorraServidor{},
            configuracion.mercado());
    }();
    return recursos_de_juego;
}

class SinAvisos : public Notificador {
public:
    void a_jugador(uint32_t, const EventoServidor&) override {
    }
    void a_todos(const EventoServidor&) override {
    }
};

DatosMundoGuardado un_mundo_guardado() {
    DatosMundoGuardado mundo;
    mundo.dia = 30;

    DatosJugadorGuardado jugador;
    jugador.id_jugador = ID_JUGADOR;
    jugador.mapa_actual = MAPA_DE_LA_CASA;
    jugador.x = 640.0;
    jugador.y = 400.0;
    jugador.vida = 50;
    jugador.energia = 35;
    jugador.monedas = 75;
    jugador.tiene_cama = true;
    jugador.mapa_cama = MAPA_DE_LA_CASA;
    jugador.columna_cama = 38;
    jugador.fila_cama = 23;
    mundo.jugadores.push_back(jugador);

    DatosMuebleGuardado cama;
    cama.id_mapa = MAPA_DE_LA_CASA;
    cama.tipo = TipoObjeto::CAMA;
    cama.columna = 38;
    cama.fila = 23;
    mundo.muebles.push_back(cama);
    return mundo;
}

}  // namespace

TEST(GuardadoDelDia, ElArchivoConservaElDiaLosMueblesYElDescansoDeCadaJugador) {
    DatosPartidaGuardada partida;
    partida.codigo = "ABC123";
    partida.nombre = "granjita";
    partida.jugadores_maximos = 4;
    partida.mundo = un_mundo_guardado();

    std::stringstream archivo;
    SerializadorPartida::serializar(archivo, partida);
    const DatosPartidaGuardada leida = SerializadorPartida::deserializar(archivo);

    EXPECT_EQ(leida, partida);
    EXPECT_EQ(leida.mundo.dia, 30u);
    ASSERT_EQ(leida.mundo.muebles.size(), 1u);
    EXPECT_EQ(leida.mundo.muebles[0].tipo, TipoObjeto::CAMA);
    ASSERT_EQ(leida.mundo.jugadores.size(), 1u);
    EXPECT_EQ(leida.mundo.jugadores[0].vida, 50);
    EXPECT_EQ(leida.mundo.jugadores[0].monedas, 75u);
    EXPECT_TRUE(leida.mundo.jugadores[0].tiene_cama);
}

TEST(GuardadoDelDia, UnJugadorSinNadaGuardadoArrancaATope) {
    const DatosJugadorGuardado nuevo;
    EXPECT_EQ(nuevo.vida, VIDA_MAXIMA_JUGADOR);
    EXPECT_EQ(nuevo.energia, ENERGIA_MAXIMA_JUGADOR);
    EXPECT_FALSE(nuevo.tiene_cama);
}

TEST(GuardadoDelDia, AlRetomarVuelvenElDiaYLaCama) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, un_mundo_guardado());

    EXPECT_EQ(mundo.obtener_dia(), 30u);
    const std::vector<DatosMuebleGuardado> muebles = mundo.obtener_muebles_guardados();
    ASSERT_EQ(muebles.size(), 1u);
    EXPECT_EQ(muebles[0], un_mundo_guardado().muebles[0]);
}

TEST(GuardadoDelDia, AlRetomarElJugadorConservaVidaEnergiaYUltimaCama) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, un_mundo_guardado());
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});

    const std::optional<DatosJugadorGuardado> datos = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(datos.has_value());
    EXPECT_EQ(datos->vida, 50);
    EXPECT_EQ(datos->energia, 35);
    EXPECT_EQ(datos->monedas, 75u);
    EXPECT_TRUE(datos->tiene_cama);
    EXPECT_EQ(datos->mapa_cama, MAPA_DE_LA_CASA);
    EXPECT_EQ(datos->columna_cama, 38u);
    EXPECT_EQ(datos->fila_cama, 23u);
    EXPECT_EQ(datos->mapa_actual, MAPA_DE_LA_CASA);
}

TEST(GuardadoDelDia, ElDesconectadoSeGuardaConLoQueTeniaAlIrse) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, un_mundo_guardado());
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});
    mundo.desconectar(ID_JUGADOR);

    const std::optional<DatosJugadorGuardado> datos = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(datos.has_value());
    EXPECT_EQ(datos->vida, 50);
    EXPECT_EQ(datos->monedas, 75u);
    EXPECT_TRUE(datos->tiene_cama);
}

TEST(GuardadoDelDia, SinGuardadoPreviaArrancaElDiaUno) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40);
    EXPECT_EQ(mundo.obtener_dia(), 1u);
    EXPECT_TRUE(mundo.obtener_muebles_guardados().empty());
}

TEST(GuardadoDelDia, MientrasSigueElDiaNoHayQueGuardar) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40);
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});
    for (int i = 0; i < 100; ++i) {
        mundo.actualizar();
        EXPECT_FALSE(mundo.tomar_dia_terminado());
    }
}

namespace {
// La casilla donde tiene los pies un jugador guardado en (x, y).
Casilla pies_de(const DatosJugadorGuardado& datos) {
    const ConfiguracionMovimientoJugador& cuerpo = recursos().juego.movimiento;
    const int lado = 16;
    return Casilla{
        static_cast<int>(datos.x + cuerpo.hitbox_x + cuerpo.hitbox_ancho / 2) / lado,
        static_cast<int>(datos.y + cuerpo.hitbox_y + cuerpo.hitbox_alto) / lado - 1};
}

DatosMundoGuardado con_el_jugador(int vida, int energia) {
    DatosMundoGuardado mundo = un_mundo_guardado();
    mundo.jugadores[0].vida = static_cast<uint16_t>(vida);
    mundo.jugadores[0].energia = static_cast<uint16_t>(energia);
    return mundo;
}
}  // namespace

TEST(GuardadoDelDia, ElQueVuelveSinVidaSeDespiertaAlLadoDeLaCamaConLaMitad) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, con_el_jugador(0, 35));
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});

    const std::optional<DatosJugadorGuardado> datos = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(datos.has_value());
    EXPECT_EQ(datos->vida, VIDA_MAXIMA_JUGADOR / 2);
    EXPECT_EQ(datos->energia, ENERGIA_MAXIMA_JUGADOR / 2);
    EXPECT_EQ(datos->mapa_actual, MAPA_DE_LA_CASA);
    // Al pie de la cama (38..39, 23..25).
    EXPECT_EQ(pies_de(*datos), (Casilla{38, 26}));
}

TEST(GuardadoDelDia, ElQueVuelveSinEnergiaTambienSeDespiertaEnLaCasa) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, con_el_jugador(50, 0));
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});

    const std::optional<DatosJugadorGuardado> datos = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(datos.has_value());
    EXPECT_EQ(datos->vida, VIDA_MAXIMA_JUGADOR / 2);
    EXPECT_EQ(datos->energia, ENERGIA_MAXIMA_JUGADOR / 2);
    EXPECT_EQ(pies_de(*datos), (Casilla{38, 26}));
}

TEST(GuardadoDelDia, ElQueVuelveSinVidaPuedeSeguirJugando) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, con_el_jugador(0, 0));
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});
    for (int i = 0; i < 10; ++i) {
        mundo.actualizar();
    }

    // Sigue despierto: no se volvió a desmayar al primer tick.
    const std::optional<DatosJugadorGuardado> datos = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(datos.has_value());
    EXPECT_GT(datos->vida, 0);
    mundo.atender(ID_JUGADOR, Moverse{1, 0});
    for (int i = 0; i < 20; ++i) {
        mundo.actualizar();
    }
    const std::optional<DatosJugadorGuardado> despues = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(despues.has_value());
    EXPECT_GT(despues->x, datos->x);
}

TEST(GuardadoDelDia, ElQueSeVaAcostadoVuelveParadoAlLadoDeLaCama) {
    const ConfiguracionMovimientoJugador& cuerpo = recursos().juego.movimiento;
    DatosMundoGuardado guardado = un_mundo_guardado();
    guardado.jugadores[0].x = 38 * 16 + (16 - cuerpo.hitbox_ancho) / 2 - cuerpo.hitbox_x;
    guardado.jugadores[0].y = 27 * 16 - cuerpo.hitbox_y - cuerpo.hitbox_alto;
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40, guardado);
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});
    mundo.atender(ID_JUGADOR, Dormir{38, 25});
    const std::optional<DatosJugadorGuardado> acostado = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(acostado.has_value());
    ASSERT_EQ(acostado->x, 38 * 16 + 8);  // en el medio de la cama

    mundo.desconectar(ID_JUGADOR);
    mundo.conectar(ID_JUGADOR, AparienciaPersonaje{});

    const std::optional<DatosJugadorGuardado> datos = mundo.obtener_datos_jugador(ID_JUGADOR);
    ASSERT_TRUE(datos.has_value());
    EXPECT_EQ(datos->mapa_actual, MAPA_DE_LA_CASA);
    EXPECT_EQ(pies_de(*datos), (Casilla{38, 26}));
}
