#include "mapa.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../../common/lector_de_mapas_tmx/lector_de_mapas.h"

namespace {

const std::map<std::string, uint8_t>& propiedades_disponibles() {
    static const std::map<std::string, uint8_t> propiedades{
        {"BLOQUEA", PropiedadDeCasilla::BLOQUEA},
        {"CULTIVABLE", PropiedadDeCasilla::CULTIVABLE},
        {"DECORABLE", PropiedadDeCasilla::DECORABLE},
        {"ADMITE_HIERBA", PropiedadDeCasilla::ADMITE_HIERBA},
        {"ADMITE_MINERAL", PropiedadDeCasilla::ADMITE_MINERAL},
        {"ADMITE_ARBOL", PropiedadDeCasilla::ADMITE_ARBOL},
        {"NINGUNA", PropiedadDeCasilla::NINGUNA},
    };
    return propiedades;
}

}  // namespace

Mapa::Mapa(
    const ConfiguracionMapaServidor& configuracion,
    const std::vector<std::string>& destinos_que_conservan_colision,
    bool todas_conservan_colision) {
    const LectorDeMapasTMX lector(configuracion.ruta_tmx);

    leer_tamanio(lector, configuracion.ruta_tmx);
    cargar_colisiones(lector, configuracion);
    cargar_capas_opcionales(lector, configuracion);
    cargar_tierra_cultivable(lector, configuracion);
    cargar_transiciones(lector, configuracion);
    if (!todas_conservan_colision) {
        ajustar_colisiones_de_transiciones(destinos_que_conservan_colision);
    }
    cargar_puestos(lector, configuracion.ruta_tmx);
}

void Mapa::leer_tamanio(const LectorDeMapasTMX& lector, const std::string& ruta_tmx) {
    const TamanioMapa tamanio = lector.leer_tamanio();

    const std::size_t cantidad_de_casillas =
        static_cast<std::size_t>(tamanio.ancho_en_casillas) * tamanio.alto_en_casillas;
    if (cantidad_de_casillas == 0) {
        throw std::runtime_error("no se pudo leer el tamaño del mapa " + ruta_tmx);
    }
    if (tamanio.ancho_de_casilla <= 0 || tamanio.alto_de_casilla <= 0) {
        throw std::runtime_error("no se pudo leer el tamaño de las casillas del mapa " + ruta_tmx);
    }

    ancho_en_casillas = tamanio.ancho_en_casillas;
    alto_en_casillas = tamanio.alto_en_casillas;
    ancho_de_casilla = tamanio.ancho_de_casilla;
    alto_de_casilla = tamanio.alto_de_casilla;
    propiedades.assign(cantidad_de_casillas, PropiedadDeCasilla::NINGUNA);
}

void Mapa::cargar_colisiones(
    const LectorDeMapasTMX& lector, const ConfiguracionMapaServidor& configuracion) {
    std::string nombre_capa_colisiones = configuracion.capa_colisiones;
    std::vector<int> colisiones = lector.cargar_capa(nombre_capa_colisiones);
    if (colisiones.empty()) {
        nombre_capa_colisiones = configuracion.capa_respaldo;
        colisiones = lector.cargar_capa(nombre_capa_colisiones);
    }
    if (colisiones.empty()) {
        throw std::runtime_error(
            std::string("falta la capa ") + configuracion.capa_colisiones + " (o " +
            configuracion.capa_respaldo + ") en " + configuracion.ruta_tmx);
    }
    marcar(colisiones, PropiedadDeCasilla::BLOQUEA, nombre_capa_colisiones);
}

void Mapa::cargar_capas_opcionales(
    const LectorDeMapasTMX& lector, const ConfiguracionMapaServidor& configuracion) {
    const std::map<std::string, uint8_t>& disponibles = propiedades_disponibles();
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

void Mapa::cargar_tierra_cultivable(
    const LectorDeMapasTMX& lector, const ConfiguracionMapaServidor& configuracion) {
    if (configuracion.propiedad_cultivable.empty()) {
        return;
    }
    const std::vector<int> piso = lector.cargar_capa(CAPA_DE_PISO);
    if (piso.empty()) {
        throw std::runtime_error(
            std::string("falta la capa ") + CAPA_DE_PISO + " para la tierra cultivable de " +
            configuracion.ruta_tmx);
    }
    marcar_si_es_alguno(
        piso,
        lector.ids_con_propiedad(configuracion.propiedad_cultivable, VALOR_VERDADERO_TMX),
        PropiedadDeCasilla::CULTIVABLE,
        CAPA_DE_PISO);
}

void Mapa::cargar_transiciones(
    const LectorDeMapasTMX& lector, const ConfiguracionMapaServidor& configuracion) {
    for (const TransicionTMX& transicion_tmx :
         lector.cargar_transiciones(configuracion.capa_transiciones)) {
        const Rectangulo zona{
            transicion_tmx.x,
            transicion_tmx.y,
            transicion_tmx.ancho,
            transicion_tmx.alto};
        const Transicion transicion{
            transicion_tmx.mapa_destino,
            transicion_tmx.columna_destino,
            transicion_tmx.fila_destino};
        transiciones.emplace_back(zona, transicion);
    }
}

void Mapa::cargar_puestos(const LectorDeMapasTMX& lector, const std::string& ruta_tmx) {
    for (const PuestoTMX& puesto_tmx : lector.cargar_puestos(CAPA_DE_INTERACCIONES)) {
        agregar_puesto(puesto_tmx, ruta_tmx);
    }
}

void Mapa::agregar_puesto(const PuestoTMX& puesto_tmx, const std::string& ruta_tmx) {
    const std::string donde = " (puesto de " + ruta_tmx + ")";
    if (puesto_tmx.id.empty()) {
        throw std::runtime_error("falta la propiedad id_puesto en un puesto" + donde);
    }
    if (buscar_puesto(puesto_tmx.id) != nullptr) {
        throw std::runtime_error("id_puesto repetido: " + puesto_tmx.id + donde);
    }
    if (!puesto_tmx.tiene_atencion) {
        throw std::runtime_error(
            "al puesto " + puesto_tmx.id + " le falta columna_atencion o fila_atencion" + donde);
    }
    if (!esta_dentro(puesto_tmx.columna_atencion, puesto_tmx.fila_atencion) ||
        tiene(puesto_tmx.columna_atencion, puesto_tmx.fila_atencion, PropiedadDeCasilla::BLOQUEA)) {
        throw std::runtime_error(
            "la casilla de atención del puesto " + puesto_tmx.id +
            " está fuera del mapa o bloqueada" + donde);
    }
    puestos.push_back(Puesto{
        puesto_tmx.id,
        Rectangulo{puesto_tmx.x, puesto_tmx.y, puesto_tmx.ancho, puesto_tmx.alto},
        puesto_tmx.columna_atencion,
        puesto_tmx.fila_atencion});
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

void Mapa::marcar_si_es_alguno(
    const std::vector<int>& capa, const std::vector<int>& ids, uint8_t propiedad,
    const std::string& nombre_capa) {
    if (capa.size() != propiedades.size()) {
        throw std::runtime_error("la capa " + nombre_capa + " no coincide con el tamaño del mapa");
    }
    const std::set<int> buscados(ids.begin(), ids.end());
    for (std::size_t i = 0; i < capa.size(); ++i) {
        if (buscados.contains(capa[i] & MASCARA_ID_DE_TILE)) {
            propiedades[i] = static_cast<uint8_t>(propiedades[i] | propiedad);
        }
    }
}

void Mapa::ajustar_colisiones_de_transiciones(
    const std::vector<std::string>& destinos_que_conservan_colision) {
    for (const std::pair<Rectangulo, Transicion>& par : transiciones) {
        const bool conserva_colision =
            std::find(
                destinos_que_conservan_colision.begin(),
                destinos_que_conservan_colision.end(),
                par.second.mapa_destino) != destinos_que_conservan_colision.end();
        if (conserva_colision) {
            continue;
        }

        const Rectangulo& zona = par.first;
        const int primera_columna = std::max(0, zona.x / ancho_de_casilla);
        const int ultima_columna =
            std::min(ancho_en_casillas - 1, (zona.x + zona.ancho - 1) / ancho_de_casilla);
        const int primera_fila = std::max(0, zona.y / alto_de_casilla);
        const int ultima_fila =
            std::min(alto_en_casillas - 1, (zona.y + zona.alto - 1) / alto_de_casilla);
        for (int fila = primera_fila; fila <= ultima_fila; ++fila) {
            for (int columna = primera_columna; columna <= ultima_columna; ++columna) {
                const std::size_t indice =
                    static_cast<std::size_t>(fila) * ancho_en_casillas + columna;
                propiedades[indice] =
                    static_cast<uint8_t>(propiedades[indice] & ~PropiedadDeCasilla::BLOQUEA);
            }
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

const Transicion* Mapa::transicion_en(const Rectangulo& zona) const {
    for (const std::pair<Rectangulo, Transicion>& par : transiciones) {
        if (se_superponen(zona, par.first)) {
            return &par.second;
        }
    }
    return nullptr;
}

const Transicion* Mapa::transicion_en_casilla(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return nullptr;
    }
    const Rectangulo casilla{
        columna * ancho_de_casilla,
        fila * alto_de_casilla,
        ancho_de_casilla,
        alto_de_casilla};
    return transicion_en(casilla);
}

bool Mapa::cerca_de_una_transicion(int columna, int fila) const {
    const Rectangulo alrededor{
        (columna - 1) * ancho_de_casilla,
        (fila - 1) * alto_de_casilla,
        3 * ancho_de_casilla,
        3 * alto_de_casilla};
    return transicion_en(alrededor) != nullptr;
}

const std::vector<std::pair<Rectangulo, Transicion>>& Mapa::obtener_transiciones() const {
    return transiciones;
}

const Puesto* Mapa::puesto_en_casilla(int columna, int fila) const {
    if (!esta_dentro(columna, fila)) {
        return nullptr;
    }
    const Rectangulo casilla{
        columna * ancho_de_casilla,
        fila * alto_de_casilla,
        ancho_de_casilla,
        alto_de_casilla};
    for (const Puesto& puesto : puestos) {
        if (se_superponen(casilla, puesto.zona)) {
            return &puesto;
        }
    }
    return nullptr;
}

const Puesto* Mapa::buscar_puesto(const std::string& id) const {
    for (const Puesto& puesto : puestos) {
        if (puesto.id == id) {
            return &puesto;
        }
    }
    return nullptr;
}

const std::vector<Puesto>& Mapa::obtener_puestos() const {
    return puestos;
}
