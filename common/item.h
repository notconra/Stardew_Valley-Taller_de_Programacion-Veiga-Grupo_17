#ifndef ITEM_H
#define ITEM_H

#include <cstdint>
#include <optional>

enum Calidad: uint8_t {
    BASE,
    BUENO,
    EXCELENTE,
    INCREIBLE,
    PERFECTA,
};

enum TipoItem : uint8_t {
    PICO,
    REGADERA,
    PALA,
    ESPADA,
    CARBON,
    LAPISLAZULI,
    PLATA,
    ORO,
    FRUTILLA,
    CEBOLLA_DE_VERDEO,
    PAPA,
    CALABAZA,
    LECHE,
    HUEVO,
    CAMA,
    COFRE,
};

struct Item {
    TipoItem tipo;
    std::optional<Calidad> calidad;

    bool operator==(const Item& otro) const = default;
};

uint8_t tope_de_pila(TipoItem tipo);
bool es_herramienta(TipoItem tipo);
bool tiene_calidad(TipoItem tipo);

#endif
