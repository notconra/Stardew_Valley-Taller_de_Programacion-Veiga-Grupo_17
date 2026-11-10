#ifndef SESION_DE_PARTIDA_H
#define SESION_DE_PARTIDA_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "../../common/apariencia_personaje.h"
#include "../../common/mensajes/mensajes.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../configuracion/configuracion_servidor_datos.h"
#include "../juego/chat/chat_de_partida.h"
#include "../juego/mundo.h"
#include "../juego/notificador.h"
#include "../juego/recursos/recursos_de_juego.h"
#include "../persistencia/datos_partida_guardada.h"
#include "error_de_partida.h"
#include "jugador.h"

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
    AparienciaPersonaje apariencia;
};

// Qué hacer cuando termina un día (todos dieron el OK): guardar la partida. Lo provee
// AdministradorDePartidas, que es quien sabe guardarla.
using GuardarAlTerminarElDia = std::function<void()>;

// Implementa Notificador para proveer el servicio de envío sin acoplar el juego a la red.
// El Mundo manda los eventos y esta clase solo los deposita en la cola del cliente.
//
// Cuando termina un día, la partida se guarda sola.
class SesionDePartida : public Thread, public Notificador {
private:
    const std::string codigo_partida;
    const std::chrono::milliseconds periodo_de_tick;

    mutable std::mutex mutex_mundo;
    Mundo mundo;
    ChatDePartida chat;

    Queue<ComandoDeJugador> comandos;
    std::mutex mutex_clientes;
    std::map<uint32_t, ColaDeEventosDelJugador> clientes;
    std::vector<CambioDeConexion> cambios_de_conexion;
    GuardarAlTerminarElDia guardar_al_terminar_el_dia;
    uint32_t segundos_jugados_iniciales = 0;
    std::chrono::steady_clock::time_point tiempo_inicio_sesion;

    // Le pasa a Mundo las altas y bajas anotadas desde el tick anterior.
    void aplicar_cambios_de_conexion();
    void procesar_comandos();
    void difundir_cierre();

    // Si en el último tick terminó el día, guarda la partida. Un error al guardar se informa
    // pero no corta el bucle de juego.
    void guardar_si_termino_el_dia(bool termino);

    std::vector<DatosJugadorGuardado> datos_de_los_jugadores(
        const std::vector<Jugador>& miembros,
        const std::map<uint32_t, DatosJugadorGuardado>& jugadores_guardados_previos);

public:
    SesionDePartida(
        const std::string& codigo_partida,
        const ConfiguracionPartidaServidor& configuracion_partida, const RecursosDeJuego& recursos,
        const DatosMundoGuardado& datos_guardados = {},
        GuardarAlTerminarElDia guardar_al_terminar_el_dia = [] {});

    bool esta_viva() const;
    std::vector<std::string> obtener_niveles_mazmorra() const;
    uint32_t obtener_dia() const;
    uint32_t obtener_segundos_jugados() const;

    void entrar(
        uint32_t id_jugador, const std::string& nombre_jugador, ColorDeJugador color_jugador,
        Queue<EventoServidor>& cola_eventos, const AparienciaPersonaje& apariencia);

    void salir(uint32_t id_jugador);
    void recibir(uint32_t id_jugador, const ComandoCliente& comando);

    // Implementación de la interfaz Notificador
    void a_jugador(uint32_t id_jugador, const EventoServidor& evento) override;
    void a_todos(const EventoServidor& evento) override;

    void run() override;
    void detener();

    DatosMundoGuardado obtener_datos_guardado(
        const std::vector<Jugador>& miembros,
        const std::map<uint32_t, DatosJugadorGuardado>& jugadores_guardados_previos);

    SesionDePartida(const SesionDePartida&) = delete;
    SesionDePartida& operator=(const SesionDePartida&) = delete;

    ~SesionDePartida();
};

#endif
