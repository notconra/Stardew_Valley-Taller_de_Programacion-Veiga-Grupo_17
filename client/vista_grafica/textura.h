#ifndef TEXTURA_H
#define TEXTURA_H

#include <SDL2/SDL.h>
#include <string>

class Textura {
public:
    Textura();
    Textura(const std::string& ruta, SDL_Renderer* renderer);
    ~Textura();

    Textura(const Textura&) = delete;
    Textura& operator=(const Textura&) = delete;

    Textura(Textura&& otra) noexcept;
    Textura& operator=(Textura&& otra) noexcept;

    SDL_Texture* ptr() const;

private:
    static SDL_Texture* cargarTextura(const std::string& ruta, SDL_Renderer* renderer);

    SDL_Texture* textura;
};

#endif
