#ifndef AZAR_H
#define AZAR_H

#include <random>

// Sorteos del juego. Un solo generador para todo el proceso: nada de esto tiene que ser
// reproducible, así que no hace falta uno por clase.
namespace Azar {

inline std::mt19937& motor() {
    static std::mt19937 generador{std::random_device{}()};
    return generador;
}

// Un entero uniforme en [minimo, maximo].
inline int entero(int minimo, int maximo) {
    return std::uniform_int_distribution<int>(minimo, maximo)(motor());
}

// true con la probabilidad dada (0.0 nunca, 1.0 siempre).
inline bool con_probabilidad(double probabilidad) {
    return std::uniform_real_distribution<double>(0.0, 1.0)(motor()) < probabilidad;
}

}  // namespace Azar

#endif
