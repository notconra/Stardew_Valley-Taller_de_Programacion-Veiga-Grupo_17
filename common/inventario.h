#ifndef INVENTARIO_H
#define INVENTARIO_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "item.h" 

struct ItemStack {
    Item item;
    uint8_t cantidad;
};

class Inventario {
private:
    std::vector<std::optional<ItemStack>> espacios;

public:

explicit Inventario(size_t capacidad);

size_t capacidad() const;

bool se_puede_agregar(const Item& item, uint32_t cantidad) const;

uint32_t cantidad_de(const Item& item) const;

const std::optional<ItemStack>& obtener_espacio(size_t indice) const;

bool agregar(const Item& item, uint32_t cantidad);

bool sacar(size_t index, uint8_t quantity);

bool eliminar(const Item& item, uint32_t quantity);
};

#endif 
