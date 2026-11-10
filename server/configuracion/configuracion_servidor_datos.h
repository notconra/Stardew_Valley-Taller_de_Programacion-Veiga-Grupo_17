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
    // Tiempo mínimo entre golpes de espada (18 ticks = 0,45 s, lo que dura la animación).
    int duracion_atacar_en_ticks = 18;
    // Golpe de espada del jugador: daño base (antes del multiplicador por vida baja) y energía
    // que gasta cada golpe que conecta.
    int danio_espada = 10;
    int energia_por_golpe = 3;
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
// rapido; si el archivo trae duracion_en_minutos, esa duración pisa la del preset.
struct ConfiguracionDiaServidor {
    std::string velocidad = "medio";
    double duracion_en_minutos = 14.0;
    int dias_por_estacion = 28;
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

// Los números de combate de un tipo de enemigo. Los tiempos se miden en ticks del servidor.
// Los valores por defecto son los del esqueleto.
struct ConfiguracionEnemigoServidor {
    int vida = 30;
    int danio = 5;
    int ticks_entre_ataques = 70;
    int ticks_para_revivir = 600;
    int ticks_por_casilla = 20;
    int radio_de_vision = 5;
    int radio_de_deambulo = 3;
    int oro_minimo = 0;
    int oro_maximo = 0;
};

// Los orcos aparecen en grupos: cada punto de aparición de orcos se llena con un grupo de entre
// tamano_grupo_minimo y tamano_grupo_maximo orcos alrededor.
struct ConfiguracionOrcosServidor {
    ConfiguracionEnemigoServidor combate{
        .vida = 60,
        .danio = 8,
        .ticks_entre_ataques = 60,
        .ticks_para_revivir = 0,
        .ticks_por_casilla = 16,
        .radio_de_vision = 6,
        .radio_de_deambulo = 3,
        .oro_minimo = 0,
        .oro_maximo = 0};
    int grupos_por_nivel = 1;  // cuántos de los enemigos_por_nivel puntos son grupos de orcos
    int tamano_grupo_minimo = 3;
    int tamano_grupo_maximo = 6;
    // Probabilidad de que al conseguir plata picando se avecine una manada de orcos (enunciado:
    // 10%).
    double probabilidad_de_manada_por_plata = 0.10;
    // A cuántas casillas del jugador aparece la manada: justo fuera de lo que se ve en la mazmorra
    // (radio_vision_mazmorra del cliente: 220 px / 32 px por casilla ≈ 7 casillas).
    int distancia_de_aparicion_de_manada = 8;
};

// Los vampiros aparecen en pareja, una vampira y un vampiro. La vampira es más rápida y el vampiro
// pega más fuerte (eso sale de sus números de combate). Al morir uno, el otro entra en frenesí y
// su habilidad se multiplica por multiplicador_de_frenesi. Muertos no reviven solos: esperan a que
// un jugador caiga o quede gravemente herido a radio_de_sangre casillas o menos de su cadáver; ahí
// cae sangre y reviven ticks_para_revivir después.
struct ConfiguracionVampirosServidor {
    ConfiguracionEnemigoServidor vampira{
        .vida = 80,
        .danio = 8,
        .ticks_entre_ataques = 50,
        .ticks_para_revivir = 200,
        .ticks_por_casilla = 10,
        .radio_de_vision = 7,
        .radio_de_deambulo = 3,
        .oro_minimo = 5,
        .oro_maximo = 15};
    ConfiguracionEnemigoServidor vampiro{
        .vida = 100,
        .danio = 14,
        .ticks_entre_ataques = 60,
        .ticks_para_revivir = 200,
        .ticks_por_casilla = 18,
        .radio_de_vision = 7,
        .radio_de_deambulo = 3,
        .oro_minimo = 5,
        .oro_maximo = 15};
    int parejas_por_nivel = 1;
    double multiplicador_de_frenesi = 2.0;
    int radio_de_sangre = 3;
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
    // [mazmorra.enemigos.esqueleto] y [mazmorra.enemigos.orco]. Vampiros y skulls se suman como
    // campos hermanos.
    ConfiguracionEnemigoServidor esqueleto;
    ConfiguracionOrcosServidor orco;
    ConfiguracionVampirosServidor vampiros;
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
