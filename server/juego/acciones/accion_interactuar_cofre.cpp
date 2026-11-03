#include "accion_interactuar_cofre.h"

#include <vector>

void AccionInteractuarCofre::notificar(
    ContextoDeAccion& c, uint32_t id_cofre, bool abierto,
    const std::vector<RanuraInventario>& ranuras) {
    CofreJugador cofre;
    cofre.id_cofre = id_cofre;
    cofre.abierto = abierto;
    cofre.ranuras = ranuras;

    ActualizacionCofre actualizacion;
    actualizacion.cofre = cofre;
    c.notificador.a_jugador(c.personaje.id_jugador, actualizacion);
}

void AccionInteractuarCofre::cerrar(ContextoDeAccion& c) {
    const uint32_t id_cofre = static_cast<uint32_t>(c.personaje.id_cofre_abierto);
    c.personaje.id_cofre_abierto = 0;
    c.personaje.mapa_cofre_abierto.clear();
    notificar(c, id_cofre, false, {});
}

TipoAccion AccionInteractuarCofre::tipo() const {
    return TipoAccion::INTERACTUAR;
}

bool AccionInteractuarCofre::aplica(const ContextoDeAccion& c) const {
    if (c.modo != ModoUso::SECUNDARIO) {
        return false;
    }
    if (c.personaje.id_cofre_abierto != 0) {
        return true;
    }
    // Permite interactuar si hay un cofre en la casilla objetivo seleccionada
    const ObjetoDelMundo* objeto = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);
    return objeto != nullptr && objeto->tipo == TipoObjeto::COFRE;
}

bool AccionInteractuarCofre::ejecutar(ContextoDeAccion& c) {
    const ObjetoDelMundo* cofre = c.sector.objetos.objeto_en(c.objetivo.columna, c.objetivo.fila);

    if (cofre == nullptr || cofre->tipo != TipoObjeto::COFRE) {
        if (c.personaje.id_cofre_abierto == 0) {
            return false;
        }
        cerrar(c);
        return true;
    }

    if (c.personaje.id_cofre_abierto == cofre->id &&
        c.personaje.mapa_cofre_abierto == c.sector.id) {
        cerrar(c);
        return true;
    }

    // Gira automáticamente hacia el cofre al interactuar
    c.sector.jugadores.mirar_hacia(c.personaje.id_jugador, c.objetivo);

    c.personaje.id_cofre_abierto = cofre->id;
    c.personaje.mapa_cofre_abierto = c.sector.id;
    const Inventario& contenido = c.sector.cofres.contenido(cofre->id);
    notificar(c, static_cast<uint32_t>(cofre->id), true, contenido.todas_las_ranuras());
    return true;
}
