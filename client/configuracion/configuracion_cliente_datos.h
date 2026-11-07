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
    int radio_vision_mazmorra = 0;
    int tamano_pixel_vision_mazmorra = 8;
    int ancho_difuminado_vision_mazmorra = 24;
};

struct ConfiguracionTileset {
    std::string ruta;
    int firstgid = 0;
    int columnas = 0;
};

struct ConfiguracionMapa {
    std::string id;
    std::string ruta;
    std::vector<std::string> capas_bajo_los_jugadores;
    std::vector<std::string> capas_sobre_los_jugadores;
    std::vector<ConfiguracionTileset> tilesets;

    bool es_mazmorra = false;
};
struct ColorRGB {
    std::uint8_t rojo = 255;
    std::uint8_t verde = 255;
    std::uint8_t azul = 255;
};

constexpr int OFFSET_Y_REMERA_FEMENINO = 16;
constexpr int OFFSET_Y_REMERA_MASCULINO = 15;
constexpr int OFFSET_Y_PELO_FEMENINO = 1;
constexpr int OFFSET_Y_PELO_MASCULINO = 0;
constexpr int OFFSET_Y_SOMBRERO_FEMENINO = 1;
constexpr int OFFSET_Y_SOMBRERO_MASCULINO = 0;

struct ConfiguracionJugador {
    std::string ruta_cuerpo;
    std::string ruta_pantalon;
    std::string ruta_pelo;
    std::string ruta_sombrero;
    std::string ruta_remera;

    int indice_pelo = 0;
    int indice_sombrero = -1;
    int indice_remera = 0;
    int indice_pantalon = 0;
    int indice_piel = 0;

    ColorRGB color_pelo;
    ColorRGB color_pantalon;

    int ms_por_cuadro_caminando = 150;
    int offset_y_remera = 16;
    int offset_y_pelo = 0;
    int offset_y_sombrero = 0;
};

struct ConfiguracionSonido {
    std::string ruta_ambiente;
    std::string ruta_caminar;
    std::string ruta_accion;
};

// Cómo se ve la tierra tratada. La textura es la de Stardew (hoeDirt.png); el resto del dibujado
// sale de ConfiguracionGraficos, igual que los tiles del mapa.
struct ConfiguracionTierra {
    std::string ruta_sprite;
};

struct ConfiguracionVista {
    ConfiguracionVentana ventana;
    ConfiguracionSDL sdl;
    ConfiguracionGraficos graficos;
    std::vector<ConfiguracionMapa> mapas;
    std::string mapa_inicial;
    ConfiguracionJugador jugador;
    ConfiguracionSonido sonido;
    ConfiguracionTierra tierra;
};

#endif
