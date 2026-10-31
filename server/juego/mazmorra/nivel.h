#ifndef NIVEL_H
#define NIVEL_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

struct Posicion {
    int x;
    int y;

    bool operator==(const Posicion& otra) const = default;
};

struct Zona {
    Posicion origen;
    uint16_t ancho;
    uint16_t alto;

    bool contiene(Posicion posicion) const;
};

struct GeografiaNivel {
    uint16_t ancho;
    uint16_t alto;
    std::vector<bool> transitable;
    Posicion entrada;
    Zona escalera;
    Zona salida;
};

class Nivel {
private:
    GeografiaNivel geo;
    std::vector<Posicion> enemigos;
    std::vector<Posicion> piedras;

    static size_t indice_de(const GeografiaNivel& geografia, Posicion posicion);
    static bool esta_adentro(const GeografiaNivel& geografia, Posicion posicion);
    static bool es_zona_especial(const GeografiaNivel& geografia, Posicion posicion);
    void validar_zona(const Zona& zona, const char* nombre) const;

public:
    /*
     * Casillas a las que se llega caminando desde `desde`, sin pasar por
     * la escalera ni la salida (así no se cuelan casillas de afuera de
     * la cueva que tocan la puerta). Incluye a `desde`.
     */
    static std::vector<Posicion> alcanzables(const GeografiaNivel& geografia, Posicion desde);

    explicit Nivel(GeografiaNivel geo);

    /*
     * Sortea enemigos y piedras en celdas libres distintas.
     */
    void sortear(std::mt19937& generador, uint16_t cantidad_enemigos, uint16_t cantidad_piedras);

    const GeografiaNivel& obtener_geo() const;
    const std::vector<Posicion>& obtener_enemigos() const;
    const std::vector<Posicion>& obtener_piedras() const;
};

#endif
