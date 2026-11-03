#ifndef CONTROLADOR_DE_JUGADOR_H
#define CONTROLADOR_DE_JUGADOR_H

#include <cstdint>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Comandos que solo afectan al jugador que los manda como moverse, elegir ranura de la barra, mover
// ítems entre ranuras y pedir la barra completa.
class ControladorDeJugador {
private:
    GestorDeSectores& sectores;
    Notificador& notificador;

    void mover(Personaje& personaje, const Moverse& pedido);
    void seleccionar_ranura(Personaje& personaje, const SeleccionarRanuraDeLaBarra& pedido);
    void mover_item(Personaje& personaje, const MoverItemEntreRanuras& pedido);

public:
    ControladorDeJugador(GestorDeSectores& sectores, Notificador& notificador);

    // Devuelve true si el comando era de este controlador (y lo consumió).
    bool atender(Personaje& personaje, const ComandoCliente& comando);
};

#endif
