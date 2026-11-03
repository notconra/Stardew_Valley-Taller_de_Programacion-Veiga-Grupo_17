#ifndef CONFIGURACION_SERVIDOR_H
#define CONFIGURACION_SERVIDOR_H

#include <string>
#include <vector>

#include "../../common/externo/tomlplusplus/toml.hpp"
#include "configuracion_servidor_datos.h"

// Lee el archivo .toml, valida lo que necesita y deja los datos listos para usar.
// Es la única clase de la configuración del servidor. Las validaciones comunes (rangos, textos no
// vacíos, listas) las resuelve lector_toml; acá solo se dice qué clave va en qué campo.
class ConfiguracionServidor {
public:
    // Lanza std::runtime_error si el archivo no existe, no es un TOML
    // válido o le falta alguna clave obligatoria.
    explicit ConfiguracionServidor(const std::string& ruta);

    const ConfiguracionRedServidor& red() const;
    const ConfiguracionPartidaServidor& partida() const;
    const std::vector<ConfiguracionMapaServidor>& mapas() const;
    const std::string& mapa_inicial() const;
    const ConfiguracionJuegoServidor& juego() const;
    const ConfiguracionMazmorraServidor& mazmorra() const;

private:
    ConfiguracionServidorDatos datos_configuracion;

    bool existe_mapa(const std::string& id) const;

    // Cada una lee una sección del archivo y completa su parte de datos_configuracion.
    void cargar_red(const toml::table& archivo_toml);
    void cargar_partida(const toml::table& archivo_toml);
    void cargar_mapas(const toml::table& archivo_toml);
    void cargar_juego(const toml::table& archivo_toml);
    void cargar_mazmorra(const toml::table& archivo_toml);

    // Subsecciones de [juego] y de [mazmorra].
    void cargar_movimiento(const toml::table& tabla);
    void cargar_aparicion(const toml::table& tabla);
    void cargar_inventario(const toml::table& tabla);
    void cargar_acciones(const toml::table& tabla);
    void cargar_drops(const toml::table& tabla);
    void cargar_transiciones(const toml::table& tabla);
    void cargar_minerales(const toml::array& lista);
};

#endif
