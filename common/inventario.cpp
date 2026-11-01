#include "inventario.h"
 
#include <stdexcept>

Inventario::Inventario(size_t capacidad): casilleros(capacidad) {}

size_t Inventario::capacidad() const {
    return casilleros.size();
}

uint32_t Inventario::cantidad_de(const Item& item) const {
    uint32_t total = 0;
    for (const std::optional<PilaItem>& contenido: casilleros) {
        if (contenido && contenido->item == item) {
            total += contenido->cantidad;
        }
    }
    return total;
}

const std::optional<PilaItem>& Inventario::obtener_espacio(size_t indice) const {
    return casilleros.at(indice);
}

bool Inventario::puede_agregar(const Item& item, uint32_t cantidad) const {
    if (cantidad == 0) {
        throw std::invalid_argument("no se puede agregar una cantidad de 0 unidades");
    }
 
    const uint8_t tope = item.tope_de_pila();
    uint32_t disponible = 0;
    for (const std::optional<PilaItem>& contenido: casilleros) {
        if (!contenido) {
            disponible += tope;
        } else if (contenido->item == item) {
            disponible += static_cast<uint32_t>(tope - contenido->cantidad);
        }
    }
    return disponible >= cantidad;
}

bool Inventario::agregar(const Item& item, uint32_t cantidad) {
    if (!puede_agregar(item, cantidad)) {
        return false;
    }
 
    const uint8_t tope = item.tope_de_pila();
    uint32_t restante = cantidad;
 
    for (std::optional<PilaItem>& contenido: casilleros) {
        if (restante == 0) {
            break;
        }
        if (contenido && contenido->item == item && contenido->cantidad < tope) {
            const uint32_t lugar = static_cast<uint32_t>(tope - contenido->cantidad);
            const uint32_t agregado = std::min(lugar, restante);
            contenido->cantidad = static_cast<uint8_t>(contenido->cantidad + agregado);
            restante -= agregado;
        }
    }
 
    for (std::optional<PilaItem>& contenido: casilleros) {
        if (restante == 0) {
            break;
        }
        if (!contenido) {
            const uint32_t agregado = std::min(static_cast<uint32_t>(tope), restante);
            contenido = PilaItem{item, static_cast<uint8_t>(agregado)};
            restante -= agregado;
        }
    }
 
    return true;
}

 
bool Inventario::sacar(size_t indice, uint8_t cantidad) {
    if (cantidad == 0) {
        throw std::invalid_argument("no se puede sacar una cantidad de 0 unidades");
    }
 
    std::optional<PilaItem>& contenido = casilleros.at(indice);
    if (!contenido || contenido->cantidad < cantidad) {
        return false;
    }
 
    contenido->cantidad = static_cast<uint8_t>(contenido->cantidad - cantidad);
    if (contenido->cantidad == 0) {
        contenido.reset();
    }
    return true;
}
 
bool Inventario::eliminar(const Item& item, uint32_t cantidad) {
    if (cantidad == 0) {
        throw std::invalid_argument("no se puede eliminar una cantidad de 0 unidades");
    }
    if (item.es_herramienta()) {
        return false;
    }

    if (cantidad_de(item) < cantidad) {
        return false;
    }
 
    uint32_t restante = cantidad;
    for (std::optional<PilaItem>& contenido: casilleros) {
        if (restante == 0) {
            break;
        }
        if (contenido && contenido->item == item) {
            const uint32_t quitado = std::min(static_cast<uint32_t>(contenido->cantidad), restante);
            contenido->cantidad = static_cast<uint8_t>(contenido->cantidad - quitado);
            restante -= quitado;
            if (contenido->cantidad == 0) {
                contenido.reset();
            }
        }
    }
    return true;
}
 
bool Inventario::transferir_a(Inventario& destino, size_t indice, uint8_t cantidad) {
    if (&destino == this) {
        throw std::invalid_argument("no se puede transferir a un inventario hacia si mismo");
    }
    if (cantidad == 0) {
        throw std::invalid_argument("no se puede transferir una cantidad de 0 unidades");
    }
 
    const std::optional<PilaItem>& contenido = obtener_espacio(indice);
    if (!contenido || contenido->cantidad < cantidad) {
        return false;
    }

    const Item item = contenido->item;
 
    if (!destino.puede_agregar(item, cantidad)) {
        return false;
    }

    sacar(indice, cantidad);
    destino.agregar(item, cantidad);
    return true;
}
 