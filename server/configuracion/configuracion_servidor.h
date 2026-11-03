#ifndef CONFIGURACION_SERVIDOR_H
#define CONFIGURACION_SERVIDOR_H

#include <string>

#include "../../common/externo/tomlplusplus/toml.hpp"
#include "configuracion_servidor_datos.h"

// Lee el archivo .toml, valida lo que necesita y deja los datos listos para usar.
// Es la única clase de la configuración del servidor: no delega en un lector aparte.
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

private:
    ConfiguracionServidorDatos datos_configuracion;

    // Recorre las secciones del archivo .toml y completa los miembros de arriba.
    void cargar_toml(const toml::table& archivo_toml);
    void cargar_red(const toml::table& archivo_toml);
    void cargar_partida(const toml::table& archivo_toml);
    void cargar_mapas(const toml::table& archivo_toml);
    void cargar_juego(const toml::table& archivo_toml);
    void cargar_inventario(const toml::table& inventario);

    // Auxiliares de lectura y validación.
    const toml::table& exigir_tabla(
        const toml::table& archivo_toml, const std::string& nombre) const;

    int leer_entero(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
    int leer_entero_positivo(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
    int leer_entero_opcional(
        const toml::table& tabla, const std::string& clave, int valor_por_defecto) const;
    std::string leer_texto(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
};

#endif
