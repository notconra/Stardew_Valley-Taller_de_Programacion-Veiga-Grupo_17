#ifndef COMANDO_LLUVIA_H
#define COMANDO_LLUVIA_H

#include <string>

#include "../chat/comando_de_chat.h"

class Mundo;

// Cheat: "/lluvia" prende o apaga la lluvia; "/lluvia si" y "/lluvia no" la fuerzan.
class ComandoLluvia : public ComandoDeChat {
private:
    Mundo& mundo;

public:
    explicit ComandoLluvia(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
