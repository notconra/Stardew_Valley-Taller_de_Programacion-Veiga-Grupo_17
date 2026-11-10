#ifndef COMANDO_DINERO_H
#define COMANDO_DINERO_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "comando_de_chat.h"

class Mundo;

class ComandoDinero : public ComandoDeChat {
private:
    static constexpr uint32_t MAXIMO_DINERO = 99'999'999;
    static constexpr std::size_t CANTIDAD_ARGUMENTOS_CON_VALOR = 2;
    static constexpr std::size_t INDICE_CANTIDAD = 1;

    Mundo& mundo;

    bool parsear_cantidad_positiva(const std::string& texto, uint32_t& resultado) const;
    void mostrar_saldo(const ContextoDeComandoDeChat& contexto) const;
    void sumar_dinero(const ContextoDeComandoDeChat& contexto) const;
    void gastar_dinero(const ContextoDeComandoDeChat& contexto) const;
    void mostrar_ayuda(const ContextoDeComandoDeChat& contexto) const;

public:
    explicit ComandoDinero(Mundo& mundo);

    bool reconoce(const std::string& palabra_clave) const override;
    void ejecutar(const ContextoDeComandoDeChat& contexto) override;
};

#endif
