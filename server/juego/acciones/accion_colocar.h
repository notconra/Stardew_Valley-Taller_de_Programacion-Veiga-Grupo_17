#ifndef ACCION_COLOCAR_H
#define ACCION_COLOCAR_H

#include <cstdint>

#include "../../../common/modo_uso.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"
#include "accion.h"

class AccionColocar : public Accion {
public:
    TipoAccion tipo() const override {
        return TipoAccion::COLOCAR;
    }

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;

    bool se_puede_colocar(const ContextoDeAccion& c) const;

    // Si ese objeto entra con su casilla de arriba a la izquierda en "esquina" del sector del
    // contexto: mapa permitido, casillas dentro del mapa y libres, sin jugadores, cultivos ni
    // transiciones. Las casillas que ocupa el objeto "ignorar_id" cuentan como libres (para girar
    // un mueble en su lugar); 0 si no hay que ignorar ninguno.
    static bool cabe(
        const ContextoDeAccion& c, const DefinicionDeObjeto& definicion, const Casilla& esquina,
        int ignorar_id = 0);

    bool modifica_inventario() const override {
        return true;
    }
};

#endif
