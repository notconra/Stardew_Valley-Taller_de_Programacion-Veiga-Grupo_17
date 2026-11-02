#ifndef CONTROLADOR_DE_DROPS_H
#define CONTROLADOR_DE_DROPS_H

#include <cstdint>
#include <map>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Regla global de recogida y soltado de ítems.
class ControladorDeDrops {
private:
    static constexpr int RADIO_DE_RECOGIDA_EN_CASILLAS = 1;

    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    Notificador& notificador;

    bool recoger_cerca(Personaje& personaje);
    void soltar(Personaje& personaje, const SoltarItem& pedido);

public:
    ControladorDeDrops(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        Notificador& notificador);

    bool atender(Personaje& personaje, const ComandoCliente& comando);
    void actualizar();
};

#endif
