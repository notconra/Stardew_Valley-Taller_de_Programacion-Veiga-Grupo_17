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

    // Devuelve la categoria del item.
    // Lanza invalid_argument si el tipo de item es desconocido.
    static Categoria categoria_de(TipoItem tipo);

public:
    // Devuelve true si el tipo del item es CULTIVO. false en caso contrario.
    static bool tiene_calidad(TipoItem tipo);

    // Devuelve 1 para las herramientas y TOPE_PILA_GENERAL para el resto de los items.
    static uint8_t tope_de_pila(TipoItem tipo);

    // Devuelve true si el tipo del item es HERRAMIENTA. false en caso contrario.
    static bool es_herramienta(TipoItem tipo);

    explicit Item(TipoItem tipo);

    Item(TipoItem tipo, Calidad calidad);

    // Devuelve el tipo del item.
    TipoItem obtener_tipo() const;

    //Devuelve la calidad del item.
    std::optional<Calidad> obtener_calidad() const;

    // Devuelve el tope de pila del item.
    uint8_t tope_de_pila() const;

    //Devuelve true si el item es una herramienta. false en caso contrario.
    bool es_herramienta() const;

    bool operator==(const Item& otro) const = default;
};

#endif
