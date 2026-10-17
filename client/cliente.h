#ifndef CLIENTE_H
#define CLIENTE_H

#include <string>

#include "../common/mensajes/mensajes.h"
#include "red/conexion_con_servidor.h"
// #include "vista_consola/consola.h"
#include "estado_del_juego.h"

// notas (no documentación final)
// Orquestador principal del cliente. Tiene la conexión con el servidor y las vistas (ahora es una,
// serán dos, la de sdl y qt), y decide en qué fase está el programa:
//
//   LOBBY --el servidor confirma la entrada a una partida--> PARTIDA
//   PARTIDA --el jugador sale de la partida-->   LOBBY
//   cualquier fase --el usuario cierra-->   SALIR
//
// después igual capaz queda mejor con más fases para no acoplar todo y ahi conviene meter un map
// (para no hacer un state) y resolver ahi todos los estados posibles (q la tiro por si sirve al
// momento de hacer la interfaz, tipo q pase a fase creacion de personaje o algo así entre lobby y
// partida por ejemplo (?) habría q identificarlas si es q sirve)
//
// pueden ignorar esto pq es como funciona la vista esta q igual va a volar :p
//
// Cada fase la ejecuta una vista, su ejecutar() bloquea hasta que la fase termina y devuelve por
// qué terminó.
//
// Si el servidor se cae, la conexión lanza ErrorConexionCerrada desde la vista que esté
// ejecutándose y la excepción llega hasta main, que informa el error.
//
// ahora las dos fases las resuelve Consola, cuando estén las vistas gráficas:
//   - vista_qt/vista_lobby: ventanas del lobby. ejecutar() las muestra y vuelve con la
//     partida a la que se entró, o con null o algo asi si el usuario cierra. Los botones llaman a
//     conexion.enviar_comando() y alguna cosa llamaria a conexion.intentar_evento() nose ni idea
//     chau loco suerte.
//
//   - vista_sdl/vista_partida: ejecutar(partida) es como el game loop de sdl, en cada frame
//     conexion.intentar_evento(), lee el input, envía comandos y dibuja. devolvwria true si
//     llega PartidaAbandonada (volver al lobby) o con false si el usuario cierra la ventana ponele.
//
//   - Cliente solo cambia los miembros (VistaLobby y VistaPartida (por ejemplo) en lugar de
//   Consola) y el cuerpo
//     de ejecutar_lobby() y ejecutar_partida(), que llaman a esas vistas. El loop de ejecutar()
//     creo queno cambiaria.
//
//   - Cada vista inicializa su librería al empezar su ejecutar() y la libera al terminar, para que
//     qt y sdl no queden abiertas a la vez. pero esto es mas del tutorial y tanto no me acuerdo
//     beso
//
//   - Nada q ver pero Los datos de ConfiguracionCliente que necesiten se los pasa Cliente por
//   constructor (?).
class Cliente {
private:
    ConexionConServidor conexion;
    // Consola consola;

    // Partida en la que está el jugador. Solo es válida durante el EstadoDelJuego PARTIDA.
    EntradaExitosaAPartida partida_actual{};

    // Cada una ejecuta su EstadoDelJuego hasta que termina y devuelve la siguiente.
    EstadoDelJuego ejecutar_lobby();
    EstadoDelJuego ejecutar_partida();

public:
    // Se conecta al servidor. Lanza LibError o ResolverError si no puede.
    Cliente(const std::string& host, const std::string& puerto);

    // Bloquea hasta que el usuario cierra el cliente. Lanza ErrorConexionCerrada si el servidor
    // se cae.
    void ejecutar();

    Cliente(const Cliente&) = delete;
    Cliente& operator=(const Cliente&) = delete;
};

#endif
