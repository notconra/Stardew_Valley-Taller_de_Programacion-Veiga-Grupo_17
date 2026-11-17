#ifndef VALIDADOR_DE_MUNDO_H
#define VALIDADOR_DE_MUNDO_H

#include <string>
#include <vector>

#include "definicion_de_mundo.h"

struct ResultadoValidacion {
    bool es_valido;
    std::vector<std::string> errores;
    std::vector<std::string> advertencias;

    ResultadoValidacion();
    void agregar_error(const std::string& error);
    void agregar_advertencia(const std::string& advertencia);
};

class ValidadorDeMundo {
public:
    ValidadorDeMundo() = delete;

    static ResultadoValidacion validar(const DefinicionDeMundo& mundo);
};

#endif  // VALIDADOR_DE_MUNDO_H
