#ifndef CONFIGURACION_CLIENTE_DATOS_H
#define CONFIGURACION_CLIENTE_DATOS_H

#include <cstdint>
#include <string>
#include <vector>

// Datos de configuración del cliente. No saben nada de TOML, son solo los valores ya leídos y
// validados, listos para pasarle a cada componente.
//
// Las rutas de los recursos son relativas al directorio desde el que se ejecuta el cliente.

struct ConfiguracionConexion {
    std::string host;
    std::string puerto;
};

struct ConfiguracionVentana {
    std::string titulo;
    int ancho = 0;
    int alto = 0;
};

struct ConfiguracionSDL {
    bool acelerado = true;
    bool vsync = true;
};

struct ColorDeFondo {
    std::uint8_t rojo = 0;
    std::uint8_t verde = 0;
    std::uint8_t azul = 0;
    std::uint8_t alfa = 255;
};

struct ConfiguracionGraficos {
    int tamano_de_tile = 0;
    int escala_de_tiles = 1;
    ColorDeFondo color_de_fondo;
};

struct ConfiguracionTileset {
    std::string ruta;
    int firstgid = 0;
    int columnas = 0;
};

struct ConfiguracionMapa {
    std::string ruta;
    std::vector<std::string> capas_bajo_los_jugadores;
    std::vector<std::string> capas_sobre_los_jugadores;
    std::vector<ConfiguracionTileset> tilesets;
};

struct ConfiguracionJugador {
    std::string ruta_sprite;
    int ancho_sprite = 0;
    int alto_sprite = 0;
    int cuadros_caminando = 0;
    int ms_por_cuadro_caminando = 0;
    int cuadros_quieto = 0;
    int ms_por_cuadro_quieto = 0;
};

struct ConfiguracionVista {
    ConfiguracionVentana ventana;
    ConfiguracionSDL sdl;
    ConfiguracionGraficos graficos;
    ConfiguracionMapa mapa;
    ConfiguracionJugador jugador;
};

#endif
