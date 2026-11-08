#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <sstream>
#include <vector>

#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
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
            ConfiguracionMazmorraServidor{});
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
    jugador.x = 40.0;
    jugador.y = 120.0;
    jugador.vida = 50;
    jugador.energia = 35;
    jugador.tiene_cama = true;
    jugador.mapa_cama = MAPA_DE_LA_CASA;
    jugador.columna_cama = 2;
    jugador.fila_cama = 4;
    mundo.jugadores.push_back(jugador);

    DatosMuebleGuardado cama;
    cama.id_mapa = MAPA_DE_LA_CASA;
    cama.tipo = TipoObjeto::CAMA;
    cama.columna = 2;
    cama.fila = 4;
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
    EXPECT_TRUE(datos->tiene_cama);
    EXPECT_EQ(datos->mapa_cama, MAPA_DE_LA_CASA);
    EXPECT_EQ(datos->columna_cama, 2u);
    EXPECT_EQ(datos->fila_cama, 4u);
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
