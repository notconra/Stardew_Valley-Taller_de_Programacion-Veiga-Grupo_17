#include "configuracion_servidor.h"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../common/configuracion/lector_toml.h"

#define MINUTOS_DE_UN_DIA_LENTO 20.0
#define MINUTOS_DE_UN_DIA_MEDIO 14.0
#define MINUTOS_DE_UN_DIA_RAPIDO 7.0
#define MINIMO_MINUTOS_DE_UN_DIA 0.5
#define MAXIMO_MINUTOS_DE_UN_DIA 600.0
#define MAXIMO_DIAS_POR_ESTACION 255

ConfiguracionServidor::ConfiguracionServidor(const std::string& ruta) {
    const toml::table archivo_toml = lector_toml::cargar_archivo(ruta);
    cargar_red(archivo_toml);
    cargar_partida(archivo_toml);
    cargar_mapas(archivo_toml);
    cargar_juego(archivo_toml);
    cargar_mazmorra(archivo_toml);
}

bool ConfiguracionServidor::existe_mapa(const std::string& id) const {
    return std::any_of(
        datos_configuracion.mapas.begin(),
        datos_configuracion.mapas.end(),
        [&id](const ConfiguracionMapaServidor& mapa) { return mapa.id == id; });
}

void ConfiguracionServidor::cargar_red(const toml::table& archivo_toml) {
    const toml::table& red = lector_toml::exigir_tabla(archivo_toml, "red");
    datos_configuracion.red.puerto = lector_toml::leer_texto(red, "puerto", "red");
}

void ConfiguracionServidor::cargar_partida(const toml::table& archivo_toml) {
    const toml::table& tabla = lector_toml::exigir_tabla(archivo_toml, "partida");
    ConfiguracionPartidaServidor& partida = datos_configuracion.partida;

    partida.jugadores_maximos_por_partida = lector_toml::leer_entero_en_rango(
        tabla,
        "jugadores_maximos_por_partida",
        "partida",
        1,
        255);
    partida.ticks_por_segundo =
        lector_toml::leer_entero_en_rango(tabla, "ticks_por_segundo", "partida", 1, 1000);
    partida.capacidad_de_comandos = lector_toml::leer_entero_opcional(
        tabla,
        "capacidad_de_comandos",
        "partida",
        partida.capacidad_de_comandos,
        1,
        INT_MAX);
}

void ConfiguracionServidor::cargar_mapas(const toml::table& archivo_toml) {
    const toml::array* mapas = archivo_toml["mapas"].as_array();
    if (mapas == nullptr || mapas->empty()) {
        throw std::runtime_error("configuración: falta la sección [[mapas]] (al menos uno)");
    }

    for (std::size_t i = 0; i < mapas->size(); ++i) {
        const std::string seccion = "mapas[" + std::to_string(i) + "]";
        const toml::table& tabla = lector_toml::tabla_en(*mapas, i, seccion);

        ConfiguracionMapaServidor mapa;
        mapa.id = lector_toml::leer_texto(tabla, "id", seccion);
        if (existe_mapa(mapa.id)) {
            throw std::runtime_error(
                "configuración: el id de mapa '" + mapa.id + "' está repetido");
        }
        mapa.ruta_tmx = lector_toml::leer_texto(tabla, "ruta_tmx", seccion);
        mapa.capa_colisiones = lector_toml::leer_texto_opcional(
            tabla,
            "capa_colisiones",
            seccion,
            mapa.capa_colisiones);
        mapa.capa_respaldo =
            lector_toml::leer_texto_opcional(tabla, "capa_respaldo", seccion, mapa.capa_respaldo);
        mapa.capa_transiciones = lector_toml::leer_texto_opcional(
            tabla,
            "capa_transiciones",
            seccion,
            mapa.capa_transiciones);
        mapa.propiedad_cultivable =
            lector_toml::leer_texto_opcional(tabla, "propiedad_cultivable", seccion, "");

        const toml::array* capas = tabla["capas_opcionales"].as_array();
        if (capas != nullptr) {
            for (std::size_t j = 0; j < capas->size(); ++j) {
                const std::string seccion_capa =
                    seccion + ".capas_opcionales[" + std::to_string(j) + "]";
                const toml::table& capa = lector_toml::tabla_en(*capas, j, seccion_capa);
                mapa.capas_opcionales.push_back(CapaOpcionalMapaServidor{
                    lector_toml::leer_texto(capa, "nombre", seccion_capa),
                    lector_toml::leer_texto(capa, "propiedad", seccion_capa)});
            }
        }

        datos_configuracion.mapas.push_back(mapa);
    }

    datos_configuracion.mapa_inicial =
        lector_toml::leer_texto(archivo_toml, "mapa_inicial", "archivo");
    if (!existe_mapa(datos_configuracion.mapa_inicial)) {
        throw std::runtime_error(
            "configuración: 'mapa_inicial' (" + datos_configuracion.mapa_inicial +
            ") no está entre los [[mapas]]");
    }
}

void ConfiguracionServidor::cargar_juego(const toml::table& archivo_toml) {
    const toml::table* juego = archivo_toml["juego"].as_table();
    if (juego == nullptr) {
        return;
    }

    if (const toml::table* tabla = (*juego)["movimiento"].as_table()) {
        cargar_movimiento(*tabla);
    }
    if (const toml::table* tabla = (*juego)["aparicion"].as_table()) {
        cargar_aparicion(*tabla);
    }
    if (const toml::table* tabla = (*juego)["inventario"].as_table()) {
        cargar_inventario(*tabla);
    }
    if (const toml::table* tabla = (*juego)["acciones"].as_table()) {
        cargar_acciones(*tabla);
    }
    if (const toml::table* tabla = (*juego)["drops"].as_table()) {
        cargar_drops(*tabla);
    }
    if (const toml::table* tabla = (*juego)["transiciones"].as_table()) {
        cargar_transiciones(*tabla);
    }
    if (const toml::table* tabla = (*juego)["dia"].as_table()) {
        cargar_dia(*tabla);
    }
}

void ConfiguracionServidor::cargar_movimiento(const toml::table& tabla) {
    const std::string seccion = "juego.movimiento";
    ConfiguracionMovimientoJugador& movimiento = datos_configuracion.juego.movimiento;

    movimiento.velocidad = lector_toml::leer_decimal_opcional(
        tabla,
        "velocidad",
        seccion,
        movimiento.velocidad,
        0.1,
        10000.0);
    movimiento.hitbox_x = lector_toml::leer_entero_opcional(
        tabla,
        "hitbox_x",
        seccion,
        movimiento.hitbox_x,
        -1000,
        1000);
    movimiento.hitbox_y = lector_toml::leer_entero_opcional(
        tabla,
        "hitbox_y",
        seccion,
        movimiento.hitbox_y,
        -1000,
        1000);
    movimiento.hitbox_ancho = lector_toml::leer_entero_opcional(
        tabla,
        "hitbox_ancho",
        seccion,
        movimiento.hitbox_ancho,
        1,
        1000);
    movimiento.hitbox_alto = lector_toml::leer_entero_opcional(
        tabla,
        "hitbox_alto",
        seccion,
        movimiento.hitbox_alto,
        1,
        1000);
}

void ConfiguracionServidor::cargar_aparicion(const toml::table& tabla) {
    const std::string seccion = "juego.aparicion";
    ConfiguracionAparicionJugador& aparicion = datos_configuracion.juego.aparicion;

    aparicion.x = lector_toml::leer_entero_opcional(tabla, "x", seccion, aparicion.x, 0, INT_MAX);
    aparicion.y = lector_toml::leer_entero_opcional(tabla, "y", seccion, aparicion.y, 0, INT_MAX);
    aparicion.paso_de_busqueda = lector_toml::leer_entero_opcional(
        tabla,
        "paso_de_busqueda",
        seccion,
        aparicion.paso_de_busqueda,
        1,
        1000);
}

void ConfiguracionServidor::cargar_inventario(const toml::table& tabla) {
    ConfiguracionInventarioServidor& inventario = datos_configuracion.juego.inventario;

    inventario.cantidad_ranuras_barra = static_cast<uint8_t>(lector_toml::leer_entero_opcional(
        tabla,
        "cantidad_ranuras_barra",
        "juego.inventario",
        inventario.cantidad_ranuras_barra,
        1,
        UINT8_MAX));
}

void ConfiguracionServidor::cargar_acciones(const toml::table& tabla) {
    const std::string seccion = "juego.acciones";
    ConfiguracionAccionesServidor& acciones = datos_configuracion.juego.acciones;

    acciones.alcance_en_casillas = lector_toml::leer_entero_opcional(
        tabla,
        "alcance_en_casillas",
        seccion,
        acciones.alcance_en_casillas,
        1,
        10);
    acciones.duracion_picar_en_ticks = lector_toml::leer_entero_opcional(
        tabla,
        "duracion_picar_en_ticks",
        seccion,
        acciones.duracion_picar_en_ticks,
        1,
        UINT16_MAX);
    acciones.duracion_tratar_tierra_en_ticks = lector_toml::leer_entero_opcional(
        tabla,
        "duracion_tratar_tierra_en_ticks",
        seccion,
        acciones.duracion_tratar_tierra_en_ticks,
        1,
        UINT16_MAX);
    acciones.duracion_atacar_en_ticks = lector_toml::leer_entero_opcional(
        tabla,
        "duracion_atacar_en_ticks",
        seccion,
        acciones.duracion_atacar_en_ticks,
        1,
        UINT16_MAX);
    acciones.danio_espada = lector_toml::leer_entero_opcional(
        tabla, "danio_espada", seccion, acciones.danio_espada, 0, INT_MAX);
    acciones.energia_por_golpe = lector_toml::leer_entero_opcional(
        tabla, "energia_por_golpe", seccion, acciones.energia_por_golpe, 0, INT_MAX);
    acciones.energia_por_picar = lector_toml::leer_entero_opcional(
        tabla, "energia_por_picar", seccion, acciones.energia_por_picar, 0, INT_MAX);
    acciones.energia_por_tratar_tierra = lector_toml::leer_entero_opcional(
        tabla, "energia_por_tratar_tierra", seccion, acciones.energia_por_tratar_tierra, 0,
        INT_MAX);
}

void ConfiguracionServidor::cargar_drops(const toml::table& tabla) {
    const std::string seccion = "juego.drops";
    ConfiguracionDropsServidor& drops = datos_configuracion.juego.drops;

    drops.radio_de_recogida_en_casillas = lector_toml::leer_entero_opcional(
        tabla,
        "radio_de_recogida_en_casillas",
        seccion,
        drops.radio_de_recogida_en_casillas,
        1,
        10);
    drops.distancia_de_soltado_en_casillas = lector_toml::leer_entero_opcional(
        tabla,
        "distancia_de_soltado_en_casillas",
        seccion,
        drops.distancia_de_soltado_en_casillas,
        1,
        10);

    if (drops.distancia_de_soltado_en_casillas <= drops.radio_de_recogida_en_casillas) {
        throw std::runtime_error(
            "configuración: 'distancia_de_soltado_en_casillas' de [juego.drops] debe ser mayor "
            "que 'radio_de_recogida_en_casillas'");
    }
}

void ConfiguracionServidor::cargar_transiciones(const toml::table& tabla) {
    ConfiguracionTransicionesServidor& transiciones = datos_configuracion.juego.transiciones;

    transiciones.destinos_con_clic = lector_toml::leer_textos_opcional(
        tabla,
        "destinos_con_clic",
        "juego.transiciones",
        transiciones.destinos_con_clic);
}

void ConfiguracionServidor::cargar_dia(const toml::table& tabla) {
    const std::string seccion = "juego.dia";
    ConfiguracionDiaServidor& dia = datos_configuracion.juego.dia;

    dia.velocidad = lector_toml::leer_texto_opcional(tabla, "velocidad", seccion, dia.velocidad);
    if (dia.velocidad == "lento") {
        dia.duracion_en_minutos = MINUTOS_DE_UN_DIA_LENTO;
    } else if (dia.velocidad == "medio") {
        dia.duracion_en_minutos = MINUTOS_DE_UN_DIA_MEDIO;
    } else if (dia.velocidad == "rapido") {
        dia.duracion_en_minutos = MINUTOS_DE_UN_DIA_RAPIDO;
    } else {
        throw std::runtime_error(
            "configuración: 'velocidad' de [juego.dia] tiene que ser \"lento\", \"medio\" o "
            "\"rapido\" (es \"" +
            dia.velocidad + "\")");
    }

    dia.duracion_en_minutos = lector_toml::leer_decimal_opcional(
        tabla,
        "duracion_en_minutos",
        seccion,
        dia.duracion_en_minutos,
        MINIMO_MINUTOS_DE_UN_DIA,
        MAXIMO_MINUTOS_DE_UN_DIA);

    dia.dias_por_estacion = lector_toml::leer_entero_opcional(
        tabla,
        "dias_por_estacion",
        seccion,
        dia.dias_por_estacion,
        1,
        MAXIMO_DIAS_POR_ESTACION);
}

void ConfiguracionServidor::cargar_mazmorra(const toml::table& archivo_toml) {
    const toml::table* tabla = archivo_toml["mazmorra"].as_table();
    if (tabla == nullptr) {
        return;
    }
    const std::string seccion = "mazmorra";
    ConfiguracionMazmorraServidor& mazmorra = datos_configuracion.mazmorra;

    mazmorra.destino =
        lector_toml::leer_texto_opcional(*tabla, "destino", seccion, mazmorra.destino);
    mazmorra.enemigos_por_nivel = lector_toml::leer_entero_opcional(
        *tabla,
        "enemigos_por_nivel",
        seccion,
        mazmorra.enemigos_por_nivel,
        0,
        INT_MAX);
    mazmorra.minerales_por_nivel = lector_toml::leer_entero_opcional(
        *tabla,
        "minerales_por_nivel",
        seccion,
        mazmorra.minerales_por_nivel,
        0,
        INT_MAX);
    mazmorra.probabilidad_de_cofre = lector_toml::leer_decimal_opcional(
        *tabla,
        "probabilidad_de_cofre",
        seccion,
        mazmorra.probabilidad_de_cofre,
        0.0,
        1.0);
    mazmorra.oro_minimo_en_cofre = lector_toml::leer_entero_opcional(
        *tabla,
        "oro_minimo_en_cofre",
        seccion,
        mazmorra.oro_minimo_en_cofre,
        0,
        UINT16_MAX);
    mazmorra.oro_maximo_en_cofre = lector_toml::leer_entero_opcional(
        *tabla,
        "oro_maximo_en_cofre",
        seccion,
        mazmorra.oro_maximo_en_cofre,
        0,
        UINT16_MAX);
    if (mazmorra.oro_minimo_en_cofre > mazmorra.oro_maximo_en_cofre) {
        throw std::runtime_error(
            "configuración: en [mazmorra] oro_minimo_en_cofre no puede ser mayor que "
            "oro_maximo_en_cofre");
    }

    if (const toml::table* enemigos = (*tabla)["enemigos"].as_table()) {
        if (const toml::table* esqueleto = (*enemigos)["esqueleto"].as_table()) {
            cargar_enemigo(*esqueleto, "mazmorra.enemigos.esqueleto", mazmorra.esqueleto);
        }
        if (const toml::table* orco = (*enemigos)["orco"].as_table()) {
            const std::string seccion_orco = "mazmorra.enemigos.orco";
            cargar_enemigo(*orco, seccion_orco, mazmorra.orco.combate);
            mazmorra.orco.grupos_por_nivel = lector_toml::leer_entero_opcional(
                *orco, "grupos_por_nivel", seccion_orco, mazmorra.orco.grupos_por_nivel, 0,
                INT_MAX);
            mazmorra.orco.tamano_grupo_minimo = lector_toml::leer_entero_opcional(
                *orco, "tamano_grupo_minimo", seccion_orco, mazmorra.orco.tamano_grupo_minimo, 1,
                INT_MAX);
            mazmorra.orco.tamano_grupo_maximo = lector_toml::leer_entero_opcional(
                *orco, "tamano_grupo_maximo", seccion_orco, mazmorra.orco.tamano_grupo_maximo, 1,
                INT_MAX);
            mazmorra.orco.probabilidad_de_plata_al_morir = lector_toml::leer_decimal_opcional(
                *orco, "probabilidad_de_plata_al_morir", seccion_orco,
                mazmorra.orco.probabilidad_de_plata_al_morir, 0.0, 1.0);
            mazmorra.orco.probabilidad_de_manada_por_plata = lector_toml::leer_decimal_opcional(
                *orco, "probabilidad_de_manada_por_plata", seccion_orco,
                mazmorra.orco.probabilidad_de_manada_por_plata, 0.0, 1.0);
            mazmorra.orco.distancia_de_aparicion_de_manada = lector_toml::leer_entero_opcional(
                *orco, "distancia_de_aparicion_de_manada", seccion_orco,
                mazmorra.orco.distancia_de_aparicion_de_manada, 1, 100);
            if (mazmorra.orco.tamano_grupo_minimo > mazmorra.orco.tamano_grupo_maximo) {
                throw std::runtime_error(
                    "configuración: [" + seccion_orco +
                    "] tiene tamano_grupo_minimo > tamano_grupo_maximo");
            }
        }
        if (const toml::table* vampiros = (*enemigos)["vampiros"].as_table()) {
            const std::string seccion_vampiros = "mazmorra.enemigos.vampiros";
            ConfiguracionVampirosServidor& config = mazmorra.vampiros;
            config.parejas_por_nivel = lector_toml::leer_entero_opcional(
                *vampiros, "parejas_por_nivel", seccion_vampiros, config.parejas_por_nivel, 0,
                INT_MAX);
            config.multiplicador_de_frenesi = lector_toml::leer_decimal_opcional(
                *vampiros, "multiplicador_de_frenesi", seccion_vampiros,
                config.multiplicador_de_frenesi, 1.0, 10.0);
            config.radio_de_sangre = lector_toml::leer_entero_opcional(
                *vampiros, "radio_de_sangre", seccion_vampiros, config.radio_de_sangre, 0, 100);
            if (const toml::table* vampira = (*vampiros)["vampira"].as_table()) {
                cargar_enemigo(*vampira, seccion_vampiros + ".vampira", config.vampira);
            }
            if (const toml::table* vampiro = (*vampiros)["vampiro"].as_table()) {
                cargar_enemigo(*vampiro, seccion_vampiros + ".vampiro", config.vampiro);
            }
        }
    }

    if (const toml::table* trampas = (*tabla)["trampas"].as_table()) {
        const std::string seccion_trampas = "mazmorra.trampas";
        ConfiguracionTrampasServidor& config = mazmorra.trampas;
        config.por_nivel = lector_toml::leer_entero_opcional(
            *trampas, "por_nivel", seccion_trampas, config.por_nivel, 0, INT_MAX);
        config.danio = lector_toml::leer_entero_opcional(
            *trampas, "danio", seccion_trampas, config.danio, 0, INT_MAX);
        config.ticks_inactiva = lector_toml::leer_entero_opcional(
            *trampas, "ticks_inactiva", seccion_trampas, config.ticks_inactiva, 1, INT_MAX);
        config.ticks_activa = lector_toml::leer_entero_opcional(
            *trampas, "ticks_activa", seccion_trampas, config.ticks_activa, 1, INT_MAX);
    }

    const toml::array* minerales = (*tabla)["minerales"].as_array();
    if (minerales != nullptr) {
        cargar_minerales(*minerales);
    }

    mazmorra.mapas = lector_toml::leer_textos(*tabla, "mapas", seccion);
    for (const std::string& id : mazmorra.mapas) {
        if (!existe_mapa(id)) {
            throw std::runtime_error(
                "configuración: el mapa '" + id + "' de [mazmorra] no está entre los [[mapas]]");
        }
    }
}

void ConfiguracionServidor::cargar_minerales(const toml::array& lista) {
    std::vector<ConfiguracionMineralServidor>& minerales = datos_configuracion.mazmorra.minerales;

    for (std::size_t i = 0; i < lista.size(); ++i) {
        const std::string seccion = "mazmorra.minerales[" + std::to_string(i) + "]";
        const toml::table& tabla = lector_toml::tabla_en(lista, i, seccion);

        ConfiguracionMineralServidor mineral;
        mineral.clave = lector_toml::leer_texto(tabla, "clave", seccion);
        mineral.golpes_para_picar = lector_toml::leer_entero_opcional(
            tabla,
            "golpes_para_picar",
            seccion,
            mineral.golpes_para_picar,
            1,
            255);
        mineral.peso =
            lector_toml::leer_decimal_opcional(tabla, "peso", seccion, mineral.peso, 0.0, 1000.0);
        mineral.probabilidad_de_no_dejar = lector_toml::leer_decimal_opcional(
            tabla,
            "probabilidad_de_no_dejar",
            seccion,
            mineral.probabilidad_de_no_dejar,
            0.0,
            1.0);
        mineral.cantidad_minima = lector_toml::leer_entero_opcional(
            tabla,
            "cantidad_minima",
            seccion,
            mineral.cantidad_minima,
            1,
            255);
        mineral.cantidad_maxima = lector_toml::leer_entero_opcional(
            tabla,
            "cantidad_maxima",
            seccion,
            mineral.cantidad_maxima,
            1,
            255);
        if (mineral.cantidad_maxima < mineral.cantidad_minima) {
            throw std::runtime_error(
                "configuración: '" + seccion + "' tiene cantidad_maxima menor que cantidad_minima");
        }
        mineral.desde_nivel = lector_toml::leer_entero_opcional(
            tabla,
            "desde_nivel",
            seccion,
            mineral.desde_nivel,
            1,
            100);
        minerales.push_back(mineral);
    }

    const bool alguno_aparece = std::any_of(
        minerales.begin(),
        minerales.end(),
        [](const ConfiguracionMineralServidor& mineral) { return mineral.peso > 0.0; });
    if (!minerales.empty() && !alguno_aparece) {
        throw std::runtime_error(
            "configuración: [[mazmorra.minerales]] necesita al menos un mineral con 'peso' mayor "
            "que cero");
    }
}

const ConfiguracionRedServidor& ConfiguracionServidor::red() const {
    return datos_configuracion.red;
}

const ConfiguracionPartidaServidor& ConfiguracionServidor::partida() const {
    return datos_configuracion.partida;
}

const std::vector<ConfiguracionMapaServidor>& ConfiguracionServidor::mapas() const {
    return datos_configuracion.mapas;
}

const std::string& ConfiguracionServidor::mapa_inicial() const {
    return datos_configuracion.mapa_inicial;
}

const ConfiguracionJuegoServidor& ConfiguracionServidor::juego() const {
    return datos_configuracion.juego;
}

const ConfiguracionMazmorraServidor& ConfiguracionServidor::mazmorra() const {
    return datos_configuracion.mazmorra;
}

void ConfiguracionServidor::cargar_enemigo(
    const toml::table& tabla, const std::string& seccion, ConfiguracionEnemigoServidor& enemigo) {
    const auto leer = [&](const char* clave, int& valor, int minimo) {
        valor = lector_toml::leer_entero_opcional(tabla, clave, seccion, valor, minimo, INT_MAX);
    };
    leer("vida", enemigo.vida, 1);
    leer("danio", enemigo.danio, 0);
    leer("ticks_entre_ataques", enemigo.ticks_entre_ataques, 1);
    leer("ticks_para_revivir", enemigo.ticks_para_revivir, 0);
    leer("ticks_por_casilla", enemigo.ticks_por_casilla, 0);
    leer("radio_de_vision", enemigo.radio_de_vision, 0);
    leer("radio_de_deambulo", enemigo.radio_de_deambulo, 0);
    leer("oro_minimo", enemigo.oro_minimo, 0);
    leer("oro_maximo", enemigo.oro_maximo, 0);
    if (enemigo.oro_minimo > enemigo.oro_maximo) {
        throw std::runtime_error("configuración: [" + seccion + "] tiene oro_minimo > oro_maximo");
    }
}
