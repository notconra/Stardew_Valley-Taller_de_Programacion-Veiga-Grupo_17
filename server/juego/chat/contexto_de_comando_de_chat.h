#ifndef CONTEXTO_DE_COMANDO_DE_CHAT_H
#define CONTEXTO_DE_COMANDO_DE_CHAT_H

#include <cstdint>
#include <string>
#include <vector>

#include "respuestas_de_chat.h"

// Todo lo que necesita un comando de chat para ejecutarse.
// "palabra_clave" ya viene en minúsculas y "argumentos" separados por espacios,
// conservando mayúsculas y acentos. La idea es usar esto para que nunca cambie la firma de
// ejecutar().
struct ContextoDeComandoDeChat {
    uint32_t id_jugador = 0;
    std::string nombre_jugador;
    std::string palabra_clave;
    std::vector<std::string> argumentos;

    // Nombre del comerciante con el que el jugador tiene una conversación abierta, o una cadena
    // vacía si no hay ninguna. El mercado lo podría usar para saber sobre qué catálogo operar, los
    // cheats lo ignoran. wip.
    std::string interlocutor;
    RespuestasDeChat& respuestas;
};

#endif
