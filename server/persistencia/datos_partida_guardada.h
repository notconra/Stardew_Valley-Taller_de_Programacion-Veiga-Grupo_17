#ifndef DATOS_PARTIDA_GUARDADA_H
#define DATOS_PARTIDA_GUARDADA_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/apariencia_personaje.h"
#include "../../common/direccion.h"
#include "../../common/mensajes/mensajes_partida.h"
#include "../../common/ranura_inventario.h"
#include "../../common/tipo_objeto.h"

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
    uint16_t vida = VIDA_MAXIMA_JUGADOR;
    uint16_t energia = ENERGIA_MAXIMA_JUGADOR;
    bool tiene_cama = false;
    std::string mapa_cama;
    uint32_t columna_cama = 0;
    uint32_t fila_cama = 0;

    bool operator==(const DatosJugadorGuardado&) const = default;
};

enum class TipoModificacionCasilla : uint8_t {
    TIERRA_TRATADA = 1,
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

struct DatosCofreGuardado {
    std::string id_mapa;
    uint32_t columna = 0;
    uint32_t fila = 0;
    std::vector<RanuraInventario> inventario;

    bool operator==(const DatosCofreGuardado&) const = default;
};

// Un mueble que el jugador colocó (la cama). Los cofres se guardan aparte, con su contenido, y lo
// que genera el mundo (árboles, minerales) no se guarda porque se vuelve a generar.
struct DatosMuebleGuardado {
    std::string id_mapa;
    TipoObjeto tipo = TipoObjeto::NINGUNO;
    uint32_t columna = 0;
    uint32_t fila = 0;

    bool operator==(const DatosMuebleGuardado&) const = default;
};

struct DatosMundoGuardado {
    uint32_t dia = 1;
    uint32_t segundos_jugados = 0;
    std::vector<DatosJugadorGuardado> jugadores;
    std::vector<DatosMapaGuardado> mapas;
    std::vector<std::string> niveles_mazmorra;
    std::vector<DatosCofreGuardado> cofres;
    std::vector<DatosMuebleGuardado> muebles;

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
