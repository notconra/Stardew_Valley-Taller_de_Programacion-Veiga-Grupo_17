#ifndef SESION_DE_PARTIDA_H
#define SESION_DE_PARTIDA_H

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../configuracion/configuracion_servidor_datos.h"
#include "../juego/base/recursos/recursos_de_juego.h"
#include "../juego/mundo.h"
#include "error_de_partida.h"

// Bucle de simulación de una partida. Corre en su propio hilo. Coordina
// comandos de red con el Mundo. La sesión se crea al conectar el primer
// jugador y se destruye al irse el último.
struct ComandoDeJugador {
    uint32_t id_jugador = 0;
    ComandoCliente comando;
};

struct ColaDeEventosDelJugador {
    Queue<EventoServidor>& cola_eventos;
};

class SesionDePartida : public Thread {
private:
    const std::string codigo_partida;
    const std::chrono::milliseconds periodo_de_tick;

    Mundo mundo;

    Queue<ComandoDeJugador> comandos;

    std::mutex mutex_clientes;
    std::map<uint32_t, ColaDeEventosDelJugador> clientes;

    // Jugadores que entraron y todavía no recibieron su inventario inicial.
    std::vector<uint32_t> inventarios_pendientes;

    void sincronizar_jugadores();
    void enviar_inventarios_pendientes();
    void procesar_comandos();
    void difundir(const EventoServidor& evento);
    void difundir_a_jugador(uint32_t id_jugador, const EventoServidor& evento);
    void difundir_cierre();

public:
    SesionDePartida(
        const std::string& codigo_de_partida,
        const ConfiguracionPartidaServidor& configuracion_partida, const RecursosDeJuego& recursos);

    bool esta_viva() const;

    void entrar(uint32_t id_jugador, Queue<EventoServidor>& cola_eventos);
    void salir(uint32_t id_jugador);
    void recibir(uint32_t id_jugador, const ComandoCliente& comando);

    void run() override;
    void detener();

    SesionDePartida(const SesionDePartida&) = delete;
    SesionDePartida& operator=(const SesionDePartida&) = delete;

    ~SesionDePartida();
};

#endif
