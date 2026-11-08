#ifndef CACHE_DE_TEXTO_H
#define CACHE_DE_TEXTO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <unordered_map>

struct TexturaDeTexto {
    SDL_Texture* textura = nullptr;
    int ancho = 0;
    int alto = 0;
};

// Texturas de textos ya rasterizados, indexadas por texto y color.
class CacheDeTexto {
public:
    CacheDeTexto(SDL_Renderer* renderer, TTF_Font* fuente);
    ~CacheDeTexto();

    CacheDeTexto(const CacheDeTexto&) = delete;
    CacheDeTexto& operator=(const CacheDeTexto&) = delete;

    // nullptr si el texto está vacío o no se pudo rasterizar. El puntero vale hasta la próxima
    // llamada a liberar_si_excede_el_limite().
    const TexturaDeTexto* obtener(const std::string& texto, SDL_Color color);

    // Se llama una vez por cuadro, antes de dibujar.
    void liberar_si_excede_el_limite();

private:
    SDL_Renderer* renderer;
    TTF_Font* fuente;
    std::unordered_map<std::string, TexturaDeTexto> texturas;

    TexturaDeTexto rasterizar(const std::string& texto, SDL_Color color) const;
    void liberar_todo();
};

#endif
