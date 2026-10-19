#ifndef ITEM_H
#define ITEM_H

#include <cstdint>
#include <optional>

#define TOPE_PILA_GENERAL 50

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
    enum Categoria {
        HERRAMIENTA,
        SEMILLA,
        CULTIVO,
        MINERAL,
        PRODUCTO_ANIMAL,
        COLOCABLE,
    };

    TipoItem tipo;
    std::optional<Calidad> calidad;

    static Categoria categoria_de(TipoItem tipo);

public:
    static bool tiene_calidad(TipoItem tipo);
    static uint8_t tope_de_pila(TipoItem tipo);
    static bool es_herramienta(TipoItem tipo);

    explicit Item(TipoItem tipo);

    Item(TipoItem tipo, Calidad calidad);

    TipoItem obtener_tipo() const;
    std::optional<Calidad> obtener_calidad() const;

    uint8_t tope_de_pila() const;
    bool es_herramienta() const;

    bool operator==(const Item& otro) const = default;
};

#endif
