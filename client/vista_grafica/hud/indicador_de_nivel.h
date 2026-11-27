#ifndef INDICADOR_DE_NIVEL_H
#define INDICADOR_DE_NIVEL_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <string>

#include "../sdl/fuente_del_juego.h"
#include "../sdl/textura.h"

class IndicadorDeNivel {
private:
    static constexpr const char* RUTA_FONDO = "Assets/LooseSprites/menu_box.png";
    static constexpr int TAMANO_FUENTE = 16;
    static constexpr int BORDE_CORTE = 16;
    static constexpr int BORDE_PANTALLA = 14;
    static constexpr int RELLENO_HORIZONTAL = 17;
    static constexpr int RELLENO_VERTICAL = 13;
    static constexpr int MARGEN_IZQUIERDO = 8;
    static constexpr int MARGEN_SUPERIOR = 64;
    static constexpr int DESPLAZAMIENTO_SOMBRA = 1;
    static constexpr int CANTIDAD_NIVELES = 10;

    FuenteDelJuego fuente;
    Textura fondo;

    void dibujar_caja_nueve_rebanadas(SDL_Renderer* renderer, const SDL_Rect& destino) const;

public:
    IndicadorDeNivel(SDL_Renderer* renderer, const std::string& ruta_fuente);

    void dibujar(SDL_Renderer* renderer, uint8_t nivel) const;

    IndicadorDeNivel(const IndicadorDeNivel&) = delete;
    IndicadorDeNivel& operator=(const IndicadorDeNivel&) = delete;
};

#endif  // INDICADOR_DE_NIVEL_H
