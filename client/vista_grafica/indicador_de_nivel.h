#ifndef INDICADOR_DE_NIVEL_H
#define INDICADOR_DE_NIVEL_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <string>

#include "fuente_del_juego.h"

class IndicadorDeNivel {
private:
    static constexpr int TAMANO_FUENTE = 22;
    static constexpr int RELLENO_HORIZONTAL = 14;
    static constexpr int RELLENO_VERTICAL = 8;
    static constexpr int MARGEN_IZQUIERDO = 8;
    static constexpr int MARGEN_SUPERIOR = 64;
    static constexpr int DESPLAZAMIENTO_SOMBRA = 2;
    static constexpr int CANTIDAD_NIVELES = 10;

    FuenteDelJuego fuente;

public:
    IndicadorDeNivel(SDL_Renderer* renderer, const std::string& ruta_fuente);

    void dibujar(SDL_Renderer* renderer, uint8_t nivel) const;

    IndicadorDeNivel(const IndicadorDeNivel&) = delete;
    IndicadorDeNivel& operator=(const IndicadorDeNivel&) = delete;
};

#endif  // INDICADOR_DE_NIVEL_H
