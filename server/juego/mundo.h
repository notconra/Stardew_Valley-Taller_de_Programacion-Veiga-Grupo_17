#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "base/colisiones/colisiones.h"
#include "base/mapas/mapa.h"
#include "base/recursos/recursos_de_juego.h"
#include "inventario/gestor_inventario_jugador.h"
#include "jugadores/jugadores.h"
#include "mazmorra/mazmorra.h"
#include "tierra/tierra_tratada.h"

// Qué cambió al atender un comando, para que la sesión sepa qué tiene que reenviar.
struct CambiosPorComando {
    bool inventario = false;
    bool tierra = false;
    std::optional<CasillaConsultada> consulta;
};

// Clase madre de toda la lógica de juego en una partida. Ahora hay un mapa por zona (granja,
// pueblo, cada nivel de la mazmorra, ...), y cada uno tiene su propia Colisiones y su propio
// Jugadores: un jugador solo existe en el Jugadores del mapa donde está parado, así que dos
// jugadores en mapas distintos ni se ven ni interactúan entre sí.
class Mundo {
private:
    GestorInventarioJugador gestor_inventario;

    const RecursosDeJuego& recursos;
    std::map<std::string, Colisiones> colisiones_por_mapa;
    std::map<std::string, Jugadores> jugadores_por_mapa;
    std::map<uint32_t, std::string> mapa_actual;
    std::optional<Mazmorra> mazmorra;
    std::vector<std::string> niveles_de_mazmorra;
    std::map<std::string, TierraTratada> tierra_por_mapa;

    // Lo saca del Jugadores de mapa_origen y lo mete en el de transicion.mapa_destino, en la
    // casilla que indica la transición.
    void cambiar_de_mapa(
        uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion);

    // Decide qué hacer con una transición: entrar a la mazmorra, bajar un nivel, salir de ella o
    // un cambio de mapa común.
    void resolver_transicion(
        uint32_t id_jugador, const std::string& mapa_origen, const Transicion& transicion);

    // Lo pasa al nivel en el que la mazmorra dice que está, parado en la entrada del nivel.
    void llevar_al_nivel_actual(uint32_t id_jugador, const std::string& mapa_origen);

    // Si la casilla es una de las 8 que rodean los pies del jugador (ni la propia ni una lejana).
    bool al_alcance(uint32_t id_jugador, const Casilla& casilla) const;

    // Si usar el ítem de la ranura sobre la casilla cambiaría algo: la pala sobre tierra que se
    // puede tratar, el pico sobre tierra tratada, siempre al alcance del jugador.
    bool tendria_efecto(uint32_t id_jugador, uint8_t indice_ranura, const Casilla& casilla) const;

    // Usa el ítem de la ranura sobre la casilla pedida: la pala trata la tierra y el pico la
    // vuelve a tierra común. Devuelve true si cambió la tierra.
    bool usar_item(uint32_t id_jugador, const UsarItem& pedido);

public:
    Mundo(const RecursosDeJuego& recursos, uint8_t ticks_por_segundo);

    // Deja en el mundo exactamente a los jugadores conectados. Los nuevos aparecen en el mapa
    // inicial (recursos.mapa_inicial).
    void sincronizar_jugadores(const std::vector<uint32_t>& ids_conectados);

    // Atiende un comando de juego, cada uno va a la parte del mundo que corresponde. Devuelve true
    // si cambió el inventario del jugador y hay que reenviárselo.
    CambiosPorComando atender(uint32_t id_jugador, const ComandoCliente& comando);

    // Avanza un tick en todos los mapas, y resuelve las transiciones que se dispararon.
    void actualizar();

    EstadoJuego estado() const;

    // Toda la tierra tratada de la partida.
    EstadoTierra estado_tierra() const;

    // Snapshot privado del inventario de un jugador.
    ActualizacionInventario obtener_inventario(uint32_t id_jugador) const;

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif
