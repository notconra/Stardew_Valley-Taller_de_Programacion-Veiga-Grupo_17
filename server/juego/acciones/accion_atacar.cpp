#include "accion_atacar.h"

#include <algorithm>
#include <random>

#include "../../../common/calidad.h"
#include "../../../common/tipo_item.h"
#include "../enemigos/catalogo_de_enemigos.h"
#include "../enemigos/enemigo.h"
#include "../enemigos/gestor_de_enemigos.h"

namespace {

// Compartido entre todos los golpes de espada del proceso: no hace falta uno por acción, el sorteo
// del oro no tiene que ser reproducible.
std::mt19937& generador_de_oro() {
    static std::mt19937 generador{std::random_device{}()};
    return generador;
}

}  // namespace

bool AccionAtacar::ejecutar(ContextoDeAccion& c) {
    const Enemigo* enemigo = c.sector.enemigos.enemigo_vivo_en(c.objetivo.columna, c.objetivo.fila);
    if (enemigo == nullptr) {
        return false;
    }
    const int id_enemigo = enemigo->id;
    const Casilla posicion = enemigo->posicion;

    const int danio = static_cast<int>(DANIO_BASE * c.personaje.multiplicador_de_ataque());
    const ResultadoDeGolpeEnemigo resultado =
        c.sector.golpear_enemigo(c.notificador, id_enemigo, danio);
    if (!resultado.golpeo) {
        return false;
    }
    c.personaje.gastar_energia(COSTO_DE_ENERGIA);
    if (!resultado.murio) {
        return true;
    }

    const DefinicionDeEnemigo* definicion = c.recursos.catalogo_de_enemigos.buscar(enemigo->tipo);
    if (definicion != nullptr && definicion->oro_maximo > 0) {
        std::uniform_int_distribution<int> rango(definicion->oro_minimo, definicion->oro_maximo);
        const int oro = rango(generador_de_oro());
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
    return true;
}
