#ifndef NIVEL_H
#define NIVEL_H

#include <cstddef>
#include <random>
#include <vector>

#include "../geometria.h"

// Una zona del nivel en casillas. (columna, fila) es la esquina superior izquierda.
struct Zona {
    Casilla origen;
    int ancho;
    int alto;

    bool contiene(Casilla casilla) const;
};

// Lo que Nivel necesita saber del mapa: qué casillas se pueden caminar y dónde están la entrada,
// la escalera y la salida. Se arma una sola vez a partir del Mapa ya cargado.
struct GeografiaNivel {
    int ancho;
    int alto;
    std::vector<bool> transitable;
    Casilla entrada;
    Zona escalera;
    Zona salida;
};

class Nivel {
private:
    GeografiaNivel geografia;
    std::vector<Casilla> enemigos;
    // Casillas donde el mundo puede poner objetos (minerales). Se eligen dejando siempre un camino
    // libre entre la entrada y la escalera.
    std::vector<Casilla> celdas_de_objetos;

    static size_t indice_de(const GeografiaNivel& datos, Casilla casilla);
    static bool esta_adentro(const GeografiaNivel& datos, Casilla casilla);
    static bool es_zona_especial(const GeografiaNivel& datos, Casilla casilla);
    static bool es_adyacente(Casilla casilla, const Zona& zona);
    void validar_zona(const Zona& zona, const char* nombre) const;

    // Casillas caminables a las que se llega desde la entrada, sin contar la entrada ni las zonas
    // especiales. Son las únicas donde tiene sentido generar algo.
    std::vector<Casilla> casillas_generables() const;

    // Si desde la entrada se llega a una casilla pegada a la escalera sin pisar las bloqueadas.
    bool hay_camino_a_la_escalera(const std::vector<bool>& bloqueadas) const;

public:
    /*
     * Casillas a las que se llega caminando desde `desde`, sin pasar por
     * la escalera ni la salida (así no se cuelan casillas de afuera de
     * la cueva que tocan la puerta). Incluye a `desde`.
     */
    static std::vector<Casilla> alcanzables(const GeografiaNivel& datos, Casilla desde);

    explicit Nivel(GeografiaNivel geografia_del_nivel);

    /*
     * Sortea los enemigos y las casillas donde se van a poner objetos, en casillas distintas y sin
     * que los objetos corten el paso entre la entrada y la escalera. Los objetos en sí los coloca
     * quien conoce el catálogo (Mundo), a partir de obtener_celdas_de_objetos().
     */
    void sortear(std::mt19937& generador, int cantidad_enemigos, int cantidad_objetos);

    const GeografiaNivel& obtener_geografia() const;
    const std::vector<Casilla>& obtener_enemigos() const;
    const std::vector<Casilla>& obtener_celdas_de_objetos() const;
};

#endif
