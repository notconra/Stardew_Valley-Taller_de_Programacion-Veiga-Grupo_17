#ifndef MAZMORRA_H
#define MAZMORRA_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>
#include <map>

#include "nivel.h"

#define CANTIDAD_NIVELES 10
class Mazmorra {
public:
    using IdJugador = uint32_t;

private:
    std::array<std::optional<Nivel>, CANTIDAD_NIVELES> niveles;
    std::map<IdJugador, size_t> nivel_de_jugador;
    std::mt19937 generador;
    size_t indice_de(IdJugador jugador) const;

public:

    Mazmorra(std::vector<GeografiaNivel> geos, std::mt19937 generador);

    Mazmorra(const Mazmorra&) = delete;
    Mazmorra& operator=(const Mazmorra&) = delete;

    /*
    * Entra a la mazmorra siempre desde el nivel 1.
    */
    void entrar(IdJugador jugador);

    /*
    * El jugador usa la escalera y pasa al siguiente nivel.
    * No verifica la posición: el gameloop la llama cuando detecta por colisión que el jugador esta en la escalera.
    */
    bool bajar(IdJugador jugador);


    /*
    * El jugador usa la puerta de salida y vuelve al pueblo.
    * No verifica la posición: el gameloop la llama cuando detecta por colisión que el jugador esta en la puerta de salida.
    */
    void salir(IdJugador jugador);

    bool esta_adentro(IdJugador jugador) const;

    bool esta_vacia() const;

    /*
    * Devuelve el número del nivel actual del jugador.
    */
    size_t numero_de_nivel(IdJugador jugador) const;

    /*
    * Nivel donde esta el jugador actualmente para que el gameloop pueda chequear colisiones con la escalera,
    * la salida.
    */
    const Nivel& nivel_actual_del_jugador(IdJugador jugador) const;

    /*
    * Devuelve los ids de los jugadores que estan en el nivel indicado.
    * Lanza std::out_of_range si el número no existe.
    */
    std::vector<IdJugador> jugadores_en_nivel(size_t numero) const;
};

#endif
