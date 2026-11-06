#ifndef DATOS_PARTIDA_GUARDADA_H
#define DATOS_PARTIDA_GUARDADA_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/apariencia_personaje.h"
#include "../../common/direccion.h"
#include "../../common/ranura_inventario.h"

struct DatosJugadorGuardado {
    uint32_t id_cliente = 0;
    uint32_t id_jugador = 0;
    std::string nombre_jugador;
    std::string mapa_actual;
    double x = 0.0;
    double y = 0.0;
    Direccion mirando = Direccion::ABAJO;
    uint8_t ranura_activa = 0;
    std::vector<RanuraInventario> inventario;
    AparienciaPersonaje apariencia;

    bool operator==(const DatosJugadorGuardado&) const = default;
};

struct ModificacionCasilla {
    uint32_t columna = 0;
    uint32_t fila = 0;
    uint8_t propiedad = 0;

    bool operator==(const ModificacionCasilla&) const = default;
};

struct DatosMapaGuardado {
    std::string id_mapa;
    std::vector<ModificacionCasilla> casillas_modificadas;

    bool operator==(const DatosMapaGuardado&) const = default;
};

struct DatosMundoGuardado {
    std::vector<DatosJugadorGuardado> jugadores;
    std::vector<DatosMapaGuardado> mapas;
    std::vector<std::string> niveles_mazmorra;

    bool operator==(const DatosMundoGuardado&) const = default;
};

struct DatosPartidaGuardada {
    std::string codigo;
    std::string nombre;
    uint8_t jugadores_maximos = 0;
    bool multijugador = false;
    DatosMundoGuardado mundo;

    bool operator==(const DatosPartidaGuardada&) const = default;
};

#endif  // DATOS_PARTIDA_GUARDADA_H
