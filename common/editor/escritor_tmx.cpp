#include "escritor_tmx.h"

#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>

EscritorTMX::EscritorTMX(
    int columnas, int filas, int ancho_casilla_pixeles, int alto_casilla_pixeles)
    : ancho_en_casillas(columnas),
      alto_en_casillas(filas),
      ancho_de_casilla(ancho_casilla_pixeles),
      alto_de_casilla(alto_casilla_pixeles),
      tilesets(),
      orden_de_capas(),
      capas_de_tiles(),
      capas_de_objetos() {
}

void EscritorTMX::agregar_tileset(const DefinicionTilesetTMX& tileset) {
    tilesets.push_back(tileset);
}

void EscritorTMX::definir_capa_de_tiles(
    const std::string& nombre_capa, const std::vector<uint32_t>& casillas) {
    if (capas_de_tiles.find(nombre_capa) == capas_de_tiles.end()) {
        orden_de_capas.push_back(nombre_capa);
    }
    capas_de_tiles[nombre_capa] = casillas;
}

void EscritorTMX::agregar_objeto(
    const std::string& nombre_capa_objetos, const ObjetoMapaTMX& objeto) {
    capas_de_objetos[nombre_capa_objetos].push_back(objeto);
}

void EscritorTMX::agregar_transicion(
    const std::string& nombre_capa_objetos, int posicion_x, int posicion_y, int ancho, int alto,
    const std::string& mapa_destino, int columna_destino, int fila_destino) {
    ObjetoMapaTMX transicion;
    transicion.nombre = "transicion";
    transicion.tipo = "transicion";
    transicion.posicion_x = posicion_x;
    transicion.posicion_y = posicion_y;
    transicion.ancho = ancho;
    transicion.alto = alto;
    transicion.propiedades["mapa_destino"] = mapa_destino;
    transicion.propiedades["columna_destino"] = std::to_string(columna_destino);
    transicion.propiedades["fila_destino"] = std::to_string(fila_destino);
    agregar_objeto(nombre_capa_objetos, transicion);
}

void EscritorTMX::agregar_puesto(
    const std::string& nombre_capa_objetos, int posicion_x, int posicion_y, int ancho, int alto,
    const std::string& identificador_puesto, int columna_atencion, int fila_atencion) {
    ObjetoMapaTMX puesto;
    puesto.nombre = identificador_puesto;
    puesto.tipo = "puesto";
    puesto.posicion_x = posicion_x;
    puesto.posicion_y = posicion_y;
    puesto.ancho = ancho;
    puesto.alto = alto;
    puesto.propiedades["tipo"] = "puesto";
    puesto.propiedades["id_puesto"] = identificador_puesto;
    puesto.propiedades["columna_atencion"] = std::to_string(columna_atencion);
    puesto.propiedades["fila_atencion"] = std::to_string(fila_atencion);
    agregar_objeto(nombre_capa_objetos, puesto);
}

bool EscritorTMX::guardar(const std::string& ruta_archivo_destino) const {
    std::ofstream archivo(ruta_archivo_destino);
    if (!archivo.is_open()) {
        return false;
    }

    archivo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    archivo << "<map version=\"1.10\" tiledversion=\"1.12.2\" orientation=\"orthogonal\" "
            << "renderorder=\"right-down\" width=\"" << ancho_en_casillas << "\" height=\""
            << alto_en_casillas << "\" tilewidth=\"" << ancho_de_casilla << "\" tileheight=\""
            << alto_de_casilla << "\" infinite=\"0\">\n";

    // 1. Escribir Tilesets
    size_t indice_tileset = 0;
    while (indice_tileset < tilesets.size()) {
        DefinicionTilesetTMX tileset = tilesets[indice_tileset];
        archivo << " <tileset firstgid=\"" << tileset.primer_identificador_global << "\" name=\""
                << tileset.nombre << "\" tilewidth=\"" << tileset.ancho_tile << "\" tileheight=\""
                << tileset.alto_tile << "\" tilecount=\"" << tileset.cantidad_de_tiles
                << "\" columns=\"" << tileset.cantidad_de_columnas << "\">\n";
        archivo << "  <image source=\"" << tileset.ruta_imagen << "\" width=\""
                << tileset.ancho_imagen << "\" height=\"" << tileset.alto_imagen << "\"/>\n";
        archivo << " </tileset>\n";
        indice_tileset++;
    }

    // 2. Escribir Capas de Tiles (en formato CSV legible)
    size_t indice_capa = 0;
    while (indice_capa < orden_de_capas.size()) {
        std::string nombre_capa = orden_de_capas[indice_capa];
        auto iterador_capa = capas_de_tiles.find(nombre_capa);
        if (iterador_capa != capas_de_tiles.end()) {
            std::vector<uint32_t> datos = iterador_capa->second;
            archivo << " <layer name=\"" << nombre_capa << "\" width=\"" << ancho_en_casillas
                    << "\" height=\"" << alto_en_casillas << "\">\n";
            archivo << "  <data encoding=\"csv\">\n";

            int fila = 0;
            while (fila < alto_en_casillas) {
                int columna = 0;
                while (columna < ancho_en_casillas) {
                    size_t indice_casilla = static_cast<size_t>(fila * ancho_en_casillas + columna);
                    uint32_t valor_gid = 0;
                    if (indice_casilla < datos.size()) {
                        valor_gid = datos[indice_casilla];
                    }
                    archivo << valor_gid;

                    bool es_ultima =
                        (fila == alto_en_casillas - 1 && columna == ancho_en_casillas - 1);
                    if (!es_ultima) {
                        archivo << ",";
                    }
                    columna++;
                }
                archivo << "\n";
                fila++;
            }
            archivo << "  </data>\n";
            archivo << " </layer>\n";
        }
        indice_capa++;
    }

    // 3. Escribir Capas de Objetos (Transiciones, Puestos, etc.)
    int identificador_objeto = 1;
    for (auto& par_capa_objetos : capas_de_objetos) {
        archivo << " <objectgroup name=\"" << par_capa_objetos.first << "\">\n";
        size_t indice_objeto = 0;
        while (indice_objeto < par_capa_objetos.second.size()) {
            ObjetoMapaTMX objeto = par_capa_objetos.second[indice_objeto];
            archivo << "  <object id=\"" << identificador_objeto << "\" name=\"" << objeto.nombre
                    << "\" type=\"" << objeto.tipo << "\" x=\"" << objeto.posicion_x << "\" y=\""
                    << objeto.posicion_y << "\" width=\"" << objeto.ancho << "\" height=\""
                    << objeto.alto << "\">\n";

            if (!objeto.propiedades.empty()) {
                archivo << "   <properties>\n";
                for (auto& propiedad : objeto.propiedades) {
                    archivo << "    <property name=\"" << propiedad.first << "\" value=\""
                            << propiedad.second << "\"/>\n";
                }
                archivo << "   </properties>\n";
            }
            archivo << "  </object>\n";
            identificador_objeto++;
            indice_objeto++;
        }
        archivo << " </objectgroup>\n";
    }

    archivo << "</map>\n";
    return true;
}
