#include "fuente_del_juego.h"

#include <stdexcept>
#include <string>
#include <utility>

#define RUTA_DE_LA_FUENTE "Assets/Fonts/Stardew_Valley.ttf"
#define TEXTOS_GUARDADOS_COMO_MAXIMO 64

#define COLOR_DEL_TEXTO \
    SDL_Color {         \
        86, 22, 12, 255 \
    }
#define COLOR_DE_LA_SOMBRA \
    SDL_Color {            \
        221, 148, 84, 255  \
    }

FuenteDelJuego::FuenteDelJuego(int tamano_en_puntos)
    : fuente(TTF_OpenFont(RUTA_DE_LA_FUENTE, tamano_en_puntos)) {
    if (fuente == nullptr) {
        throw std::runtime_error(
            std::string("no se pudo abrir la fuente " RUTA_DE_LA_FUENTE ": ") + TTF_GetError());
    }
}

FuenteDelJuego::~FuenteDelJuego() {
    olvidar_textos();
    TTF_CloseFont(fuente);
}

void FuenteDelJuego::olvidar_textos() const {
    for (const auto& [clave, textura] : textos) {
        SDL_DestroyTexture(textura);
    }
    textos.clear();
}

SDL_Texture* FuenteDelJuego::textura_de(
    SDL_Renderer* renderer, const std::string& texto, SDL_Color color) const {
    const uint32_t color_junto = (static_cast<uint32_t>(color.r) << 24) |
                                 (static_cast<uint32_t>(color.g) << 16) |
                                 (static_cast<uint32_t>(color.b) << 8) | color.a;
    const std::pair<std::string, uint32_t> clave{texto, color_junto};
    const auto guardada = textos.find(clave);
    if (guardada != textos.end()) {
        return guardada->second;
    }
    if (textos.size() >= TEXTOS_GUARDADOS_COMO_MAXIMO) {
        olvidar_textos();
    }

    SDL_Surface* superficie = TTF_RenderUTF8_Blended(fuente, texto.c_str(), color);
    if (superficie == nullptr) {
        return nullptr;
    }
    SDL_Texture* textura = SDL_CreateTextureFromSurface(renderer, superficie);
    SDL_FreeSurface(superficie);
    if (textura != nullptr) {
        textos.emplace(clave, textura);
    }
    return textura;
}

void FuenteDelJuego::dibujar(
    SDL_Renderer* renderer, const std::string& texto, int x, int y, SDL_Color color) const {
    if (texto.empty()) {
        return;
    }
    SDL_Texture* textura = textura_de(renderer, texto, color);
    if (textura == nullptr) {
        return;
    }
    SDL_Rect destino{x, y, 0, 0};
    SDL_QueryTexture(textura, nullptr, nullptr, &destino.w, &destino.h);
    SDL_RenderCopy(renderer, textura, nullptr, &destino);
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
    if (TTF_SizeUTF8(fuente, texto.c_str(), &ancho_texto, &alto_texto) != 0) {
        return 0;
    }
    return ancho_texto;
}

int FuenteDelJuego::alto_de_renglon() const {
    return TTF_FontLineSkip(fuente);
}

int FuenteDelJuego::centro_de_las_letras() const {
    int minimo_x = 0;
    int maximo_x = 0;
    int minimo_y = 0;
    int maximo_y = 0;
    int avance = 0;
    if (TTF_GlyphMetrics(fuente, '0', &minimo_x, &maximo_x, &minimo_y, &maximo_y, &avance) != 0) {
        return TTF_FontHeight(fuente) / 2;
    }
    return TTF_FontAscent(fuente) - (maximo_y + minimo_y) / 2;
}
