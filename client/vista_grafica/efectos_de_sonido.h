#ifndef EFECTOS_DE_SONIDO_H
#define EFECTOS_DE_SONIDO_H

#include <SDL2/SDL_mixer.h>

#include <map>
#include <string>

#include "../configuracion/configuracion_cliente_datos.h"

// Los efectos de sonido del personaje: ambiente de fondo (en loop), paso al caminar, y la
// acción (herramienta). El ambiente y los pasos pueden cambiar según el mapa.
// Cada .wav se carga una sola vez, la primera vez que se pide.
// Lanza std::runtime_error si no puede cargar alguno.
class EfectosDeSonido {
private:
    static constexpr int CANAL_AMBIENTE = 0;  // reservado, así los demás efectos no lo pisan

    std::string ruta_ambiente_general;
    std::string ruta_paso_general;
    std::map<std::string, Mix_Chunk*> sonidos_por_ruta;
    Mix_Chunk* ambiente_sonando = nullptr;
    Mix_Chunk* paso_actual = nullptr;
    Mix_Chunk* accion = nullptr;

    Mix_Chunk* cargar(const std::string& ruta);

public:
    explicit EfectosDeSonido(const ConfiguracionSonido& configuracion);
    ~EfectosDeSonido();

    EfectosDeSonido(const EfectosDeSonido&) = delete;
    EfectosDeSonido& operator=(const EfectosDeSonido&) = delete;

    // ruta vacía = el sonido general del [sonido] de la configuración.
    // Si ya está sonando ese ambiente, no lo reinicia.
    void cambiar_ambiente(const std::string& ruta);
    void cambiar_paso(const std::string& ruta);

    void reproducir_paso() const;
    void reproducir_accion() const;
};

#endif
