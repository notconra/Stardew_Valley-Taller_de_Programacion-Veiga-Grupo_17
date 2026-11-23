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
private:
    static constexpr int COLUMNA_PUERTA_CABANIA = 5;
    static constexpr int FILA_PUERTA_CABANIA = 7;
    static constexpr int COLUMNA_PUERTA_CASETA_MINA = 2;
    static constexpr int FILA_PUERTA_CASETA_MINA = 6;
    static constexpr int COLUMNA_PUERTA_CENTRO_COMUNITARIO = 5;
    static constexpr int FILA_PUERTA_CENTRO_COMUNITARIO = 9;

    static bool tiene_acceso_perimetral_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& entidad);
    static bool casilla_puerta_cabania_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& cabania);
    static bool casilla_puerta_entrada_caverna_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& entrada_caverna);
    static bool casilla_puerta_centro_comunitario_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& centro_comunitario);
    static bool existe_camino_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& origen, const EntidadDelMapa& destino);

public:
    ValidadorDeMundo() = delete;

    static ResultadoValidacion validar(const DefinicionDeMundo& mundo);
};

#endif  // VALIDADOR_DE_MUNDO_H
