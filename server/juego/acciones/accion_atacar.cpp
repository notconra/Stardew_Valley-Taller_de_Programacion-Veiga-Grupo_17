#include "accion_atacar.h"

#include <algorithm>

#include "../../../common/calidad.h"
#include "../../../common/tipo_item.h"
#include "../azar.h"
#include "../enemigos/catalogo_de_enemigos.h"
#include "../enemigos/enemigo.h"
#include "../enemigos/gestor_de_enemigos.h"

bool AccionAtacar::ejecutar(ContextoDeAccion& c) {
    const Enemigo* enemigo = c.sector.enemigos.enemigo_vivo_en(c.objetivo.columna, c.objetivo.fila);
    if (enemigo == nullptr) {
        return false;
    }
    const int id_enemigo = enemigo->id;
    const Casilla posicion = enemigo->posicion;
    const TipoEnemigo tipo = enemigo->tipo;

    const int danio = static_cast<int>(
        c.recursos.juego.acciones.danio_espada * c.personaje.multiplicador_de_ataque());
    const ResultadoDeGolpeEnemigo resultado =
        c.sector.golpear_enemigo(c.notificador, id_enemigo, danio);
    if (!resultado.golpeo) {
        return false;
    }
    c.personaje.gastar_energia(c.recursos.juego.acciones.energia_por_golpe);
    if (!resultado.murio) {
        return true;
    }

    const DefinicionDeEnemigo* definicion = c.recursos.catalogo_de_enemigos.buscar(tipo);
    if (definicion != nullptr && definicion->oro_maximo > 0) {
        const int oro = Azar::entero(definicion->oro_minimo, definicion->oro_maximo);
        if (oro > 0) {
            c.sector.soltar(
                c.notificador,
                TipoItem::ORO,
                static_cast<uint16_t>(oro),
                Calidad::BASE,
                posicion.columna,
                posicion.fila);
        }
    }
    if (definicion != nullptr && Azar::con_probabilidad(definicion->probabilidad_de_plata)) {
        c.sector.soltar(
            c.notificador, TipoItem::PLATA, 1, Calidad::BASE, posicion.columna, posicion.fila);
    }
    return true;
}
