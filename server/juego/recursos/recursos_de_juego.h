#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include <map>
#include <string>
#include <vector>

#include "../../configuracion/configuracion_servidor_datos.h"
#include "../mapa/mapa.h"
#include "../objetos/catalogo_de_items.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"

// Todo lo que se carga una sola vez y comparten todas las partidas: mapas, catálogos y
// configuraciones. Se arma en Servidor al arrancar, así un nombre mal escrito en el TOML falla
// antes de aceptar clientes.
struct RecursosDeJuego {
    // Se guardan completas porque cada partida necesita la configuración de objetos de cada mapa.
    std::vector<ConfiguracionMapaServidor> configuraciones_de_mapas;
    std::map<std::string, Mapa> mapas;
    std::string mapa_inicial;
    ConfiguracionJuegoServidor juego;
    CatalogoDeItems catalogo_de_items;

    // Objetos colocables en el mundo (minerales, y a futuro árboles, muebles, drops...).
    // Es distinto de catalogo_de_items: este describe cómo ocupan el mapa y qué dejan al
    // romperse; aquel describe cómo se apilan y se venden en el inventario.
    CatalogoDeObjetos catalogo_de_objetos;

    // Construye los mapas y arma el catálogo de objetos del mundo. Lanza std::runtime_error si
    // falla un mapa o si [mapas.objetos] menciona un tipo que no existe.
    RecursosDeJuego(
        const std::vector<ConfiguracionMapaServidor>& configuracion_mapas,
        const std::string& id_mapa_inicial, const ConfiguracionJuegoServidor& configuracion_juego);

private:
    // Junta las definiciones de todos los módulos que aportan objetos del mundo. Para sumar
    // árboles o muebles alcanza con concatenar sus definiciones acá.
    static CatalogoDeObjetos crear_catalogo_de_objetos();

    // Verifica que los tipos escritos en [mapas.objetos] existan en el catálogo.
    void validar_objetos_de_los_mapas() const;
};

#endif
