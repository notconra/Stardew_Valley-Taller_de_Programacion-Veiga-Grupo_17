#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/mapa/mapa.h"
#include "../server/juego/mazmorra/cargador_mazmorra.h"
#include "../server/juego/recursos/recursos_de_juego.h"

#define ESCALERA_DE_SUBIDA 116
#define ESCALERA_DE_BAJADA 174

namespace {

const ConfiguracionServidor& configuracion() {
    static const ConfiguracionServidor configuracion_servidor("config/servidor.toml");
    return configuracion_servidor;
}

// Los mapas armados como los arma el juego, para que las colisiones de las escaleras sean las
// mismas que en una partida.
const RecursosDeJuego& recursos() {
    static const RecursosDeJuego recursos_de_juego(
        configuracion().mapas(),
        configuracion().mapa_inicial(),
        configuracion().juego(),
        configuracion().mazmorra());
    return recursos_de_juego;
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

}  // namespace

TEST(MapaDeLaMazmorra, ElNivelNoTieneCapaColisiones) {
    for (const ConfiguracionMapaServidor& nivel : niveles_de_la_mazmorra()) {
        EXPECT_TRUE(LectorDeMapasTMX(nivel.ruta_tmx).cargar_capa("Colisiones").empty())
            << nivel.id << ": las colisiones salen de Buildings, la capa Colisiones sobra";
    }
}

TEST(MapaDeLaMazmorra, LasParedesYLasEscalerasBloquean) {
    const std::vector<ConfiguracionMapaServidor> niveles = niveles_de_la_mazmorra();
    ASSERT_FALSE(niveles.empty());

    for (const ConfiguracionMapaServidor& nivel : niveles) {
        const Mapa& mapa = recursos().mapas.at(nivel.id);
        const std::vector<int> buildings =
            LectorDeMapasTMX(nivel.ruta_tmx).cargar_capa("Buildings");
        ASSERT_FALSE(buildings.empty()) << nivel.id;

        int escaleras = 0;
        for (size_t i = 0; i < buildings.size(); ++i) {
            if (buildings[i] == 0) {
                continue;
            }
            if (buildings[i] == ESCALERA_DE_SUBIDA || buildings[i] == ESCALERA_DE_BAJADA) {
                ++escaleras;
            }
            const int columna = static_cast<int>(i) % mapa.cantidad_de_columnas();
            const int fila = static_cast<int>(i) / mapa.cantidad_de_columnas();
            EXPECT_TRUE(mapa.tiene(columna, fila, PropiedadDeCasilla::BLOQUEA))
                << nivel.id << ": (" << columna << ", " << fila << ")";
        }
        EXPECT_EQ(escaleras, 2) << nivel.id;
    }
}

TEST(MapaDeLaMazmorra, LaEscaleraDeSubidaSeAlcanzaDesdeLaEntrada) {
    const int alcance = configuracion().juego().acciones.alcance_en_casillas;

    for (const ConfiguracionMapaServidor& nivel : niveles_de_la_mazmorra()) {
        const Mapa& mapa = recursos().mapas.at(nivel.id);
        const GeografiaNivel geografia = CargadorMazmorra::crear_geografia(nivel.id, mapa);
        bool al_alcance = false;
        for (int columna = geografia.salida.origen.columna;
             columna < geografia.salida.origen.columna + geografia.salida.ancho;
             ++columna) {
            for (int fila = geografia.salida.origen.fila;
                 fila < geografia.salida.origen.fila + geografia.salida.alto;
                 ++fila) {
                const int distancia = std::max(
                    std::abs(columna - geografia.entrada.columna),
                    std::abs(fila - geografia.entrada.fila));
                al_alcance = al_alcance || distancia <= alcance;
            }
        }
        EXPECT_TRUE(al_alcance) << nivel.id << ": la salida queda lejos de la entrada";
        EXPECT_FALSE(mapa.tiene(
            geografia.entrada.columna,
            geografia.entrada.fila,
            PropiedadDeCasilla::BLOQUEA))
            << nivel.id << ": la entrada tiene que poder pisarse";
    }
}
