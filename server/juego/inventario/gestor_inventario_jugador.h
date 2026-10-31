#ifndef GESTOR_INVENTARIO_JUGADOR_H
#define GESTOR_INVENTARIO_JUGADOR_H

#include <cstdint>
#include <map>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../objetos/catalogo_de_items.h"
#include "inventario.h"

// Sistema de inventario de una partida.
// Un Inventario y una ranura activa por jugador.
class GestorInventarioJugador {
private:
    // Lo que tiene cada jugador, su barra y cuál de sus ranuras está seleccionada.
    struct EstadoJugadorInventario {
        Inventario inventario;
        uint8_t ranura_activa = 0;

        EstadoJugadorInventario(
            const CatalogoDeItems& catalogo_de_items, uint8_t cantidad_de_ranuras)
            : inventario(catalogo_de_items, cantidad_de_ranuras) {
        }
    };

    const CatalogoDeItems& catalogo;
    const uint8_t cantidad_ranuras_barra;
    std::map<uint32_t, EstadoJugadorInventario> estados;

    EstadoJugadorInventario* buscar(uint32_t id_jugador);
    const EstadoJugadorInventario* buscar(uint32_t id_jugador) const;

    void crear_para_jugador(uint32_t id_jugador);
    void insertar_items_iniciales(Inventario& inventario_nuevo);

    // INDICE_RANURA_ACTIVA se resuelve a la ranura activa del jugador.
    uint8_t resolver_ranura(const EstadoJugadorInventario& estado, uint8_t indice_ranura) const;

public:
    GestorInventarioJugador(
        const CatalogoDeItems& catalogo_de_items,
        const ConfiguracionInventarioServidor& configuracion);

    // Crea inventarios para los conectados y descarta los de quienes se fueron.
    void sincronizar_conectados(const std::vector<uint32_t>& ids_conectados);

    bool seleccionar_ranura_de_la_barra(uint32_t id_jugador, uint8_t indice_ranura);

    // Mueve ítems dentro de la barra. Si el destino es el tacho, los elimina.
    bool mover_item(uint32_t id_jugador, uint8_t origen, uint8_t destino, uint16_t cantidad);

    // Copia de la ranura (INDICE_RANURA_ACTIVA es la activa). Si el jugador o el índice no
    // existen, devuelve una ranura vacía. Es lo que consulta Mundo para decidir qué hace UsarItem.
    RanuraInventario ranura(uint32_t id_jugador, uint8_t indice_ranura) const;

    // Quita cantidad de una ranura. Devuelve false si no había tanto. Es lo que consume el ítem
    // cuando se coloca un cofre.
    bool quitar_de_ranura(uint32_t id_jugador, uint8_t indice_ranura, uint16_t cantidad);

    // El inventario del jugador, para que otros sistemas (los cofres) transfieran ítems. Devuelve
    // nullptr si el jugador no está. Usarlo en el momento: no guardarlo entre ticks.
    Inventario* inventario(uint32_t id_jugador);

    uint8_t obtener_ranura_activa(uint32_t id_jugador) const;

    // Sirve para mandar por la red al dueño del inventario.
    InventarioJugador obtener_inventario_para_enviar(uint32_t id_jugador) const;
};

#endif
