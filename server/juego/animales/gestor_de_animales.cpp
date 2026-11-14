#include "gestor_de_animales.h"

#include <map>
#include <utility>
#include <vector>

#include "../azar.h"

GestorDeAnimales::GestorDeAnimales(Colisiones& colisiones_mapa) : colisiones(colisiones_mapa) {
}

std::pair<int, int> GestorDeAnimales::desplazamiento_segun(Direccion direccion) {
    static const std::map<Direccion, std::pair<int, int>> desplazamientos{
        {Direccion::ARRIBA, {0, -1}},
        {Direccion::ABAJO, {0, 1}},
        {Direccion::IZQUIERDA, {-1, 0}},
        {Direccion::DERECHA, {1, 0}},
    };
    auto iterador = desplazamientos.find(direccion);
    if (iterador != desplazamientos.end()) {
        return iterador->second;
    }
    return {0, 0};
}

bool GestorDeAnimales::puede_ubicarse(const Animal& animal) const {
    return casillas_libres(animal.casillas_de_colision()) &&
           !colisiones.bloquea(animal.rectangulo_de_colision());
}

void GestorDeAnimales::ocupar_colisiones(const Animal& animal) {
    colisiones.colocar_obstaculo(animal.id, animal.rectangulo_de_colision());
}

void GestorDeAnimales::liberar_colisiones(const Animal& animal) {
    colisiones.eliminar_obstaculo(animal.id);
}

bool GestorDeAnimales::casillas_libres(const std::vector<Casilla>& casillas) const {
    bool libres = true;
    for (Casilla casilla : casillas) {
        if (!colisiones.esta_libre(casilla.columna, casilla.fila)) {
            libres = false;
        }
    }
    return libres;
}

uint8_t GestorDeAnimales::sortear_variante_vaca() {
    return static_cast<uint8_t>(Azar::entero(0, Vaca::CANTIDAD_VARIANTES - 1));
}

uint8_t GestorDeAnimales::sortear_variante_gallina() {
    return static_cast<uint8_t>(Azar::entero(0, Gallina::CANTIDAD_VARIANTES - 1));
}

bool GestorDeAnimales::agregar_vaca(uint32_t columna, uint32_t fila) {
    return agregar_vaca(columna, fila, sortear_variante_vaca());
}

bool GestorDeAnimales::agregar_vaca(uint32_t columna, uint32_t fila, uint8_t variante) {
    if (vacas.size() >= Vaca::LIMITE_POR_GRANJA) {
        return false;
    }
    uint32_t id = siguiente_id++;
    Vaca vaca(id, columna, fila, variante);
    if (!puede_ubicarse(vaca)) {
        return false;
    }
    ocupar_colisiones(vaca);
    vacas.emplace(id, vaca);
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
    Gallina gallina(id, columna, fila, variante);
    if (!puede_ubicarse(gallina)) {
        return false;
    }
    ocupar_colisiones(gallina);
    gallinas.emplace(id, gallina);
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

bool GestorDeAnimales::ordeniar_vaca_en(
    uint32_t columna, uint32_t fila, uint32_t& id_vaca_ordenada) {
    Vaca* vaca = buscar_vaca_en(columna, fila);
    if (!vaca || !vaca->esta_lista_para_ordeniar()) {
        return false;
    }
    id_vaca_ordenada = vaca->id;
    return vaca->ordeniar();
}

bool GestorDeAnimales::vaca_lista_en(uint32_t columna, uint32_t fila) const {
    auto vaca = vaca_en(columna, fila);
    return vaca != nullptr && vaca->esta_lista_para_ordeniar();
}

const Vaca* GestorDeAnimales::vaca_en(uint32_t columna, uint32_t fila) const {
    for (const auto& [id, vaca] : vacas) {
        if (vaca.ocupa_casilla(columna, fila)) {
            return &vaca;
        }
    }
    return nullptr;
}

Vaca* GestorDeAnimales::buscar_vaca_en(uint32_t columna, uint32_t fila) {
    for (auto& [id, vaca] : vacas) {
        if (vaca.ocupa_casilla(columna, fila)) {
            return &vaca;
        }
    }
    return nullptr;
}

const Gallina* GestorDeAnimales::gallina_en(uint32_t columna, uint32_t fila) const {
    for (const auto& [id, gallina] : gallinas) {
        if (gallina.ocupa_casilla(columna, fila)) {
            return &gallina;
        }
    }
    return nullptr;
}

Gallina* GestorDeAnimales::buscar_gallina_en(uint32_t columna, uint32_t fila) {
    for (auto& [id, gallina] : gallinas) {
        if (gallina.ocupa_casilla(columna, fila)) {
            return &gallina;
        }
    }
    return nullptr;
}

EstadoAnimal GestorDeAnimales::estado_de(const Animal& animal) {
    EstadoAnimal estado;
    estado.id = animal.id;
    estado.tipo = animal.tipo;
    estado.variante = animal.variante;
    estado.columna = animal.columna;
    estado.fila = animal.fila;
    estado.mirando = animal.mirando;
    estado.caminando = animal.caminando;
    estado.durmiendo = animal.durmiendo;
    return estado;
}

void GestorDeAnimales::actualizar_rutina(
    Animal& animal, const HoraDelJuego& hora, std::vector<EstadoAnimal>& actualizados) {
    if (hora.hora >= HORA_DORMIR) {
        if (!animal.durmiendo || animal.caminando) {
            animal.durmiendo = true;
            animal.caminando = false;
            animal.ticks_para_decision = 0;
            actualizados.push_back(estado_de(animal));
        }
    } else if (hora.hora >= HORA_QUIETOS) {
        if (animal.durmiendo || animal.caminando) {
            animal.durmiendo = false;
            animal.caminando = false;
            animal.ticks_para_decision = 0;
            actualizados.push_back(estado_de(animal));
        }
    } else if (animal.durmiendo) {
        animal.durmiendo = false;
        actualizados.push_back(estado_de(animal));
    }
}

bool GestorDeAnimales::intentar_mover(
    Animal& animal, Direccion dir, int ancho_mapa, int alto_mapa, const Jugadores& jugadores) {
    std::pair<int, int> desplazamiento = desplazamiento_segun(dir);
    int paso_columna = desplazamiento.first;
    int paso_fila = desplazamiento.second;

    int nueva_columna = static_cast<int>(animal.columna) + paso_columna;
    int nueva_fila = static_cast<int>(animal.fila) + paso_fila;

    if (nueva_columna < 0 || nueva_columna + animal.ancho_en_casillas > ancho_mapa ||
        nueva_fila < 0 || nueva_fila + animal.alto_en_casillas > alto_mapa) {
        return false;
    }

    uint32_t columna_anterior = animal.columna;
    uint32_t fila_anterior = animal.fila;
    Direccion mirando_anterior = animal.mirando;

    liberar_colisiones(animal);
    animal.columna = static_cast<uint32_t>(nueva_columna);
    animal.fila = static_cast<uint32_t>(nueva_fila);
    animal.mirando = dir;

    bool terreno_libre = casillas_libres(animal.casillas_de_colision());
    bool colisiona_con_obstaculo = colisiones.bloquea(animal.rectangulo_de_colision());
    bool jugador_en_destino = jugadores.hay_alguien_en(animal.rectangulo_de_colision());

    if (terreno_libre && !colisiona_con_obstaculo && !jugador_en_destino) {
        animal.caminando = true;
        ocupar_colisiones(animal);
        return true;
    }

    animal.columna = columna_anterior;
    animal.fila = fila_anterior;
    animal.mirando = mirando_anterior;
    ocupar_colisiones(animal);
    return false;
}

void GestorDeAnimales::decidir_movimiento(
    Animal& animal, int ancho_mapa, int alto_mapa, const Jugadores& jugadores,
    std::vector<EstadoAnimal>& actualizados) {
    if (animal.ticks_para_decision > 0) {
        --animal.ticks_para_decision;
        return;
    }

    animal.ticks_para_decision =
        static_cast<uint32_t>(Azar::entero(TICKS_DECISION_MIN, TICKS_DECISION_MAX));

    if (Azar::entero(0, PROBABILIDAD_ACCION_MAX) < PROBABILIDAD_QUIETO) {
        if (animal.caminando) {
            animal.caminando = false;
            actualizados.push_back(estado_de(animal));
        }
        return;
    }

    static constexpr Direccion DIRECCIONES[CANTIDAD_DIRECCIONES] = {
        Direccion::ABAJO,
        Direccion::DERECHA,
        Direccion::IZQUIERDA,
        Direccion::ARRIBA};
    Direccion direccion_elegida = DIRECCIONES[Azar::entero(0, CANTIDAD_DIRECCIONES - 1)];

    if (intentar_mover(animal, direccion_elegida, ancho_mapa, alto_mapa, jugadores)) {
        actualizados.push_back(estado_de(animal));
    } else if (animal.mirando != direccion_elegida || animal.caminando) {
        animal.mirando = direccion_elegida;
        animal.caminando = false;
        ocupar_colisiones(animal);
        actualizados.push_back(estado_de(animal));
    }
}

void GestorDeAnimales::actualizar_animal(
    Animal& animal, const HoraDelJuego& hora, int ancho_mapa, int alto_mapa,
    const Jugadores& jugadores, std::vector<EstadoAnimal>& actualizados) {
    actualizar_rutina(animal, hora, actualizados);
    if (!animal.durmiendo && hora.hora < HORA_QUIETOS) {
        decidir_movimiento(animal, ancho_mapa, alto_mapa, jugadores, actualizados);
    }
}

std::vector<EstadoAnimal> GestorDeAnimales::actualizar(
    const HoraDelJuego& hora, int ancho_mapa, int alto_mapa, const Jugadores& jugadores) {
    std::vector<EstadoAnimal> actualizados;

    for (auto& [id, vaca] : vacas) {
        actualizar_animal(vaca, hora, ancho_mapa, alto_mapa, jugadores, actualizados);
    }
    for (auto& [id, gallina] : gallinas) {
        actualizar_animal(gallina, hora, ancho_mapa, alto_mapa, jugadores, actualizados);
    }

    return actualizados;
}

void GestorDeAnimales::agregar_a(EstadoJuego& estado) const {
    for (const auto& [id, vaca] : vacas) {
        estado.animales.push_back(estado_de(vaca));
        if (vaca.esta_lista_para_ordeniar()) {
            estado.vacas_para_ordeniar.push_back(vaca.id);
        }
    }
    for (const auto& [id, gallina] : gallinas) {
        estado.animales.push_back(estado_de(gallina));
    }
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

int GestorDeAnimales::cantidad_vacas_listas_para_ordeniar() const {
    int cantidad = 0;
    for (const auto& [id, vaca] : vacas) {
        if (vaca.esta_lista_para_ordeniar()) {
            cantidad++;
        }
    }
    return cantidad;
}

std::vector<uint32_t> GestorDeAnimales::obtener_ids_vacas_listas_para_ordeniar() const {
    std::vector<uint32_t> ids;
    for (const auto& [id, vaca] : vacas) {
        if (vaca.esta_lista_para_ordeniar()) {
            ids.push_back(vaca.id);
        }
    }
    return ids;
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

DatosAnimalGuardado GestorDeAnimales::datos_guardados_de(const Animal& animal) {
    DatosAnimalGuardado datos;
    datos.id = animal.id;
    datos.tipo = animal.tipo;
    datos.variante = animal.variante;
    datos.columna = animal.columna;
    datos.fila = animal.fila;
    datos.mirando = animal.mirando;
    datos.dias_acumulados = animal.dias_acumulados;
    return datos;
}

void GestorDeAnimales::agregar_animales_guardados_a(std::vector<DatosAnimalGuardado>& lista) const {
    for (const auto& [id, vaca] : vacas) {
        lista.push_back(datos_guardados_de(vaca));
    }
    for (const auto& [id, gallina] : gallinas) {
        lista.push_back(datos_guardados_de(gallina));
    }
}

void GestorDeAnimales::cargar_animales_guardados(
    const std::vector<DatosAnimalGuardado>& lista,
    const std::vector<uint32_t>& vacas_para_ordeniar) {
    if (lista.empty()) {
        return;
    }
    for (const auto& [id, vaca] : vacas) {
        liberar_colisiones(vaca);
    }
    for (const auto& [id, gallina] : gallinas) {
        liberar_colisiones(gallina);
    }
    vacas.clear();
    gallinas.clear();
    for (const auto& datos : lista) {
        if (datos.tipo == TipoAnimal::VACA) {
            Vaca vaca(datos.id, datos.columna, datos.fila, datos.variante);
            vaca.mirando = datos.mirando;
            vaca.dias_acumulados = datos.dias_acumulados;
            ocupar_colisiones(vaca);
            vacas.emplace(datos.id, vaca);
        } else if (datos.tipo == TipoAnimal::GALLINA) {
            Gallina gallina(datos.id, datos.columna, datos.fila, datos.variante);
            gallina.mirando = datos.mirando;
            gallina.dias_acumulados = datos.dias_acumulados;
            ocupar_colisiones(gallina);
            gallinas.emplace(datos.id, gallina);
        }
        if (datos.id >= siguiente_id) {
            siguiente_id = datos.id + 1;
        }
    }
    for (uint32_t id_vaca : vacas_para_ordeniar) {
        auto it = vacas.find(id_vaca);
        if (it != vacas.end()) {
            it->second.marcar_lista_para_ordeniar(true);
        }
    }
}
