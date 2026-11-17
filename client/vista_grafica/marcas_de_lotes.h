#ifndef MARCAS_DE_LOTES_H
#define MARCAS_DE_LOTES_H

#include <SDL2/SDL.h>

#include <map>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "iconos_de_lote.h"

// El ícono que flota sobre cada lote del centro comunitario, para que se sepa dónde hacer clic. Los
// lotes ya completos llevan además una tilde verde. Todo sale de los LoteDelCentro que mandó el
// servidor (los guarda VistaPartida): en qué mapa y casilla está cada lote y si ya se completó.
class MarcasDeLotes {
private:
    const IconosDeLotes& iconos;
    int tamano_de_tile;
    int escala_de_tiles;

public:
    MarcasDeLotes(const IconosDeLotes& iconos, const ConfiguracionGraficos& configuracion_graficos);

    // Dibuja las marcas de los lotes que están en ese mapa. cam_x y cam_y son la esquina de la
    // cámara, en píxeles de pantalla.
    void dibujar(
        SDL_Renderer* renderer, const std::map<TipoLote, LoteDelCentro>& lotes,
        const std::string& mapa, int cam_x, int cam_y) const;
};

#endif
