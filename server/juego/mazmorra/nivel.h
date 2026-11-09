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
    // Casillas donde el mundo puede poner objetos (minerales).
    std::vector<Casilla> celdas_de_objetos;
    // Casillas donde el mundo puede poner cofres. Como máximo hay una por nivel.
    std::vector<Casilla> celdas_de_cofres;

    static size_t indice_de(const GeografiaNivel& datos, Casilla casilla);
    static bool esta_adentro(const GeografiaNivel& datos, Casilla casilla);
    static bool es_zona_especial(const GeografiaNivel& datos, Casilla casilla);
    void validar_zona(const Zona& zona, const char* nombre) const;

    // Casillas caminables a las que se llega desde la entrada, sin contar la entrada ni las zonas
    // especiales. Son las únicas donde tiene sentido generar algo.
    std::vector<Casilla> casillas_generables() const;

public:
    /*
     * Casillas a las que se llega caminando desde `desde`, sin pasar por
     * la escalera ni la salida (así no se cuelan casillas de afuera de
     * la cueva que tocan la puerta). Incluye a `desde`.
     */
    static std::vector<Casilla> alcanzables(const GeografiaNivel& datos, Casilla desde);

    explicit Nivel(GeografiaNivel geografia_del_nivel);

    /*
     * Sortea los enemigos y las casillas donde se van a poner objetos y cofres, en casillas
     * distintas. Los objetos y cofres en sí los coloca quien conoce el catálogo (Mundo), a partir
     * de obtener_celdas_de_objetos() y obtener_celdas_de_cofres().
     */
    void sortear(
        std::mt19937& generador, int cantidad_enemigos, int cantidad_objetos, bool colocar_cofre);

    const GeografiaNivel& obtener_geografia() const;
    const std::vector<Casilla>& obtener_enemigos() const;
    const std::vector<Casilla>& obtener_celdas_de_objetos() const;
    const std::vector<Casilla>& obtener_celdas_de_cofres() const;
};

#endif
