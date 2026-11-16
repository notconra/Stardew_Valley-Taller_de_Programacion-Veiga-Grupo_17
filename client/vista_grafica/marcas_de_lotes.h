#ifndef MARCAS_DE_LOTES_H
#define MARCAS_DE_LOTES_H

#include <SDL2/SDL.h>

#include <map>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "iconos_de_lote.h"

// El ícono que flota sobre cada lote del centro comunitario que todavía no se completó, para que se
// sepa dónde hacer clic. Todo sale de lo que manda el servidor (LoteDelCentro, al entrar y cuando
// un lote cambia): en qué mapa y casilla está cada lote y si ya se completó.
class MarcasDeLotes {
private:
    const IconosDeLotes& iconos;
    int tamano_de_tile;
    int escala_de_tiles;
    std::map<TipoLote, LoteDelCentro> lotes;

public:
    MarcasDeLotes(const IconosDeLotes& iconos, const ConfiguracionGraficos& configuracion_graficos);

    void recibir(const LoteDelCentro& lote);

    // Dibuja las marcas de los lotes de ese mapa. cam_x y cam_y son la esquina de la cámara, en
    // píxeles de pantalla.
    void dibujar(SDL_Renderer* renderer, const std::string& mapa, int cam_x, int cam_y) const;
};

#endif
