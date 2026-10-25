#ifndef SERVICIO_APARICION_H
#define SERVICIO_APARICION_H

#include "../colisiones/colisiones.h"
#include "../mapas/mapa.h"
#include "posicion_aparicion.h"

// Busca el primer punto libre alrededor del spawn fijo. Dónde aparece el personaje
// es una regla de spawn, no del movimiento.
class ServicioAparicion {
public:
    static PosicionAparicion buscar(const Mapa& mapa, const Colisiones& colisiones);
};

#endif
