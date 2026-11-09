#include "minerales.h"

#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Lo que no se configura de cada mineral, cómo se ve en el mapa y qué ítem deja. Siempre se pican
// con el pico.
struct MineralFijo {
    TipoObjeto tipo;
    const char* clave;
    TipoItem producto;
};

const std::vector<MineralFijo>& minerales_fijos() {
    static const std::vector<MineralFijo> lista{
        {TipoObjeto::PIEDRA, "PIEDRA", TipoItem::PIEDRA},
        {TipoObjeto::CARBON, "CARBON", TipoItem::CARBON},
        {TipoObjeto::LAPISLAZULI, "LAPISLAZULI", TipoItem::LAPISLAZULI},
        {TipoObjeto::PLATA, "PLATA", TipoItem::PLATA},
        {TipoObjeto::ORO, "ORO", TipoItem::ORO},
    };
    return lista;
}

const MineralFijo& fijo_de(const std::string& clave) {
    for (const MineralFijo& fijo : minerales_fijos()) {
        if (clave == fijo.clave) {
            return fijo;
        }
    }
    throw std::runtime_error(
        "configuración: mineral desconocido '" + clave + "' en [[mazmorra.minerales]]");
}

void validar(const std::vector<ConfiguracionMineralServidor>& configurados) {
    std::set<std::string> vistas;
    for (const ConfiguracionMineralServidor& mineral : configurados) {
        fijo_de(mineral.clave);
        if (!vistas.insert(mineral.clave).second) {
            throw std::runtime_error(
                "configuración: el mineral '" + mineral.clave +
                "' está repetido en [[mazmorra.minerales]]");
        }
    }
}

}  // namespace

std::vector<ConfiguracionMineralServidor> Minerales::por_defecto() {
    // Las frecuencias salen del enunciado, el carbón es muy común, el lapislázuli medianamente
    // común, la plata poco común y el oro extremadamente raro (desde la quinta sala). La piedra
    // es el relleno de la caverna.
    return {
        {.clave = "PIEDRA",
         .golpes_para_picar = 1,
         .peso = 100.0,
         .cantidad_minima = 1,
         .cantidad_maxima = 3},
        {.clave = "CARBON",
         .golpes_para_picar = 1,
         .peso = 25.0,
         .cantidad_minima = 1,
         .cantidad_maxima = 2},
        {.clave = "LAPISLAZULI",
         .golpes_para_picar = 2,
         .peso = 10.0,
         .probabilidad_de_no_dejar = 0.15,
         .cantidad_minima = 1,
         .cantidad_maxima = 2},
        {.clave = "PLATA",
         .golpes_para_picar = 3,
         .peso = 6.0,
         .cantidad_minima = 1,
         .cantidad_maxima = 2},
        {.clave = "ORO",
         .golpes_para_picar = 4,
         .peso = 1.0,
         .cantidad_minima = 1,
         .cantidad_maxima = 2,
         .desde_nivel = 5},
    };
}

std::vector<DefinicionDeObjeto> Minerales::definiciones(
    const std::vector<ConfiguracionMineralServidor>& configurados) {
    validar(configurados);

    std::vector<ConfiguracionMineralServidor> todos = por_defecto();
    for (const ConfiguracionMineralServidor& configurado : configurados) {
        for (ConfiguracionMineralServidor& base : todos) {
            if (base.clave == configurado.clave) {
                base = configurado;
            }
        }
    }

    std::vector<DefinicionDeObjeto> resultado;
    resultado.reserve(todos.size());
    for (const ConfiguracionMineralServidor& mineral : todos) {
        const MineralFijo& fijo = fijo_de(mineral.clave);
        DefinicionDeObjeto definicion{};
        definicion.tipo = fijo.tipo;
        definicion.clave = fijo.clave;
        definicion.golpes_para_picar = mineral.golpes_para_picar;
        definicion.herramienta = TipoItem::PICO;
        definicion.probabilidad_de_no_dejar = mineral.probabilidad_de_no_dejar;
        definicion.producto = fijo.producto;
        definicion.cantidad_minima = mineral.cantidad_minima;
        definicion.cantidad_maxima = mineral.cantidad_maxima;
        resultado.push_back(definicion);
    }
    return resultado;
}

std::vector<PesoDeObjeto> Minerales::pesos(
    int numero_de_nivel, const std::vector<ConfiguracionMineralServidor>& configurados) {
    validar(configurados);

    const std::vector<ConfiguracionMineralServidor> efectivos =
        configurados.empty() ? por_defecto() : configurados;

    std::vector<PesoDeObjeto> resultado;
    resultado.reserve(efectivos.size());
    for (const ConfiguracionMineralServidor& mineral : efectivos) {
        const bool disponible = numero_de_nivel >= mineral.desde_nivel;
        resultado.push_back(
            PesoDeObjeto{fijo_de(mineral.clave).tipo, disponible ? mineral.peso : 0.0});
    }
    return resultado;
}
