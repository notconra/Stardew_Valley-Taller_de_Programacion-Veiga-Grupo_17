#ifndef CHAT_DE_PARTIDA_H
#define CHAT_DE_PARTIDA_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes_partida.h"
#include "../notificador.h"
#include "comando_de_chat.h"
#include "contexto_de_comando_de_chat.h"
#include "respuestas_de_chat.h"

// Lo que el chat sabe de cada jugador conectado.
struct JugadorEnChat {
    std::string nombre;
    ColorDeJugador color;
    std::string interlocutor;  // vacío si no está hablando con ningún NPC
};

// Los jugadores conectados al chat, por id.
using JugadoresEnChat = std::map<uint32_t, JugadorEnChat>;

// Chat de una partida. Vive dentro de la sesión y corre en su mismo hilo.
// No guarda historial ni persiste nada, un jugador solo recibe lo que se envía mientras está
// conectado.
class ChatDePartida : public RespuestasDeChat {
private:
    Notificador& notificador;
    JugadoresEnChat jugadores;
    std::vector<std::unique_ptr<ComandoDeChat>> comandos;

    void enviar_global(const JugadorEnChat& emisor, const std::string& texto);
    void enviar_privado(
        uint32_t id_emisor, const JugadorEnChat& emisor, const std::string& destinatario,
        const std::string& texto);
    void enviar_sistema(uint32_t id_jugador, const std::string& texto);
    void ejecutar_comando(
        uint32_t id_jugador, const JugadorEnChat& emisor, const std::string& texto);

    // Mensaje del sistema para todos los jugadores conectados menos "id_excluido".
    void avisar_a_los_demas(uint32_t id_excluido, const std::string& texto);

    // Busca un jugador conectado por nombre, sin distinguir mayúsculas ni minúsculas. Devuelve
    // jugadores.end() si no hay ninguno.
    JugadoresEnChat::const_iterator buscar_por_nombre(const std::string& nombre_buscado) const;

    static std::string sin_espacios_extremos(const std::string& texto);
    static std::string a_minusculas(const std::string& texto);
    static std::vector<std::string> separar_argumentos(const std::string& texto);

public:
    explicit ChatDePartida(Notificador& notificador);

    // Registra o actualiza el nombre y el color visible del jugador. Se llama al entrar a la
    // partida. Si no se indica color, se usa el azul por defecto.
    void conectar(
        uint32_t id_jugador, const std::string& nombre_jugador,
        ColorDeJugador color = ColorDeJugador::AZUL);

    // Deja de entregarle mensajes. Se llama al salir de la partida.
    void desconectar(uint32_t id_jugador);

    // Procesa el texto que escribió un jugador, que puede ser global, privado o comando. Si el
    // jugador no está conectado al chat, lo ignora.
    void recibir(uint32_t id_jugador, const EnviarMensajeDeChat& mensaje);

    // Conversación con un NPC. El mercado la abriría al hacer clic en un comerciante y la cerraría
    // al terminar la interacción. Los comandos la recibirían en
    // ContextoDeComandoDeChat::interlocutor. Si el jugador no está conectado, no hacen nada.
    void iniciar_conversacion(uint32_t id_jugador, const std::string& nombre_interlocutor);
    void terminar_conversacion(uint32_t id_jugador);
    std::string interlocutor_de(uint32_t id_jugador) const;

    // Suma un comando. Se llama durante la construcción de la sesión, antes de que arranque el
    // hilo. Si lo que escribe el jugador no coincide con ningún comando, recibe un aviso.
    void agregar_comando(std::unique_ptr<ComandoDeChat> comando);

    // Implementación de RespuestasDeChat para los comandos.
    void responder_a_jugador(uint32_t id_jugador, const std::string& texto) override;
    void responder_como_comerciante(
        uint32_t id_jugador, const std::string& nombre_comerciante,
        const std::string& texto) override;
};

#endif
