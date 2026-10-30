#ifndef CONFIGURACION_SERVIDOR_DATOS_H
#define CONFIGURACION_SERVIDOR_DATOS_H

#include <cstdint>
#include <string>
#include <vector>

// Datos de configuración del servidor.
struct ConfiguracionRedServidor {
    std::string puerto;
};

struct ConfiguracionPartidaServidor {
    int jugadores_maximos_por_partida = 0;
    int ticks_por_segundo = 0;
    int capacidad_de_comandos = 1024;
};

struct CapaOpcionalMapaServidor {
    std::string nombre;
    std::string propiedad;  // nombre de la propiedad (se resuelve al cargar el mapa)
};

struct ConfiguracionMapaServidor {
    std::string id;
    std::string ruta_tmx;
    std::string capa_colisiones = "Colisiones";
    std::string capa_respaldo = "Buildings";
    std::string capa_transiciones = "Transiciones";
    std::vector<CapaOpcionalMapaServidor> capas_opcionales;
};
struct ConfiguracionMovimientoJugador {
    double velocidad = 120.0;
    int hitbox_x = 2;
    int hitbox_y = 24;
    int hitbox_ancho = 12;
    int hitbox_alto = 8;
};

struct ConfiguracionAparicionJugador {
    int x = 200;
    int y = 150;
    int paso_de_busqueda = 8;
};

// Inventario del jugador. Solo se configura el tamaño de la barra para poder probar con más o menos
// espacio, el stack de cada ítem y la capacidad de los cofres son reglas fijas del juego.
struct ConfiguracionInventarioServidor {
    // Por ahora el jugador solo tiene la barra de herramientas (sin mochila ni inventario extra).
    uint8_t cantidad_ranuras_barra = 10;
};

struct ConfiguracionJuegoServidor {
    ConfiguracionMovimientoJugador movimiento;
    ConfiguracionAparicionJugador aparicion;
    ConfiguracionInventarioServidor inventario;
};

struct ConfiguracionServidorDatos {
    ConfiguracionRedServidor red;
    ConfiguracionPartidaServidor partida;
    std::vector<ConfiguracionMapaServidor> mapas;
    std::string mapa_inicial;
    ConfiguracionJuegoServidor juego;
};

#endif
