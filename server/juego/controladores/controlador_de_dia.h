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
#include "../muebles/camas_ocupadas.h"
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
    CamasOcupadas camas_ocupadas;
    std::map<uint32_t, EstadoDelDescanso> descansos_enviados;
    bool dia_terminado = false;
    double probabilidad_de_lluvia;
    bool lloviendo = false;
    // Al empezar un día se sortea si llueve.
    void sortear_lluvia();

    // Los que se quedaron sin vida o sin energía se desmayan, y a las 2 am todos los que siguen
    // despiertos (también el que entra a esa hora).
    void desmayar();

    void desmayar_persona(Personaje& personaje);

    // Les avisa a todos por el chat que se desmayó por quedarse sin vida o sin energía.
    void avisar_el_desmayo(const Personaje& personaje);

    // La respuesta "Si" a la pregunta de dormir: si la cama sigue libre y al alcance, lo acuesta
    // en el medio de la cama.
    void acostar(Personaje& personaje, const Dormir& pedido);

    // Se levanta de la cama antes de que pase el día y queda parado al lado.
    void levantar(Personaje& personaje);

    // Lo pone en esa posición (la esquina del dibujo, en píxeles) dentro de su mapa.
    void reubicar(Personaje& personaje, double x, double y);

    // Los acostados y desmayados no se mueven.
    void frenar_a_los_que_descansan();

    // Si hay al menos un jugador conectado y todos descansan y dieron el OK.
    bool todos_listos() const;

    void terminar_el_dia();
    // Al empezar el día: con toda la vida y la energía si durmió en la cama, con la mitad si no, y
    // al lado de su cama.
    void despertar(Personaje& personaje);
    // Lo saca de la mazmorra y lo deja al lado de su última cama, o en la entrada de la casa si no
    // tiene una (o ya no está).
    void llevar_a_la_casa(Personaje& personaje);

    // La casilla libre al lado de la cama donde se despierta, si la cama sigue en su lugar.
    // Prefiere una donde no haya otro jugador.
    std::optional<Casilla> casilla_al_lado_de_la_cama(
        const UbicacionDeCama& cama, uint32_t id_que_despierta) const;

    // Donde aparece el que nunca durmió: la casilla a la que lleva la puerta de la casa.
    std::optional<Casilla> casilla_de_la_entrada_de_la_casa() const;

    // Lo pone con los pies en esa casilla del mapa indicado.
    void parar_en(Personaje& personaje, const std::string& id_mapa, const Casilla& casilla);

    // Acostado, sin el OK y antes de las 2 am: si se mueve, se levanta.
    bool puede_levantarse(const Personaje& personaje) const;

    EstadoDelDescanso descanso_de(const Personaje& personaje) const;

    // Le avisa a cada jugador conectado su estado de descanso, solo si cambió.
    void avisar_descansos();

public:
    ControladorDelDia(
        const ConfiguracionDiaServidor& configuracion, uint8_t ticks_por_segundo,
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        const RecursosDeJuego& recursos, Notificador& notificador,
        SacarDeLaMazmorra sacar_de_la_mazmorra);

    // Dormir, Levantarse, DarElOk, y los comandos que no puede hacer quien está descansando.
    // Devuelve true si lo consumió. Si un jugador acostado que todavía no dio el OK se mueve antes
    // de las 2 am, se levanta al lado de la cama (y el movimiento sigue su camino, devuelve
    // false).
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // Un tick del servidor.
    void actualizar();

    // El que entra recibe la hora de ahora, así no espera hasta el próximo aviso para verla.
    void al_conectarse(const Personaje& personaje);

    // El que se va deja de contar para el fin del día y vuelve despierto. Si estaba acostado, lo
    // deja parado al lado de la cama (no arriba de ella).
    void al_desconectarse(Personaje& personaje);

    // Si se había ido sin vida o sin energía: vuelve como si se hubiera despertado de un desmayo,
    // con la mitad de la vida y de la energía y al lado de su cama. Tiene que estar ya en un mapa.
    // Devuelve si lo tuvo que despertar.
    bool despertar_si_volvio_desmayado(Personaje& personaje);

    // La hora de ahora (la misma que ve el jugador).
    HoraDelJuego hora() const;

    // Al retomar una partida guardada: arranca a las 6 am de ese día.
    void continuar_en_el_dia(uint32_t dia);

    // Avanza al día siguiente (a las 6:00 am) actualizando animales y producción sin mover a los
    // jugadores.
    void avanzar_dia();

    // Avanza el reloj la cantidad de minutos indicada y notifica a los jugadores.
    void avanzar_minutos(uint32_t minutos);

    // Cheat: pasa a esa estación y se lo avisa a todos.
    void cambiar_estacion(Estacion estacion);

    // Cheat: hace llover o para la lluvia ya mismo y se lo avisa a todos.
    void cambiar_lluvia(bool llueve);
    bool esta_lloviendo() const;

    // Si desde la última vez que se preguntó terminó un día (todos dieron el OK). Devuelve true
    // una sola vez por día terminado: es el aviso para guardar la partida.
    bool tomar_dia_terminado();
};

#endif
