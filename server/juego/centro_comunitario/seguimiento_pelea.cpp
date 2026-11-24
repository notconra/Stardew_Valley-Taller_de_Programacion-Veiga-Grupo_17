#include "seguimiento_pelea.h"

#include <algorithm>

#define PISOS_SEGUIDOS_CON_ESQUELETOS 3
#define ORCOS_DEL_MISMO_GRUPO 4

std::optional<int> SeguimientoDePelea::ubicar(uint32_t id_jugador, int piso) {
    if (piso <= 0) {
        reiniciar(id_jugador);
        return std::nullopt;
    }
    auto encontrado = avances.find(id_jugador);
    if (encontrado == avances.end()) {
        avances[id_jugador].piso = piso;
        return std::nullopt;
    }
    Avance& avance = encontrado->second;
    if (avance.piso == piso) {
        return std::nullopt;
    }
    if (piso != avance.piso + 1) {
        reiniciar(id_jugador);
        avances[id_jugador].piso = piso;
        return std::nullopt;
    }
    std::optional<int> piso_sin_danio;
    if (!avance.recibio_danio_en_el_piso) {
        piso_sin_danio = avance.piso;
    }
    avance.piso = piso;
    avance.recibio_danio_en_el_piso = false;
    return piso_sin_danio;
}

void SeguimientoDePelea::recibio_danio_de_un_enemigo(uint32_t id_jugador) {
    auto encontrado = avances.find(id_jugador);
    if (encontrado != avances.end()) {
        encontrado->second.recibio_danio_en_el_piso = true;
    }
}

std::optional<LogroDePelea> SeguimientoDePelea::mato_a(
    uint32_t id_jugador, int piso, const Enemigo& muerto, const GestorDeEnemigos& enemigos) {
    if (piso <= 0) {
        return std::nullopt;
    }
    ubicar(id_jugador, piso);
    switch (muerto.tipo) {
        case TipoEnemigo::ESQUELETO:
            if (mato_un_esqueleto(avances[id_jugador], piso)) {
                return LogroDePelea::ESQUELETOS;
            }
            break;
        case TipoEnemigo::ORCO:
            if (mato_un_orco(id_jugador, piso, muerto)) {
                return LogroDePelea::ORCOS;
            }
            break;
        case TipoEnemigo::VAMPIRA:
        case TipoEnemigo::VAMPIRO:
            if (mato_un_vampiro(id_jugador, piso, muerto, enemigos)) {
                return LogroDePelea::VAMPIROS;
            }
            break;
        default:
            break;
    }
    return std::nullopt;
}

bool SeguimientoDePelea::mato_un_esqueleto(Avance& avance, int piso) const {
    if (avance.ultimo_piso_con_esqueleto == piso) {
        return false;
    }
    const bool sigue_la_racha =
        avance.racha_de_esqueletos > 0 && avance.ultimo_piso_con_esqueleto == piso - 1;
    avance.racha_de_esqueletos = sigue_la_racha ? avance.racha_de_esqueletos + 1 : 1;
    avance.ultimo_piso_con_esqueleto = piso;
    return avance.racha_de_esqueletos >= PISOS_SEGUIDOS_CON_ESQUELETOS;
}

bool SeguimientoDePelea::mato_un_orco(uint32_t id_jugador, int piso, const Enemigo& muerto) {
    anotar_muerte(id_jugador, piso, muerto);
    if (muerto.grupo == 0) {
        return false;
    }
    const auto del_mismo_grupo = [&](const Muerte& muerte) {
        return muerte.id_jugador == id_jugador && muerte.piso == piso &&
               muerte.grupo == muerto.grupo;
    };
    return std::count_if(muertes.begin(), muertes.end(), del_mismo_grupo) >= ORCOS_DEL_MISMO_GRUPO;
}

bool SeguimientoDePelea::mato_un_vampiro(
    uint32_t id_jugador, int piso, const Enemigo& muerto, const GestorDeEnemigos& enemigos) {
    anotar_muerte(id_jugador, piso, muerto);
    if (muerto.grupo == 0) {
        return false;
    }
    bool tiene_pareja = false;
    for (const Enemigo* otro : enemigos.del_grupo(muerto.grupo)) {
        if (otro->id == muerto.id) {
            continue;
        }
        tiene_pareja = true;
        if (otro->esta_vivo() || !lo_mato(id_jugador, piso, otro->id)) {
            return false;
        }
    }
    return tiene_pareja;
}

void SeguimientoDePelea::anotar_muerte(uint32_t id_jugador, int piso, const Enemigo& muerto) {
    // Si ya lo había matado alguien (revivió), vale la última muerte.
    std::erase_if(muertes, [&](const Muerte& muerte) {
        return muerte.piso == piso && muerte.id_enemigo == muerto.id;
    });
    muertes.push_back(Muerte{id_jugador, piso, muerto.id, muerto.grupo});
}

bool SeguimientoDePelea::lo_mato(uint32_t id_jugador, int piso, int id_enemigo) const {
    return std::any_of(muertes.begin(), muertes.end(), [&](const Muerte& muerte) {
        return muerte.id_jugador == id_jugador && muerte.piso == piso &&
               muerte.id_enemigo == id_enemigo;
    });
}

void SeguimientoDePelea::reiniciar(uint32_t id_jugador) {
    avances.erase(id_jugador);
    std::erase_if(muertes, [&](const Muerte& muerte) { return muerte.id_jugador == id_jugador; });
}
