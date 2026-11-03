#ifndef CONTROLADOR_DE_COFRES_H
#define CONTROLADOR_DE_COFRES_H

#include <cstdint>
#include <map>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Comando MoverItemConCofre y cierre automático del cofre. Un cofre se mantiene abierto solo
// mientras el jugador siga junto a él (en una casilla adyacente, diagonales incluidas), en el mismo
// sector, y el cofre exista. Si se aleja, se teletransporta o alguien lo rompe, se cierra solo y el
// cliente se entera. Abrir y cerrar es una interacción con la mano vacía y la resuelve
// AccionInteractuarCofre.
class ControladorDeCofres {
private:
    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    Notificador& notificador;

    // El cofre abierto por el jugador, solo si sigue estando junto a él y en el mismo sector donde
    // lo abrió.
    const ObjetoDelMundo* cofre_abierto_valido(const Personaje& personaje) const;

    void notificar_estado(
        const Personaje& personaje, uint32_t id_cofre, bool abierto,
        const std::vector<RanuraInventario>& ranuras);

    // Cierra el cofre que tenía abierto (si tenía) y se lo avisa al cliente.
    void cerrar(Personaje& personaje);
    void mover(Personaje& personaje, const MoverItemConCofre& pedido);

public:
    ControladorDeCofres(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        Notificador& notificador);

    // Devuelve true si el comando era de este controlador (y lo consumió).
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // Un tick: cierra los cofres que ya no corresponde tener abiertos. Saltea a los desconectados.
    void actualizar();

    // El jugador se desconecta: se olvida del cofre que tenía abierto, sin avisar. Al volver el
    // cliente arranca sin ningún cofre abierto.
    void al_desconectarse(Personaje& personaje);
};

#endif
