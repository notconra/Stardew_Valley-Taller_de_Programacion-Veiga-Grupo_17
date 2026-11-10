#include "personaje.h"

#include <random>
#include <string>

uint32_t Personaje::sortear_monedas_iniciales() {
    static std::mt19937 generador{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> dist(MONEDAS_INICIALES_MIN, MONEDAS_INICIALES_MAX);
    return dist(generador);
}

Personaje::Personaje(
    uint32_t id, const std::string& mapa_inicial, const CatalogoDeItems& catalogo,
    uint8_t cantidad_ranuras_barra)
    : id_jugador(id),
      mapa_actual(mapa_inicial),
      inventario(catalogo, cantidad_ranuras_barra),
      ranura_activa(0),
      monedas(sortear_monedas_iniciales()) {
    // Al crearse (solo la primera vez que el jugador entra) recibe las herramientas que el
    // catálogo marque con cantidad inicial y su cama, que pone él donde quiera de la casa. Nada
    // más: lo demás se consigue jugando.
    for (const auto& [tipo, definicion] : catalogo.definiciones()) {
        const bool es_inicial = catalogo.es_herramienta(tipo) || tipo == TipoItem::CAMA;
        if (es_inicial && definicion.cantidad_inicial > 0) {
            inventario.agregar_item(tipo, definicion.cantidad_inicial);
        }
    }
}

void Personaje::recibir_monedas(uint32_t cantidad) {
    if (MONEDAS_MAXIMAS - monedas < cantidad) {
        monedas = MONEDAS_MAXIMAS;
    } else {
        monedas += cantidad;
    }
}

bool Personaje::gastar_monedas(uint32_t cantidad) {
    if (monedas < cantidad) {
        return false;
    }
    monedas -= cantidad;
    return true;
}

uint8_t Personaje::resolver_ranura(uint8_t indice_ranura) const {
    return indice_ranura == INDICE_RANURA_ACTIVA ? ranura_activa : indice_ranura;
}

RanuraInventario Personaje::ranura(uint8_t indice_ranura) const {
    return inventario.ranura(resolver_ranura(indice_ranura));
}

bool Personaje::seleccionar_ranura(uint8_t indice_ranura) {
    if (!inventario.indice_valido(indice_ranura)) {
        return false;
    }
    ranura_activa = indice_ranura;
    return true;
}

bool Personaje::mover_item(uint8_t origen, uint8_t destino, uint16_t cantidad) {
    if (destino == inventario.cantidad_ranuras()) {
        return inventario.eliminar(origen, cantidad);
    }
    return inventario.mover_entre_ranuras(origen, destino, cantidad);
}

bool Personaje::quitar_de_ranura(uint8_t indice_ranura, uint16_t cantidad) {
    return inventario.quitar_item(resolver_ranura(indice_ranura), cantidad);
}

bool Personaje::esta_ocupado() const {
    return ticks_restantes_accion > 0;
}

double Personaje::multiplicador_de_ataque() const {
    constexpr int UMBRAL_VIDA_BAJA = VIDA_MAXIMA / 5;  // 20%
    return vida <= UMBRAL_VIDA_BAJA ? 0.5 : 1.0;
}

bool Personaje::esta_inconsciente() const {
    return vida <= 0;
}

void Personaje::recibir_golpe(int danio) {
    if (danio <= 0 || esta_inconsciente()) {
        return;
    }
    vida = (danio >= vida) ? 0 : vida - danio;
}

void Personaje::gastar_energia(int cantidad) {
    if (cantidad <= 0) {
        return;
    }
    energia = (cantidad >= energia) ? 0 : energia - cantidad;
}

ActualizacionInventario Personaje::actualizacion_de_inventario() const {
    ActualizacionInventario actualizacion;
    actualizacion.inventario.id_jugador = id_jugador;
    actualizacion.inventario.ranura_activa = ranura_activa;
    actualizacion.inventario.ranuras = inventario.todas_las_ranuras();
    return actualizacion;
}
