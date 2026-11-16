#include "puesto_pierre.h"

#include <string>

PuestoPierre::PuestoPierre(const ConfiguracionMercadoServidor& config)
    : PuestoDeMercado(
          "pierre", config,
          {TipoItem::PAPA,
           TipoItem::FRUTILLA,
           TipoItem::CEBOLLA_DE_VERDEO,
           TipoItem::CALABAZA,
           TipoItem::LECHE,
           TipoItem::HUEVO}) {
    agregar_semilla(
        TipoItem::SEMILLA_PAPA,
        "Semilla de papa",
        TipoItem::PAPA,
        "Se riega 2 veces por dia (tolera saltear alguno), y se cosecha a los 3 dias.");
    agregar_semilla(
        TipoItem::SEMILLA_FRUTILLA,
        "Semilla de frutilla",
        TipoItem::FRUTILLA,
        "Se riega 2 veces por dia, y se cosecha a los 4 dias, despues rinde una cosecha extra cada "
        "2 dias sin volver a plantar.");
    agregar_semilla(
        TipoItem::SEMILLA_CEBOLLA_DE_VERDEO,
        "Semilla de cebolla de verdeo",
        TipoItem::CEBOLLA_DE_VERDEO,
        "Se riega 3 veces por dia, y se cosecha a los 2 dias.");
    agregar_semilla(
        TipoItem::SEMILLA_CALABAZA,
        "Semilla de calabaza",
        TipoItem::CALABAZA,
        "Se riega 1 vez por dia, y se cosecha a los 7 dias.");
}

void PuestoPierre::agregar_semilla(
    TipoItem semilla, const std::string& nombre, TipoItem cultivo, const std::string& cuidados) {
    const std::string descripcion = cuidados + "\nPrecio de venta base de la cosecha: " +
                                    std::to_string(precio_base_que_recibe_el_jugador(cultivo)) +
                                    " monedas (se multiplica segun la calidad).";
    catalogo.push_back(
        {semilla, TipoAnimal::NINGUNO, nombre, descripcion, precio_que_paga_el_jugador(semilla)});
}
