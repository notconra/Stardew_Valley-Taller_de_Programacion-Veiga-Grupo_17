#ifndef ACCION_DORMIR_H
#define ACCION_DORMIR_H

#include <cstdint>
#include <map>

#include "accion.h"

// Clic secundario sobre una cama (cualquier casilla de las que ocupa): el jugador se acuesta y
// espera el día siguiente. Cualquiera puede usar cualquier cama, pero no dos jugadores la misma:
// si otro conectado ya está acostado en ella, no pasa nada. Queda anotada como su última cama,
// que es donde se despierta.
class AccionDormir : public Accion {
private:
    const std::map<uint32_t, Personaje>& personajes;

    // Si algún otro jugador conectado está acostado en esa cama.
    bool esta_ocupada(const Personaje& quien_pregunta, const UbicacionDeCama& cama) const;

public:
    explicit AccionDormir(const std::map<uint32_t, Personaje>& personajes);

    TipoAccion tipo() const override;

    bool aplica(const ContextoDeAccion& c) const override;

    bool ejecutar(ContextoDeAccion& c) override;
};

#endif
