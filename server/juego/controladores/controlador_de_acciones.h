#ifndef CONTROLADOR_DE_ACCIONES_H
#define CONTROLADOR_DE_ACCIONES_H

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../acciones/accion.h"
#include "../jugadores/personaje.h"
#include "../muebles/camas_ocupadas.h"
#include "../notificador.h"
#include "../recursos/recursos_de_juego.h"
#include "../sector/gestor_de_sectores.h"

// Comando UsarItem (click con un ítem en mano o con la mano vacía). Resuelve la casilla objetivo,
// valida que el cliente no pida algo imposible y prueba las acciones del juego en orden. Ejecuta la
// primera que aplica y funciona. Además lleva la cuenta de cuánto tiempo sigue ocupado el
// personaje con su acción. También contesta ConsultarCasilla: qué se marcaría en el piso si se
// usara el ítem de la mano en esa casilla, con las mismas reglas que al usarlo.
class ControladorDeAcciones {
private:
    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    ResolverDeTransicion resolver_transicion;
    CamasOcupadas camas_ocupadas;
    std::vector<std::unique_ptr<Accion>> acciones;

    bool al_alcance(const Sector& sector, const Casilla& propia, const Casilla& objetivo) const;

    // Ejecuta la primera acción que aplica y funciona. Devuelve esa acción, o nullptr si ninguna
    // ocurrió.
    const Accion* ejecutar(ContextoDeAccion& contexto);

    // Qué marcar en el piso para el ítem de la ranura activa en esa casilla: el rectángulo del
    // mueble en verde o rojo, o una casilla en verde si la pala o el pico harían algo ahí.
    CasillaConsultada consultar(Personaje& personaje, const ConsultarCasilla& pedido);

public:
    ControladorDeAcciones(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        const RecursosDeJuego& recursos, Notificador& notificador,
        ResolverDeTransicion resolver_transicion);

    // Devuelve true si el comando era de este controlador (y lo consumió).
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // Un tick: descuenta el tiempo de acción de cada personaje conectado.
    void actualizar();

    // El jugador se desconecta.
    void al_desconectarse(Personaje& personaje);
};

#endif
