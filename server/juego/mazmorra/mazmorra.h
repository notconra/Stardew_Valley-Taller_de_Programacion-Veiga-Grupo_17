#ifndef MAZMORRA_H
#define MAZMORRA_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <vector>

#include "nivel.h"

inline constexpr int CANTIDAD_NIVELES = 10;

// Los niveles de la mazmorra de una partida y en cuál está cada jugador. La geografía de los
// niveles no cambia durante la partida, pero el contenido (enemigos y objetos) se vuelve a
// sortear cada vez que se entra con la mazmorra vacía.
class Mazmorra {
public:
    using IdJugador = uint32_t;

private:
    std::array<std::optional<Nivel>, CANTIDAD_NIVELES> niveles;
    std::map<IdJugador, size_t> nivel_de_jugador;
    std::mt19937 generador;
    int enemigos_por_nivel;
    int objetos_por_nivel;
    double probabilidad_de_cofre;

    size_t indice_de(IdJugador jugador) const;
    size_t indice_de_nivel(size_t numero) const;

public:
    Mazmorra(
        std::vector<GeografiaNivel> geografias, int enemigos_por_nivel, int objetos_por_nivel,
        double probabilidad_de_cofre, std::mt19937 generador);

    Mazmorra(const Mazmorra&) = delete;
    Mazmorra& operator=(const Mazmorra&) = delete;

    /*
     * Entra a la mazmorra siempre desde el nivel 1. Si no quedaba nadie adentro, vuelve a sortear
     * los enemigos y las casillas de objetos de todos los niveles.
     */
    void entrar(IdJugador jugador);

    /*
     * El jugador usa la escalera y pasa al siguiente nivel.
     * No verifica la posición: el gameloop la llama cuando detecta por colisión que el jugador esta
     * en la escalera.
     */
    bool bajar(IdJugador jugador);

    /*
     * El jugador usa la puerta de salida y vuelve al pueblo.
     * No verifica la posición: el gameloop la llama cuando detecta por colisión que el jugador esta
     * en la puerta de salida.
     */
    void salir(IdJugador jugador);

    bool esta_adentro(IdJugador jugador) const;

    bool esta_vacia() const;

    /*
     * Devuelve el número del nivel actual del jugador.
     */
    size_t numero_de_nivel(IdJugador jugador) const;

    /*
     * Nivel donde esta el jugador actualmente, para que el gameloop pueda chequear la escalera, la
     * salida y la entrada.
     */
    const Nivel& nivel_actual_del_jugador(IdJugador jugador) const;

    /*
     * El nivel indicado, contando desde 1. Lanza std::out_of_range si el número no existe.
     */
    const Nivel& nivel(size_t numero) const;

    /*
     * Devuelve los ids de los jugadores que estan en el nivel indicado.
     * Lanza std::out_of_range si el número no existe.
     */
    std::vector<IdJugador> jugadores_en_nivel(size_t numero) const;
};

#endif
