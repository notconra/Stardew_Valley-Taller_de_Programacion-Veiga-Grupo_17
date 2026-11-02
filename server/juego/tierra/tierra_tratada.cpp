#include "tierra_tratada.h"

#include <string>
#include <utility>

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

void TierraTratada::agregar_a(EstadoTierra& estado_tierra, const std::string& id_mapa) const {
    if (tratadas.empty()) {
        return;
    }
    TierraDeUnMapa tierra;
    tierra.mapa = id_mapa;
    tierra.casillas.reserve(tratadas.size());
    for (const auto& [columna, fila] : tratadas) {
        tierra.casillas.push_back(
            CasillaTratada{static_cast<uint16_t>(columna), static_cast<uint16_t>(fila)});
    }
    estado_tierra.mapas.push_back(std::move(tierra));
}
