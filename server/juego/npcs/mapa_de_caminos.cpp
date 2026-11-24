#include "mapa_de_caminos.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <string>
#include <vector>

constexpr int RADIO_DE_BUSQUEDA_DE_PUERTA = 4;
constexpr int PENALIZACION_FUERA_DE_LA_ZONA = 100;
constexpr int LIMITE_DE_TAMANIO_DE_MAPA = 4096;

MapaDeCaminos::MapaDeCaminos(GestorDeSectores& sectores) {
    medir_mapas(sectores);
    unir_mapas_por_las_puertas(sectores);
}

void MapaDeCaminos::medir_mapas(GestorDeSectores& sectores) {
    int total = 0;
    for (const std::string& id : sectores.ids_de_sectores()) {
        const Sector& sector = sectores.en(id);

        int ancho = 0;
        while (ancho < LIMITE_DE_TAMANIO_DE_MAPA && sector.mapa.esta_dentro(ancho, 0)) {
            ++ancho;
        }
        int alto = 0;
        while (alto < LIMITE_DE_TAMANIO_DE_MAPA && sector.mapa.esta_dentro(0, alto)) {
            ++alto;
        }

        indice_de_mapa_por_id[id] = static_cast<int>(mapas.size());
        mapas.push_back(MapaMedido{id, total, ancho, alto});
        total += ancho * alto;
    }

    mapa_del_nodo.assign(total, 0);
    caminable.assign(total, 0);
    ocupado.assign(total, 0);
    salto_de_puerta.assign(total, SIN_NODO);
    anterior.assign(total, 0);
    visitado.assign(total, 0);

    for (std::size_t indice = 0; indice < mapas.size(); ++indice) {
        const MapaMedido& mapa = mapas[indice];
        const Sector& sector = sectores.en(mapa.id);
        for (int fila = 0; fila < mapa.alto; ++fila) {
            for (int columna = 0; columna < mapa.ancho; ++columna) {
                const int nodo = mapa.primer_nodo + fila * mapa.ancho + columna;
                mapa_del_nodo[nodo] = static_cast<int>(indice);
                caminable[nodo] = sector.colisiones.esta_libre(columna, fila) ? 1 : 0;
            }
        }
    }
}

void MapaDeCaminos::unir_mapas_por_las_puertas(GestorDeSectores& sectores) {
    for (std::size_t indice = 0; indice < mapas.size(); ++indice) {
        const Sector& sector = sectores.en(mapas[indice].id);
        for (const auto& [zona, transicion] : sector.mapa.obtener_transiciones()) {
            if (indice_de_mapa_por_id.count(transicion.mapa_destino) == 0) {
                continue;
            }
            const int mapa_destino = indice_de_mapa_por_id.at(transicion.mapa_destino);

            const int puerta = buscar_puerta(sector, static_cast<int>(indice), zona);
            if (puerta == SIN_NODO) {
                continue;
            }

            const Casilla casilla =
                sectores.casilla_de_llegada(transicion.mapa_destino, transicion);
            int llegada = nodo_en(mapa_destino, casilla.columna, casilla.fila + 1);
            if (llegada == SIN_NODO) {
                llegada = nodo_en(mapa_destino, casilla.columna, casilla.fila);
            }
            llegada = casilla_libre_cercana(llegada, false);
            if (llegada != SIN_NODO) {
                salto_de_puerta[puerta] = llegada;
            }
        }
    }
}

int MapaDeCaminos::buscar_puerta(
    const Sector& sector, int indice_de_mapa, const Rectangulo& zona) const {
    const int ancho = sector.mapa.ancho_de_casilla_en_pixeles();
    const int alto = sector.mapa.alto_de_casilla_en_pixeles();
    const int primera_columna = zona.x / ancho;
    const int ultima_columna = (zona.x + std::max(0, zona.ancho - 1)) / ancho;
    const int primera_fila = zona.y / alto;
    const int ultima_fila = (zona.y + std::max(0, zona.alto - 1)) / alto;
    const int centro_columna = (primera_columna + ultima_columna) / 2;
    const int centro_fila = (primera_fila + ultima_fila) / 2;

    int mejor = SIN_NODO;
    int mejor_puntaje = INT_MAX;
    for (int fila = primera_fila - RADIO_DE_BUSQUEDA_DE_PUERTA;
         fila <= ultima_fila + RADIO_DE_BUSQUEDA_DE_PUERTA;
         ++fila) {
        for (int columna = primera_columna - RADIO_DE_BUSQUEDA_DE_PUERTA;
             columna <= ultima_columna + RADIO_DE_BUSQUEDA_DE_PUERTA;
             ++columna) {
            const int nodo = nodo_en(indice_de_mapa, columna, fila);
            if (nodo == SIN_NODO || !caminable[nodo]) {
                continue;
            }
            const bool dentro_de_la_zona = columna >= primera_columna &&
                                           columna <= ultima_columna && fila >= primera_fila &&
                                           fila <= ultima_fila;
            const int puntaje = (dentro_de_la_zona ? 0 : PENALIZACION_FUERA_DE_LA_ZONA) +
                                std::abs(columna - centro_columna) + std::abs(fila - centro_fila);
            if (puntaje < mejor_puntaje) {
                mejor = nodo;
                mejor_puntaje = puntaje;
            }
        }
    }
    return mejor;
}

int MapaDeCaminos::nodo_en(int indice_de_mapa, int columna, int fila) const {
    const MapaMedido& mapa = mapas[indice_de_mapa];
    if (columna < 0 || fila < 0 || columna >= mapa.ancho || fila >= mapa.alto) {
        return SIN_NODO;
    }
    return mapa.primer_nodo + fila * mapa.ancho + columna;
}

int MapaDeCaminos::nodo_de(const std::string& mapa, int columna, int fila) const {
    if (indice_de_mapa_por_id.count(mapa) == 0) {
        return SIN_NODO;
    }
    int indice_mapa = indice_de_mapa_por_id.at(mapa);
    const MapaMedido& datos_mapa = mapas[indice_mapa];
    if (datos_mapa.ancho <= 0 || datos_mapa.alto <= 0) {
        return SIN_NODO;
    }
    int columna_acotada = columna;
    if (columna_acotada < 0) {
        columna_acotada = 0;
    } else if (columna_acotada >= datos_mapa.ancho) {
        columna_acotada = datos_mapa.ancho - 1;
    }

    int fila_acotada = fila;
    if (fila_acotada < 0) {
        fila_acotada = 0;
    } else if (fila_acotada >= datos_mapa.alto) {
        fila_acotada = datos_mapa.alto - 1;
    }

    return nodo_en(indice_mapa, columna_acotada, fila_acotada);
}

int MapaDeCaminos::nodo_desplazado(int nodo, int columnas, int filas) const {
    if (nodo == SIN_NODO) {
        return SIN_NODO;
    }
    return nodo_en(mapa_del_nodo[nodo], columna_de(nodo) + columnas, fila_de(nodo) + filas);
}

const std::string& MapaDeCaminos::mapa_de(int nodo) const {
    return mapas[mapa_del_nodo[nodo]].id;
}

int MapaDeCaminos::columna_de(int nodo) const {
    const MapaMedido& mapa = mapas[mapa_del_nodo[nodo]];
    return (nodo - mapa.primer_nodo) % mapa.ancho;
}

int MapaDeCaminos::fila_de(int nodo) const {
    const MapaMedido& mapa = mapas[mapa_del_nodo[nodo]];
    return (nodo - mapa.primer_nodo) / mapa.ancho;
}

bool MapaDeCaminos::es_caminable(int nodo) const {
    return nodo != SIN_NODO && caminable[nodo];
}

bool MapaDeCaminos::esta_ocupado(int nodo) const {
    return nodo != SIN_NODO && ocupado[nodo];
}

void MapaDeCaminos::ocupar(int nodo) {
    ocupado[nodo] = 1;
}

void MapaDeCaminos::liberar(int nodo) {
    ocupado[nodo] = 0;
}

int MapaDeCaminos::vecinos_en_el_mapa(int nodo, int vecinos[4]) const {
    const MapaMedido& mapa = mapas[mapa_del_nodo[nodo]];
    const int columna = (nodo - mapa.primer_nodo) % mapa.ancho;
    const int fila = (nodo - mapa.primer_nodo) / mapa.ancho;

    int cantidad = 0;
    if (columna + 1 < mapa.ancho) {
        vecinos[cantidad++] = nodo + 1;
    }
    if (columna > 0) {
        vecinos[cantidad++] = nodo - 1;
    }
    if (fila + 1 < mapa.alto) {
        vecinos[cantidad++] = nodo + mapa.ancho;
    }
    if (fila > 0) {
        vecinos[cantidad++] = nodo - mapa.ancho;
    }
    return cantidad;
}

std::vector<int> MapaDeCaminos::camino(int origen, int destino, bool esquivar_ocupados) {
    if (origen == SIN_NODO || destino == SIN_NODO) {
        return {};
    }
    if (origen == destino) {
        return {origen};
    }
    if (esquivar_ocupados) {
        return buscar_camino(origen, destino, true);
    }

    const uint64_t clave =
        static_cast<uint64_t>(origen) * mapa_del_nodo.size() + static_cast<uint64_t>(destino);
    if (caminos_guardados.count(clave) == 0) {
        caminos_guardados[clave] = buscar_camino(origen, destino, false);
    }
    return caminos_guardados[clave];
}

std::vector<int> MapaDeCaminos::buscar_camino(int origen, int destino, bool esquivar_ocupados) {
    ++numero_de_busqueda;
    cola.clear();
    cola.push_back(origen);
    visitado[origen] = numero_de_busqueda;
    anterior[origen] = origen;

    bool llego = false;
    for (std::size_t i = 0; i < cola.size() && !llego; ++i) {
        const int actual = cola[i];

        int candidatos[5];
        int cantidad = vecinos_en_el_mapa(actual, candidatos);
        if (salto_de_puerta[actual] != SIN_NODO) {
            candidatos[cantidad++] = salto_de_puerta[actual];
        }

        for (int k = 0; k < cantidad; ++k) {
            const int vecino = candidatos[k];
            if (visitado[vecino] == numero_de_busqueda || !caminable[vecino]) {
                continue;
            }
            if (esquivar_ocupados && ocupado[vecino] && vecino != destino) {
                continue;
            }
            visitado[vecino] = numero_de_busqueda;
            anterior[vecino] = actual;
            cola.push_back(vecino);
            if (vecino == destino) {
                llego = true;
                break;
            }
        }
    }

    if (!llego) {
        return {};
    }
    std::vector<int> resultado;
    for (int nodo = destino; nodo != origen; nodo = anterior[nodo]) {
        resultado.push_back(nodo);
    }
    resultado.push_back(origen);
    std::reverse(resultado.begin(), resultado.end());
    return resultado;
}

int MapaDeCaminos::casilla_libre_cercana(int nodo, bool respetar_ocupadas) {
    if (nodo == SIN_NODO) {
        return SIN_NODO;
    }

    ++numero_de_busqueda;
    cola.clear();
    cola.push_back(nodo);
    visitado[nodo] = numero_de_busqueda;

    for (std::size_t i = 0; i < cola.size(); ++i) {
        const int actual = cola[i];
        if (caminable[actual] && !(respetar_ocupadas && ocupado[actual])) {
            return actual;
        }

        int vecinos[4];
        const int cantidad = vecinos_en_el_mapa(actual, vecinos);
        for (int k = 0; k < cantidad; ++k) {
            if (visitado[vecinos[k]] != numero_de_busqueda) {
                visitado[vecinos[k]] = numero_de_busqueda;
                cola.push_back(vecinos[k]);
            }
        }
    }
    return SIN_NODO;
}
