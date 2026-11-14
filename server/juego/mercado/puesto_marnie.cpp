#include "puesto_marnie.h"

#include <string>

PuestoMarnie::PuestoMarnie(const ConfiguracionMercadoServidor& config)
    : PuestoDeMercado("marnie", config, {}) {
    agregar_animal(
        TipoAnimal::VACA,
        "Vaca",
        "Produce leche cada 2 dias, avisa cuando esta lista para ordenar.",
        "Leche",
        TipoItem::LECHE);
    agregar_animal(
        TipoAnimal::GALLINA,
        "Gallina",
        "Pone huevos todos los dias.",
        "Huevo",
        TipoItem::HUEVO);
}

void PuestoMarnie::agregar_animal(
    TipoAnimal tipo, const std::string& nombre, const std::string& cuidados,
    const std::string& nombre_producto, TipoItem producto) {
    const ConfiguracionAnimalServidor& datos = *animal(tipo);
    const std::string descripcion =
        cuidados + "\n" + nombre_producto + ": se vende a " +
        std::to_string(precio_base_que_recibe_el_jugador(producto)) +
        " monedas.\nMaximo por granja: " + std::to_string(datos.tope_por_granja) + ".";
    catalogo.push_back({TipoItem::NINGUNO, tipo, nombre, descripcion, datos.precio_de_compra});
}

const ConfiguracionAnimalServidor* PuestoMarnie::animal(TipoAnimal tipo) const {
    if (tipo == TipoAnimal::VACA) {
        return &config.vaca;
    }
    if (tipo == TipoAnimal::GALLINA) {
        return &config.gallina;
    }
    return nullptr;
}
