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
    // Propiedad de los tiles del piso que marca las casillas cultivables (por ejemplo "Diggable").
    // Vacía significa que el mapa no tiene tierra cultivable.
    std::string propiedad_cultivable;
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

// Inventario del jugador. Solo se configura el tamaño de la barra para poder probar con más o
// menos espacio; el stack de cada ítem es una regla fija del juego.
struct ConfiguracionInventarioServidor {
    // Por ahora el jugador solo tiene la barra de herramientas (sin mochila ni inventario extra).
    uint8_t cantidad_ranuras_barra = 10;
};

// Cuánto dura cada acción y hasta dónde llega. Las duraciones se miden en ticks del servidor.
struct ConfiguracionAccionesServidor {
    int alcance_en_casillas = 1;
    int duracion_picar_en_ticks = 8;
    int duracion_tratar_tierra_en_ticks = 8;
};

// Reglas de los ítems tirados en el piso.
struct ConfiguracionDropsServidor {
    int radio_de_recogida_en_casillas = 1;
    // Tiene que ser mayor que el radio, si no el ítem se levanta solo apenas se suelta.
    int distancia_de_soltado_en_casillas = 2;
};

// Qué transiciones se usan con clic derecho. El resto se activa al pisarlas.
struct ConfiguracionTransicionesServidor {
    // Ids de "mapa_destino" cuyas transiciones piden clic (puertas, escaleras).
    std::vector<std::string> destinos_con_clic{"siguiente"};
};

// Cuánto dura un día de juego (de 6 am a 2 am) en tiempo real. "velocidad" es lento, medio o
// rapido.
struct ConfiguracionDiaServidor {
    std::string velocidad = "medio";
    double duracion_en_minutos = 14.0;
};

struct ConfiguracionJuegoServidor {
    ConfiguracionMovimientoJugador movimiento;
    ConfiguracionAparicionJugador aparicion;
    ConfiguracionInventarioServidor inventario;
    ConfiguracionAccionesServidor acciones;
    ConfiguracionDropsServidor drops;
    ConfiguracionTransicionesServidor transiciones;
    ConfiguracionDiaServidor dia;
};

// Un mineral de la mazmorra. La clave elige cuál es (PIEDRA, CARBON, LAPISLAZULI, PLATA u ORO),
// con qué se pica y qué deja son reglas fijas del juego.
struct ConfiguracionMineralServidor {
    std::string clave;
    int golpes_para_picar = 1;
    double peso = 0.0;  // frecuencia relativa, no hace falta que sumen 100
    double probabilidad_de_no_dejar = 0.0;
    int cantidad = 1;
    int desde_nivel = 1;  // primer nivel donde puede aparecer
};

// La mazmorra: qué mapas pueden ser niveles y qué mapa_destino lleva a ella.
// Si no hay mazmorra en el archivo, "mapas" queda vacío y la partida no tiene mazmorra.
struct ConfiguracionMazmorraServidor {
    std::string destino = "mazmorra";
    std::vector<std::string> mapas;
    int enemigos_por_nivel = 4;
    int minerales_por_nivel = 30;
    // Vacía significa "usar los minerales por defecto". Si hay alguno, reemplaza a los de defecto.
    std::vector<ConfiguracionMineralServidor> minerales;
};

struct ConfiguracionServidorDatos {
    ConfiguracionRedServidor red;
    ConfiguracionPartidaServidor partida;
    std::vector<ConfiguracionMapaServidor> mapas;
    std::string mapa_inicial;
    ConfiguracionJuegoServidor juego;
    ConfiguracionMazmorraServidor mazmorra;
};

#endif
