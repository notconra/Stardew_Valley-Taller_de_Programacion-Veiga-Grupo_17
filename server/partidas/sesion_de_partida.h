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
#include "../juego/chat/chat_de_partida.h"
#include "../juego/mundo.h"
#include "../juego/notificador.h"
#include "../juego/recursos/recursos_de_juego.h"
#include "error_de_partida.h"

struct ComandoDeJugador {
    uint32_t id_jugador = 0;
    ComandoCliente comando;
};

struct ColaDeEventosDelJugador {
    Queue<EventoServidor>& cola_eventos;
};

// Un jugador que entró o se fue.
struct CambioDeConexion {
    uint32_t id_jugador = 0;
    std::string nombre_jugador;
    ColorDeJugador color_jugador = ColorDeJugador::AZUL;
    bool entra = false;
};

// Implementa Notificador para proveer el servicio de envío sin acoplar el juego a la red.
// El Mundo manda los eventos y esta clase solo los deposita en la cola del cliente.
class SesionDePartida : public Thread, public Notificador {
private:
    const std::string codigo_partida;
    const std::chrono::milliseconds periodo_de_tick;

    Mundo mundo;
    ChatDePartida chat;

    Queue<ComandoDeJugador> comandos;
    std::mutex mutex_clientes;
    std::map<uint32_t, ColaDeEventosDelJugador> clientes;
    std::vector<CambioDeConexion> cambios_de_conexion;

    // Le pasa a Mundo las altas y bajas anotadas desde el tick anterior.
    void aplicar_cambios_de_conexion();
    void procesar_comandos();
    void difundir_cierre();

public:
    SesionDePartida(
        const std::string& codigo_partida,
        const ConfiguracionPartidaServidor& configuracion_partida, const RecursosDeJuego& recursos);

    bool esta_viva() const;

    void entrar(
        uint32_t id_jugador, const std::string& nombre_jugador, ColorDeJugador color_jugador,
        Queue<EventoServidor>& cola_eventos);
    void salir(uint32_t id_jugador);
    void recibir(uint32_t id_jugador, const ComandoCliente& comando);

    // Implementación de la interfaz Notificador
    void a_jugador(uint32_t id_jugador, const EventoServidor& evento) override;
    void a_todos(const EventoServidor& evento) override;

    void run() override;
    void detener();

    SesionDePartida(const SesionDePartida&) = delete;
    SesionDePartida& operator=(const SesionDePartida&) = delete;

    ~SesionDePartida();
};

#endif
