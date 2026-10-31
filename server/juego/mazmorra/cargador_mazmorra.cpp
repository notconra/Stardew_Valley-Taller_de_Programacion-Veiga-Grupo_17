#include "cargador_mazmorra.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "mazmorra.h"

#define CAPA_PISO "Back"
#define CAPA_COLISIONES "Colisiones"
#define CAPA_COLISIONES_RESPALDO "Buildings"
#define CAPA_TRANSICIONES "Transiciones"

std::vector<std::string> CargadorMazmorra::elegir_niveles(
    const std::vector<std::string>& disponibles, std::mt19937&& generador) {
    if (disponibles.size() < CANTIDAD_NIVELES) {
        throw std::invalid_argument(
            "se necesitan al menos " + std::to_string(CANTIDAD_NIVELES) +
            " mapas de mazmorra y hay " + std::to_string(disponibles.size()));
    }
    std::vector<std::string> elegidos = disponibles;
    std::shuffle(elegidos.begin(), elegidos.end(), generador);
    elegidos.resize(CANTIDAD_NIVELES);
    return elegidos;
}

std::vector<GeografiaNivel> CargadorMazmorra::cargar_niveles(
    const std::string& carpeta, const std::vector<std::string>& nombres) {
    std::vector<GeografiaNivel> geos;
    geos.reserve(nombres.size());
    for (const std::string& nombre : nombres) {
        geos.push_back(cargar_nivel(carpeta + "/" + nombre));
    }
    return geos;
}

Zona CargadorMazmorra::a_casillas(const TransicionTMX& transicion, const TamanioMapa& tamanio) {
    const int x_inicio = std::max(0, transicion.x) / tamanio.ancho_de_casilla;
    const int y_inicio = std::max(0, transicion.y) / tamanio.ancho_de_casilla;
    const int x_fin = std::min(
        tamanio.ancho_en_casillas - 1,
        (transicion.x + transicion.ancho - 1) / tamanio.ancho_de_casilla);
    const int y_fin = std::min(
        tamanio.alto_en_casillas - 1,
        (transicion.y + transicion.alto - 1) / tamanio.alto_de_casilla);

    return Zona{
        {static_cast<uint16_t>(x_inicio), static_cast<uint16_t>(y_inicio)},
        static_cast<uint16_t>(std::max(0, x_fin - x_inicio + 1)),
        static_cast<uint16_t>(std::max(0, y_fin - y_inicio + 1))};
}

Posicion CargadorMazmorra::asignar_entrada(const GeografiaNivel& geografia) {
    return Posicion{
        static_cast<uint16_t>(geografia.salida.origen.x + geografia.salida.ancho),
        geografia.salida.origen.y};
}

GeografiaNivel CargadorMazmorra::cargar_nivel(const std::string& ruta_tmx) {
    const LectorDeMapasTMX lector(ruta_tmx);

    const TamanioMapa tamanio = lector.leer_tamanio();
    if (tamanio.ancho_en_casillas <= 0 || tamanio.alto_en_casillas <= 0 ||
        tamanio.ancho_de_casilla <= 0 || tamanio.alto_de_casilla <= 0) {
        throw std::runtime_error(ruta_tmx + ": no se pudo leer el mapa");
    }
    const size_t casillas =
        static_cast<size_t>(tamanio.ancho_en_casillas) * tamanio.alto_en_casillas;

    const std::vector<int> piso = lector.cargar_capa(CAPA_PISO);
    std::vector<int> colisiones = lector.cargar_capa(CAPA_COLISIONES);
    if (colisiones.empty()) {
        colisiones = lector.cargar_capa(CAPA_COLISIONES_RESPALDO);
    }

    if (piso.size() != casillas || colisiones.size() != casillas) {
        throw std::runtime_error(ruta_tmx + ": faltan las capas de piso o de colisiones");
    }

    GeografiaNivel geografia{};
    geografia.ancho = static_cast<uint16_t>(tamanio.ancho_en_casillas);
    geografia.alto = static_cast<uint16_t>(tamanio.alto_en_casillas);
    geografia.transitable.resize(casillas);
    for (size_t i = 0; i < casillas; ++i) {
        geografia.transitable[i] = piso[i] != 0 && colisiones[i] == 0;
    }

    int escaleras = 0;
    int salidas = 0;
    int entradas = 0;
    for (const TransicionTMX& transicion : lector.cargar_transiciones(CAPA_TRANSICIONES)) {
        const Zona zona = a_casillas(transicion, tamanio);
        if (transicion.mapa_destino == DESTINO_ESCALERA) {
            geografia.escalera = zona;
            ++escaleras;
        } else if (transicion.mapa_destino == DESTINO_ENTRADA) {
            geografia.entrada = zona.origen;
            ++entradas;
        } else {
            geografia.salida = zona;
            ++salidas;
        }
    }
    if (escaleras != 1 || salidas != 1 || entradas > 1) {
        throw std::runtime_error(
            ruta_tmx + ": debe tener exactamente una escalera (mapa_destino = \"" +
            DESTINO_ESCALERA + "\"), una salida y como mucho una entrada (\"" + DESTINO_ENTRADA +
            "\"); tiene " + std::to_string(escaleras) + ", " + std::to_string(salidas) + " y " +
            std::to_string(entradas));
    }

    if (entradas == 0) {
        geografia.entrada = asignar_entrada(geografia);
    }
    return geografia;
}
