#ifndef COMANDO_ENERGIA_H
#define COMANDO_ENERGIA_H

#include <string>

#include "../chat/comando_de_chat.h"

class Mundo;

class ComandoEnergia : public ComandoDeChat {
private:
    static constexpr int MAXIMO_ENERGIA = 65535;

    Mundo& mundo;

    bool parsear_cantidad_no_negativa(const std::string& texto, int& resultado) const;

public:
    explicit ComandoEnergia(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
