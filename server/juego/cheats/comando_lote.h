#ifndef COMANDO_LOTE_H
#define COMANDO_LOTE_H

#include <string>

#include "../chat/comando_de_chat.h"

class Mundo;

// "/lote <clave>" completa ese lote del centro comunitario (cultivos, animales, pelea, dinero o
// metales) y "/lote todos" completa los que falten. Los premios se reparten como si se hubieran
// completado jugando.
class ComandoLote : public ComandoDeChat {
private:
    Mundo& mundo;

public:
    explicit ComandoLote(Mundo& mundo_de_juego);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
