#ifndef MENSAJES_PARTIDA_H
#define MENSAJES_PARTIDA_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../apariencia_personaje.h"
#include "../color_de_jugador.h"
#include "../direccion.h"
#include "../estacion.h"
#include "../estado_de_descanso.h"
#include "../modo_uso.h"
#include "../ranura_inventario.h"
#include "../tipo_accion.h"
#include "../tipo_enemigo.h"
#include "../tipo_item.h"
#include "../tipo_objeto.h"

// Política de tipos del protocolo: las cantidades, ids y posiciones viajan sin signo (u8/u16/u32),
// porque nunca son negativas. El único campo con signo es el eje de Moverse (i8), donde el signo
// distingue izquierda/arriba de derecha/abajo.

// Largo máximo de un mensaje escrito, medido en bytes. Tiene que ser igual en cliente y servidor
// para que el cliente no deje escribir algo que el servidor vaya a rechazar.
constexpr size_t LARGO_MAXIMO_DE_MENSAJE_DE_CHAT = 240;

// Cliente a servidor

struct ActivarMultijugador {};

struct SalirDePartida {};

struct GuardarPartida {};

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

// El jugador acostado o desmayado confirma que está listo para pasar al día siguiente. Cuando
// todos los conectados lo dieron, empieza el día siguiente.
struct DarElOk {};

// Pregunta qué pasaría si se usara el ítem de la ranura activa sobre esa casilla, para que el
// cliente sepa qué marcar en el piso. No cambia nada: la respuesta es CasillaConsultada.
struct ConsultarCasilla {
    uint32_t columna = 0;
    uint32_t fila = 0;
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

// El texto que escribió el jugador, tal cual. El servidor valida el largo, los
// espacios y el significado (global, privado o comando).
struct EnviarMensajeDeChat {
    std::string texto;
};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
};

struct PartidaAbandonada {};

// La sesión se cerró sola por un error interno del juego, la partida sigue
// existiendo y el jugador vuelve al lobby.
struct PartidaCerradaPorError {};

// Vida y energía máximas de un jugador. Única fuente de verdad compartida entre servidor
// (Personaje::VIDA_MAXIMA / ENERGIA_MAXIMA las toman de acá) y cliente (para saber a qué escala
// dibujar sus gauges del HUD).
constexpr uint8_t VIDA_MAXIMA_JUGADOR = 100;
constexpr uint8_t ENERGIA_MAXIMA_JUGADOR = 100;

// Lo que el cliente necesita para dibujar a un jugador. La posición es en píxeles del mapa.
// "mirando", "camina" y "accion_actual" bastan para elegir el sprite y la animación. "vida" y
// "energia" son para los gauges del HUD, de 0 a VIDA_MAXIMA_JUGADOR / ENERGIA_MAXIMA_JUGADOR.
struct EstadoJugador {
    uint32_t id_jugador = 0;
    std::string mapa;
    uint32_t x = 0;
    uint32_t y = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
    uint8_t ranura_activa = 0;
    TipoAccion accion_actual = TipoAccion::NINGUNA;
    uint8_t vida = VIDA_MAXIMA_JUGADOR;
    uint8_t energia = ENERGIA_MAXIMA_JUGADOR;
    uint32_t monedas = 0;
    AparienciaPersonaje apariencia;

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

// vida le queda (0 si está muerto esperando revivir; el cliente simplemente no lo dibuja en ese
// caso). "id" es único dentro de su mapa, igual que con los objetos y los drops.
struct EstadoEnemigo {
    uint32_t id = 0;
    TipoEnemigo tipo = TipoEnemigo::NINGUNO;
    uint32_t columna = 0;
    uint32_t fila = 0;
    uint16_t vida = 0;
    uint16_t vida_maxima = 0;
    // Si está en medio del gesto de golpear a un jugador, para la animación de ataque del cliente.
    bool atacando = false;
    // Si está en medio de la animación de morir. Con vida en 0 y esto en false, ya terminó de
    // morir: el cliente no tiene que dibujar nada hasta que vuelva a estar vivo.
    bool muriendo = false;
};

// Una casilla de tierra tratada con la pala, por columna y fila del mapa.
struct CasillaTratada {
    uint32_t columna = 0;
    uint32_t fila = 0;

    bool operator==(const CasillaTratada&) const = default;
};

// Snapshot completa de UN sector. Se manda una sola vez cuando el jugador entra o cambia de
// sector. Después, los cambios llegan como eventos.
struct EstadoJuego {
    std::string mapa;
    std::vector<EstadoJugador> jugadores;
    std::vector<EstadoObjetoDelMundo> objetos;
    std::vector<EstadoDrop> drops;
    std::vector<EstadoEnemigo> enemigos;
    std::vector<CasillaTratada> tierra;
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

// Un enemigo del sector del receptor que cambió: lo golpearon, murió (vida en 0) o revivió. Si el
// id no existía todavía del lado del cliente (recién apareció, por ejemplo al poblarse un nivel),
// lo agrega; si ya existía, lo reemplaza entero.
struct EnemigoActualizado {
    EstadoEnemigo enemigo;
};
// Cómo está el jugador que lo recibe respecto del fin del día, y cuántos ya están listos. Se manda
// a cada jugador cuando cambia algo (alguien se acuesta, se levanta, se desmaya o da el Ok).
// "listos" son los que están acostados o desmayados y ya dieron el Ok; "jugadores", los
// conectados.
struct EstadoDelDescanso {
    EstadoDeDescanso estado = EstadoDeDescanso::DESPIERTO;
    bool dio_el_ok = false;
    uint8_t listos = 0;
    uint8_t jugadores = 0;
    bool puede_levantarse = false;

    bool operator==(const EstadoDelDescanso&) const = default;
};

// Cómo marcar en el piso lo que se va a hacer con el ítem de la mano.
enum class MarcaDeCasilla : uint8_t {
    NINGUNA = 0,
    VALIDA = 1,
    INVALIDA = 2,
};

// Respuesta a ConsultarCasilla, solo para el jugador que preguntó. (columna, fila) es la casilla
// consultada y la esquina de arriba a la izquierda del rectángulo a marcar, de ancho x alto
// casillas: 1x1 para la pala y el pico, el tamaño del mueble para un mueble.
struct CasillaConsultada {
    uint32_t columna = 0;
    uint32_t fila = 0;
    uint8_t ancho = 1;
    uint8_t alto = 1;
    MarcaDeCasilla marca = MarcaDeCasilla::NINGUNA;
};

// La hora del juego. El servidor la manda a todos cada 10 minutos de juego, cuando empieza un día
// nuevo y a cada jugador que entra. "hora" va de 0 a 23: el día arranca a las 6 y termina a las 2
// de la mañana del día siguiente (hora = 0 y 1 son la madrugada del mismo día de juego).
// "dia" cuenta todos los días desde que empezó la partida; dia_de_la_estacion, estacion y anio
// salen de él (por defecto, 28 días por estación y 4 estaciones por año).
#define DIA_DE_LA_SEMANA_MAXIMO 6

struct HoraDelJuego {
    uint32_t dia = 1;
    uint8_t hora = 6;
    uint8_t minuto = 0;
    uint8_t dia_de_la_estacion = 1;
    Estacion estacion = Estacion::PRIMAVERA;
    uint16_t anio = 1;
    uint8_t dia_de_la_semana = 0;

    bool operator==(const HoraDelJuego&) const = default;
};

// Cambió el estado de una casilla de tierra del sector del receptor. "tratada" indica si quedó
// tratada con la pala (true) o si volvió a tierra común con el pico (false).
struct TierraCambiada {
    CasillaTratada casilla;
    bool tratada = false;
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

// Canal de un mensaje que el servidor manda al cliente. "PRIVADO" avisa que el mensaje era para un
// único destinatario, "SISTEMA" son respuestas del propio servidor (errores de envío, comandos
// desconocidos) y "COMERCIANTE" queda reservado para las respuestas de los NPCs del mercado.
enum class CanalDeChat : uint8_t {
    GLOBAL = 0,
    PRIVADO = 1,
    SISTEMA = 2,
    COMERCIANTE = 3,
};

// Un mensaje ya interpretado por el servidor. "remitente" es el nombre del
// jugador o del NPC que habla, "destinatario" solo tiene valor en los privados. "texto" es el
// contenido sin el prefijo "@nombre". "color_del_remitente" lo elige el servidor al entrar a la
// partida y se conserva durante toda la partida.
struct MensajeDeChat {
    CanalDeChat canal = CanalDeChat::GLOBAL;
    std::string remitente;
    std::string destinatario;
    std::string texto;
    ColorDeJugador color_del_remitente = ColorDeJugador::AZUL;
};

#endif
