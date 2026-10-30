#ifndef CATALOGO_DE_ITEMS_H
#define CATALOGO_DE_ITEMS_H

#include <cstdint>
#include <map>
#include <string>

#include "../../../common/juego/tipo_item.h"

// Categoría de un ítem. Sirve para las reglas generales del catálogo, las herramientas no se
// apilan ni se tiran, los cultivos llevan calidad y los colocables se pueden poner en el mapa.
enum class CategoriaItem : uint8_t {
    HERRAMIENTA,
    SEMILLA,
    CULTIVO,
    RECURSO,
    COLOCABLE,
};

// Datos inmutables de un ítem. Todo lo que el servidor necesita saber para gobernar el
// inventario sin crear una clase por ítem. El tipo no se guarda acá, es la clave del catálogo.
struct DefinicionItem {
    std::string nombre;
    CategoriaItem categoria = CategoriaItem::RECURSO;
    uint16_t maximo_apilable = 1;
    bool se_puede_eliminar = true;
    uint16_t cantidad_inicial = 0;
};

// Qué ítems existen y sus reglas. Se arma una vez y después solo se consulta.
class CatalogoDeItems {
private:
    std::map<TipoItem, DefinicionItem> items;

    void registrar(
        TipoItem tipo, const std::string& nombre, CategoriaItem categoria, uint16_t maximo_apilable,
        bool se_puede_eliminar, uint16_t cantidad_inicial);

public:
    CatalogoDeItems();

    // Devuelve nullptr si el tipo no está registrado (NINGUNO nunca lo está).
    const DefinicionItem* buscar(TipoItem tipo) const;

    // Todas las definiciones, ordenadas por tipo.
    const std::map<TipoItem, DefinicionItem>& definiciones() const;

    uint16_t maximo_apilable(TipoItem tipo) const;
    bool es_herramienta(TipoItem tipo) const;
    bool se_puede_eliminar(TipoItem tipo) const;
    bool tiene_calidad(TipoItem tipo) const;
};

#endif
