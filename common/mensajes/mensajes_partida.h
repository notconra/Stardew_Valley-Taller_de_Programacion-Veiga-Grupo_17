#ifndef MENSAJES_PARTIDA_H
#define MENSAJES_PARTIDA_H

#include <cstdint>
#include <string>
#include <vector>

#include "../direccion.h"
#include "../modo_uso.h"
#include "../ranura_inventario.h"
#include "../tipo_accion.h"
#include "../tipo_item.h"
#include "../tipo_objeto.h"

// Política de tipos del protocolo: las cantidades, ids y posiciones viajan sin signo (u8/u16/u32),
// porque nunca son negativas. El único campo con signo es el eje de Moverse (i8), donde el signo
// distingue izquierda/arriba de derecha/abajo.

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

// Usa el ítem de la ranura activa (o de la indicada). El servidor interpreta según el tipo de
// ítem. Si "tiene_casilla_objetivo" es true, la acción apunta a esa casilla del mapa (por ejemplo,
// la que marcó el mouse); si no, el servidor usa la casilla de enfrente del jugador.
// "modo" indica si es Clic Izquierdo (PRINCIPAL) para herramientas/colocar o Clic Derecho
// (SECUNDARIO) para interactuar.
struct UsarItem {
    ModoUso modo = ModoUso::PRINCIPAL;
    uint8_t indice_ranura = INDICE_RANURA_ACTIVA;
    bool tiene_casilla_objetivo = false;
    uint32_t columna_objetivo = 0;
    uint32_t fila_objetivo = 0;
};

// Mueve ítems entre ranuras de la barra. Si el destino es el tacho, el ítem se elimina
// permanentemente, las herramientas no se pueden eliminar.
struct MoverItemEntreRanuras {
    uint8_t origen = 0;
    uint8_t destino = 0;
    uint16_t cantidad = 0;  // 0 significa mover todo lo posible
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

// Tira al piso parte de una ranura de la barra. El servidor decide dónde cae.
// cantidad == 0 significa todo el stack.
struct SoltarItem {
    uint8_t ranura = 0;
    uint16_t cantidad = 0;
};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
};

struct PartidaAbandonada {};

// La sesión se cerró sola por un error interno del juego, la partida sigue
// existiendo y el jugador vuelve al lobby.
struct PartidaCerradaPorError {};

// Lo que el cliente necesita para dibujar a un jugador. La posición es en píxeles del mapa.
// "mirando", "camina" y "accion_actual" bastan para elegir el sprite y la animación.
struct EstadoJugador {
    uint32_t id_jugador = 0;
    std::string mapa;
    uint32_t x = 0;
    uint32_t y = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
    uint8_t ranura_activa = 0;
    TipoAccion accion_actual = TipoAccion::NINGUNA;

    // Esto sirve pq el servidor compara con lo último que mandó para no repetir un
    // JugadoresEnSector igual.
    bool operator==(const EstadoJugador&) const = default;
};

// Un objeto del mundo que hay en el mapa (mineral, cofre, puerta, mueble...). "id" es único dentro
// de su mapa y "tipo" dice cómo se ve. Ocupa una sola casilla, la (columna, fila).
struct EstadoObjetoDelMundo {
    uint32_t id = 0;
    std::string mapa;
    uint32_t columna = 0;
    uint32_t fila = 0;
    TipoObjeto tipo = TipoObjeto::NINGUNO;
};

// Un ítem tirado en el piso de un mapa. "id" es único dentro de su mapa.
struct EstadoDrop {
    uint32_t id = 0;
    uint32_t columna = 0;
    uint32_t fila = 0;
    TipoItem tipo = TipoItem::NINGUNO;
    uint16_t cantidad = 0;
    Calidad calidad = Calidad::BASE;
};

// Snapshot completa de UN sector. Se manda una sola vez cuando el jugador entra o cambia de
// sector. Después, los cambios llegan como eventos.
struct EstadoJuego {
    std::string mapa;
    std::vector<EstadoJugador> jugadores;
    std::vector<EstadoObjetoDelMundo> objetos;
    std::vector<EstadoDrop> drops;
};

// Posición y estado de todos los jugadores del sector del receptor. Se manda cada tick, pero solo
// a los jugadores de ese sector, nunca viajan los jugadores de otros mapas.
struct JugadoresEnSector {
    std::string mapa;
    std::vector<EstadoJugador> jugadores;
};

// Un objeto nuevo en el sector del receptor. Si el id ya existía, el cliente lo reemplaza.
struct ObjetoColocado {
    uint32_t id = 0;
    uint32_t columna = 0;
    uint32_t fila = 0;
    TipoObjeto tipo = TipoObjeto::NINGUNO;
};

// Un objeto que dejó de existir en el sector del receptor (se picó, se rompió, se retiró).
struct ObjetoRetirado {
    uint32_t id = 0;
};

// Un ítem que apareció en el piso del sector del receptor.
struct DropAparecido {
    EstadoDrop drop;
};

// Un ítem que dejó de estar en el piso. id_jugador == 0 significa que lo perdió el mundo (por
// ejemplo, se limpió un nivel de la mazmorra), no que lo recogió alguien.
struct DropRetirado {
    uint32_t id = 0;
    uint32_t id_jugador = 0;
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
