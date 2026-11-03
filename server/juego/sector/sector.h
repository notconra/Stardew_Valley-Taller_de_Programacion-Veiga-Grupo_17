#ifndef SECTOR_H
#define SECTOR_H

#include <cstdint>
#include <string>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_objeto.h"
#include "../cofres/gestor_de_cofres.h"
#include "../colisiones/colisiones.h"
#include "../drops/drops.h"
#include "../enemigos/gestor_de_enemigos.h"
#include "../jugadores/jugadores.h"
#include "../mapa/mapa.h"
#include "../notificador.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"
#include "../recursos/recursos_de_juego.h"

// Un mapa de la partida con todo lo que vive en él. El terreno (Mapa) es de solo lectura y se
// comparte entre todas las partidas: vive en RecursosDeJuego y acá solo se lo referencia. Los
// miembros se referencian entre sí (Colisiones, GestorDeObjetos y Jugadores guardan referencias a
// "mapa").
// Todo cambio del mundo que los clientes tienen que ver (colocar,
// retirar o golpear un objeto, soltar un drop) se hace con los métodos de acá, que avisan a los
// jugadores del sector. Las acciones no llaman a objetos ni a drops para modificar.
class Sector {
public:
    std::string id;
    const Mapa& mapa;
    Colisiones colisiones;
    GestorDeObjetos objetos;
    GestorDeCofres cofres;
    Jugadores jugadores;
    Drops drops;
    GestorDeEnemigos enemigos;

    // "recursos" tiene que vivir más que el Sector y tener el mapa "id_mapa" cargado.
    Sector(const std::string& id_mapa, const RecursosDeJuego& recursos, uint8_t ticks_por_segundo)
        : id(id_mapa),
          mapa(recursos.mapas.at(id_mapa)),
          colisiones(mapa),
          objetos(id, mapa, recursos.catalogo_de_objetos, colisiones),
          cofres(recursos.catalogo_de_items),
          jugadores(
              mapa, colisiones, ticks_por_segundo, recursos.juego.movimiento,
              recursos.juego.aparicion),
          enemigos(recursos.catalogo_de_enemigos) {
    }

    Sector(const Sector&) = delete;
    Sector& operator=(const Sector&) = delete;
    Sector(Sector&&) = delete;
    Sector& operator=(Sector&&) = delete;

    // El objeto que el jugador tiene adelante, o nullptr si no hay ninguno.
    const ObjetoDelMundo* objeto_al_frente(uint32_t id_jugador) const;

    // El cofre que el jugador tiene adelante, o nullptr si lo de enfrente no es un cofre.
    const ObjetoDelMundo* cofre_al_frente(uint32_t id_jugador) const;

    // Manda el evento a todos los jugadores que están en este sector.
    void avisar(Notificador& notificador, const EventoServidor& evento) const;

    // Coloca un objeto y avisa a los jugadores del sector. Devuelve el id del objeto, o 0 si la
    // casilla ya tiene un objeto o está bloqueada.
    int colocar_objeto(Notificador& notificador, TipoObjeto tipo, int columna, int fila);

    // Retira un objeto que existe y avisa a los jugadores del sector.
    void retirar_objeto(Notificador& notificador, int id_objeto);

    // Golpea un objeto con la herramienta. Si el golpe lo rompe, avisa que se retiró. El contenido
    // y los productos que deja los suelta quien llama, con soltar().
    ResultadoDeGolpe golpear_objeto(Notificador& notificador, int id_objeto, TipoItem herramienta);

    // Golpea a un enemigo con la espada y, si conectó, avisa a los jugadores del sector con
    // EnemigoActualizado (con vida en 0 si lo mató). El oro que suelta lo maneja quien llama, con
    // soltar().
    ResultadoDeGolpeEnemigo golpear_enemigo(Notificador& notificador, int id_enemigo, int danio);

    // Deja un ítem en el piso y avisa a los jugadores del sector. Devuelve el id del drop, o 0 si
    // la casilla está fuera del mapa o no había nada que dejar.
    int soltar(
        Notificador& notificador, TipoItem tipo, uint16_t cantidad, Calidad calidad, int columna,
        int fila);
};

#endif
