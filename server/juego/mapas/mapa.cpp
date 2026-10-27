#include "mapa.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "propiedad_de_casilla.h"

namespace {
// Capa que se pinta en Tiled solo para marcar dónde NO se puede pasar, cualquier casilla no vacía
// bloquea. Si el mapa todavía no la tiene se usa la de respaldo.
// esto no sé donde o cómo iria, configurable o x mapa
const char CAPA_DE_COLISIONES[] = "Colisiones";
const char CAPA_DE_RESPALDO[] = "Buildings";

// Capas opcionales, si el TMX no las tiene, ninguna casilla tiene esa propiedad.
// ver como se puede arreglar esto
const std::vector<std::pair<std::string, uint8_t>> CAPAS_OPCIONALES{
    {"Cultivable", PropiedadDeCasilla::CULTIVABLE},
    {"Decorable", PropiedadDeCasilla::DECORABLE},
    {"Hierba", PropiedadDeCasilla::ADMITE_HIERBA},
};
}  // namespace

Mapa::Mapa(const std::string& ruta_tmx) {
    const LectorDeMapasTMX lector(ruta_tmx);
    const TamanioMapa tamanio = lector.leer_tamanio();

    const std::size_t cantidad_de_casillas =
        static_cast<std::size_t>(tamanio.ancho_en_casillas) * tamanio.alto_en_casillas;
    if (cantidad_de_casillas == 0) {
        throw std::runtime_error("no se pudo leer el tamaño del mapa " + ruta_tmx);
    }

    ancho_en_casillas = tamanio.ancho_en_casillas;
    alto_en_casillas = tamanio.alto_en_casillas;
    propiedades.assign(cantidad_de_casillas, PropiedadDeCasilla::NINGUNA);

    std::vector<int> colisiones = lector.cargar_capa(CAPA_DE_COLISIONES);
    if (colisiones.empty()) {
        colisiones = lector.cargar_capa(CAPA_DE_RESPALDO);
    }
    if (colisiones.empty()) {
        throw std::runtime_error(
            std::string("falta la capa ") + CAPA_DE_COLISIONES + " (o " + CAPA_DE_RESPALDO +
            ") en " + ruta_tmx);
    }
    marcar(colisiones, PropiedadDeCasilla::BLOQUEA, CAPA_DE_COLISIONES);

    for (const auto& [nombre, propiedad] : CAPAS_OPCIONALES) {
        const std::vector<int> capa = lector.cargar_capa(nombre);
        if (!capa.empty()) {
            marcar(capa, propiedad, nombre);
        }
    }
}

void Mapa::marcar(const std::vector<int>& capa, uint8_t propiedad, const std::string& nombre_capa) {
    if (capa.size() != propiedades.size()) {
        throw std::runtime_error("la capa " + nombre_capa + " no coincide con el tamaño del mapa");
    }
    for (std::size_t i = 0; i < capa.size(); ++i) {
        if (capa[i] != 0) {
            propiedades[i] = static_cast<uint8_t>(propiedades[i] | propiedad);
        }
    }
}

int Mapa::cantidad_de_columnas() const {
    return ancho_en_casillas;
}

int Mapa::cantidad_de_filas() const {
    return alto_en_casillas;
}

int Mapa::ancho_en_pixeles() const {
    return ancho_en_casillas * TAMANIO_DE_CASILLA;
}

int Mapa::alto_en_pixeles() const {
    return alto_en_casillas * TAMANIO_DE_CASILLA;
}

bool Mapa::esta_dentro(int columna, int fila) const {
    return columna >= 0 && columna < ancho_en_casillas && fila >= 0 && fila < alto_en_casillas;
}

bool Mapa::tiene(int columna, int fila, uint8_t propiedad) const {
    if (!esta_dentro(columna, fila)) {
        return false;
    }
    const std::size_t indice = static_cast<std::size_t>(fila) * ancho_en_casillas + columna;
    return (propiedades[indice] & propiedad) != 0;
}
