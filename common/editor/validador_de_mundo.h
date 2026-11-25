#ifndef VALIDADOR_DE_MUNDO_H
#define VALIDADOR_DE_MUNDO_H

#include <string>
#include <vector>

#include "definicion_de_mundo.h"

struct ErrorValidacion {
    std::string mensaje;
    int indice_sector;
};

struct ResultadoValidacion {
    bool es_valido;
    std::vector<ErrorValidacion> errores;

    ResultadoValidacion();
    void agregar_error(const std::string& error, int indice_sector = -1);
};

class ValidadorDeMundo {
private:
    static constexpr int COLUMNA_PUERTA_CABANIA = 5;
    static constexpr int FILA_PUERTA_CABANIA = 7;
    static constexpr int COLUMNA_PUERTA_CASETA_MINA = 1;
    static constexpr int FILA_PUERTA_CASETA_MINA = 3;
    static constexpr int COLUMNA_PUERTA_CENTRO_COMUNITARIO = 5;
    static constexpr int FILA_PUERTA_CENTRO_COMUNITARIO = 9;
    static constexpr int COLUMNA_PUERTA_MERCADO = 5;
    static constexpr int FILA_PUERTA_MERCADO = 9;

    static bool tiene_acceso_perimetral_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& entidad);
    static bool casilla_puerta_cabania_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& cabania);
    static bool casilla_puerta_entrada_caverna_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& entrada_caverna);
    static bool casilla_puerta_centro_comunitario_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& centro_comunitario);
    static bool casilla_puerta_mercado_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& mercado);
    static bool calcular_casilla_llegada_borde(
        const MapaEditable& mapa, const EntidadDelMapa& transicion, int& columna_llegada,
        int& fila_llegada);
    static bool existe_camino_transitable(
        const MapaEditable& mapa, const EntidadDelMapa& origen, const EntidadDelMapa& destino);
    static bool existe_camino_desde_coordenada(
        const MapaEditable& mapa, int columna_inicio, int fila_inicio,
        const EntidadDelMapa& destino);
    static bool tiene_entrada_de_nivel_libre(
        const MapaEditable& mapa, const EntidadDelMapa& salida, const EntidadDelMapa& escalera);

public:
    ValidadorDeMundo() = delete;

    static ResultadoValidacion validar(const DefinicionDeMundo& mundo);
};

#endif  // VALIDADOR_DE_MUNDO_H
