#ifndef PUESTO_DE_MERCADO_H
#define PUESTO_DE_MERCADO_H

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/tipo_item.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../jugadores/personaje.h"
#include "precios_del_dia.h"

struct ProductoEnVenta {
    TipoItem tipo_item = TipoItem::NINGUNO;
    TipoAnimal tipo_animal = TipoAnimal::NINGUNO;
    std::string nombre;
    std::string descripcion;
    uint32_t precio_que_paga_el_jugador = 0;

    bool es_animal() const {
        return tipo_animal != TipoAnimal::NINGUNO;
    }
};

// Base de los cuatro comerciantes. Cada uno define qué vende (catalogo) y qué le compra al jugador
// (items_que_acepta). Los precios salen siempre de ConfiguracionMercadoServidor.
// Los precios se expresan desde el punto de vista del jugador.
class PuestoDeMercado {
private:
    static constexpr uint32_t PORCENTAJE_BASE = 100;

    std::string id_puesto;
    std::set<TipoItem> items_que_acepta;

    // Lanza std::runtime_error si el ítem no tiene precio en esa lista.
    uint32_t precio_en(const std::map<TipoItem, uint32_t>& precios, TipoItem tipo) const;

    // Multiplicador de precio por calidad, en porcentaje (según el enunciado serían 1, 1,25, 2, 4 y
    // 8).
    uint32_t porcentaje_por_calidad(Calidad calidad) const;
    uint16_t cantidad_poseida(
        const std::map<TipoAnimal, uint16_t>& animales, TipoAnimal tipo) const;

    ItemCatalogoInfo armar_info(
        const Personaje& personaje, const ProductoEnVenta& producto,
        const std::map<TipoAnimal, uint16_t>& animales_jugador, const PreciosDelDia& precios) const;

protected:
    const ConfiguracionMercadoServidor& config;
    std::vector<ProductoEnVenta> catalogo;

    // Lanza std::runtime_error si algún ítem que el puesto acepta no tiene precio de venta.
    PuestoDeMercado(
        std::string id_puesto, const ConfiguracionMercadoServidor& config,
        std::set<TipoItem> items_que_acepta);

    uint32_t precio_que_paga_el_jugador(TipoItem tipo) const;
    uint32_t precio_base_que_recibe_el_jugador(TipoItem tipo) const;

public:
    const std::string& id() const {
        return id_puesto;
    }

    virtual std::vector<ProductoEnVenta> obtener_productos_en_venta(uint32_t dia) const;

    virtual const ConfiguracionAnimalServidor* animal(TipoAnimal tipo) const;

    bool acepta_item_del_jugador(TipoItem tipo) const;

    uint32_t precio_que_recibe_el_jugador(TipoItem tipo, Calidad calidad) const;

    // Mensaje completo del catálogo, listo para enviar al cliente, con los precios de hoy.
    ActualizacionMercado armar_actualizacion(
        const Personaje& personaje, uint32_t dia,
        const std::map<TipoAnimal, uint16_t>& animales_jugador, const PreciosDelDia& precios) const;

    virtual ~PuestoDeMercado() = default;
};

#endif
