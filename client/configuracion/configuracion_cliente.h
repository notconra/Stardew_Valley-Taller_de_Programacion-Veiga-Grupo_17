#ifndef CONFIGURACION_CLIENTE_H
#define CONFIGURACION_CLIENTE_H

#include <string>
#include <vector>

#include "../../common/externo/tomlplusplus/toml.hpp"
#include "configuracion_cliente_datos.h"

// Lee el archivo .toml, valida lo que necesita y deja los datos listos para usar. Es la única
// clase de la configuración.
//
// Incluye toml++ porque los métodos auxiliares reciben el archivo TOML. Los componentes que
// solo necesitan los datos (vistas, mapa, jugador) incluyen configuracion_cliente_datos.h.
class ConfiguracionCliente {
public:
    // Lanza std::runtime_error si el archivo no existe, no es un TOML
    // válido o le falta alguna clave obligatoria.
    explicit ConfiguracionCliente(const std::string& ruta);

    const ConfiguracionConexion& servidor() const;
    const ConfiguracionVista& vista() const;

private:
    ConfiguracionConexion configuracion_servidor;
    ConfiguracionVista configuracion_vista;

    // Recorre las secciones del archivo_toml y completa los miembros de arriba.
    void cargar_toml(const toml::table& archivo_toml);
    void cargar_servidor(const toml::table& archivo_toml);
    void cargar_ventana(const toml::table& archivo_toml);
    void cargar_sdl(const toml::table& archivo_toml);
    void cargar_graficos(const toml::table& archivo_toml);
    void cargar_mapa(const toml::table& archivo_toml);
    void cargar_jugador(const toml::table& archivo_toml);

    // Auxiliares de lectura y validación.
    const toml::table& exigir_tabla(
        const toml::table& archivo_toml, const std::string& nombre) const;
    const toml::array& exigir_arreglo(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;

    int leer_entero(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
    int leer_entero_positivo(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
    bool leer_booleano(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
    std::string leer_texto(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;
    std::vector<std::string> leer_textos(
        const toml::table& tabla, const std::string& clave, const std::string& seccion) const;

    ColorDeFondo leer_color(const toml::table& graficos) const;
    std::vector<ConfiguracionTileset> leer_tilesets(const toml::table& mapa) const;
};

#endif
