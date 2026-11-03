#ifndef MAPA_GRAFICO_H
#define MAPA_GRAFICO_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "../configuracion/configuracion_cliente_datos.h"
#include "dibujado.h"
#include "textura.h"
#include "tileset.h"

// El mapa listo para dibujar, sus capas del TMX y las texturas de sus tilesets. Se dibuja en dos
// tandas para que los jugadores queden en el medio: lo que está debajo de ellos (suelo, edificios)
// y lo que está encima (copas de árboles, techos).
class MapaGrafico {
private:
    std::vector<Textura> texturas;
    std::vector<Tileset> tilesets;
    Dibujado dibujado;
    std::vector<std::vector<int>> capas_bajo_los_jugadores;
    std::vector<std::vector<int>> capas_sobre_los_jugadores;
    int ancho_del_mapa = 0;
    int alto_del_mapa = 0;

    std::vector<Textura> cargar_texturas(
        const std::vector<ConfiguracionTileset>& configuraciones, SDL_Renderer* renderer);

    std::vector<Tileset> crear_tilesets(const std::vector<ConfiguracionTileset>& configuraciones);

    // Lanza std::runtime_error si alguna capa no existe o está vacía.
    std::vector<std::vector<int>> cargar_capas(
        const std::string& ruta_mapa, const std::vector<std::string>& nombres);

    // Lanza std::runtime_error si no se puede leer el tamaño del mapa.
    void cargar_tamanio_del_mapa(const std::string& ruta_mapa);

    void dibujar_capas(
        SDL_Renderer* renderer, const std::vector<std::vector<int>>& capas, int cam_x,
        int cam_y) const;

public:
    // Lanza std::runtime_error si falta una imagen, una capa del mapa o el tamaño del mapa.
    MapaGrafico(
        SDL_Renderer* renderer, const ConfiguracionMapa& configuracion_mapa,
        const ConfiguracionGraficos& configuracion_graficos);

    int ancho_en_casillas() const;
    int alto_en_casillas() const;

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar_bajo_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const;
    void dibujar_sobre_los_jugadores(SDL_Renderer* renderer, int cam_x, int cam_y) const;

    // Copiarlo dejaría los tilesets y el dibujado apuntando a datos del original.
    MapaGrafico(const MapaGrafico&) = delete;
    MapaGrafico& operator=(const MapaGrafico&) = delete;
};

#endif
