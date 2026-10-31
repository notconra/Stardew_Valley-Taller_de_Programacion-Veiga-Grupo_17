#ifndef NIVEL_H
#define NIVEL_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

struct PosicionEnNivel {
    int x;
    int y;

    bool operator==(const PosicionEnNivel& otra) const = default;
};

struct Zona {
    PosicionEnNivel origen;
    uint16_t ancho;
    uint16_t alto;

    bool contiene(PosicionEnNivel posicion) const;
};

struct GeografiaNivel {
    uint16_t ancho;
    uint16_t alto;
    std::vector<bool> transitable;
    PosicionEnNivel entrada;
    Zona escalera;
    Zona salida;
};

class Nivel {
private:
    GeografiaNivel geo;
    std::vector<PosicionEnNivel> enemigos;
    std::vector<PosicionEnNivel> piedras;

    static size_t indice_de(const GeografiaNivel& geografia, PosicionEnNivel posicion);
    static bool esta_adentro(const GeografiaNivel& geografia, PosicionEnNivel posicion);
    static bool es_zona_especial(const GeografiaNivel& geografia, PosicionEnNivel posicion);
    void validar_zona(const Zona& zona, const char* nombre) const;

public:
    /*
     * Casillas a las que se llega caminando desde `desde`, sin pasar por
     * la escalera ni la salida (así no se cuelan casillas de afuera de
     * la cueva que tocan la puerta). Incluye a `desde`.
     */
    static std::vector<PosicionEnNivel> alcanzables(
        const GeografiaNivel& geografia, PosicionEnNivel desde);

    explicit Nivel(GeografiaNivel geo);

    /*
     * Sortea enemigos y piedras en celdas libres distintas.
     */
    void sortear(std::mt19937& generador, uint16_t cantidad_enemigos, uint16_t cantidad_piedras);

    const GeografiaNivel& obtener_geo() const;
    const std::vector<PosicionEnNivel>& obtener_enemigos() const;
    const std::vector<PosicionEnNivel>& obtener_piedras() const;
};

#endif
