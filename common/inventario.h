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

// Revusa si la cantidad de items entra dentro del inventario. No modifica el mismo.
// Devuelve true en caso de entraro todo, false en caso contrario. Tira invalid argument si la cantidad es 0.
bool puede_agregar(const Item& item, uint32_t cantidad) const;

//Cuenta la cantidad de items del mismo tipo y calidad que el item pasado por parametro.
uint32_t cantidad_de(const Item& item) const;

// Devuelve el contenido del casillero en el indice pasado.
// Lanza out_of_range si el indice es invalido.
const std::optional<PilaItem>& obtener_espacio(size_t indice) const;

// Agrega la cantidad de items al inventario. Devuelve true si se agrego todo, 
// false si no entran. De ser false no se modifica nada
// Lanza invalid_argument si la cantidad es 0.
bool agregar(const Item& item, uint32_t cantidad);

// Quita la cantidad de items del casillero indice (el que el jugador tenga seleccionado). 
// Devuelve false si el casillero esta vacio o tiene menos unidades. True en caso contrario.
// Lanza out_of_range si el indice es invalido. Lanza invalid_argument si la cantidad es 0.
bool sacar(size_t indice, uint8_t cantidad);

// Elimina la cantidad de unidades del item pasado. Elimina en el orden en el que se encuentren en el inventario.
// Devuelve true si alcanzaban. Devuelve false en caso contraro o si era una herramienta. En caso de false no modifica nada.
// Lanza invalid_argument si la cantidad es 0.
bool eliminar(const Item& item, uint32_t cantidad);

// Transfiere la cantdad de items del casillero indice a otro inventario.
// Devuelve false si no se puede transferir (no hay suficiente cantidad en el casillero o no hay lugar en el inventario destino). Propaga las excepciones de sacar().
// Devuelve true si se pudo transferir todo. En caso de false no modifica nada.
// Lanza invalid_argument si el inventario destino es el mismo que el origen.
bool transferir_a(Inventario& destino, size_t indice, uint8_t);
};

#endif 
