#ifndef COMANDO_SEMILLAS_H
#define COMANDO_SEMILLAS_H

#include <string>

#include "../chat/comando_de_chat.h"
#include "../cultivos/cultivos.h"
#include "../mundo.h"

// Cheat "/semillas [cantidad]": da semillas de cada cultivo al que lo escribe (10 de cada una si no
// dice cuántas). Sirve para probar los cultivos hasta que Pierre las venda.
class ComandoSemillas : public ComandoDeChat {
private:
    Mundo& mundo;
    const CatalogoDeCultivos& cultivos;

    // La cantidad pedida, la de por defecto si no se dio ninguna, o -1 si el argumento no es
    // válido.
    int cantidad_pedida(const ContextoDeComandoDeChat& contexto) const;

public:
    ComandoSemillas(Mundo& mundo, const CatalogoDeCultivos& cultivos);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
