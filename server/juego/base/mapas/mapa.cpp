#include "mapa.h"

#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../../../common/lector_de_mapas_tmx/lector_de_mapas.h"

std::map<std::string, uint8_t> Mapa::propiedades_disponibles() const {
    std::map<std::string, uint8_t> propiedades;
    propiedades["BLOQUEA"] = PropiedadDeCasilla::BLOQUEA;
    propiedades["CULTIVABLE"] = PropiedadDeCasilla::CULTIVABLE;
    propiedades["DECORABLE"] = PropiedadDeCasilla::DECORABLE;
    propiedades["ADMITE_HIERBA"] = PropiedadDeCasilla::ADMITE_HIERBA;
    propiedades["NINGUNA"] = PropiedadDeCasilla::NINGUNA;
    return propiedades;
}

Mapa::Mapa(const ConfiguracionMapaServidor& configuracion) {
    const LectorDeMapasTMX lector(configuracion.ruta_tmx);
    const TamanioMapa tamanio = lector.leer_tamanio();

    const std::size_t cantidad_de_casillas =
        static_cast<std::size_t>(tamanio.ancho_en_casillas) * tamanio.alto_en_casillas;
    if (cantidad_de_casillas == 0) {
        throw std::runtime_error("no se pudo leer el tamaño del mapa " + configuracion.ruta_tmx);
    }
    if (tamanio.ancho_de_casilla <= 0 || tamanio.alto_de_casilla <= 0) {
        throw std::runtime_error(
            "no se pudo leer el tamaño de las casillas del mapa " + configuracion.ruta_tmx);
    }

    ancho_en_casillas = tamanio.ancho_en_casillas;
    alto_en_casillas = tamanio.alto_en_casillas;
    ancho_de_casilla = tamanio.ancho_de_casilla;
    alto_de_casilla = tamanio.alto_de_casilla;
    propiedades.assign(cantidad_de_casillas, PropiedadDeCasilla::NINGUNA);

    std::vector<int> colisiones = lector.cargar_capa(configuracion.capa_colisiones);
    if (colisiones.empty()) {
        colisiones = lector.cargar_capa(configuracion.capa_respaldo);
    }
    if (colisiones.empty()) {
        throw std::runtime_error(
            std::string("falta la capa ") + configuracion.capa_colisiones + " (o " +
            configuracion.capa_respaldo + ") en " + configuracion.ruta_tmx);
    }
    marcar(colisiones, PropiedadDeCasilla::BLOQUEA, configuracion.capa_colisiones);

    const std::map<std::string, uint8_t> disponibles = propiedades_disponibles();
    for (const CapaOpcionalMapaServidor& capa_opcional : configuracion.capas_opcionales) {
        const std::map<std::string, uint8_t>::const_iterator encontrado =
            disponibles.find(capa_opcional.propiedad);
        if (encontrado == disponibles.end()) {
            throw std::runtime_error(
                "configuración: propiedad desconocida '" + capa_opcional.propiedad +
                "' en mapa.capas_opcionales");
        }

        const std::vector<int> capa = lector.cargar_capa(capa_opcional.nombre);
        if (!capa.empty()) {
            marcar(capa, encontrado->second, capa_opcional.nombre);
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

int Mapa::ancho_de_casilla_en_pixeles() const {
    return ancho_de_casilla;
}

int Mapa::alto_de_casilla_en_pixeles() const {
    return alto_de_casilla;
}

int Mapa::ancho_en_pixeles() const {
    return ancho_en_casillas * ancho_de_casilla;
}

int Mapa::alto_en_pixeles() const {
    return alto_en_casillas * alto_de_casilla;
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
