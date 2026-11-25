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
#include "../logro_de_pelea.h"
#include "../modo_uso.h"
#include "../ranura_inventario.h"
#include "../tipo_accion.h"
#include "../tipo_animal.h"
#include "../tipo_cultivo.h"
#include "../tipo_enemigo.h"
#include "../tipo_item.h"
#include "../tipo_lote.h"
#include "../tipo_objeto.h"
#include "../tipo_trampa.h"

// Política de tipos del protocolo: las cantidades, ids y posiciones viajan sin signo (u8/u16/u32),
// porque nunca son negativas. El único campo con signo es el eje de Moverse (i8), donde el signo
// distingue izquierda/arriba de derecha/abajo.

// Largo máximo de un mensaje escrito, medido en bytes. Tiene que ser igual en cliente y servidor
// para que el cliente no deje escribir algo que el servidor vaya a rechazar.
constexpr size_t LARGO_MAXIMO_DE_MENSAJE_DE_CHAT = 240;

// Cliente a servidor

struct ActivarMultijugador {
    bool operator==(const ActivarMultijugador&) const = default;
};

struct SalirDePartida {
    bool operator==(const SalirDePartida&) const = default;
};

struct GuardarPartida {
    bool operator==(const GuardarPartida&) const = default;
};

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
    bool operator==(const SeleccionarRanuraDeLaBarra&) const = default;
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
    bool operator==(const UsarItem&) const = default;
};

// El jugador acostado o desmayado confirma que está listo para pasar al día siguiente. Cuando
// todos los conectados lo dieron, empieza el día siguiente.
struct DarElOk {
    bool operator==(const DarElOk&) const = default;
};

// Pregunta qué pasaría si se usara el ítem de la ranura activa sobre esa casilla, para que el
// cliente sepa qué marcar en el piso. No cambia nada: la respuesta es CasillaConsultada.
struct ConsultarCasilla {
    uint32_t columna = 0;
    uint32_t fila = 0;
    bool operator==(const ConsultarCasilla&) const = default;
};

// Mueve ítems entre ranuras de la barra. Si el destino es el tacho, el ítem se elimina
// permanentemente, las herramientas no se pueden eliminar.
struct MoverItemEntreRanuras {
    uint8_t origen = 0;
    uint8_t destino = 0;
    uint16_t cantidad = 0;  // 0 significa mover todo lo posible
    bool operator==(const MoverItemEntreRanuras&) const = default;
};

// Mueve un stack entre la barra del jugador y el cofre que tiene abierto.
// origen y destino son ranuras del contenedor que corresponda según desde_el_cofre.
// destino == INDICE_RANURA_AUTOMATICA hace que el servidor elija la primera ranura compatible con
// espacio o, si no hay, una vacía. cantidad == 0 mueve el stack completo. Si no entra todo, no se
// mueve nada.
struct MoverItemConCofre {
    uint8_t origen = 0;
    uint8_t destino = 0;
    uint16_t cantidad = 0;
    bool desde_el_cofre = false;
    bool operator==(const MoverItemConCofre&) const = default;
};

// Pide cerrar el cofre que el jugador tiene abierto. Si no tiene ninguno, el servidor lo ignora.
struct CerrarCofre {
    bool operator==(const CerrarCofre&) const = default;
};

// Solicita el inventario completo (respuesta será ActualizacionInventario).
struct SolicitarInventario {
    bool operator==(const SolicitarInventario&) const = default;
};

// Tira al piso parte de una ranura de la barra. El servidor decide dónde cae.
// cantidad == 0 significa todo el stack.
struct SoltarItem {
    uint8_t ranura = 0;
    uint16_t cantidad = 0;
    bool operator==(const SoltarItem&) const = default;
};

// El texto que escribió el jugador, tal cual. El servidor valida el largo, los
// espacios y el significado (global, privado o comando).
struct EnviarMensajeDeChat {
    std::string texto;
    bool operator==(const EnviarMensajeDeChat&) const = default;
};

// El servidor recuerda qué puesto tiene abierto cada jugador, así que los comandos de compra y
// venta no necesitan mandar el id del puesto sino que siempre operan sobre el que está abierto.
struct ComprarEnMercado {
    uint16_t indice_item = 0;  // Índice dentro del catálogo del puesto
    uint16_t cantidad = 1;
    bool operator==(const ComprarEnMercado&) const = default;
};

// Vender un ítem del inventario al puesto.
struct VenderEnMercado {
    uint8_t ranura_inventario = 0;
    uint16_t cantidad = 0;  // 0 o cantidad completa según click izquierdo/derecho
    bool operator==(const VenderEnMercado&) const = default;
};

// Comprar un animal (específico para Marnie, que no va al inventario).
struct ComprarAnimalMercado {
    TipoAnimal tipo_animal = TipoAnimal::NINGUNO;
    uint16_t cantidad = 1;
    bool operator==(const ComprarAnimalMercado&) const = default;
};

// Vender un animal (específico para Marnie, desde el panel de animales de la granja).
struct VenderAnimalMercado {
    TipoAnimal tipo_animal = TipoAnimal::NINGUNO;
    uint16_t cantidad = 1;
    bool operator==(const VenderAnimalMercado&) const = default;
};

// Cierra la ventana del mercado activa.
struct CerrarMercado {
    bool operator==(const CerrarMercado&) const = default;
};

// Le regala a otro jugador de la partida (conectado o no) parte de una ranura de la barra.
// cantidad == 0 regala el stack completo. Las herramientas y la cama no se pueden regalar. El
// regalo queda pendiente hasta que el destinatario lo abre (AbrirRegalos).
struct EnviarRegalo {
    uint32_t id_destinatario = 0;
    uint8_t ranura = 0;
    uint16_t cantidad = 0;
    bool operator==(const EnviarRegalo&) const = default;
};

// Pasa al inventario todos los regalos pendientes que entren. Los que no entran siguen pendientes.
struct AbrirRegalos {
    bool operator==(const AbrirRegalos&) const = default;
};

// Rechaza el regalo pendiente número "indice" (en el orden de RegalosPendientes): no pasa al
// inventario del receptor y vuelve al buzón de quien lo mandó.
struct RechazarRegalo {
    uint8_t indice = 0;
    bool operator==(const RechazarRegalo&) const = default;
};

// Dona al lote lo que acepte del ítem de esa ranura del inventario (índice absoluto, como en el
// cofre). Lo que se dona sale del inventario y queda en el centro.
struct DonarAlLote {
    TipoLote lote = TipoLote::CULTIVOS;
    uint8_t indice_ranura = 0;
    bool operator==(const DonarAlLote&) const = default;
};

// Dona las monedas que pide el lote (el de dinero). El cliente pide confirmación antes.
struct DonarMonedasAlLote {
    TipoLote lote = TipoLote::DINERO;
    bool operator==(const DonarMonedasAlLote&) const = default;
};

// La respuesta "Si" a la pregunta de dormir: acostarse en la cama de esa casilla (la misma que
// vino en PreguntaDeDormir).
struct Dormir {
    uint32_t columna = 0;
    uint32_t fila = 0;
    bool operator==(const Dormir&) const = default;
};

// Levantarse de la cama antes de que pase el día (lo mismo que moverse estando acostado).
struct Levantarse {
    bool operator==(const Levantarse&) const = default;
};

// Servidor a cliente

struct MultijugadorActivado {
    std::string codigo_partida;
    bool operator==(const MultijugadorActivado&) const = default;
};

struct PartidaAbandonada {
    bool operator==(const PartidaAbandonada&) const = default;
};

// La sesión se cerró sola por un error interno del juego, la partida sigue
// existiendo y el jugador vuelve al lobby.
struct PartidaCerradaPorError {
    bool operator==(const PartidaCerradaPorError&) const = default;
};

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
    // Lo que tiene en la ranura activa, para que los demás lo vean en su mano.
    TipoItem item_en_mano = TipoItem::NINGUNO;
    Calidad calidad_en_mano = Calidad::BASE;
    // Cambia con cada acción que se anima (golpe, riego), así los demás ven también la que se
    // repite con la misma herramienta. Da la vuelta al pasar de 255.
    uint8_t numero_de_accion = 0;
    // Acostado en una cama o desmayado, para que los demás lo vean así.
    EstadoDeDescanso descanso = EstadoDeDescanso::DESPIERTO;
    uint8_t vida = VIDA_MAXIMA_JUGADOR;
    uint8_t energia = ENERGIA_MAXIMA_JUGADOR;
    // Los topes de las barras de este jugador: más altos con la mejora del lote de pelea.
    uint8_t vida_maxima = VIDA_MAXIMA_JUGADOR;
    uint8_t energia_maxima = ENERGIA_MAXIMA_JUGADOR;
    // Ya completó el lote de pelea: el HUD dibuja los gauges más largos.
    bool con_mejora_de_pelea = false;
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
    bool operator==(const EstadoObjetoDelMundo&) const = default;
};

// Un ítem tirado en el piso de un mapa. "id" es único dentro de su mapa.
struct EstadoDrop {
    uint32_t id = 0;
    uint32_t columna = 0;
    uint32_t fila = 0;
    TipoItem tipo = TipoItem::NINGUNO;
    uint16_t cantidad = 0;
    Calidad calidad = Calidad::BASE;
    bool operator==(const EstadoDrop&) const = default;
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
    // Hacia dónde mira, para elegir la fila de la hoja de sprites al caminar.
    Direccion mirando = Direccion::ABAJO;
    // Lo que tarda en cruzar una casilla el paso que está dando, para que el cliente deslice el
    // dibujo a la misma velocidad (cambia por tipo y con el frenesí de los vampiros). 0 = el
    // valor por defecto del cliente.
    uint16_t ms_por_casilla = 0;
    // Vampiros: su pareja murió y está en frenesí (más rápida la vampira, más fuerte el vampiro).
    bool frenesi = false;
    // Vampiros: muerto, y ya cayó sangre sobre su cadáver: está por revivir.
    bool sangre = false;
    bool operator==(const EstadoEnemigo&) const = default;
};

// Una llama que lanzó una Skull. Su centro viaja en centésimas de casilla (350 = casilla 3 y media)
// para que el cliente la dibuje suave.
struct EstadoLlama {
    uint32_t id = 0;
    bool verde = false;  // false: azul (básica)
    uint32_t x_centesimas = 0;
    uint32_t y_centesimas = 0;

    bool operator==(const EstadoLlama&) const = default;
};

// Una trampa de la mazmorra, en la casilla (columna, fila). "activa": los pinches están afuera y
// lastiman.
struct EstadoTrampa {
    uint32_t id = 0;
    TipoTrampa tipo = TipoTrampa::NINGUNA;
    uint32_t columna = 0;
    uint32_t fila = 0;
    bool activa = false;
    bool operator==(const EstadoTrampa&) const = default;
};

// Una casilla de tierra tratada con la pala, por columna y fila del mapa.
#define ETAPA_DE_CULTIVO_MAXIMA 3

// Una casilla de tierra tratada y lo que tiene plantado. "etapa" va de 0 (recién plantada) a
// ETAPA_DE_CULTIVO_MAXIMA mientras crece; cuando "madura" es true está lista para cosechar.
// "regada" es si ya se regó en el turno de riego de ahora y "pide_agua" si todavía hay que regarla.
// "ya_dio_fruto" es si se cosechó y está volviendo a dar (la frutilla): se dibuja la planta entera
// sin frutos en vez de la etapa.
struct CasillaTratada {
    uint32_t columna = 0;
    uint32_t fila = 0;
    TipoCultivo cultivo = TipoCultivo::NINGUNO;
    uint8_t etapa = 0;
    bool madura = false;
    bool regada = false;
    bool pide_agua = false;
    bool ya_dio_fruto = false;

    bool operator==(const CasillaTratada&) const = default;
};

// Un animal de granja colocado en el mapa (vaca, gallina).
struct EstadoAnimal {
    uint32_t id = 0;
    TipoAnimal tipo = TipoAnimal::VACA;
    uint8_t variante = 0;
    uint32_t columna = 0;
    uint32_t fila = 0;
    Direccion mirando = Direccion::ABAJO;
    bool caminando = false;
    bool durmiendo = false;

    bool operator==(const EstadoAnimal&) const = default;
};

// Evento cuando cambia el estado, posición o dirección de un animal en el sector
struct AnimalActualizado {
    EstadoAnimal animal;

    bool operator==(const AnimalActualizado&) const = default;
};

// Un animal que dejó de estar en el sector del receptor (se vendió).
struct AnimalRetirado {
    uint32_t id = 0;

    bool operator==(const AnimalRetirado&) const = default;
};

// Notificación cuando una vaca queda lista para ordeñar o cuando se completa su ordeñe
struct VacaOrdeniable {
    uint32_t id_vaca = 0;
    bool lista = false;

    bool operator==(const VacaOrdeniable&) const = default;
};

// Posición y estado de un NPC. "x" e "y" son la esquina superior izquierda del sprite en píxeles
// del mapa, igual que para los jugadores. "ms_por_casilla" es lo que tarda en recorrer una casilla
// mientras camina.
struct EstadoNpc {
    std::string id_npc;
    std::string mapa;
    uint32_t x = 0;
    uint32_t y = 0;
    Direccion mirando = Direccion::ABAJO;
    bool camina = false;
    uint16_t ms_por_casilla = 0;

    bool operator==(const EstadoNpc&) const = default;
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
    std::vector<EstadoTrampa> trampas;
    std::vector<EstadoAnimal> animales;
    std::vector<uint32_t> vacas_para_ordeniar;
    std::vector<EstadoNpc> npcs;
    // No viaja en ESTADO_JUEGO: llega con LlamasEnSector en cada tick que haya llamas volando.
    std::vector<EstadoLlama> llamas;
    bool operator==(const EstadoJuego&) const = default;
};

// Posición y estado de todos los jugadores del sector del receptor. Se manda cada tick, pero solo
// a los jugadores de ese sector, nunca viajan los jugadores de otros mapas.
struct JugadoresEnSector {
    std::string mapa;
    std::vector<EstadoJugador> jugadores;
    bool operator==(const JugadoresEnSector&) const = default;
};

// Posición y estado de todos los NPC del sector del receptor.
struct NpcsEnSector {
    std::string mapa;
    std::vector<EstadoNpc> npcs;
    bool operator==(const NpcsEnSector&) const = default;
};

// Dónde está un jugador conectado, para el mapa general. "x" e "y" son píxeles del mapa en el que
// está, como en EstadoJugador. "piso_de_mazmorra" es 0 si no está en la mazmorra.
struct PosicionDeJugador {
    uint32_t id_jugador = 0;
    std::string mapa;
    uint32_t x = 0;
    uint32_t y = 0;
    uint8_t piso_de_mazmorra = 0;

    bool operator==(const PosicionDeJugador&) const = default;
};

// Dónde está cada jugador conectado, esté en el mapa que esté. Se manda a todos unas pocas veces
// por segundo cuando alguien se movió o cambió de mapa, y al que entra apenas se conecta. Los
// nombres no viajan acá: el cliente los saca de JugadoresDeLaPartida.
struct PosicionesDeLosJugadores {
    std::vector<PosicionDeJugador> jugadores;

    bool operator==(const PosicionesDeLosJugadores&) const = default;
};

// Todas las llamas que están volando en el sector del receptor. Se manda en cada tick en que cambia
// algo (incluida la vez en que se apaga la última, con la lista vacía).
struct LlamasEnSector {
    std::string mapa;
    std::vector<EstadoLlama> llamas;
    bool operator==(const LlamasEnSector&) const = default;
};

// Un objeto nuevo en el sector del receptor. Si el id ya existía, el cliente lo reemplaza.
struct ObjetoColocado {
    uint32_t id = 0;
    uint32_t columna = 0;
    uint32_t fila = 0;
    TipoObjeto tipo = TipoObjeto::NINGUNO;
    bool operator==(const ObjetoColocado&) const = default;
};

// Un objeto que dejó de existir en el sector del receptor (se picó, se rompió, se retiró).
struct ObjetoRetirado {
    uint32_t id = 0;
    bool operator==(const ObjetoRetirado&) const = default;
};

// Un ítem que apareció en el piso del sector del receptor.
struct DropAparecido {
    EstadoDrop drop;
    bool operator==(const DropAparecido&) const = default;
};

// Un ítem que dejó de estar en el piso. id_jugador == 0 significa que lo perdió el mundo (por
// ejemplo, se limpió un nivel de la mazmorra), no que lo recogió alguien.
struct DropRetirado {
    uint32_t id = 0;
    uint32_t id_jugador = 0;
    bool operator==(const DropRetirado&) const = default;
};

// Un enemigo del sector del receptor que cambió: lo golpearon, murió (vida en 0) o revivió. Si el
// id no existía todavía del lado del cliente (recién apareció, por ejemplo al poblarse un nivel),
// lo agrega; si ya existía, lo reemplaza entero.
struct EnemigoActualizado {
    EstadoEnemigo enemigo;
    bool operator==(const EnemigoActualizado&) const = default;
};

// Una trampa del sector del receptor que se activó o se desactivó.
struct TrampaActualizada {
    EstadoTrampa trampa;
    bool operator==(const TrampaActualizada&) const = default;
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
    bool operator==(const CasillaConsultada&) const = default;
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
    // Si hoy llueve: los cultivos se riegan solos y el cliente dibuja la lluvia afuera.
    bool lloviendo = false;

    bool operator==(const HoraDelJuego&) const = default;
};

// Cambió el estado de una casilla de tierra del sector del receptor. "tratada" indica si quedó
// tratada con la pala (true) o si volvió a tierra común con el pico (false).
struct TierraCambiada {
    CasillaTratada casilla;
    bool tratada = false;
    bool operator==(const TierraCambiada&) const = default;
};

// La barra de un jugador. Las ranuras reales van de 0 a ranuras.size() - 1.
// El tacho no es una ranura del arreglo, el cliente lo dibuja aparte con el índice
// ranuras.size(), que el servidor interpreta como ranura para eliminar.
struct InventarioJugador {
    uint32_t id_jugador = 0;
    uint8_t ranura_activa = 0;
    std::vector<RanuraInventario> ranuras;
    bool operator==(const InventarioJugador&) const = default;
};

// Cofre que un jugador tiene abierto. abierto == false significa que se cerró.
// Si está abierto, ranuras trae todo el contenido del cofre.
struct CofreJugador {
    uint32_t id_cofre = 0;
    bool abierto = false;
    std::vector<RanuraInventario> ranuras;
    bool operator==(const CofreJugador&) const = default;
};

// Actualización de inventario dirigida a un cliente específico (se envía por su cola).
struct ActualizacionInventario {
    InventarioJugador inventario;
    bool operator==(const ActualizacionInventario&) const = default;
};

// Actualización del cofre abierto por un jugador. Si el cofre se cerró, abierto == false.
struct ActualizacionCofre {
    CofreJugador cofre;
    bool operator==(const ActualizacionCofre&) const = default;
};

// Un jugador de la partida, esté conectado o no, para elegir a quién regalarle.
struct JugadorDeLaPartida {
    uint32_t id_jugador = 0;
    std::string nombre;
    bool conectado = false;

    bool operator==(const JugadorDeLaPartida&) const = default;
};

// Todos los jugadores de la partida. Se manda a todos cuando alguien entra o se va.
struct JugadoresDeLaPartida {
    std::vector<JugadorDeLaPartida> jugadores;
    bool operator==(const JugadoresDeLaPartida&) const = default;
};

// Un regalo que todavía no se abrió: quién lo mandó y qué es.
struct RegaloRecibido {
    std::string remitente;
    RanuraInventario item;

    bool operator==(const RegaloRecibido&) const = default;
};

// Los regalos pendientes del jugador que lo recibe. Se manda al entrar y cada vez que cambian.
struct RegalosPendientes {
    std::vector<RegaloRecibido> regalos;
    bool operator==(const RegalosPendientes&) const = default;
};

// Canal de un mensaje que el servidor manda al cliente. "PRIVADO" avisa que el mensaje era para un
// único destinatario y "SISTEMA" son respuestas del propio servidor (errores de envío, comandos
// desconocidos y cheats). El mercado no usa el chat.
enum class CanalDeChat : uint8_t {
    GLOBAL = 0,
    PRIVADO = 1,
    SISTEMA = 2,
};

#define REMITENTE_DEL_SISTEMA "(Sistema)"

// Un mensaje ya interpretado por el servidor. "remitente" es el nombre del jugador que lo envió
// (o "(Sistema)" en los avisos), "destinatario" solo tiene valor en los privados. "texto" es el
// contenido sin el prefijo "@nombre". "color_del_remitente" lo elige el servidor al entrar a la
// partida y se conserva durante toda la partida.
struct MensajeDeChat {
    CanalDeChat canal = CanalDeChat::GLOBAL;
    std::string remitente;
    std::string destinatario;
    std::string texto;
    ColorDeJugador color_del_remitente = ColorDeJugador::AZUL;

    static MensajeDeChat del_sistema(const std::string& texto) {
        return MensajeDeChat{CanalDeChat::SISTEMA, REMITENTE_DEL_SISTEMA, std::string(), texto};
    }
    bool operator==(const MensajeDeChat&) const = default;
};

// Información detallada de cada ítem/animal en el catálogo del puesto de mercado.
struct ItemCatalogoInfo {
    TipoItem tipo_item = TipoItem::NINGUNO;        // Si es un ítem normal
    TipoAnimal tipo_animal = TipoAnimal::NINGUNO;  // Si es un animal (Marnie)
    std::string nombre;
    uint32_t precio = 0;
    bool se_puede_comprar = true;   // False si no alcanza el dinero o no hay lugar en el inventario
    uint16_t cantidad_poseida = 0;  // Solo para animales, cuántos tiene el jugador
    std::string descripcion;        // Información de cuidados o descripción para el hover

    bool es_animal() const {
        return tipo_animal != TipoAnimal::NINGUNO;
    }
    bool operator==(const ItemCatalogoInfo&) const = default;
};

// Envía la ventana de mercado completa con el catálogo configurado del NPC correspondiente.
// "id_npc" es lógico, el cliente decide con qué asset mostrarlo.
struct ActualizacionMercado {
    std::string id_npc;
    std::vector<ItemCatalogoInfo> catalogo;
    bool operator==(const ActualizacionMercado&) const = default;
};

// Ordena al cliente cerrar la ventana de mercado.
struct CerrarVentanaMercado {
    bool operator==(const CerrarVentanaMercado&) const = default;
};

// Cartel genérico para avisar algo al jugador (por ejemplo, que el mercado está cerrado). El
// cliente lo muestra como un cuadro modal que se cierra con un clic y bloquea el juego mientras
// esté visible.
struct AvisoEnPantalla {
    std::string mensaje;
    bool operator==(const AvisoEnPantalla&) const = default;
};

// El jugador hace clic derecho sobre un NPC para hablarle. El servidor decide el texto
// y responde con DialogoNpc.
struct HablarConNpc {
    std::string id_npc;
    bool operator==(const HablarConNpc&) const = default;
};

// Lo que dice un NPC. El cliente lo muestra en un cuadro modal con el fondo dialogo_<id>.png.
struct DialogoNpc {
    std::string id_npc;
    std::string texto;
    bool operator==(const DialogoNpc&) const = default;
};

// Una parte de un lote del centro comunitario y cuánto se lleva aportado. Según "tipo", lo que
// importa es "item" y "calidad_minima" (donar un ítem), "logro" (pelea) o solo "cantidad"
// (monedas).
struct RequisitoDeLote {
    TipoDeRequisito tipo = TipoDeRequisito::ITEM;
    TipoItem item = TipoItem::NINGUNO;
    Calidad calidad_minima = Calidad::BASE;
    LogroDePelea logro = LogroDePelea::ESQUELETOS;
    uint32_t cantidad = 0;
    uint32_t aportado = 0;

    bool operator==(const RequisitoDeLote&) const = default;
};

// El estado de un lote del centro comunitario, con todo lo que hace falta para dibujarlo. Se le
// manda al que hace clic en el lote con "abrir" en true (abre la ventana), y a todos con "abrir" en
// false cada vez que el lote cambia (actualiza la ventana de quien lo tenga abierto).
struct LoteDelCentro {
    TipoLote lote = TipoLote::CULTIVOS;
    bool abrir = false;
    std::string nombre;
    bool completo = false;
    std::vector<RequisitoDeLote> requisitos;
    std::string mapa;
    uint32_t columna = 0;
    uint32_t fila = 0;

    bool operator==(const LoteDelCentro&) const = default;
};

// Se completaron los cinco lotes del centro comunitario. El cliente felicita al jugador con un
// cartel y lo deja seguir jugando o volver al lobby. Cada jugador que estaba en la partida lo
// recibe una vez.
struct CentroTerminado {
    bool operator==(const CentroTerminado&) const = default;
};

// El jugador hizo clic en una cama libre que tiene al alcance: el cliente le pregunta si quiere
// dormir. La casilla es la que clickeó, y vuelve en Dormir si dice que sí.
struct PreguntaDeDormir {
    uint32_t columna = 0;
    uint32_t fila = 0;
    bool operator==(const PreguntaDeDormir&) const = default;
};

#endif
