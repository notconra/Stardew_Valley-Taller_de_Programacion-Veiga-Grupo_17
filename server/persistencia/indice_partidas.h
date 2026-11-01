#ifndef INDICE_PARTIDAS_H
#define INDICE_PARTIDAS_H

#include <map>
#include <string>
#include <vector>

struct EntradaIndicePartida {
    std::string codigo;
    std::string nombre;
    std::string ruta_archivo;

    bool operator==(const EntradaIndicePartida&) const = default;
};

class IndicePartidas {
private:
    std::string ruta_archivo;
    std::map<std::string, EntradaIndicePartida> partidas;

public:
    IndicePartidas();

    void registrar_o_actualizar(const EntradaIndicePartida& entrada);
    std::vector<EntradaIndicePartida> listar_todas() const;

    void guardar() const;
    void cargar();

    IndicePartidas(const IndicePartidas&) = delete;
    IndicePartidas& operator=(const IndicePartidas&) = delete;
};

#endif  // INDICE_PARTIDAS_H
