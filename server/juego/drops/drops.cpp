#include "drops.h"

#include <cstdlib>
#include <vector>

int Drops::dejar(TipoItem tipo, uint16_t cantidad, Calidad calidad, int columna, int fila) {
    if (tipo == TipoItem::NINGUNO || cantidad == 0) {
        return 0;
    }

    const int id = siguiente_id++;
    Drop nuevo;
    nuevo.id = id;
    nuevo.columna = columna;
    nuevo.fila = fila;
    nuevo.tipo = tipo;
    nuevo.cantidad = cantidad;
    nuevo.calidad = calidad;
    drops.emplace(id, nuevo);
    return id;
}

const Drop* Drops::buscar(int id) const {
    const auto encontrado = drops.find(id);
    return encontrado == drops.end() ? nullptr : &encontrado->second;
}

std::vector<int> Drops::ids_cerca_de(const Casilla& centro, int radio_en_casillas) const {
    std::vector<int> resultado;
    for (const auto& entrada : drops) {
        const Drop& drop = entrada.second;
        const int distancia_en_columnas = std::abs(drop.columna - centro.columna);
        const int distancia_en_filas = std::abs(drop.fila - centro.fila);
        if (distancia_en_columnas <= radio_en_casillas && distancia_en_filas <= radio_en_casillas) {
            resultado.push_back(drop.id);
        }
    }
    return resultado;
}

bool Drops::recoger(int id, Inventario& destino) {
    const Drop* drop = buscar(id);
    if (drop == nullptr || !destino.puede_agregar(drop->tipo, drop->cantidad, drop->calidad)) {
        return false;
    }

    destino.agregar_item(drop->tipo, drop->cantidad, drop->calidad);
    drops.erase(id);
    return true;
}

bool Drops::quitar(int id) {
    return drops.erase(id) > 0;
}

void Drops::limpiar() {
    drops.clear();
}

bool Drops::vacio() const {
    return drops.empty();
}

void Drops::agregar_a(EstadoJuego& estado) const {
    estado.drops.reserve(estado.drops.size() + drops.size());
    for (const auto& entrada : drops) {
        const Drop& drop = entrada.second;
        EstadoDrop estado_drop;
        estado_drop.id = static_cast<uint32_t>(drop.id);
        estado_drop.columna = static_cast<uint32_t>(drop.columna);
        estado_drop.fila = static_cast<uint32_t>(drop.fila);
        estado_drop.tipo = drop.tipo;
        estado_drop.cantidad = drop.cantidad;
        estado_drop.calidad = drop.calidad;
        estado.drops.push_back(estado_drop);
    }
}
