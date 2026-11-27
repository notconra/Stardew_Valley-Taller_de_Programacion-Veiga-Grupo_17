#include "estado_de_la_partida.h"

#include <algorithm>
#include <vector>

namespace {

template <class T, class Id>
auto buscar_por_id(std::vector<T>& elementos, Id id) {
    return std::find_if(elementos.begin(), elementos.end(), [id](const T& otro) {
        return otro.id == id;
    });
}

}  // namespace

void EstadoDeLaPartida::reemplazar_mundo(const EstadoJuego& nuevo) {
    mundo = nuevo;
    tierra_tratada.clear();
    for (const CasillaTratada& casilla : nuevo.tierra) {
        tierra_tratada[{static_cast<int>(casilla.columna), static_cast<int>(casilla.fila)}] =
            casilla;
    }
}

void EstadoDeLaPartida::actualizar_jugadores(const JugadoresEnSector& evento) {
    if (mundo.mapa != evento.mapa) {
        mundo.enemigos.clear();
        mundo.trampas.clear();
        mundo.drops.clear();
        mundo.objetos.clear();
    }
    mundo.mapa = evento.mapa;
    mundo.jugadores = evento.jugadores;
}

void EstadoDeLaPartida::cambiar_tierra(const TierraCambiada& evento) {
    const std::pair<int, int> casilla{
        static_cast<int>(evento.casilla.columna),
        static_cast<int>(evento.casilla.fila)};
    if (evento.tratada) {
        tierra_tratada[casilla] = evento.casilla;
    } else {
        tierra_tratada.erase(casilla);
    }
}

void EstadoDeLaPartida::colocar_objeto(const ObjetoColocado& e) {
    const EstadoObjetoDelMundo objeto{e.id, mundo.mapa, e.columna, e.fila, e.tipo};
    const auto existente = buscar_por_id(mundo.objetos, e.id);
    if (existente == mundo.objetos.end()) {
        mundo.objetos.push_back(objeto);
    } else {
        *existente = objeto;
    }
}

bool EstadoDeLaPartida::retirar_objeto(const ObjetoRetirado& evento) {
    const auto retirado = buscar_por_id(mundo.objetos, evento.id);
    const bool era_mineral = retirado != mundo.objetos.end() &&
                             retirado->tipo != TipoObjeto::COFRE &&
                             retirado->tipo != TipoObjeto::NINGUNO;
    std::erase_if(mundo.objetos, [&evento](const EstadoObjetoDelMundo& objeto) {
        return objeto.id == evento.id;
    });
    return era_mineral;
}

void EstadoDeLaPartida::retirar_drop(const DropRetirado& evento) {
    std::erase_if(mundo.drops, [&evento](const EstadoDrop& drop) { return drop.id == evento.id; });
}

std::optional<EstadoEnemigo> EstadoDeLaPartida::actualizar_enemigo(const EstadoEnemigo& enemigo) {
    const auto existente = buscar_por_id(mundo.enemigos, enemigo.id);
    if (existente == mundo.enemigos.end()) {
        mundo.enemigos.push_back(enemigo);
        return std::nullopt;
    }
    const EstadoEnemigo anterior = *existente;
    *existente = enemigo;
    return anterior;
}

bool EstadoDeLaPartida::actualizar_trampa(const EstadoTrampa& trampa) {
    const auto existente = buscar_por_id(mundo.trampas, trampa.id);
    const bool se_encendio =
        trampa.activa && (existente == mundo.trampas.end() || !existente->activa);
    if (existente == mundo.trampas.end()) {
        mundo.trampas.push_back(trampa);
    } else {
        *existente = trampa;
    }
    return se_encendio;
}

void EstadoDeLaPartida::actualizar_animal(const EstadoAnimal& animal) {
    const auto existente = buscar_por_id(mundo.animales, animal.id);
    if (existente == mundo.animales.end()) {
        mundo.animales.push_back(animal);
    } else {
        *existente = animal;
    }
}

void EstadoDeLaPartida::retirar_animal(const AnimalRetirado& evento) {
    std::erase_if(mundo.animales, [&evento](const EstadoAnimal& otro) {
        return otro.id == evento.id;
    });
    std::erase(mundo.vacas_para_ordeniar, evento.id);
}

void EstadoDeLaPartida::marcar_vaca(const VacaOrdeniable& evento) {
    if (!evento.lista) {
        std::erase(mundo.vacas_para_ordeniar, evento.id_vaca);
        return;
    }
    if (std::find(
            mundo.vacas_para_ordeniar.begin(),
            mundo.vacas_para_ordeniar.end(),
            evento.id_vaca) == mundo.vacas_para_ordeniar.end()) {
        mundo.vacas_para_ordeniar.push_back(evento.id_vaca);
    }
}
