#ifndef RESPUESTAS_DE_CHAT_H
#define RESPUESTAS_DE_CHAT_H

#include <cstdint>
#include <string>

// Forma de contestarle a un jugador desde un comando de chat. ChatDePartida la implementa y se la
// pasa a cada ComandoDeChat, así un comando del mercado o un cheat no necesita conocer el
// protocolo ni la red para responder. Además evita un #include circular.
class RespuestasDeChat {
public:
    // Mensaje del sistema dirigido solo al jugador que ejecutó el comando.
    virtual void responder_a_jugador(uint32_t id_jugador, const std::string& texto) = 0;

    // Respuesta de un NPC del mercado, para que el cliente la muestre como una conversación y no
    // como un mensaje entre jugadores (wip).
    virtual void responder_como_comerciante(
        uint32_t id_jugador, const std::string& nombre_comerciante, const std::string& texto) = 0;

    virtual ~RespuestasDeChat() = default;
};

#endif
