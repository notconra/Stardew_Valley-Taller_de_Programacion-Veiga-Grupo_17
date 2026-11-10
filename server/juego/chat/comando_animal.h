#ifndef COMANDO_ANIMAL_H
#define COMANDO_ANIMAL_H

#include <string>

#include "comando_de_chat.h"

class Mundo;

class ComandoAnimal : public ComandoDeChat {
private:
    Mundo& mundo;

    void ejecutar_animal(const ContextoDeComandoDeChat& contexto);
    void ejecutar_avanzar_dia(const ContextoDeComandoDeChat& contexto);

public:
    explicit ComandoAnimal(Mundo& mundo_de_juego);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
