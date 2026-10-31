#include "minerales.h"

#include <vector>

std::vector<DefinicionDeObjeto> Minerales::definiciones() {
    return {
        // Piedra: relleno común de la caverna, se rompe de un golpe y deja piedra.
        DefinicionDeObjeto{
            .tipo = TipoObjeto::PIEDRA,
            .clave = "PIEDRA",
            .golpes_para_picar = 1,
            .herramienta = TipoItem::PICO,
            .probabilidad_de_no_dejar = 0.0,
            .producto = TipoItem::PIEDRA,
            .cantidad = 1,
        },

        // Carbón: poco tiempo, 1 golpe.
        DefinicionDeObjeto{
            .tipo = TipoObjeto::CARBON,
            .clave = "CARBON",
            .golpes_para_picar = 1,
            .herramienta = TipoItem::PICO,
            .probabilidad_de_no_dejar = 0.0,
            .producto = TipoItem::CARBON,
            .cantidad = 1,
        },

        // Lapislázuli: tiempo medio, 2 golpes, y 15% de romperse sin dejar producto.
        DefinicionDeObjeto{
            .tipo = TipoObjeto::LAPISLAZULI,
            .clave = "LAPISLAZULI",
            .golpes_para_picar = 2,
            .herramienta = TipoItem::PICO,
            .probabilidad_de_no_dejar = 0.15,
            .producto = TipoItem::LAPISLAZULI,
            .cantidad = 1,
        },

        // Plata: tiempo alto, 3 golpes. La manada de orcos (10%) no se resuelve acá: es un
        // efecto posterior a obtenerla y le toca al sistema de acciones.
        DefinicionDeObjeto{
            .tipo = TipoObjeto::PLATA,
            .clave = "PLATA",
            .golpes_para_picar = 3,
            .herramienta = TipoItem::PICO,
            .probabilidad_de_no_dejar = 0.0,
            .producto = TipoItem::PLATA,
            .cantidad = 1,
        },

        // Oro: tiempo muy alto, 4 golpes. "tipo" elige cómo se ve en el mapa y "producto" qué
        // ítem deja.
        DefinicionDeObjeto{
            .tipo = TipoObjeto::ORO,
            .clave = "ORO",
            .golpes_para_picar = 4,
            .herramienta = TipoItem::PICO,
            .probabilidad_de_no_dejar = 0.0,
            .producto = TipoItem::ORO,
            .cantidad = 1,
        },
    };
}
