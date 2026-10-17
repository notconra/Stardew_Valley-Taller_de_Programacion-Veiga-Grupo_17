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
private:
    TipoItem tipo;
    std::optional<Calidad> calidad;

public:
    explicit Item(TipoItem tipo);

    Item(TipoItem tipo, Calidad calidad);

    TipoItem obtener_tipo() const;
    std::optional<Calidad> obtener_calidad() const;


    uint8_t tope_de_pila() const;
    bool es_herramienta() const;
 
    bool operator==(const Item& otro) const = default;
};

#endif
