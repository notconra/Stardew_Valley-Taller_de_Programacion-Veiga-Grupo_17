#ifndef GESTOR_DE_SECTORES_H
#define GESTOR_DE_SECTORES_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../mapa/mapa.h"
#include "../notificador.h"
#include "../recursos/recursos_de_juego.h"
#include "sector.h"

struct JugadorEnTransicion {
    uint32_t id_jugador;
    Transicion transicion;
};

// Dueño de todos los sectores (mapas) de la partida. Es el único que sabe cómo mover a un
// personaje de un sector a otro, así ningún otro sistema repite esa lógica. También es el que arma
// las instantáneas y los avisos de cada sector, porque es el único que conoce a todos sus
// jugadores.
class GestorDeSectores {
private:
    Notificador& notificador;
    const std::map<uint32_t, Personaje>& personajes;
    std::map<std::string, Sector> sectores;
    std::map<std::string, std::vector<EstadoJugador>> jugadores_enviados;

    const int ticks_entre_refrescos;
    int ticks_desde_el_refresco = 0;

    void enviar_instantanea(uint32_t id_jugador, const std::string& id_mapa);

    // Manda JugadoresEnSector a los jugadores del sector, pero solo si cambió respecto del último
    // envío o si "forzar" es true.
    void difundir(const std::string& id_mapa, bool forzar);

public:
    // Cada sector usa por referencia el mapa que ya está cargado en "recursos", que tiene que
    // vivir más que el gestor.
    GestorDeSectores(
        const RecursosDeJuego& recursos, uint8_t ticks_por_segundo,
        const std::map<uint32_t, Personaje>& personajes, Notificador& notificador);

    GestorDeSectores(const GestorDeSectores&) = delete;
    GestorDeSectores& operator=(const GestorDeSectores&) = delete;

    bool existe(const std::string& id_mapa) const;

    // Los ids de todos los mapas con sector, para quien necesite recorrerlos todos (por ejemplo,
    // ControladorDeEnemigos). Sin orden garantizado.
    std::vector<std::string> ids_de_sectores() const;

    // Lanzan std::out_of_range si el sector no existe.
    Sector& en(const std::string& id_mapa);
    const Sector& en(const std::string& id_mapa) const;

    // Un jugador que entra al mundo, aparece en el punto de aparición del mapa indicado.
    void colocar_en_aparicion(Personaje& personaje, const std::string& id_mapa);

    // Lo saca de su sector actual y lo pone en (x, y) píxeles del destino.
    void trasladar(Personaje& personaje, const std::string& destino, double x, double y);

    // Igual, pero indicando la casilla de llegada.
    void trasladar_a_casilla(
        Personaje& personaje, const std::string& destino, int columna, int fila);

    // Lo saca de su sector (se desconectó). No hace nada si su cuerpo ya no estaba.
    void quitar(const Personaje& personaje);

    // Un tick de física en todos los sectores.
    void actualizar();

    // Quién está pisando una transición, en cualquier sector. Se llama después de actualizar().
    std::vector<JugadorEnTransicion> jugadores_en_transicion() const;

    // Posición y estado de los jugadores de un sector, listos para mandar.
    std::vector<EstadoJugador> armar_jugadores(const std::string& id_mapa) const;

    // Instantánea completa de un sector: jugadores, objetos y drops.
    EstadoJuego armar_instantanea(const std::string& id_mapa) const;

    // Avisa los jugadores de ese sector si cambiaron desde el último aviso.
    void difundir_jugadores(const std::string& id_mapa);

    // Lo mismo para todos los sectores. Se llama una vez por tick, con los jugadores quietos no
    // viaja nada, salvo el refresco de seguridad de una vez por segundo.
    void difundir_jugadores();
};

#endif
