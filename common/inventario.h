#ifndef INVENTARIO_H
#define INVENTARIO_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "item.h" 

struct PilaItem {
    Item item;
    uint8_t cantidad;
};

class Inventario {
private:
    std::vector<std::optional<PilaItem>> casilleros;

public:

explicit Inventario(size_t capacidad);

size_t capacidad() const;

bool se_puede_agregar(const Item& item, uint32_t cantidad) const;

uint32_t cantidad_de(const Item& item) const;

const std::optional<PilaItem>& obtener_espacio(size_t indice) const;

bool agregar(const Item& item, uint32_t cantidad);

bool sacar(size_t indice, uint8_t cantidad);

bool eliminar(const Item& item, uint32_t cantidad);

bool transferir_a(Inventario& destino, size_t indice, uint8_t);
};

#endif 
