#ifndef CONTROLADOR_DE_MAZMORRA_H
#define CONTROLADOR_DE_MAZMORRA_H

#include <optional>
#include <random>
#include <string>
#include <vector>

#include "../jugadores/personaje.h"
#include "../mapa/mapa.h"
#include "../recursos/recursos_de_juego.h"
#include "../sector/gestor_de_sectores.h"
#include "./../mazmorra/mazmorra.h"

// Todo lo procedural de la mazmorra como elegir los niveles al arrancar, poblarlos cuando entra el
// primer jugador, y resolver las transiciones especiales (entrada, escalera, salida). Mundo lo
// tiene como miembro y le ofrece cada transición y cada desconexión.
class ControladorDeMazmorra {
private:
    const RecursosDeJuego& recursos;
    GestorDeSectores& sectores;
    std::optional<Mazmorra> mazmorra;
    std::vector<std::string> niveles_de_mazmorra;  // id del mapa de cada nivel, en orden
    std::mt19937 generador_de_cofres;

    // Reparte los puntos de aparición sorteados del nivel: los primeros son grupos de orcos y el
    // resto esqueletos solos.
    void poblar_enemigos(Sector& sector, const Nivel& nivel);
    // Pone los pinches en casillas del piso lejos de la entrada, fuera de la escalera y la salida y
    // donde no haya un enemigo ni un mineral.
    void poblar_trampas(Sector& sector, const Nivel& nivel);
    // Las casillas libres más cercanas a "centro" (incluida), hasta "cantidad".
    std::vector<Casilla> casillas_para_grupo(
        const Sector& sector, const Nivel& nivel, const Casilla& centro, int cantidad,
        const std::vector<Casilla>& ocupadas) const;

    void poblar_niveles();
    void limpiar_drops_de_niveles();
    void retirar_cofres_de_nivel(Sector& sector);
    void colocar_cofre(Sector& sector, Casilla casilla);
    void salir_de_la_mazmorra(Personaje& personaje);
    void llevar_al_nivel_actual(Personaje& personaje);
    void entrar(Personaje& personaje);
    void bajar(Personaje& personaje);

public:
    ControladorDeMazmorra(
        const RecursosDeJuego& recursos, GestorDeSectores& sectores,
        const std::vector<std::string>& niveles_guardados);

    const std::vector<std::string>& obtener_niveles() const;

    // Se llama cuando un jugador toca una zona de transición. Devuelve true si el controlador se
    // hizo cargo (incluso para ignorarla). Si devuelve false, Mundo hace el viaje común entre
    // mapas. personaje.mapa_actual sigue siendo el mapa de origen.
    bool resolver_transicion(Personaje& personaje, const Transicion& transicion);

    // Se llama justo antes de sacar al jugador de la partida.
    void al_desconectarse(Personaje& personaje);
};

#endif
