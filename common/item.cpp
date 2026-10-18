#include "item.h"

#include <stdexcept>
#include <string>

Item::Categoria Item::categoria_de(TipoItem tipo) {
    switch (tipo) {
        case PICO:
        case REGADERA:
        case PALA:
        case ESPADA:
            return HERRAMIENTA;
        case FRUTILLA:
        case CEBOLLA_DE_VERDEO:
        case PAPA:
        case CALABAZA:
            return SEMILLA;
        case LECHE:
        case HUEVO:
            return PRODUCTO_ANIMAL;
        case CARBON:
        case LAPISLAZULI:
        case PLATA:
        case ORO:
            return MINERAL;
        case CAMA:
        case COFRE:
            return COLOCABLE;
    }
    throw std::invalid_argument("Tipo de item desconocido: " + std::to_string(tipo));
}

bool Item::tiene_calidad(TipoItem tipo) {
    return categoria_de(tipo) == CULTIVO;
}

bool Item::es_herramienta(TipoItem tipo) {
    return categoria_de(tipo) == HERRAMIENTA;
}

uint8_t Item::tope_de_pila(TipoItem tipo) {
    return es_herramienta(tipo) ? 1 : TOPE_PILA_GENERAL;
}

Item::Item(TipoItem tipo): tipo(tipo), calidad(std::nullopt) {
    if (tiene_calidad(tipo)) {
        throw std::invalid_argument("el tipo de item " + std::to_string(tipo) + " requiere calidad y no se indico ninguna");
    }
}

Item::Item(TipoItem tipo, Calidad calidad): tipo(tipo), calidad(calidad) {
    if (!tiene_calidad(tipo)) {
        throw std::invalid_argument("el tipo de item " + std::to_string(tipo) + " no lleva calidad");
    }
}

TipoItem Item::obtener_tipo() const { 
    return tipo; 
}
 
std::optional<Calidad> Item::obtener_calidad() const { 
    return calidad; 
}
 
uint8_t Item::tope_de_pila() const { 
    return tope_de_pila(tipo); 
}
 
bool Item::es_herramienta() const { 
    return es_herramienta(tipo); 
}
