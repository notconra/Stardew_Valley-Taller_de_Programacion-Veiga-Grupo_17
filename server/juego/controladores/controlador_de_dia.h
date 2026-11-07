#ifndef CONTROLADOR_DEL_DIA_H
#define CONTROLADOR_DEL_DIA_H

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../recursos/recursos_de_juego.h"
#include "../sector/gestor_de_sectores.h"
#include "../tiempo/reloj.h"

// Qué hacer con un personaje que está en la mazmorra cuando termina el día (sacarlo). Lo provee
// Mundo, que es el dueño de ControladorDeMazmorra.
using SacarDeLaMazmorra = std::function<void(Personaje&)>;

// El paso del tiempo en la partida y el fin del día.
//
// Cada tick avanza el reloj y, cuando cambia la hora que se muestra (cada 10 minutos de juego) o
// empieza un día nuevo, se la avisa a todos los jugadores.
//
// El día siguiente empieza (a las 6 am) cuando todos los jugadores conectados están acostados en
// una cama o desmayados, y todos dieron el OK. Si llegan las 2 am, el reloj se frena ahí y los que
// seguían despiertos se desmayan, así que a todos les aparece la pantalla del OK; el día sigue sin
// pasar hasta que todos lo den.
// El que se acostó en una cama se despierta al lado de esa cama con la vida y la energía al
// máximo. El que se desmayó (por la hora, o por quedarse sin vida o sin energía) se despierta al
// lado de la última cama donde durmió, o en la casa si nunca durmió, con la mitad.
class ControladorDelDia {
private:
    Reloj reloj;
    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    SacarDeLaMazmorra sacar_de_la_mazmorra;
    std::map<uint32_t, EstadoDelDescanso> descansos_enviados;

    // Los que se quedaron sin vida o sin energía se desmayan, y a las 2 am todos los que siguen
    // despiertos (también el que entra a esa hora).
    void desmayar();

    void desmayar_persona(Personaje& personaje);

    // Los acostados y desmayados no se mueven.
    void frenar_a_los_que_descansan();

    // Si hay al menos un jugador conectado y todos descansan y dieron el OK.
    bool todos_listos() const;

    void terminar_el_dia();
    void despertar(Personaje& personaje);

    // La casilla libre al lado de la cama donde se despierta, si la cama sigue en su lugar.
    // Prefiere una donde no haya otro jugador.
    std::optional<Casilla> casilla_al_lado_de_la_cama(
        const UbicacionDeCama& cama, uint32_t id_que_despierta) const;

    // Donde aparece el que nunca durmió: la casilla a la que lleva la puerta de la casa.
    std::optional<Casilla> casilla_de_la_entrada_de_la_casa() const;

    // Lo pone con los pies en esa casilla del mapa indicado.
    void parar_en(Personaje& personaje, const std::string& id_mapa, const Casilla& casilla);

    EstadoDelDescanso descanso_de(const Personaje& personaje) const;

    // Le avisa a cada jugador conectado su estado de descanso, solo si cambió.
    void avisar_descansos();

public:
    ControladorDelDia(
        const ConfiguracionDiaServidor& configuracion, uint8_t ticks_por_segundo,
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        const RecursosDeJuego& recursos, Notificador& notificador,
        SacarDeLaMazmorra sacar_de_la_mazmorra);

    // DarElOk, y los comandos que no puede hacer quien está descansando. Devuelve true si lo
    // consumió. Si un jugador acostado que todavía no dio el OK se mueve antes de las 2 am, se
    // levanta (y el movimiento sigue su camino, devuelve false).
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // Un tick del servidor.
    void actualizar();

    // El que entra recibe la hora de ahora, así no espera hasta el próximo aviso para verla.
    void al_conectarse(const Personaje& personaje);

    // El que se va deja de contar para el fin del día y vuelve despierto.
    void al_desconectarse(Personaje& personaje);

    // La hora de ahora (la misma que ve el jugador).
    HoraDelJuego hora() const;
};

#endif
