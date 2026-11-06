#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/colisiones/colisiones.h"
#include "../server/juego/jugadores/jugadores.h"
#include "../server/juego/mapa/mapa.h"
#include "../server/juego/mazmorra/cargador_mazmorra.h"

#define ESCALERA_DE_SUBIDA 116
#define ESCALERA_DE_BAJADA 174
#define TICKS_PARA_SALIR 200

namespace {

const ConfiguracionServidor& configuracion() {
    static const ConfiguracionServidor configuracion_servidor("config/servidor.toml");
    return configuracion_servidor;
}

std::vector<ConfiguracionMapaServidor> niveles_de_la_mazmorra() {
    std::vector<ConfiguracionMapaServidor> niveles;
    for (const ConfiguracionMapaServidor& mapa : configuracion().mapas()) {
        for (const std::string& id : configuracion().mazmorra().mapas) {
            if (mapa.id == id) {
                niveles.push_back(mapa);
            }
        }
    }
    return niveles;
}

int8_t direccion(int desde, int hasta) {
    if (hasta > desde) {
        return 1;
    }
    return hasta < desde ? -1 : 0;
}

}  // namespace

TEST(MapaDeLaMazmorra, ElNivelNoTieneCapaColisiones) {
    for (const ConfiguracionMapaServidor& nivel : niveles_de_la_mazmorra()) {
        EXPECT_TRUE(LectorDeMapasTMX(nivel.ruta_tmx).cargar_capa("Colisiones").empty())
            << nivel.id << ": las colisiones salen de Buildings, la capa Colisiones sobra";
    }
}

TEST(MapaDeLaMazmorra, LasEscalerasSePuedenPisar) {
    const std::vector<ConfiguracionMapaServidor> niveles = niveles_de_la_mazmorra();
    ASSERT_FALSE(niveles.empty());

    for (const ConfiguracionMapaServidor& nivel : niveles) {
        const Mapa mapa(nivel);
        for (const auto& [zona, transicion] : mapa.obtener_transiciones()) {
            for (int x = zona.x; x < zona.x + zona.ancho; x += mapa.ancho_de_casilla_en_pixeles()) {
                for (int y = zona.y; y < zona.y + zona.alto;
                     y += mapa.alto_de_casilla_en_pixeles()) {
                    const int columna = x / mapa.ancho_de_casilla_en_pixeles();
                    const int fila = y / mapa.alto_de_casilla_en_pixeles();
                    EXPECT_FALSE(mapa.tiene(columna, fila, PropiedadDeCasilla::BLOQUEA))
                        << nivel.id << ": la transición a '" << transicion.mapa_destino
                        << "' bloquea en (" << columna << ", " << fila << ")";
                }
            }
        }
    }
}

TEST(MapaDeLaMazmorra, LasParedesSiguenBloqueando) {
    for (const ConfiguracionMapaServidor& nivel : niveles_de_la_mazmorra()) {
        const Mapa mapa(nivel);
        const std::vector<int> buildings =
            LectorDeMapasTMX(nivel.ruta_tmx).cargar_capa("Buildings");
        ASSERT_FALSE(buildings.empty()) << nivel.id;

        int paredes = 0;
        for (size_t i = 0; i < buildings.size(); ++i) {
            const int tile = buildings[i];
            if (tile == 0 || tile == ESCALERA_DE_SUBIDA || tile == ESCALERA_DE_BAJADA) {
                continue;
            }
            ++paredes;
            const int columna = static_cast<int>(i) % mapa.cantidad_de_columnas();
            const int fila = static_cast<int>(i) / mapa.cantidad_de_columnas();
            EXPECT_TRUE(mapa.tiene(columna, fila, PropiedadDeCasilla::BLOQUEA))
                << nivel.id << ": (" << columna << ", " << fila << ")";
        }
        EXPECT_GT(paredes, 0) << nivel.id;
    }
}

TEST(MapaDeLaMazmorra, DesdeLaEntradaSeLlegaCaminandoALaSalida) {
    const ConfiguracionMovimientoJugador movimiento;
    const ConfiguracionAparicionJugador aparicion;

    for (const ConfiguracionMapaServidor& nivel : niveles_de_la_mazmorra()) {
        const Mapa mapa(nivel);
        Colisiones colisiones(mapa);
        Jugadores jugadores(mapa, colisiones, 40, movimiento, aparicion);
        const GeografiaNivel geografia = CargadorMazmorra::crear_geografia(nivel.id, mapa);
        const int ancho = mapa.ancho_de_casilla_en_pixeles();
        const int alto = mapa.alto_de_casilla_en_pixeles();

        const uint32_t id = 1;
        jugadores.agregar_en(
            id,
            geografia.entrada.columna * ancho + (ancho - movimiento.hitbox_ancho) / 2.0 -
                movimiento.hitbox_x,
            (geografia.entrada.fila + 1) * alto - movimiento.hitbox_y - movimiento.hitbox_alto);

        const Casilla objetivo = geografia.salida.origen;
        bool salio = false;
        for (int tick = 0; tick < TICKS_PARA_SALIR && !salio; ++tick) {
            const Casilla pies = *jugadores.casilla_del_jugador(id);
            const int8_t horizontal = direccion(pies.columna, objetivo.columna);
            jugadores.moverse(
                id,
                horizontal,
                horizontal == 0 ? direccion(pies.fila, objetivo.fila) : 0);
            jugadores.actualizar();
            for (const auto& [id_jugador, transicion] : jugadores.en_transicion()) {
                salio = salio || (id_jugador == id && transicion->mapa_destino == "granja");
            }
        }
        EXPECT_TRUE(salio) << nivel.id << ": el jugador no llega a pisar la escalera de subida";
    }
}
