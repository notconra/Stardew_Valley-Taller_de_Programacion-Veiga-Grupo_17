#include "mapa_tmx.h"

#include <iostream>
#include <sstream>
#include "externo/tinyxml2/tinyxml2.h"

using namespace tinyxml2;

std::vector<int> cargarCapaTMX(const std::string& rutaTMX, const std::string& nombreCapa) {
    std::vector<int> mapa;
    XMLDocument doc;

    if (doc.LoadFile(rutaTMX.c_str()) != XML_SUCCESS) {
        std::cout << "Error al abrir TMX: " << rutaTMX << std::endl;
        return mapa;
    }

    XMLElement* nodoMapa = doc.FirstChildElement("map");
    if (!nodoMapa) return mapa;

    for (XMLElement* nodoCapa = nodoMapa->FirstChildElement("layer"); nodoCapa != nullptr;
         nodoCapa = nodoCapa->NextSiblingElement("layer")) {
        const char* attrNombre = nodoCapa->Attribute("name");
        if (attrNombre && std::string(attrNombre) == nombreCapa) {
            XMLElement* nodoData = nodoCapa->FirstChildElement("data");
            if (nodoData && nodoData->GetText()) {
                std::stringstream ss(nodoData->GetText());
                int tileID;
                char coma;
                while (ss >> tileID) {
                    mapa.push_back(tileID);
                    ss >> coma;
                }
            }
            break;
        }
    }
    return mapa;
}
