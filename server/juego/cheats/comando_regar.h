#ifndef COMANDO_REGAR_H
#define COMANDO_REGAR_H

#include <string>

#include "../chat/comando_de_chat.h"

class Mundo;

class ComandoRegar : public ComandoDeChat {
private:
    Mundo& mundo;

public:
    explicit ComandoRegar(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
