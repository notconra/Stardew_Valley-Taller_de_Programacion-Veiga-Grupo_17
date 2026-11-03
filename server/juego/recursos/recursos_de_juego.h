#ifndef RECURSOS_DE_JUEGO_H
#define RECURSOS_DE_JUEGO_H

#include <map>
#include <string>
#include <vector>

#include "../../configuracion/configuracion_servidor_datos.h"
#include "../enemigos/catalogo_de_enemigos.h"
#include "../mapa/mapa.h"
#include "../mazmorra/nivel.h"
#include "../objetos/catalogo_de_items.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"

// Todo lo que se carga una sola vez y comparten todas las partidas: mapas, catálogos y
// configuraciones. Se arma en Servidor al arrancar, así un nombre mal escrito en el TOML falla
// antes de aceptar clientes.
struct RecursosDeJuego {
    std::map<std::string, Mapa> mapas;
    std::map<std::string, ConfiguracionMapaServidor> configuraciones_mapas;
    std::string mapa_inicial;
    ConfiguracionJuegoServidor juego;
    CatalogoDeItems catalogo_de_items;

    // Objetos colocables en el mundo (minerales, y a futuro árboles, muebles, drops...).
    // Es distinto de catalogo_de_items: este describe cómo ocupan el mapa y qué dejan al
    // romperse; aquel describe cómo se apilan y se venden en el inventario.
    CatalogoDeObjetos catalogo_de_objetos;

    // Los tipos de enemigo que puede haber en la mazmorra (vida, daño, si revive). Lo arma la
    // mazmorra para poblar los niveles; lo usa cada Sector para tener su GestorDeEnemigos.
    CatalogoDeEnemigos catalogo_de_enemigos;

    // Configuración de la mazmorra y la geografía de cada mapa que puede ser un nivel. Los
    // niveles concretos de cada partida se sortean en Mundo.
    ConfiguracionMazmorraServidor mazmorra;
    std::map<std::string, GeografiaNivel> geografias_de_mazmorra;

    // Construye los mapas, arma el catálogo de objetos del mundo y prepara la mazmorra. Lanza
    // std::runtime_error si falla un mapa, el catálogo, un objeto menciona un ítem que no existe
    // o la mazmorra pide un mapa desconocido.
    RecursosDeJuego(
        const std::vector<ConfiguracionMapaServidor>& configuracion_mapas,
        const std::string& id_mapa_inicial, const ConfiguracionJuegoServidor& configuracion_juego,
        const ConfiguracionMazmorraServidor& configuracion_mazmorra);

private:
    // Junta las definiciones de todos los módulos que aportan objetos del mundo. Para sumar
    // árboles o muebles alcanza con concatenar sus definiciones acá.
    static CatalogoDeObjetos crear_catalogo_de_objetos(
        const std::vector<ConfiguracionMineralServidor>& minerales);

    // Junta las definiciones de todos los módulos que aportan tipos de enemigo. Para sumar Orcos,
    // Vampiros o Skulls alcanza con sumar su definición acá, igual que con los objetos.
    static CatalogoDeEnemigos crear_catalogo_de_enemigos();

    // Verifica que el producto, la herramienta y el ítem colocable de cada objeto del mundo sean
    // ítems registrados en catalogo_de_items (y el colocable, de categoría COLOCABLE), para que un
    // ítem olvidado falle al arrancar y no en plena partida.
    void validar_catalogo_de_objetos() const;

    // Arma la geografía de cada mapa que la mazmorra puede usar como nivel.
    void cargar_geografias_de_mazmorra();
};

#endif
