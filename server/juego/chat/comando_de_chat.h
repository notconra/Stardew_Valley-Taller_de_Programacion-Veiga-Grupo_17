#ifndef COMANDO_DE_CHAT_H
#define COMANDO_DE_CHAT_H

#include <string>

#include "contexto_de_comando_de_chat.h"

// Un comando con barra del chat ("/comprar", "/oro", etc.). Cada feature (mercado, cheats, ayuda)
// implementaría esta interfaz y la registraría en el ChatDePartida.
// Dos comandos no deberían reconocer la misma palabra, gana el primero que se registró.
class ComandoDeChat {
public:
    // Recibe la palabra clave en minúsculas, sin la barra y sin argumentos.
    virtual bool reconoce(const std::string& palabra_clave) const = 0;

    // Recibe el contexto ya separado y responde con contexto.respuestas.
    virtual void ejecutar(const ContextoDeComandoDeChat& contexto) = 0;

    virtual ~ComandoDeChat() = default;
};

#endif
