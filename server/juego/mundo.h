#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "../persistencia/datos_partida_guardada.h"
#include "cofres/gestor_de_cofres.h"
#include "colisiones/colisiones.h"
#include "inventario/gestor_inventario_jugador.h"
#include "jugadores/jugadores.h"
#include "mapa/mapa.h"
#include "mazmorra/mazmorra.h"
#include "objetos_del_mundo/objetos_del_mundo.h"
#include "recursos/recursos_de_juego.h"

// Qué cambió en el mundo al atender un comando. La sesión lo usa para mandar solo lo necesario al
// jugador que lo pidió, sin reenviar todo el estado.
struct ResultadoDeComando {
    bool inventario_cambiado = false;
    // Si tiene valor, hay que mandar una ActualizacionCofre. Un cofre cerrado se representa con
    // abierto == false.
    std::optional<CofreJugador> cofre;
};

// Clase madre de toda la lógica de juego en una partida. Ahora hay un mapa por zona (granja,
// pueblo, cada nivel de la mazmorra, ...), y cada uno tiene su propia Colisiones, sus propios
// objetos del mundo y su propio Jugadores: un jugador solo existe en el Jugadores del mapa donde
// está parado, así que dos jugadores en mapas distintos ni se ven ni interactúan entre sí.
class Mundo {
private:
    GestorInventarioJugador gestor_inventario;

    const RecursosDeJuego& recursos;
    std::map<std::string, Colisiones> colisiones_por_mapa;
    std::map<std::string, GestorDeObjetos> objetos_por_mapa;
    std::map<std::string, GestorDeCofres> cofres_por_mapa;
    std::map<std::string, Jugadores> jugadores_por_mapa;
    std::map<uint32_t, std::string> mapa_actual;  // en qué mapa está cada jugador conectado
    std::map<uint32_t, DatosJugadorGuardado> datos_jugadores;
    std::map<uint32_t, int> cofres_abiertos;  // id del cofre que tiene abierto cada jugador
    std::optional<Mazmorra> mazmorra;
    std::vector<std::string> niveles_de_mazmorra;

    // Lo saca del Jugadores de mapa_origen y lo mete en el de transicion.mapa_destino, en la
    // casilla que indica la transición.
    void cambiar_de_mapa(
        uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion);

    // Coloca como objeto del mundo el ítem de la ranura (cualquiera con item_colocable en el
    // catálogo de objetos: hoy el cofre, a futuro muebles) en la casilla pedida o en la de
    // enfrente. El servidor valida que la casilla esté a distancia uno del jugador y libre.
    ResultadoDeComando usar_item(uint32_t id_jugador, const UsarItem& pedido);

    // Abre el cofre de la casilla de enfrente o cierra el que el jugador tenga abierto.
    ResultadoDeComando abrir_cofre(uint32_t id_jugador, const AbrirCofre& pedido);

    // Transfiere ítems entre la barra del jugador y el cofre que tiene abierto.
    ResultadoDeComando mover_item_con_cofre(uint32_t id_jugador, const MoverItemConCofre& pedido);
    // Decide qué hacer con una transición: entrar a la mazmorra, bajar un nivel, salir de ella o
    // un cambio de mapa común.
    void resolver_transicion(
        uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion);

    // Lo pasa al nivel en el que la mazmorra dice que está, parado en la entrada del nivel.
    void llevar_al_nivel_actual(uint32_t id_jugador, const std::string& mapa_origen);

    // Vuelve a generar los minerales de los diez niveles. Se llama cuando se entra a la mazmorra
    // vacía, para que los spawns cambien entre visita y visita. Lo que colocó un jugador (un
    // cofre) se conserva.
    void poblar_niveles_de_mazmorra();

public:
    Mundo(
        const RecursosDeJuego& recursos, uint8_t ticks_por_segundo,
        const std::vector<DatosJugadorGuardado>& jugadores_guardados);

    // Deja en el mundo exactamente a los jugadores conectados. Los nuevos aparecen en el mapa
    // inicial (recursos.mapa_inicial) o en su posición guardada si existe.
    void sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados);

    // Atiende un comando de juego, cada uno va a la parte del mundo que corresponde. Devuelve qué
    // cambió para que la sesión mande al jugador solo lo necesario.
    ResultadoDeComando atender(uint32_t id_jugador, const ComandoCliente& comando);

    // Avanza un tick en todos los mapas, y resuelve las transiciones que se dispararon.
    void actualizar();

    // El estado de la partida. Incluye los jugadores de todos los mapas, pero los objetos solo de
    // los mapas donde hay alguien.
    EstadoJuego estado() const;

    // Snapshot privado del inventario de un jugador.
    ActualizacionInventario obtener_inventario(uint32_t id_jugador) const;

    // Devuelve los datos guardados de un jugador que no está en el mundo en este tick.
    std::optional<DatosJugadorGuardado> obtener_datos_desconectado(uint32_t id_jugador) const;

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif
