#ifndef INICIO_DE_SDL_H
#define INICIO_DE_SDL_H

// Inicializa SDL, SDL_image, SDL_mixer y SDL_ttf al construirse y los libera al destruirse.
// Lanza std::runtime_error si no puede iniciarlas.
class InicioDeSDL {
public:
    InicioDeSDL();

    InicioDeSDL(const InicioDeSDL&) = delete;
    InicioDeSDL& operator=(const InicioDeSDL&) = delete;
    ~InicioDeSDL();
};

#endif
