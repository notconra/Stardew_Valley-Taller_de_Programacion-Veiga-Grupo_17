#include "lector_de_mapas.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

LectorDeMapasTMX::LectorDeMapasTMX(const std::string& ruta) : ruta_archivo(ruta) {
}

tinyxml2::XMLElement* LectorDeMapasTMX::abrir_mapa(tinyxml2::XMLDocument& documento) const {
    if (documento.LoadFile(ruta_archivo.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error al abrir el archivo TMX: " << ruta_archivo << '\n';
        return nullptr;
    }
    return documento.FirstChildElement("map");
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
            int identificador_baldosa;
            char separador;

            while (flujo >> identificador_baldosa) {
                casillas.push_back(identificador_baldosa);

                if (!(flujo >> separador)) {
                    break;
                }
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

    tinyxml2::XMLElement* nodo_capa = nullptr;
    for (tinyxml2::XMLElement* candidato = nodo_mapa->FirstChildElement("objectgroup");
         candidato != nullptr; candidato = candidato->NextSiblingElement("objectgroup")) {
        const char* nombre = candidato->Attribute("name");
        if (nombre && nombre_capa == nombre) {
            nodo_capa = candidato;
            break;
        }
    }
    if (!nodo_capa) {
        return transiciones;
    }

    for (tinyxml2::XMLElement* nodo_objeto = nodo_capa->FirstChildElement("object");
         nodo_objeto != nullptr; nodo_objeto = nodo_objeto->NextSiblingElement("object")) {
        TransicionTMX transicion;
        bool tiene_mapa_destino = false;

        nodo_objeto->QueryIntAttribute("x", &transicion.x);
        nodo_objeto->QueryIntAttribute("y", &transicion.y);
        nodo_objeto->QueryIntAttribute("width", &transicion.ancho);
        nodo_objeto->QueryIntAttribute("height", &transicion.alto);

        tinyxml2::XMLElement* nodo_propiedades = nodo_objeto->FirstChildElement("properties");
        if (nodo_propiedades) {
            for (tinyxml2::XMLElement* nodo_propiedad =
                     nodo_propiedades->FirstChildElement("property");
                 nodo_propiedad != nullptr;
                 nodo_propiedad = nodo_propiedad->NextSiblingElement("property")) {
                const char* nombre_propiedad = nodo_propiedad->Attribute("name");
                const char* valor = nodo_propiedad->Attribute("value");
                if (!nombre_propiedad || !valor) {
                    continue;
                }
                const std::string nombre_str(nombre_propiedad);
                if (nombre_str == "mapa_destino") {
                    transicion.mapa_destino = valor;
                    tiene_mapa_destino = true;
                } else if (nombre_str == "columna_destino") {
                    transicion.columna_destino = std::atoi(valor);
                } else if (nombre_str == "fila_destino") {
                    transicion.fila_destino = std::atoi(valor);
                }
            }
        }

        if (tiene_mapa_destino && transicion.ancho > 0 && transicion.alto > 0) {
            transiciones.push_back(transicion);
        }
    }

    return transiciones;
}