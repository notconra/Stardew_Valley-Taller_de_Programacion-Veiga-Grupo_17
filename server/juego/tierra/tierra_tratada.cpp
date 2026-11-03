#include "tierra_tratada.h"

#include <utility>
#include <vector>

TierraTratada::TierraTratada(const Mapa& mapa_de_la_tierra) : mapa(mapa_de_la_tierra) {
}

bool TierraTratada::se_puede_tratar(const Casilla& casilla) const {
    return mapa.tiene(casilla.columna, casilla.fila, PropiedadDeCasilla::CULTIVABLE) &&
           !mapa.tiene(casilla.columna, casilla.fila, PropiedadDeCasilla::BLOQUEA) &&
           !esta_tratada(casilla);
}

bool TierraTratada::tratar(const Casilla& casilla) {
    if (!se_puede_tratar(casilla)) {
        return false;
    }
    tratadas.insert({casilla.columna, casilla.fila});
    return true;
}

bool TierraTratada::volver_a_comun(const Casilla& casilla) {
    return tratadas.erase({casilla.columna, casilla.fila}) > 0;
}

bool TierraTratada::esta_tratada(const Casilla& casilla) const {
    return tratadas.contains({casilla.columna, casilla.fila});
}

void TierraTratada::agregar_a(std::vector<CasillaTratada>& casillas) const {
    casillas.reserve(casillas.size() + tratadas.size());
    for (const auto& [columna, fila] : tratadas) {
        casillas.push_back(
            CasillaTratada{static_cast<uint32_t>(columna), static_cast<uint32_t>(fila)});
    }
}
