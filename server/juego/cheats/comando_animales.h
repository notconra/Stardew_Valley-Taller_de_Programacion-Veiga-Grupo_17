#ifndef COMANDO_ANIMALES_H
#define COMANDO_ANIMALES_H

#include <string>

#include "../chat/comando_de_chat.h"

class Mundo;

class ComandoAnimales : public ComandoDeChat {
private:
    Mundo& mundo;

public:
    explicit ComandoAnimales(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
