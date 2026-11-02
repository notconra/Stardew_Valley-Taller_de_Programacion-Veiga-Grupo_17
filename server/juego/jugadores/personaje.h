#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <cstdint>
#include <string>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_accion.h"
#include "../inventario/inventario.h"
#include "../notificador.h"
#include "../objetos/catalogo_de_items.h"

// Datos lógicos de un jugador en la partida como su inventario, qué ranura tiene elegida, qué cofre
// está mirando y qué acción está ejecutando. El cuerpo (posición, dirección, colisiones) vive en
// Jugadores, dentro del Sector donde esté. Acá podría ir también lo que maneje las stats y la plata
// (?)
//
// Acá también están las pocas reglas del inventario que necesitan saber de la barra y del tacho.
// Todo lo demás del inventario lo resuelve Inventario.
struct Personaje {
    uint32_t id_jugador = 0;
    std::string mapa_actual;
    int id_cofre_abierto = 0;  // 0 si no tiene ningún cofre abierto
    std::string mapa_cofre_abierto;
    Inventario inventario;
    uint8_t ranura_activa = 0;
    TipoAccion accion_actual = TipoAccion::NINGUNA;
    uint16_t ticks_restantes_accion = 0;

    Personaje(
        uint32_t id, const std::string& mapa_inicial, const CatalogoDeItems& catalogo,
        uint8_t cantidad_ranuras_barra);

    // Traduce el índice que mandó el cliente (INDICE_RANURA_ACTIVA o una ranura real).
    uint8_t resolver_ranura(uint8_t indice_ranura) const;

    // Una copia de la ranura, ya resuelta la ranura activa. Si el índice no existe, viene vacía.
    RanuraInventario ranura(uint8_t indice_ranura) const;

    bool seleccionar_ranura(uint8_t indice_ranura);

    // Mueve entre ranuras de la barra. Si el destino es el tacho, elimina el ítem.
    bool mover_item(uint8_t origen, uint8_t destino, uint16_t cantidad);

    bool quitar_de_ranura(uint8_t indice_ranura, uint16_t cantidad);

    bool esta_ocupado() const;

    // El mensaje con la barra completa tal como lo espera el cliente. Se manda con
    // notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario()).
    ActualizacionInventario actualizacion_de_inventario() const;

    Personaje(const Personaje&) = delete;
    Personaje& operator=(const Personaje&) = delete;
    Personaje(Personaje&&) = default;
    Personaje& operator=(Personaje&&) = default;
};

#endif
