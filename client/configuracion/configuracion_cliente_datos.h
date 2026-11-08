#ifndef CONFIGURACION_CLIENTE_DATOS_H
#define CONFIGURACION_CLIENTE_DATOS_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/apariencia_personaje.h"

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

    // En pantalla completa se usa el tamaño del escritorio, así funciona en cualquier monitor.
    // "ancho" y "alto" quedan solo como tamaño de la ventana cuando no está en pantalla completa.
    bool pantalla_completa = true;

    // Solo tiene efecto en modo ventana porque en pantalla completa la ventana no se redimensiona.
    bool redimensionable = false;
};

struct ConfiguracionSDL {
    bool acelerado = true;
    bool vsync = true;
    // 0 desactiva el límite.
    int fps_maximos = 60;
};

struct ColorDeFondo {
    std::uint8_t rojo = 0;
    std::uint8_t verde = 0;
    std::uint8_t azul = 0;
    std::uint8_t alfa = 255;
};

struct ConfiguracionGraficos {
    int tamano_de_tile = 0;
    // Escala mínima. Con zoom automático se agranda en pantallas grandes para que no se vea
    // demasiado mapa, sin pasar de "escala_maxima_de_tiles".
    int escala_de_tiles = 1;
    int escala_maxima_de_tiles = 8;
    bool zoom_automatico = true;

    // Zoom adicional, no entero, que se aplica sobre la escala de tiles. 1.0 no hace nada, 1.25
    // muestra el 80% del mapa que se vería con 1.0. Permite ajustes más finos que los saltos de
    // la escala entera (por ejemplo, entre escala 2 y 3).
    double zoom_extra = 1.0;
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

constexpr int OFFSET_Y_REMERA_FEMENINO = 16;
constexpr int OFFSET_Y_REMERA_MASCULINO = 15;
constexpr int OFFSET_Y_PELO_FEMENINO = 1;
constexpr int OFFSET_Y_PELO_MASCULINO = 0;

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

    ColorRGB color_pelo;
    ColorRGB color_pantalon;

    int ms_por_cuadro_caminando = 150;
    int offset_y_remera = 16;
    int offset_y_pelo = 0;
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

// Chat in-game.
struct ConfiguracionChat {
    std::string ruta_fuente;
    int tamano_de_fuente = 16;
    int maximo_de_mensajes = 100;

    // Imagen (.png) del botón cuadrado con el que se abre el chat.
    std::string ruta_icono;

    // Carpetas con un .png por emoji/emote, el nombre del archivo es el nombre del emoji/emote.
    // Los archivos se cargan solos al iniciar.
    std::string ruta_carpeta_de_emojis = "Assets/chat/emojis";
    std::string ruta_carpeta_de_emotes = "Assets/chat/emotes";
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
    ConfiguracionChat chat;
};

#endif
