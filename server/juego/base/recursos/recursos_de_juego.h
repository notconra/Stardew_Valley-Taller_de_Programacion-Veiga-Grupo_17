#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include <map>
#include <string>

#include "../../../configuracion/configuracion_servidor_datos.h"
#include "../../objetos/catalogo_de_items.h"
#include "../mapas/mapa.h"

struct RecursosDeJuego {
    std::map<std::string, Mapa> mapas;
    std::string mapa_inicial;
    ConfiguracionJuegoServidor juego;
    CatalogoDeItems catalogo_de_items;

    RecursosDeJuego(
        const std::vector<ConfiguracionMapaServidor>& configuracion_mapas,
        const std::string& id_mapa_inicial, const ConfiguracionJuegoServidor& configuracion_juego);
};

#endif