#include "gestor_de_animales.h"

#include <map>
#include <random>
#include <vector>

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

bool GestorDeAnimales::agregar_vaca(uint32_t columna, uint32_t fila) {
    return agregar_vaca(columna, fila, sortear_variante_vaca());
}

bool GestorDeAnimales::agregar_vaca(uint32_t columna, uint32_t fila, uint8_t variante) {
    if (vacas.size() >= Vaca::LIMITE_POR_GRANJA) {
        return false;
    }
    uint32_t id = siguiente_id++;
    vacas.emplace(id, Vaca(id, columna, fila, variante));
    return true;
}

bool GestorDeAnimales::agregar_gallina(uint32_t columna, uint32_t fila) {
    return agregar_gallina(columna, fila, sortear_variante_gallina());
}

bool GestorDeAnimales::agregar_gallina(uint32_t columna, uint32_t fila, uint8_t variante) {
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

void GestorDeAnimales::agregar_animales_guardados_a(std::vector<DatosAnimalGuardado>& lista) const {
    for (const auto& [id, vaca] : vacas) {
        DatosAnimalGuardado datos;
        datos.id = vaca.id;
        datos.tipo = TipoAnimal::VACA;
        datos.variante = vaca.variante;
        datos.columna = vaca.columna;
        datos.fila = vaca.fila;
        datos.mirando = vaca.mirando;
        datos.dias_acumulados = vaca.dias_acumulados;
        datos.lista_para_ordeniar = vaca.esta_lista_para_ordeniar();
        lista.push_back(datos);
    }
    for (const auto& [id, gallina] : gallinas) {
        DatosAnimalGuardado datos;
        datos.id = gallina.id;
        datos.tipo = TipoAnimal::GALLINA;
        datos.variante = gallina.variante;
        datos.columna = gallina.columna;
        datos.fila = gallina.fila;
        datos.mirando = gallina.mirando;
        datos.dias_acumulados = gallina.dias_acumulados;
        datos.lista_para_ordeniar = false;
        lista.push_back(datos);
    }
}

void GestorDeAnimales::cargar_animales_guardados(const std::vector<DatosAnimalGuardado>& lista) {
    if (lista.empty()) {
        return;
    }
    vacas.clear();
    gallinas.clear();
    for (const auto& datos : lista) {
        if (datos.tipo == TipoAnimal::VACA) {
            Vaca vaca(datos.id, datos.columna, datos.fila, datos.variante);
            vaca.mirando = datos.mirando;
            vaca.dias_acumulados = datos.dias_acumulados;
            vaca.marcar_lista_para_ordeniar(datos.lista_para_ordeniar);
            vacas.emplace(datos.id, vaca);
        } else if (datos.tipo == TipoAnimal::GALLINA) {
            Gallina gallina(datos.id, datos.columna, datos.fila, datos.variante);
            gallina.mirando = datos.mirando;
            gallina.dias_acumulados = datos.dias_acumulados;
            gallinas.emplace(datos.id, gallina);
        }
        if (datos.id >= siguiente_id) {
            siguiente_id = datos.id + 1;
        }
    }
}
