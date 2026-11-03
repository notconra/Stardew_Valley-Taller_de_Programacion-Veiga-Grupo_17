#ifndef CONTROLADOR_DE_DROPS_H
#define CONTROLADOR_DE_DROPS_H

#include <cstdint>
#include <map>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Regla global de recogida y soltado de ítems. Cada tick, todo jugador conectado levanta los drops
// que tiene cerca. Con SoltarItem el jugador tira un ítem de su barra al piso.
class ControladorDeDrops {
private:
    // Radio en casillas (diagonales incluidas) desde la casilla del jugador.
    static constexpr int RADIO_DE_RECOGIDA_EN_CASILLAS = 1;

    // Cuántas casillas adelante cae lo que se suelta, una más que el radio de recogida, así el
    // ítem no se levanta solo cuando se droppea.
    static constexpr int DISTANCIA_DE_SOLTADO = RADIO_DE_RECOGIDA_EN_CASILLAS + 1;

    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    Notificador& notificador;

    bool recoger_cerca(Personaje& personaje);
    void soltar(Personaje& personaje, const SoltarItem& pedido);

public:
    ControladorDeDrops(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        Notificador& notificador);

    // Devuelve true si el comando era de este controlador (y lo consumió).
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // Un tick: los jugadores conectados levantan lo que tienen cerca.
    void actualizar();
};

#endif
