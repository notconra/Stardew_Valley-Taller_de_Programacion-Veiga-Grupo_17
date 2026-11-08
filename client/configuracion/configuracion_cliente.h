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
    // Parsea los argumentos de línea de comandos (ruta al archivo TOML y flag opcional --id <N>),
    // valida la configuración y deja los datos listos para usar.
    // Lanza std::invalid_argument si los argumentos son inválidos, o std::runtime_error si el
    // archivo no existe, no es un TOML válido o le falta alguna clave obligatoria.
    ConfiguracionCliente(int argc, char* argv[]);

    const ConfiguracionConexion& servidor() const;
    const ConfiguracionVista& vista() const;
    uint32_t id_cliente() const;
    ConfiguracionJugador& jugador();

private:
    ConfiguracionConexion configuracion_servidor;
    ConfiguracionVista configuracion_vista;
    uint32_t id_del_cliente = 0;

    void inicializar(const std::string& ruta, uint32_t id_override);
    uint32_t validar_id(const std::string& id_en_texto) const;

    // Recorre las secciones del archivo_toml y completa los miembros de arriba.
    void cargar_toml(const toml::table& archivo_toml);
    void cargar_servidor(const toml::table& archivo_toml);
    void cargar_ventana(const toml::table& archivo_toml);
    void cargar_sdl(const toml::table& archivo_toml);
    void cargar_graficos(const toml::table& archivo_toml);
    void cargar_mapas(const toml::table& archivo_toml);
    void cargar_jugador(const toml::table& archivo_toml);
    void cargar_sonido(const toml::table& archivo_toml);
    void cargar_tierra(const toml::table& archivo_toml);
    void cargar_chat(const toml::table& archivo_toml);

    ColorDeFondo leer_color(const toml::table& graficos) const;
    ColorRGB leer_color_rgb(const toml::table& color, const std::string& seccion) const;
    std::vector<ConfiguracionTileset> leer_tilesets(
        const toml::table& mapa, const std::string& seccion) const;
};

#endif
