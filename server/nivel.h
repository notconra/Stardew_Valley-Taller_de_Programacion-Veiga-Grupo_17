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

struct GeografiaNivel {
    uint16_t ancho;
    uint16_t alto;
    std::vector<bool> transitable;
    Posicion entrada;
    std::optional<Posicion> escalera;
    Posicion salida;
    uint16_t cantidad_enemigos;
    uint16_t cantidad_piedras;
};


class Nivel {
private:
    GeografiaNivel geo;
    std::vector<Posicion> enemigos;
    std::vector<Posicion> piedras;

public:
    explicit Nivel(GeografiaNivel geo);

    void sortear(std::mt19937& generador);

    const GeografiaNivel& obtener_geo() const;
    const std::vector<Posicion>& obtener_enemigos() const;
    const std::vector<Posicion>& obtener_piedras() const;
};