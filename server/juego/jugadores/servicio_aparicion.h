#ifndef SERVICIO_APARICION_H
#define SERVICIO_APARICION_H

#include "../../../server/configuracion/configuracion_servidor_datos.h"
#include "../colisiones/colisiones.h"
#include "../mapas/mapa.h"
#include "posicion_aparicion.h"

// Busca el primer punto libre alrededor del spawn fijo. Dónde aparece el personaje
// es una regla de spawn, no del movimiento.
class ServicioAparicion {
public:
    PosicionAparicion buscar(
        const Mapa& mapa, const Colisiones& colisiones,
        const ConfiguracionJuegoServidor& configuracion) const;
};

#endif
