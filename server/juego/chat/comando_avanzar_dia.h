#ifndef COMANDO_AVANZAR_DIA_H
#define COMANDO_AVANZAR_DIA_H

#include <string>

#include "comando_de_chat.h"

class Mundo;

class ComandoAvanzarDia : public ComandoDeChat {
private:
    Mundo& mundo;

public:
    explicit ComandoAvanzarDia(Mundo& mundo_partida);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
