#ifndef ACCION_H
#define ACCION_H

#include <cstdint>
#include <functional>

#include "../../../common/modo_uso.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_accion.h"
#include "../geometria.h"
#include "../jugadores/personaje.h"
#include "../mapa/mapa.h"
#include "../notificador.h"
#include "../recursos/recursos_de_juego.h"
#include "../sector/sector.h"

// Qué hacer cuando una acción manda a un personaje a través de una transición. Lo provee Mundo,
// que es quien sabe resolver mazmorras y traslados entre sectores.
using ResolverDeTransicion = std::function<void(Personaje&, const Transicion&)>;

struct ContextoDeAccion {
    uint32_t id_jugador;
    RanuraInventario item;
    uint8_t indice_ranura;
    Casilla objetivo;
    Sector& sector;
    Personaje& personaje;
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    ModoUso modo;  // distingue principal (izq) o secundario (der)
    const ResolverDeTransicion& resolver_transicion;
};

class Accion {
public:
    virtual TipoAccion tipo() const = 0;

    virtual bool aplica(const ContextoDeAccion& contexto) const = 0;

    virtual bool ejecutar(ContextoDeAccion& contexto) = 0;

    virtual uint16_t duracion_en_ticks() const {
        return 0;
    }

    virtual bool modifica_inventario() const {
        return false;
    }

    virtual ~Accion() = default;
};

#endif
