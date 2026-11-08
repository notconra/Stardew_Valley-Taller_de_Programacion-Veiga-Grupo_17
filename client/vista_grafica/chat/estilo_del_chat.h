#ifndef ESTILO_DEL_CHAT_H
#define ESTILO_DEL_CHAT_H

#include <SDL2/SDL.h>

#include <cstddef>
#include <cstdint>

// Todo lo que define cómo se ve el chat y dónde está. Las medidas están en píxeles.
namespace estilo_del_chat {

struct RectanguloEnLaImagen {
    int x;
    int y;
    int ancho;
    int alto;
};

// Imágenes

constexpr const char* RUTA_DE_LA_IMAGEN = "Assets/chat/chat.png";
constexpr const char* CARPETA_DE_EMOTES = "Assets/chat/emotes";

// SDL_ScaleModeLinear suaviza la imagen al agrandarla. SDL_ScaleModeNearest deja los píxeles
// marcados (estilo pixel art).
constexpr SDL_ScaleMode FILTRO_DE_LA_IMAGEN = SDL_ScaleModeLinear;

// chat.png

constexpr int ANCHO_DE_LA_IMAGEN = 74;
constexpr int ALTO_DE_LA_IMAGEN = 71;
constexpr RectanguloEnLaImagen TITULO_EN_LA_IMAGEN{2, 3, 55, 8};
constexpr RectanguloEnLaImagen BOTON_CERRAR_EN_LA_IMAGEN{60, 1, 13, 12};
constexpr RectanguloEnLaImagen CUERPO_EN_LA_IMAGEN{2, 17, 69, 51};

// Medidas

constexpr int MARGEN_DE_PANTALLA = 8;
constexpr int ANCHO_DE_LA_VENTANA = 400;  // se achican si la pantalla es más chica
constexpr int ALTO_DE_LA_VENTANA = 384;   // misma proporción que chat.png (74 x 71)
constexpr int RELLENO_DEL_CUERPO = 8;
constexpr int RELLENO_DEL_TITULO = 8;
constexpr int RELLENO_DE_ENTRADA = 4;
constexpr int SEPARACION = 4;
constexpr int LADO_DEL_BOTON_DE_APERTURA = 48;
constexpr int LADO_DEL_BOTON_DE_BAJAR = 22;
constexpr int LADO_DEL_BOTON_DE_EMOJIS = 22;
constexpr int RELLENO_DEL_ICONO_DE_EMOJIS = 2;
constexpr int MAXIMO_DE_LINEAS_DE_ENTRADA = 2;
constexpr int MENSAJES_POR_MUESCA = 3;
constexpr size_t MAXIMO_DE_NO_LEIDOS_VISIBLE = 99;
constexpr uint32_t PERIODO_DEL_CURSOR_MS = 700;
constexpr int ANCHO_DEL_CURSOR = 2;
constexpr int MARGEN_VERTICAL_DEL_CURSOR = 2;

// Título del encabezado
constexpr float ESCALA_DEL_TITULO = 1.3f;
constexpr int AJUSTE_VERTICAL_DEL_TITULO = -10;

// Globo rojo de mensajes sin leer
constexpr int RELLENO_DEL_AVISO = 10;
constexpr int ALTO_EXTRA_DEL_AVISO = 2;
constexpr int SALIENTE_DEL_AVISO = 4;

// Un emote mide ALTO_DE_EMOTE de lado y deja SEPARACION_ENTRE_EMOTES libres abajo, así dos emotes
// seguidos no quedan pegados. Los emojis, en cambio, miden lo mismo que la fuente.
constexpr int ALTO_DE_EMOTE = 72;
constexpr int SEPARACION_ENTRE_EMOTES = 6;

// Menú de emojis y emotes
constexpr int RELLENO_DEL_MENU = 4;
constexpr int RELLENO_DE_CELDA = 3;
constexpr int MAXIMO_DE_FILAS_DEL_MENU = 4;
constexpr int ALTO_DE_SOLAPAS = 30;
constexpr int ANCHO_DE_SOLAPA = 60;
constexpr int MARGEN_DE_SOLAPA = 4;
constexpr int TEXTO_DE_SOLAPA_X = 6;
constexpr int TEXTO_DE_SOLAPA_Y = 2;

// Colores

constexpr SDL_Color TEXTO{245, 240, 230, 255};
constexpr SDL_Color TITULO{255, 235, 140, 255};
constexpr SDL_Color TEXTO_APAGADO{150, 140, 130, 255};
constexpr SDL_Color SISTEMA{110, 200, 235, 255};
constexpr SDL_Color COMERCIANTE{250, 205, 90, 255};

constexpr SDL_Color JUGADOR_ROJO{235, 90, 90, 255};
constexpr SDL_Color JUGADOR_VERDE{110, 210, 120, 255};
constexpr SDL_Color JUGADOR_AMARILLO{240, 215, 100, 255};
constexpr SDL_Color JUGADOR_AZUL{100, 160, 240, 255};

constexpr SDL_Color FLECHA{240, 225, 190, 255};
constexpr SDL_Color AVISO_DE_NO_LEIDOS{200, 45, 40, 255};
constexpr SDL_Color BORDE_DEL_AVISO{220, 45, 40, 255};

constexpr SDL_Color FONDO_DE_LA_ENTRADA{0, 0, 0, 150};
constexpr SDL_Color FONDO_DEL_SELECTOR{20, 14, 13, 240};
constexpr SDL_Color BORDE_DE_LOS_RECUADROS{218, 125, 22, 255};
constexpr SDL_Color CELDA_BAJO_EL_RATON{110, 85, 70, 255};
constexpr SDL_Color PESTANA_ACTIVA{80, 64, 58, 255};
constexpr SDL_Color PESTANA_INACTIVA{48, 36, 34, 255};

}  // namespace estilo_del_chat

#endif
