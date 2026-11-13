#include "gestor_de_animales.h"

#include <map>
#include <random>
#include <utility>
#include <vector>

namespace {
constexpr uint8_t HORA_QUIETOS = 17;
constexpr uint8_t HORA_DORMIR = 19;
constexpr int TICKS_DECISION_MIN = 40;
constexpr int TICKS_DECISION_MAX = 100;
constexpr int PROBABILIDAD_QUIETO = 4;

const std::map<Direccion, std::pair<int, int>> DESPLAZAMIENTO_POR_DIRECCION{
    {Direccion::ARRIBA, {0, -1}},
    {Direccion::ABAJO, {0, 1}},
    {Direccion::IZQUIERDA, {-1, 0}},
    {Direccion::DERECHA, {1, 0}},
};
}  // namespace

GestorDeAnimales::GestorDeAnimales(Colisiones& colisiones_mapa) : colisiones(colisiones_mapa) {
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
    Vaca vaca(id, columna, fila, variante);
    if (!casillas_libres(vaca.casillas_de_colision()) ||
        colisiones.bloquea(vaca.rectangulo_de_colision())) {
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
    if (!casillas_libres(gallina.casillas_de_colision()) ||
        colisiones.bloquea(gallina.rectangulo_de_colision())) {
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
    auto encontrado = DESPLAZAMIENTO_POR_DIRECCION.find(dir);
    if (encontrado == DESPLAZAMIENTO_POR_DIRECCION.end()) {
        return false;
    }
    int paso_columna = encontrado->second.first;
    int paso_fila = encontrado->second.second;

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
    static std::mt19937 generador(std::random_device{}());

    if (animal.ticks_para_decision > 0) {
        --animal.ticks_para_decision;
        return;
    }

    std::uniform_int_distribution<int> dist_ticks(TICKS_DECISION_MIN, TICKS_DECISION_MAX);
    animal.ticks_para_decision = dist_ticks(generador);

    std::uniform_int_distribution<int> dist_accion(0, 10);
    if (dist_accion(generador) < PROBABILIDAD_QUIETO) {
        if (animal.caminando) {
            animal.caminando = false;
            actualizados.push_back(estado_de(animal));
        }
        return;
    }

    static const Direccion DIRECCIONES[] = {
        Direccion::ABAJO,
        Direccion::DERECHA,
        Direccion::IZQUIERDA,
        Direccion::ARRIBA};
    std::uniform_int_distribution<int> dist_dir(0, 3);
    Direccion dir = DIRECCIONES[dist_dir(generador)];

    if (intentar_mover(animal, dir, ancho_mapa, alto_mapa, jugadores)) {
        actualizados.push_back(estado_de(animal));
    } else if (animal.mirando != dir || animal.caminando) {
        animal.mirando = dir;
        animal.caminando = false;
        ocupar_colisiones(animal);
        actualizados.push_back(estado_de(animal));
    }
}

std::vector<EstadoAnimal> GestorDeAnimales::actualizar(
    const HoraDelJuego& hora, int ancho_mapa, int alto_mapa, const Jugadores& jugadores) {
    std::vector<EstadoAnimal> actualizados;

    for (auto& [id, vaca] : vacas) {
        actualizar_rutina(vaca, hora, actualizados);
        if (!vaca.durmiendo && hora.hora < HORA_QUIETOS) {
            decidir_movimiento(vaca, ancho_mapa, alto_mapa, jugadores, actualizados);
        }
    }

    for (auto& [id, gallina] : gallinas) {
        actualizar_rutina(gallina, hora, actualizados);
        if (!gallina.durmiendo && hora.hora < HORA_QUIETOS) {
            decidir_movimiento(gallina, ancho_mapa, alto_mapa, jugadores, actualizados);
        }
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
            vaca.marcar_lista_para_ordeniar(datos.lista_para_ordeniar);
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
}
