#include "gestor_de_animales.h"

#include <map>
#include <random>

GestorDeAnimales::GestorDeAnimales() {
}

uint8_t GestorDeAnimales::sortear_variante_vaca() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, Vaca::CANTIDAD_VARIANTES - 1);
    return dist(gen);
}

uint8_t GestorDeAnimales::sortear_variante_gallina() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, Gallina::CANTIDAD_VARIANTES - 1);
    return dist(gen);
}

bool GestorDeAnimales::agregar_vaca(int columna, int fila) {
    return agregar_vaca(columna, fila, sortear_variante_vaca());
}

bool GestorDeAnimales::agregar_vaca(int columna, int fila, uint8_t variante) {
    if (vacas.size() >= Vaca::LIMITE_POR_GRANJA) {
        return false;
    }
    uint32_t id = siguiente_id++;
    vacas.emplace(id, Vaca(id, columna, fila, variante));
    return true;
}

bool GestorDeAnimales::agregar_gallina(int columna, int fila) {
    return agregar_gallina(columna, fila, sortear_variante_gallina());
}

bool GestorDeAnimales::agregar_gallina(int columna, int fila, uint8_t variante) {
    if (gallinas.size() >= Gallina::LIMITE_POR_GRANJA) {
        return false;
    }
    uint32_t id = siguiente_id++;
    gallinas.emplace(id, Gallina(id, columna, fila, variante));
    return true;
}

void GestorDeAnimales::inicializar_granja_provisoria() {
    // NOTA PROVISORIA: Spawnea 1 vaca y 1 gallina para pruebas en tiempo de ejecución mientras
    // no existan los comerciantes. Eliminar cuando se agregue la compra en el mercado.
    agregar_vaca(20, 15);
    agregar_gallina(22, 15);
}

void GestorDeAnimales::avanzar_dia(Drops& drops) {
    for (auto& [id, vaca] : vacas) {
        vaca.avanzar_dia();
    }
    for (auto& [id, gallina] : gallinas) {
        gallina.avanzar_dia();
        drops.dejar(TipoItem::HUEVO, 1, Calidad::BASE, gallina.columna, gallina.fila);
    }
}

bool GestorDeAnimales::ordeniar_vaca(uint32_t id_vaca) {
    auto encontrado = vacas.find(id_vaca);
    if (encontrado == vacas.end()) {
        return false;
    }
    return encontrado->second.ordeniar();
}

int GestorDeAnimales::cantidad_vacas() const {
    return static_cast<int>(vacas.size());
}

int GestorDeAnimales::cantidad_gallinas() const {
    return static_cast<int>(gallinas.size());
}

int GestorDeAnimales::cantidad_total() const {
    return static_cast<int>(vacas.size() + gallinas.size());
}

const Vaca* GestorDeAnimales::buscar_vaca(uint32_t id) const {
    auto encontrado = vacas.find(id);
    if (encontrado == vacas.end()) {
        return nullptr;
    }
    return &encontrado->second;
}

const Gallina* GestorDeAnimales::buscar_gallina(uint32_t id) const {
    auto encontrado = gallinas.find(id);
    if (encontrado == gallinas.end()) {
        return nullptr;
    }
    return &encontrado->second;
}

const Animal* GestorDeAnimales::buscar(uint32_t id) const {
    auto it_vaca = vacas.find(id);
    if (it_vaca != vacas.end()) {
        return &it_vaca->second;
    }
    auto it_gallina = gallinas.find(id);
    if (it_gallina != gallinas.end()) {
        return &it_gallina->second;
    }
    return nullptr;
}

const std::map<uint32_t, Vaca>& GestorDeAnimales::obtener_vacas() const {
    return vacas;
}

const std::map<uint32_t, Gallina>& GestorDeAnimales::obtener_gallinas() const {
    return gallinas;
}
