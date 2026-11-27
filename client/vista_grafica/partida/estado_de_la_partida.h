#ifndef ESTADO_DE_LA_PARTIDA_H
#define ESTADO_DE_LA_PARTIDA_H

#include <cstdint>
#include <map>
#include <optional>
#include <utility>

#include "../../../common/mensajes/mensajes.h"

// Lo que el cliente sabe de la partida porque se lo mando el servidor. Solo guarda y actualiza
// datos: no dibuja, no hace sonar nada ni decide reglas.
struct EstadoDeLaPartida {
    EstadoJuego mundo{};
    InventarioJugador inventario{};
    CofreJugador cofre{};
    std::map<std::pair<int, int>, CasillaTratada> tierra_tratada;
    std::optional<HoraDelJuego> hora;
    JugadoresDeLaPartida jugadores{};
    // Donde esta cada jugador conectado, para el mapa general.
    PosicionesDeLosJugadores posiciones{};
    ResumenAnimalesGranja resumen_animales{};
    RegalosPendientes regalos{};
    // El ultimo estado de cada lote del centro comunitario.
    std::map<TipoLote, LoteDelCentro> lotes_del_centro;

    // Reemplaza el mundo entero (al entrar o cambiar de sector) y su tierra.
    void reemplazar_mundo(const EstadoJuego& nuevo);
    // Cambia los jugadores del sector; si es otro sector, descarta lo del anterior.
    void actualizar_jugadores(const JugadoresEnSector& evento);
    void cambiar_tierra(const TierraCambiada& evento);

    void colocar_objeto(const ObjetoColocado& evento);
    // Devuelve true si lo retirado era un mineral.
    bool retirar_objeto(const ObjetoRetirado& evento);
    void retirar_drop(const DropRetirado& evento);

    // Devuelve como estaba antes, o nada si es nuevo.
    std::optional<EstadoEnemigo> actualizar_enemigo(const EstadoEnemigo& enemigo);
    // Devuelve true si la trampa se acaba de encender.
    bool actualizar_trampa(const EstadoTrampa& trampa);

    void actualizar_animal(const EstadoAnimal& animal);
    void retirar_animal(const AnimalRetirado& evento);
    void marcar_vaca(const VacaOrdeniable& evento);
};

#endif
