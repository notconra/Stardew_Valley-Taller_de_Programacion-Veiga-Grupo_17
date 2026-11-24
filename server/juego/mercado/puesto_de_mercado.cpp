#include "puesto_de_mercado.h"

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

PuestoDeMercado::PuestoDeMercado(
    std::string id_puesto, const ConfiguracionMercadoServidor& config,
    std::set<TipoItem> items_que_acepta)
    : id_puesto(std::move(id_puesto)),
      items_que_acepta(std::move(items_que_acepta)),
      config(config) {
    for (TipoItem tipo : this->items_que_acepta) {
        precio_base_que_recibe_el_jugador(tipo);
    }
}

uint32_t PuestoDeMercado::precio_en(
    const std::map<TipoItem, uint32_t>& precios, TipoItem tipo) const {
    const std::map<TipoItem, uint32_t>::const_iterator precio = precios.find(tipo);
    if (precio == precios.end()) {
        throw std::runtime_error(
            "mercado: al puesto '" + id_puesto + "' le falta el precio del ítem " +
            std::to_string(static_cast<int>(tipo)) + " en la configuración");
    }
    return precio->second;
}

uint32_t PuestoDeMercado::porcentaje_por_calidad(Calidad calidad) const {
    switch (calidad) {
        case Calidad::BUENO:
            return 125;
        case Calidad::EXCELENTE:
            return 200;
        case Calidad::INCREIBLE:
            return 400;
        case Calidad::PERFECTA:
            return 800;
        case Calidad::BASE:
            break;
    }
    return PORCENTAJE_BASE;
}

uint16_t PuestoDeMercado::cantidad_poseida(
    const std::map<TipoAnimal, uint16_t>& animales, TipoAnimal tipo) const {
    const std::map<TipoAnimal, uint16_t>::const_iterator poseidos = animales.find(tipo);
    return (poseidos != animales.end()) ? poseidos->second : 0;
}

uint32_t PuestoDeMercado::precio_que_paga_el_jugador(TipoItem tipo) const {
    return precio_en(config.precios_de_compra, tipo);
}

uint32_t PuestoDeMercado::precio_base_que_recibe_el_jugador(TipoItem tipo) const {
    return precio_en(config.precios_de_venta, tipo);
}

std::vector<ProductoEnVenta> PuestoDeMercado::obtener_productos_en_venta(uint32_t) const {
    return catalogo;
}

const ConfiguracionAnimalServidor* PuestoDeMercado::animal(TipoAnimal) const {
    return nullptr;
}

bool PuestoDeMercado::acepta_item_del_jugador(TipoItem tipo) const {
    return items_que_acepta.count(tipo) > 0;
}

uint32_t PuestoDeMercado::precio_que_recibe_el_jugador(TipoItem tipo, Calidad calidad) const {
    if (!acepta_item_del_jugador(tipo)) {
        return 0;
    }
    return precio_base_que_recibe_el_jugador(tipo) * porcentaje_por_calidad(calidad) /
           PORCENTAJE_BASE;
}

ItemCatalogoInfo PuestoDeMercado::armar_info(
    const Personaje& personaje, const ProductoEnVenta& producto,
    const std::map<TipoAnimal, uint16_t>& animales_jugador, const PreciosDelDia& precios) const {
    ItemCatalogoInfo info;
    info.tipo_item = producto.tipo_item;
    info.tipo_animal = producto.tipo_animal;
    info.nombre = producto.nombre;
    info.precio = precios.de_compra(producto.precio_que_paga_el_jugador);
    info.descripcion = producto.descripcion;

    const bool alcanzan_las_monedas = personaje.monedas >= info.precio;
    if (producto.es_animal()) {
        info.cantidad_poseida = cantidad_poseida(animales_jugador, producto.tipo_animal);
        info.se_puede_comprar =
            alcanzan_las_monedas &&
            info.cantidad_poseida < animal(producto.tipo_animal)->tope_por_granja;
    } else {
        info.cantidad_poseida = 0;
        info.se_puede_comprar =
            alcanzan_las_monedas &&
            personaje.inventario.puede_agregar(producto.tipo_item, 1, Calidad::BASE);
    }
    return info;
}

ActualizacionMercado PuestoDeMercado::armar_actualizacion(
    const Personaje& personaje, uint32_t dia,
    const std::map<TipoAnimal, uint16_t>& animales_jugador, const PreciosDelDia& precios) const {
    ActualizacionMercado msg;
    msg.id_npc = id_puesto;

    for (const ProductoEnVenta& producto : obtener_productos_en_venta(dia)) {
        msg.catalogo.push_back(armar_info(personaje, producto, animales_jugador, precios));
    }
    return msg;
}
