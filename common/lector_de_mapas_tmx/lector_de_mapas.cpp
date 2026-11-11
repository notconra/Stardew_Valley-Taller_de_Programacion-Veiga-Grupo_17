#include "lector_de_mapas.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

ObjetoTMX::ObjetoTMX(
    int x, int y, int ancho, int alto, const std::map<std::string, std::string>& propiedades)
    : pos_x(x), pos_y(y), ancho_de_zona(ancho), alto_de_zona(alto), propiedades(propiedades) {
}

int ObjetoTMX::x() const {
    return pos_x;
}

int ObjetoTMX::y() const {
    return pos_y;
}

int ObjetoTMX::ancho() const {
    return ancho_de_zona;
}

int ObjetoTMX::alto() const {
    return alto_de_zona;
}

bool ObjetoTMX::tiene_propiedad(const std::string& propiedad) const {
    return propiedades.contains(propiedad);
}

std::string ObjetoTMX::texto_de(const std::string& propiedad) const {
    const auto encontrada = propiedades.find(propiedad);
    return encontrada == propiedades.end() ? std::string() : encontrada->second;
}

int ObjetoTMX::entero_de(const std::string& propiedad) const {
    const auto encontrada = propiedades.find(propiedad);
    return encontrada == propiedades.end() ? 0 : std::atoi(encontrada->second.c_str());
}

LectorDeMapasTMX::LectorDeMapasTMX(const std::string& ruta) : ruta_archivo(ruta) {
}

tinyxml2::XMLElement* LectorDeMapasTMX::abrir_mapa(tinyxml2::XMLDocument& documento) const {
    if (documento.LoadFile(ruta_archivo.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error al abrir el archivo TMX: " << ruta_archivo << '\n';
        return nullptr;
    }
    return documento.FirstChildElement("map");
}

std::vector<ObjetoTMX> LectorDeMapasTMX::leer_objetos(
    tinyxml2::XMLElement* nodo_mapa, const std::string& nombre_capa) const {
    std::vector<ObjetoTMX> objetos;

    tinyxml2::XMLElement* nodo_capa = nullptr;
    for (tinyxml2::XMLElement* candidato = nodo_mapa->FirstChildElement("objectgroup");
         candidato != nullptr;
         candidato = candidato->NextSiblingElement("objectgroup")) {
        const char* nombre = candidato->Attribute("name");
        if (nombre && nombre_capa == nombre) {
            nodo_capa = candidato;
            break;
        }
    }
    if (!nodo_capa) {
        return objetos;
    }

    for (tinyxml2::XMLElement* nodo_objeto = nodo_capa->FirstChildElement("object");
         nodo_objeto != nullptr;
         nodo_objeto = nodo_objeto->NextSiblingElement("object")) {
        std::map<std::string, std::string> propiedades;
        tinyxml2::XMLElement* nodo_propiedades = nodo_objeto->FirstChildElement("properties");
        if (nodo_propiedades) {
            for (tinyxml2::XMLElement* nodo_propiedad =
                     nodo_propiedades->FirstChildElement("property");
                 nodo_propiedad != nullptr;
                 nodo_propiedad = nodo_propiedad->NextSiblingElement("property")) {
                const char* nombre_propiedad = nodo_propiedad->Attribute("name");
                const char* valor = nodo_propiedad->Attribute("value");
                if (nombre_propiedad && valor) {
                    propiedades[nombre_propiedad] = valor;
                }
            }
        }
        objetos.emplace_back(
            leer_entero_de_objeto(nodo_objeto, "x"),
            leer_entero_de_objeto(nodo_objeto, "y"),
            leer_entero_de_objeto(nodo_objeto, "width"),
            leer_entero_de_objeto(nodo_objeto, "height"),
            propiedades);
    }
    return objetos;
}

int LectorDeMapasTMX::leer_entero_de_objeto(
    tinyxml2::XMLElement* nodo, const char* atributo) const {
    double valor = 0.0;
    nodo->QueryDoubleAttribute(atributo, &valor);
    return static_cast<int>(std::floor(valor));
}

TamanioMapa LectorDeMapasTMX::leer_tamanio() const {
    tinyxml2::XMLDocument documento;
    tinyxml2::XMLElement* nodo_mapa = abrir_mapa(documento);
    if (!nodo_mapa) {
        return {};
    }

    TamanioMapa tamanio;

    if (nodo_mapa->QueryIntAttribute("width", &tamanio.ancho_en_casillas) !=
            tinyxml2::XML_SUCCESS ||
        nodo_mapa->QueryIntAttribute("height", &tamanio.alto_en_casillas) !=
            tinyxml2::XML_SUCCESS ||
        nodo_mapa->QueryIntAttribute("tilewidth", &tamanio.ancho_de_casilla) !=
            tinyxml2::XML_SUCCESS ||
        nodo_mapa->QueryIntAttribute("tileheight", &tamanio.alto_de_casilla) !=
            tinyxml2::XML_SUCCESS) {
        return {};
    }

    return tamanio;
}

std::vector<int> LectorDeMapasTMX::cargar_capa(const std::string& nombre_capa) const {
    std::vector<int> casillas;
    for (std::uint32_t valor : cargar_capa_con_flags(nombre_capa)) {
        casillas.push_back(static_cast<int>(valor & MASCARA_GID));
    }
    return casillas;
}

std::vector<std::uint32_t> LectorDeMapasTMX::cargar_capa_con_flags(
    const std::string& nombre_capa) const {
    std::vector<std::uint32_t> casillas;
    tinyxml2::XMLDocument documento;
    tinyxml2::XMLElement* nodo_mapa = abrir_mapa(documento);
    if (!nodo_mapa) {
        return casillas;
    }

    for (tinyxml2::XMLElement* nodo_capa = nodo_mapa->FirstChildElement("layer");
         nodo_capa != nullptr;
         nodo_capa = nodo_capa->NextSiblingElement("layer")) {
        const char* nombre = nodo_capa->Attribute("name");
        if (!nombre || nombre_capa != nombre) {
            continue;
        }

        tinyxml2::XMLElement* nodo_datos = nodo_capa->FirstChildElement("data");
        if (nodo_datos && nodo_datos->GetText()) {
            std::stringstream flujo(nodo_datos->GetText());
            std::string token;
            while (std::getline(flujo, token, ',')) {
                const char* inicio = token.c_str();
                char* fin = nullptr;
                unsigned long long valor = std::strtoull(inicio, &fin, 10);
                if (fin == inicio) {
                    continue;  // token vacío (salto de línea final)
                }
                casillas.push_back(static_cast<std::uint32_t>(valor));
            }
        }
        break;
    }
    return casillas;
}

std::vector<TransicionTMX> LectorDeMapasTMX::cargar_transiciones(
    const std::string& nombre_capa) const {
    std::vector<TransicionTMX> transiciones;
    tinyxml2::XMLDocument documento;
    tinyxml2::XMLElement* nodo_mapa = abrir_mapa(documento);
    if (!nodo_mapa) {
        return transiciones;
    }

    for (const ObjetoTMX& objeto : leer_objetos(nodo_mapa, nombre_capa)) {
        const bool tiene_mapa_destino = objeto.tiene_propiedad("mapa_destino");
        if (!tiene_mapa_destino || objeto.ancho() <= 0 || objeto.alto() <= 0) {
            continue;
        }

        TransicionTMX transicion;
        transicion.x = objeto.x();
        transicion.y = objeto.y();
        transicion.ancho = objeto.ancho();
        transicion.alto = objeto.alto();
        transicion.mapa_destino = objeto.texto_de("mapa_destino");
        transicion.columna_destino = objeto.entero_de("columna_destino");
        transicion.fila_destino = objeto.entero_de("fila_destino");
        transiciones.push_back(transicion);
    }
    return transiciones;
}

std::vector<PuestoTMX> LectorDeMapasTMX::cargar_puestos(const std::string& nombre_capa) const {
    std::vector<PuestoTMX> puestos;
    tinyxml2::XMLDocument documento;
    tinyxml2::XMLElement* nodo_mapa = abrir_mapa(documento);
    if (!nodo_mapa) {
        return puestos;
    }

    for (const ObjetoTMX& objeto : leer_objetos(nodo_mapa, nombre_capa)) {
        if (objeto.texto_de("tipo") != "puesto") {
            continue;
        }

        PuestoTMX puesto;
        puesto.x = objeto.x();
        puesto.y = objeto.y();
        puesto.ancho = objeto.ancho();
        puesto.alto = objeto.alto();
        if (puesto.ancho <= 0 || puesto.alto <= 0) {
            puesto.ancho = 1;
            puesto.alto = 1;
        }
        puesto.id = objeto.texto_de("id_puesto");
        puesto.columna_atencion = objeto.entero_de("columna_atencion");
        puesto.fila_atencion = objeto.entero_de("fila_atencion");
        puesto.tiene_atencion =
            objeto.tiene_propiedad("columna_atencion") && objeto.tiene_propiedad("fila_atencion");
        puestos.push_back(puesto);
    }
    return puestos;
}

std::vector<int> LectorDeMapasTMX::ids_con_propiedad(
    const std::string& propiedad, const std::string& valor) const {
    std::vector<int> ids;
    tinyxml2::XMLDocument documento;
    tinyxml2::XMLElement* nodo_mapa = abrir_mapa(documento);
    if (!nodo_mapa) {
        return ids;
    }

    for (tinyxml2::XMLElement* nodo_tileset = nodo_mapa->FirstChildElement("tileset");
         nodo_tileset != nullptr;
         nodo_tileset = nodo_tileset->NextSiblingElement("tileset")) {
        int primer_id = 0;
        nodo_tileset->QueryIntAttribute("firstgid", &primer_id);

        for (tinyxml2::XMLElement* nodo_tile = nodo_tileset->FirstChildElement("tile");
             nodo_tile != nullptr;
             nodo_tile = nodo_tile->NextSiblingElement("tile")) {
            tinyxml2::XMLElement* nodo_propiedades = nodo_tile->FirstChildElement("properties");
            if (!nodo_propiedades) {
                continue;
            }
            for (tinyxml2::XMLElement* nodo_propiedad =
                     nodo_propiedades->FirstChildElement("property");
                 nodo_propiedad != nullptr;
                 nodo_propiedad = nodo_propiedad->NextSiblingElement("property")) {
                const char* nombre = nodo_propiedad->Attribute("name");
                const char* valor_propiedad = nodo_propiedad->Attribute("value");
                if (nombre == nullptr || valor_propiedad == nullptr || propiedad != nombre ||
                    valor != valor_propiedad) {
                    continue;
                }
                int id_local = 0;
                nodo_tile->QueryIntAttribute("id", &id_local);
                ids.push_back(primer_id + id_local);
                break;
            }
        }
    }
    return ids;
}
