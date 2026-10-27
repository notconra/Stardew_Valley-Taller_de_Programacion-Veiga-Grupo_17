#include "lector_de_mapas.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "externo/tinyxml2/tinyxml2.h"

LectorDeMapasTMX::LectorDeMapasTMX(const std::string& ruta) : ruta_archivo(ruta) {
}

TamanioMapa LectorDeMapasTMX::leer_tamanio() const {
    tinyxml2::XMLDocument documento;

    if (documento.LoadFile(ruta_archivo.c_str()) != tinyxml2::XML_SUCCESS) {
        return {};
    }

    tinyxml2::XMLElement* nodo_mapa = documento.FirstChildElement("map");

    if (!nodo_mapa) {
        return {};
    }

    TamanioMapa tamanio;

    if (nodo_mapa->QueryIntAttribute("width", &tamanio.ancho_en_casillas) !=
            tinyxml2::XML_SUCCESS ||
        nodo_mapa->QueryIntAttribute("height", &tamanio.alto_en_casillas) !=
            tinyxml2::XML_SUCCESS) {
        return {};
    }

    return tamanio;
}

std::vector<int> LectorDeMapasTMX::cargar_capa(const std::string& nombre_capa) const {
    std::vector<int> casillas;
    tinyxml2::XMLDocument documento;

    if (documento.LoadFile(ruta_archivo.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Error al abrir el archivo TMX: " << ruta_archivo << '\n';
        return casillas;
    }

    tinyxml2::XMLElement* nodo_mapa = documento.FirstChildElement("map");

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
