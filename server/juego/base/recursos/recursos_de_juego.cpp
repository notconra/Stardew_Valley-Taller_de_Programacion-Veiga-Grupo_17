#include "recursos_de_juego.h"

#include <string>
#include <vector>

#include "../../mazmorra/cargador_mazmorra.h"

RecursosDeJuego::RecursosDeJuego(
    const std::vector<ConfiguracionMapaServidor>& configuracion_mapas,
    const std::string& id_mapa_inicial, const ConfiguracionJuegoServidor& configuracion_juego,
    const ConfiguracionMazmorraServidor& configuracion_mazmorra)
    : mapa_inicial(id_mapa_inicial), juego(configuracion_juego), mazmorra(configuracion_mazmorra) {
    for (const ConfiguracionMapaServidor& configuracion_mapa : configuracion_mapas) {
        mapas.emplace(configuracion_mapa.id, Mapa(configuracion_mapa));
    }

    for (const std::string& id_mapa : mazmorra.mapas) {
        for (const ConfiguracionMapaServidor& configuracion_mapa : configuracion_mapas) {
            if (configuracion_mapa.id == id_mapa) {
                geografias_de_mazmorra.emplace(
                    id_mapa,
                    CargadorMazmorra::cargar_nivel(configuracion_mapa.ruta_tmx));
            }
        }
    }
}
