#ifndef ESCRITOR_TMX_H
#define ESCRITOR_TMX_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../direccion.h"

struct DefinicionTilesetTMX {
    std::string nombre;
    std::string ruta_imagen;
    uint32_t primer_identificador_global;
    int ancho_tile;
    int alto_tile;
    int cantidad_de_columnas;
    int cantidad_de_tiles;
    int ancho_imagen;
    int alto_imagen;
    std::map<int, std::map<std::string, std::string>> propiedades_de_tiles;
};

struct ObjetoMapaTMX {
    std::string nombre;
    std::string tipo;
    int posicion_x;
    int posicion_y;
    int ancho;
    int alto;
    std::map<std::string, std::string> propiedades;
};

class EscritorTMX {
private:
    int ancho_en_casillas;
    int alto_en_casillas;
    int ancho_de_casilla;
    int alto_de_casilla;

    std::vector<DefinicionTilesetTMX> tilesets;
    std::vector<std::string> orden_de_capas;
    std::map<std::string, std::vector<uint32_t>> capas_de_tiles;
    std::map<std::string, std::vector<ObjetoMapaTMX>> capas_de_objetos;

public:
    EscritorTMX(
        int columnas, int filas, int ancho_casilla_pixeles = 16, int alto_casilla_pixeles = 16);

    void agregar_tileset(const DefinicionTilesetTMX& tileset);
    void definir_capa_de_tiles(
        const std::string& nombre_capa, const std::vector<uint32_t>& casillas);
    void agregar_objeto(const std::string& nombre_capa_objetos, const ObjetoMapaTMX& objeto);

    // "llegada" es opcional: el lugar con nombre del mapa destino adonde lleva (ver llegadas.h).
    void agregar_transicion(
        const std::string& nombre_capa_objetos, int posicion_x, int posicion_y, int ancho, int alto,
        const std::string& mapa_destino, int columna_destino, int fila_destino,
        const std::string& llegada = "");

    // Un lugar con nombre del mapa, en la casilla indicada.
    void agregar_llegada(
        const std::string& nombre_capa_objetos, const std::string& nombre, int columna, int fila);

    void agregar_puesto(
        const std::string& nombre_capa_objetos, int posicion_x, int posicion_y, int ancho, int alto,
        const std::string& identificador_puesto, int columna_atencion, int fila_atencion,
        int columna_npc = 0, int fila_npc = 0, Direccion orientacion_npc = Direccion::ABAJO,
        bool tiene_posicion_npc = false);

    bool guardar(const std::string& ruta_archivo_destino) const;
};

#endif  // ESCRITOR_TMX_H
