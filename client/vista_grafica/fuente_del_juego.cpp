#include "fuente_del_juego.h"

#include <stdexcept>
#include <string>

// Los colores del texto de los menús de Stardew.
#define COLOR_DEL_TEXTO \
    SDL_Color {         \
        86, 22, 12, 255 \
    }
#define COLOR_DE_LA_SOMBRA \
    SDL_Color {            \
        221, 148, 84, 255  \
    }

FuenteDelJuego::FuenteDelJuego(
    SDL_Renderer* renderer, const std::string& ruta, int tamano_en_puntos)
    : fuente(TTF_OpenFont(ruta.c_str(), tamano_en_puntos), TTF_CloseFont),
      textos(renderer, fuente.get()) {
    if (!fuente) {
        throw std::runtime_error("no se pudo abrir la fuente " + ruta + ": " + TTF_GetError());
    }
}

void FuenteDelJuego::dibujar(
    SDL_Renderer* renderer, const std::string& texto, int x, int y, SDL_Color color) const {
    textos.liberar_si_excede_el_limite();
    const TexturaDeTexto* imagen = textos.obtener(texto, color);
    if (imagen == nullptr) {
        return;
    }
    const SDL_Rect destino{x, y, imagen->ancho, imagen->alto};
    SDL_RenderCopy(renderer, imagen->textura, nullptr, &destino);
}

void FuenteDelJuego::dibujar_con_sombra(
    SDL_Renderer* renderer, const std::string& texto, int x, int y, int sombra) const {
    dibujar(renderer, texto, x - sombra, y + sombra, COLOR_DE_LA_SOMBRA);
    dibujar(renderer, texto, x, y + sombra, COLOR_DE_LA_SOMBRA);
    dibujar(renderer, texto, x - sombra, y, COLOR_DE_LA_SOMBRA);
    dibujar(renderer, texto, x, y, COLOR_DEL_TEXTO);
}

int FuenteDelJuego::ancho(const std::string& texto) const {
    int ancho_texto = 0;
    int alto_texto = 0;
    if (TTF_SizeUTF8(fuente.get(), texto.c_str(), &ancho_texto, &alto_texto) != 0) {
        return 0;
    }
    return ancho_texto;
}

int FuenteDelJuego::alto_de_renglon() const {
    return TTF_FontLineSkip(fuente.get());
}

int FuenteDelJuego::centro_de_las_letras() const {
    int minimo_x = 0;
    int maximo_x = 0;
    int minimo_y = 0;
    int maximo_y = 0;
    int avance = 0;
    if (TTF_GlyphMetrics(fuente.get(), '0', &minimo_x, &maximo_x, &minimo_y, &maximo_y, &avance) !=
        0) {
        return TTF_FontHeight(fuente.get()) / 2;
    }
    // Las medidas del glifo son desde la línea de base para arriba.
    return TTF_FontAscent(fuente.get()) - (maximo_y + minimo_y) / 2;
}
