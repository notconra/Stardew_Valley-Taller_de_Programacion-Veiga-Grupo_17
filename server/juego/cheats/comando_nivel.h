#ifndef COMANDO_NIVEL_H
#define COMANDO_NIVEL_H

#include <string>

#include "../chat/comando_de_chat.h"

class Mundo;

class ComandoNivel : public ComandoDeChat {
private:
    Mundo& mundo;

    bool parsear_numero_de_nivel(const std::string& texto, int& resultado) const;

public:
    explicit ComandoNivel(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
