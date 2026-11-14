#ifndef COMANDO_ESTACION_H
#define COMANDO_ESTACION_H

#include <string>

#include "comando_de_chat.h"

class Mundo;

// Cheat: "/estacion <primavera|verano|otono|invierno>" pasa la partida al día 1 de esa estación.
class ComandoEstacion : public ComandoDeChat {
private:
    Mundo& mundo;

public:
    explicit ComandoEstacion(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
