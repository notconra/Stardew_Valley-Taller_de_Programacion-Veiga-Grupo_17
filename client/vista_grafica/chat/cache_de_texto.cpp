#include "cache_de_texto.h"

#include <string>

constexpr size_t MAXIMO_DE_ENTRADAS = 600;

CacheDeTexto::CacheDeTexto(SDL_Renderer* renderer_de_dibujo, TTF_Font* fuente_a_usar)
    : renderer(renderer_de_dibujo), fuente(fuente_a_usar) {
}

const TexturaDeTexto* CacheDeTexto::obtener(const std::string& texto, SDL_Color color) {
    if (texto.empty()) {
        return nullptr;
    }

    std::string clave{
        static_cast<char>(color.r),
        static_cast<char>(color.g),
        static_cast<char>(color.b),
        static_cast<char>(color.a)};
    clave += texto;

    auto existente = texturas.find(clave);
    if (existente == texturas.end()) {
        TexturaDeTexto nueva;
        if (SDL_Surface* superficie = TTF_RenderUTF8_Blended(fuente, texto.c_str(), color)) {
            nueva.textura = SDL_CreateTextureFromSurface(renderer, superficie);
            nueva.ancho = superficie->w;
            nueva.alto = superficie->h;
            SDL_FreeSurface(superficie);
        }
        existente = texturas.emplace(clave, nueva).first;
    }
    return existente->second.textura != nullptr ? &existente->second : nullptr;
}

void CacheDeTexto::liberar_si_excede_el_limite() {
    if (texturas.size() > MAXIMO_DE_ENTRADAS) {
        liberar_todo();
    }
}

void CacheDeTexto::liberar_todo() {
    for (auto& [clave, entrada] : texturas) {
        if (entrada.textura != nullptr) {
            SDL_DestroyTexture(entrada.textura);
        }
    }
    texturas.clear();
}

CacheDeTexto::~CacheDeTexto() {
    liberar_todo();
}
