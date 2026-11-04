#ifndef EFECTOS_DE_SONIDO_H
#define EFECTOS_DE_SONIDO_H

#include <SDL2/SDL_mixer.h>

#include <string>

#include "../configuracion/configuracion_cliente_datos.h"

// Los efectos de sonido del personaje: ambiente de fondo (en loop), paso al caminar, y la
// acción (herramienta). Carga los .wav una sola vez al construirse.
// Lanza std::runtime_error si no puede cargar alguno.
class EfectosDeSonido {
private:
    Mix_Chunk* ambiente = nullptr;
    Mix_Chunk* caminar = nullptr;
    Mix_Chunk* accion = nullptr;

    Mix_Chunk* cargar(const std::string& ruta) const;

public:
    explicit EfectosDeSonido(const ConfiguracionSonido& configuracion);
    ~EfectosDeSonido();

    EfectosDeSonido(const EfectosDeSonido&) = delete;
    EfectosDeSonido& operator=(const EfectosDeSonido&) = delete;

    void reproducir_ambiente() const;
    void reproducir_paso() const;
    void reproducir_accion() const;
};

#endif