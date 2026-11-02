#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <map>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "controladores/controlador_de_acciones.h"
#include "controladores/controlador_de_cofres.h"
#include "controladores/controlador_de_drops.h"
#include "controladores/controlador_de_jugador.h"
#include "controladores/controlador_de_mazmorra.h"
#include "jugadores/personaje.h"
#include "mapa/mapa.h"
#include "notificador.h"
#include "recursos/recursos_de_juego.h"
#include "sector/gestor_de_sectores.h"

// Router del juego. Es dueño de los datos compartidos (sectores y personajes) y de los
// controladores, recibe los comandos de red y el tick, y los reparte llamando a cada controlador
// explícitamente. Para sumar un feature nuevo se agregaría un controlador, un miembro acá y una
// línea en atender() o actualizar().
class Mundo {
private:
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    std::map<uint32_t, Personaje> personajes;
    GestorDeSectores sectores;

    ControladorDeJugador controlador_de_jugador;
    ControladorDeAcciones controlador_de_acciones;
    ControladorDeCofres controlador_de_cofres;
    ControladorDeDrops controlador_de_drops;
    ControladorDeMazmorra controlador_de_mazmorra;

    void resolver_transicion(Personaje& personaje, const Transicion& transicion);

public:
    Mundo(const RecursosDeJuego& recursos, Notificador& notificador, uint8_t ticks_por_segundo);

    void sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados);
    void atender(uint32_t id_jugador, const ComandoCliente& comando);
    void actualizar();

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif
