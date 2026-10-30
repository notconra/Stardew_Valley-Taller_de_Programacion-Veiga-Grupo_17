#include "configuracion_servidor.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../common/configuracion/lector_toml.h"

ConfiguracionServidor::ConfiguracionServidor(const std::string& ruta) {
    const toml::table archivo_toml = lector_toml::cargar_archivo(ruta);
    cargar_toml(archivo_toml);
}

void ConfiguracionServidor::cargar_toml(const toml::table& archivo_toml) {
    cargar_red(archivo_toml);
    cargar_partida(archivo_toml);
    cargar_mapas(archivo_toml);
    cargar_juego(archivo_toml);
}

void ConfiguracionServidor::cargar_red(const toml::table& archivo_toml) {
    const toml::table& red = lector_toml::exigir_tabla(archivo_toml, "red");
    datos_configuracion.red.puerto = lector_toml::leer_texto(red, "puerto", "red");
}

void ConfiguracionServidor::cargar_partida(const toml::table& archivo_toml) {
    const toml::table& partida = lector_toml::exigir_tabla(archivo_toml, "partida");
    datos_configuracion.partida.jugadores_maximos_por_partida =
        lector_toml::leer_entero_positivo(partida, "jugadores_maximos_por_partida", "partida");
    if (datos_configuracion.partida.jugadores_maximos_por_partida > 255) {
        throw std::runtime_error(
            "configuración: 'jugadores_maximos_por_partida' de [partida] no puede superar 255");
    }
    datos_configuracion.partida.ticks_por_segundo =
        lector_toml::leer_entero_positivo(partida, "ticks_por_segundo", "partida");
    if (datos_configuracion.partida.ticks_por_segundo > 1000) {
        throw std::runtime_error(
            "configuración: 'ticks_por_segundo' de [partida] no puede superar 1000");
    }

    const auto capacidad = partida["capacidad_de_comandos"].value<int>();
    if (capacidad) {
        if (*capacidad <= 0) {
            throw std::runtime_error(
                "configuración: la clave 'capacidad_de_comandos' de [partida] debe ser mayor "
                "que cero");
        }
        datos_configuracion.partida.capacidad_de_comandos = *capacidad;
    }
}

void ConfiguracionServidor::cargar_mapas(const toml::table& archivo_toml) {
    const toml::array* mapas = archivo_toml["mapas"].as_array();
    if (mapas == nullptr || mapas->empty()) {
        throw std::runtime_error("configuración: falta la sección [[mapas]] (al menos uno)");
    }

    for (std::size_t i = 0; i < mapas->size(); ++i) {
        const toml::table* mapa = (*mapas)[i].as_table();
        const std::string seccion = "mapas[" + std::to_string(i) + "]";
        if (mapa == nullptr) {
            throw std::runtime_error(
                "configuración: la entrada '" + seccion + "' debe ser una tabla");
        }

        ConfiguracionMapaServidor configuracion_mapa;
        configuracion_mapa.id = lector_toml::leer_texto(*mapa, "id", seccion);
        configuracion_mapa.ruta_tmx = lector_toml::leer_texto(*mapa, "ruta_tmx", seccion);

        const bool id_repetido = std::any_of(
            datos_configuracion.mapas.begin(),
            datos_configuracion.mapas.end(),
            [&configuracion_mapa](const ConfiguracionMapaServidor& existente) {
                return existente.id == configuracion_mapa.id;
            });
        if (id_repetido) {
            throw std::runtime_error(
                "configuración: el id de mapa '" + configuracion_mapa.id + "' está repetido");
        }

        const auto valor_capa_colisiones = (*mapa)["capa_colisiones"].value<std::string>();
        if (valor_capa_colisiones) {
            if (valor_capa_colisiones->empty()) {
                throw std::runtime_error(
                    "configuración: la clave 'capa_colisiones' de " + seccion +
                    " no puede estar vacía");
            }
            configuracion_mapa.capa_colisiones = *valor_capa_colisiones;
        }

        const auto valor_capa_respaldo = (*mapa)["capa_respaldo"].value<std::string>();
        if (valor_capa_respaldo) {
            if (valor_capa_respaldo->empty()) {
                throw std::runtime_error(
                    "configuración: la clave 'capa_respaldo' de " + seccion +
                    " no puede estar vacía");
            }
            configuracion_mapa.capa_respaldo = *valor_capa_respaldo;
        }

        const auto valor_capa_transiciones = (*mapa)["capa_transiciones"].value<std::string>();
        if (valor_capa_transiciones) {
            if (valor_capa_transiciones->empty()) {
                throw std::runtime_error(
                    "configuración: la clave 'capa_transiciones' de " + seccion +
                    " no puede estar vacía");
            }
            configuracion_mapa.capa_transiciones = *valor_capa_transiciones;
        }

        const toml::array* capas_opcionales = (*mapa)["capas_opcionales"].as_array();
        if (capas_opcionales != nullptr) {
            for (std::size_t j = 0; j < capas_opcionales->size(); ++j) {
                const toml::table* capa = (*capas_opcionales)[j].as_table();
                const std::string seccion_capa =
                    seccion + ".capas_opcionales[" + std::to_string(j) + "]";
                if (capa == nullptr) {
                    throw std::runtime_error(
                        "configuración: la entrada '" + seccion_capa + "' debe ser una tabla");
                }
                const auto nombre = (*capa)["nombre"].value<std::string>();
                if (!nombre || nombre->empty()) {
                    throw std::runtime_error(
                        "configuración: falta o es inválida la clave 'nombre' de " + seccion_capa);
                }
                const auto propiedad = (*capa)["propiedad"].value<std::string>();
                if (!propiedad || propiedad->empty()) {
                    throw std::runtime_error(
                        "configuración: falta o es inválida la clave 'propiedad' de " +
                        seccion_capa);
                }
                configuracion_mapa.capas_opcionales.push_back(
                    CapaOpcionalMapaServidor{*nombre, *propiedad});
            }
        }

        datos_configuracion.mapas.push_back(configuracion_mapa);
    }

    const auto mapa_inicial = archivo_toml["mapa_inicial"].value<std::string>();
    if (!mapa_inicial || mapa_inicial->empty()) {
        throw std::runtime_error("configuración: falta la clave 'mapa_inicial'");
    }
    const bool existe = std::any_of(
        datos_configuracion.mapas.begin(),
        datos_configuracion.mapas.end(),
        [&mapa_inicial](const ConfiguracionMapaServidor& m) { return m.id == *mapa_inicial; });
    if (!existe) {
        throw std::runtime_error(
            "configuración: 'mapa_inicial' (" + *mapa_inicial + ") no está entre los [[mapas]]");
    }
    datos_configuracion.mapa_inicial = *mapa_inicial;
}

void ConfiguracionServidor::cargar_juego(const toml::table& archivo_toml) {
    const toml::table* juego = archivo_toml["juego"].as_table();
    if (juego == nullptr) {
        return;
    }

    const toml::table* movimiento = (*juego)["movimiento"].as_table();
    if (movimiento != nullptr) {
        const auto velocidad = (*movimiento)["velocidad"].value<double>();
        if (velocidad) {
            if (*velocidad <= 0.0) {
                throw std::runtime_error(
                    "configuración: la clave 'velocidad' de [juego.movimiento] debe ser mayor "
                    "que cero");
            }
            datos_configuracion.juego.movimiento.velocidad = *velocidad;
        }

        const auto hitbox_x = (*movimiento)["hitbox_x"].value<int>();
        if (hitbox_x) {
            datos_configuracion.juego.movimiento.hitbox_x = *hitbox_x;
        }
        const auto hitbox_y = (*movimiento)["hitbox_y"].value<int>();
        if (hitbox_y) {
            datos_configuracion.juego.movimiento.hitbox_y = *hitbox_y;
        }
        const auto hitbox_ancho = (*movimiento)["hitbox_ancho"].value<int>();
        if (hitbox_ancho) {
            if (*hitbox_ancho <= 0) {
                throw std::runtime_error(
                    "configuración: la clave 'hitbox_ancho' de [juego.movimiento] debe ser mayor "
                    "que cero");
            }
            datos_configuracion.juego.movimiento.hitbox_ancho = *hitbox_ancho;
        }
        const auto hitbox_alto = (*movimiento)["hitbox_alto"].value<int>();
        if (hitbox_alto) {
            if (*hitbox_alto <= 0) {
                throw std::runtime_error(
                    "configuración: la clave 'hitbox_alto' de [juego.movimiento] debe ser mayor "
                    "que cero");
            }
            datos_configuracion.juego.movimiento.hitbox_alto = *hitbox_alto;
        }
    }

    const toml::table* aparicion = (*juego)["aparicion"].as_table();
    if (aparicion != nullptr) {
        const auto x = (*aparicion)["x"].value<int>();
        if (x) {
            datos_configuracion.juego.aparicion.x = *x;
        }
        const auto y = (*aparicion)["y"].value<int>();
        if (y) {
            datos_configuracion.juego.aparicion.y = *y;
        }
        const auto paso = (*aparicion)["paso_de_busqueda"].value<int>();
        if (paso) {
            if (*paso <= 0) {
                throw std::runtime_error(
                    "configuración: la clave 'paso_de_busqueda' de [juego.aparicion] debe ser "
                    "mayor que cero");
            }
            datos_configuracion.juego.aparicion.paso_de_busqueda = *paso;
        }
    }

    const toml::table* inventario = (*juego)["inventario"].as_table();
    if (inventario != nullptr) {
        cargar_inventario(*inventario);
    }
}

void ConfiguracionServidor::cargar_inventario(const toml::table& inventario) {
    ConfiguracionInventarioServidor& destino = datos_configuracion.juego.inventario;

    const int barra = lector_toml::leer_entero_opcional(
        inventario,
        "cantidad_ranuras_barra",
        destino.cantidad_ranuras_barra);
    if (barra <= 0) {
        throw std::runtime_error(
            "configuración: 'cantidad_ranuras_barra' de [juego.inventario] debe ser mayor que "
            "cero");
    }
    if (barra > UINT8_MAX) {
        throw std::runtime_error(
            "configuración: 'cantidad_ranuras_barra' de [juego.inventario] no puede superar " +
            std::to_string(UINT8_MAX));
    }

    destino.cantidad_ranuras_barra = static_cast<uint8_t>(barra);
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
