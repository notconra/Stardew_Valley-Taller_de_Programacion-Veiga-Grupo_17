#ifndef INICIO_DE_SDL_H
#define INICIO_DE_SDL_H

// Inicializa SDL y SDL_image al construirse y los libera al destruirse, así las librerías solo
// están abiertas mientras dura la partida (y no quedan a la vez con Qt).
// Lanza std::runtime_error si no puede iniciarlas.
class InicioDeSDL {
public:
    InicioDeSDL();

    InicioDeSDL(const InicioDeSDL&) = delete;
    InicioDeSDL& operator=(const InicioDeSDL&) = delete;
    ~InicioDeSDL();
};

#endif
