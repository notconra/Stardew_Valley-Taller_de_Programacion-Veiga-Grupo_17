#ifndef MENSAJES_PARTIDA_H
#define MENSAJES_PARTIDA_H

#include <cstdint>
#include <string>
#include <vector>

#include "./../juego/direccion.h"
#include "./../juego/ranura_inventario.h"

// Índice para la ranura activa de la barra de herramientas.
constexpr uint8_t INDICE_RANURA_ACTIVA = 0xFF;

// Cliente a servidor

struct ActivarMultijugador {};

struct SalirDePartida {};

// Hacia dónde quiere moverse el jugador, por eje: horizontal -1 (izquierda), 0 o 1 (derecha);
// vertical -1 (arriba), 0 o 1 (abajo). Es un estado, NO es un paso, osea, el servidor lo sigue
// aplicando en cada tick hasta que llegue otro Moverse que actualice el estado. {0, 0} es "quedarse
// quieto".
struct Moverse {
    int8_t horizontal = 0;
    int8_t vertical = 0;

    bool operator==(const Moverse&) const = default;
};

// Selecciona la ranura activa de la barra de herramientas (0-based, y el servidor valida el rango).
struct SeleccionarRanuraDeLaBarra {
    uint8_t indice_ranura = 0;
};

// Usa el ítem de la ranura activa (o de la indicada). El servidor interpreta según tipo.
struct UsarItem {
    uint8_t indice_ranura = INDICE_RANURA_ACTIVA;
};

// Mueve ítems entre ranuras de la barra. Si el destino es el tacho, el ítem se elimina
// permanentemente, las herramientas no se pueden eliminar.
struct MoverItemEntreRanuras {
    uint8_t origen = 0;
    uint8_t destino = 0;
    uint16_t cantidad = 0;  // 0 significa mover todo lo posible
};

// (revisar si no llega a andar el cofre cuando se pueda colocar en el mapa)
// Abre o cierra el cofre que el jugador tiene en la casilla de enfrente. El servidor responde
// siempre con ActualizacionCofre, y si no hay cofre, responde un cofre cerrado.
struct AbrirCofre {
    bool cerrar = false;
};

// Mueve un stack entre la barra del jugador y el cofre que tiene abierto.
// origen y destino son ranuras del contenedor que corresponda según desde_el_cofre.
// cantidad == 0 mueve el stack completo. Si no entra todo, no se mueve nada.
struct MoverItemConCofre {
    uint8_t origen = 0;
    uint8_t destino = 0;
    uint16_t cantidad = 0;
    bool desde_el_cofre = false;
};

// Solicita el inventario completo (respuesta será ActualizacionInventario).
struct SolicitarInventario {};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
};

struct PartidaAbandonada {};

// La sesión se cerró sola por un error interno del juego, la partida sigue
// existiendo y el jugador vuelve al lobby.
struct PartidaCerradaPorError {};

// Lo que el cliente necesita para dibujar a un jugador. La posición es en píxeles del mapa.
// "mirando" y "camina" bastan para elegir el sprite y la animación.
struct EstadoJugador {
    uint32_t id_jugador = 0;
    int32_t x = 0;
    int32_t y = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
    uint8_t ranura_activa = 0;
};
// Estado completo de los jugadores de una partida en un tick.
struct EstadoJuego {
    std::vector<EstadoJugador> jugadores;
};

// La barra de un jugador. Las ranuras reales van de 0 a ranuras.size() - 1.
// El tacho no es una ranura del arreglo, el cliente lo dibuja aparte con el índice
// ranuras.size(), que el servidor interpreta como ranura para eliminar.
struct InventarioJugador {
    uint32_t id_jugador = 0;
    uint8_t ranura_activa = 0;
    std::vector<RanuraInventario> ranuras;
};

// Cofre que un jugador tiene abierto. abierto == false significa que se cerró.
// Si está abierto, ranuras trae todo el contenido del cofre.
struct CofreJugador {
    uint32_t id_cofre = 0;
    bool abierto = false;
    std::vector<RanuraInventario> ranuras;
};

// Actualización de inventario dirigida a un cliente específico (se envía por su cola).
struct ActualizacionInventario {
    InventarioJugador inventario;
};

// Actualización del cofre abierto por un jugador. Si el cofre se cerró, abierto == false.
struct ActualizacionCofre {
    CofreJugador cofre;
};

#endif
