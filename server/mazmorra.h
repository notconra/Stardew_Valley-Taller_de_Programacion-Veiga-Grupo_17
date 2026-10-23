#ifndef MAZMORRA_H
#define MAZMORRA_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>
#include "nivel.h"

#define CANTIDAD_NIVELES 10
class Mazmorra {
private:
    std::array<std::optional<Nivel>, CANTIDAD_NIVELES> niveles;
    std::optional<size_t> nivel_actual;
    std::mt19937 generador;

public:

    Mazmorra(std::vector<GeografiaNivel> geos, std::mt19937 generador);

    Mazmorra(const Mazmorra&) = delete;
    Mazmorra& operator=(const Mazmorra&) = delete;

    /*
    * Entra a la mazmorra siempre desde el nivel 1.
    */
    void entrar();

    /*
    * El jugador usa la escalera y pasa al siguiente nivel.
    * No verifica la posición: el gameloop la llama cuando detecta por colisión que el jugador esta en la escalera.
    */
    bool bajar();


    /*
    * El jugador usa la puerta de salida y vuelve al pueblo.
    * No verifica la posición: el gameloop la llama cuando detecta por colisión que el jugador esta en la puerta de salida.
    */
    void salir();

    bool esta_adentro() const;


    /*
    * Devuelve el número del nivel actual del jugador.
    */
    size_t numero_de_nivel() const;

    /*
    * Nivel donde esta el jugador actualmente para que el gameloop pueda chequear colisiones con la escalera,
    * la salida.
    */
    const Nivel& nivel_actual_del_jugador() const;
};

#endif
