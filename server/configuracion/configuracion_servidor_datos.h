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
    std::string ruta_tmx;
    std::string capa_colisiones = "Colisiones";
    std::string capa_respaldo = "Buildings";
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

struct ConfiguracionJuegoServidor {
    ConfiguracionMovimientoJugador movimiento;
    ConfiguracionAparicionJugador aparicion;
};

struct ConfiguracionServidorDatos {
    ConfiguracionRedServidor red;
    ConfiguracionPartidaServidor partida;
    ConfiguracionMapaServidor mapa;
    ConfiguracionJuegoServidor juego;
};

#endif
