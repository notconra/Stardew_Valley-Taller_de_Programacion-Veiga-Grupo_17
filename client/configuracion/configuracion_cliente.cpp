#include "configuracion_cliente.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

ConfiguracionCliente::ConfiguracionCliente(const std::string& ruta) {
    toml::table archivo_toml;
    try {
        archivo_toml = toml::parse_file(ruta);
    } catch (const toml::parse_error& error) {
        throw std::runtime_error(
            "no se pudo leer la configuración '" + ruta + "': " + std::string(error.description()) +
            " (línea " + std::to_string(error.source().begin.line) + ")");
    }

    cargar_toml(archivo_toml);
}

void ConfiguracionCliente::cargar_toml(const toml::table& archivo_toml) {
    cargar_servidor(archivo_toml);
    cargar_ventana(archivo_toml);
    cargar_sdl(archivo_toml);
    cargar_graficos(archivo_toml);
    cargar_mapa(archivo_toml);
    cargar_jugador(archivo_toml);
}

void ConfiguracionCliente::cargar_servidor(const toml::table& archivo_toml) {
    const toml::table& servidor = exigir_tabla(archivo_toml, "servidor");

    configuracion_servidor.host = leer_texto(servidor, "host", "servidor");
    configuracion_servidor.puerto = leer_texto(servidor, "puerto", "servidor");
}

void ConfiguracionCliente::cargar_ventana(const toml::table& archivo_toml) {
    const toml::table& ventana = exigir_tabla(archivo_toml, "ventana");

    configuracion_vista.ventana.titulo = leer_texto(ventana, "titulo", "ventana");
    configuracion_vista.ventana.ancho = leer_entero_positivo(ventana, "ancho", "ventana");
    configuracion_vista.ventana.alto = leer_entero_positivo(ventana, "alto", "ventana");
}

void ConfiguracionCliente::cargar_sdl(const toml::table& archivo_toml) {
    const toml::table& sdl = exigir_tabla(archivo_toml, "sdl");

    configuracion_vista.sdl.acelerado = leer_booleano(sdl, "acelerado", "sdl");
    configuracion_vista.sdl.vsync = leer_booleano(sdl, "vsync", "sdl");
}

void ConfiguracionCliente::cargar_graficos(const toml::table& archivo_toml) {
    const toml::table& graficos = exigir_tabla(archivo_toml, "graficos");

    configuracion_vista.graficos.tamano_de_tile =
        leer_entero_positivo(graficos, "tamano_de_tile", "graficos");
    configuracion_vista.graficos.escala_de_tiles =
        leer_entero_positivo(graficos, "escala_de_tiles", "graficos");
    configuracion_vista.graficos.color_de_fondo = leer_color(graficos);
}

void ConfiguracionCliente::cargar_mapa(const toml::table& archivo_toml) {
    const toml::table& mapa = exigir_tabla(archivo_toml, "mapa");

    configuracion_vista.mapa.ruta = leer_texto(mapa, "ruta", "mapa");
    configuracion_vista.mapa.capas_bajo_los_jugadores =
        leer_textos(mapa, "capas_bajo_los_jugadores", "mapa");
    configuracion_vista.mapa.capas_sobre_los_jugadores =
        leer_textos(mapa, "capas_sobre_los_jugadores", "mapa");
    configuracion_vista.mapa.tilesets = leer_tilesets(mapa);
}

void ConfiguracionCliente::cargar_jugador(const toml::table& archivo_toml) {
    const toml::table& jugador = exigir_tabla(archivo_toml, "jugador");

    configuracion_vista.jugador.ruta_sprite = leer_texto(jugador, "ruta_sprite", "jugador");
    configuracion_vista.jugador.ancho_sprite =
        leer_entero_positivo(jugador, "ancho_sprite", "jugador");
    configuracion_vista.jugador.alto_sprite =
        leer_entero_positivo(jugador, "alto_sprite", "jugador");
    configuracion_vista.jugador.cuadros_caminando =
        leer_entero_positivo(jugador, "cuadros_caminando", "jugador");
    configuracion_vista.jugador.ms_por_cuadro_caminando =
        leer_entero_positivo(jugador, "ms_por_cuadro_caminando", "jugador");
    configuracion_vista.jugador.cuadros_quieto =
        leer_entero_positivo(jugador, "cuadros_quieto", "jugador");
    configuracion_vista.jugador.ms_por_cuadro_quieto =
        leer_entero_positivo(jugador, "ms_por_cuadro_quieto", "jugador");
}

const toml::table& ConfiguracionCliente::exigir_tabla(
    const toml::table& archivo_toml, const std::string& nombre) const {
    const toml::table* tabla = archivo_toml[nombre].as_table();
    if (tabla == nullptr) {
        throw std::runtime_error("configuración: falta la sección [" + nombre + "]");
    }
    return *tabla;
}

const toml::array& ConfiguracionCliente::exigir_arreglo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) const {
    const toml::array* valores = tabla[clave].as_array();
    if (valores == nullptr) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    return *valores;
}

int ConfiguracionCliente::leer_entero(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) const {
    const auto valor = tabla[clave].value<int>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    return *valor;
}

int ConfiguracionCliente::leer_entero_positivo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) const {
    const int valor = leer_entero(tabla, clave, seccion);
    if (valor <= 0) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] debe ser mayor que cero");
    }
    return valor;
}

bool ConfiguracionCliente::leer_booleano(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) const {
    const auto valor = tabla[clave].value<bool>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    return *valor;
}

std::string ConfiguracionCliente::leer_texto(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) const {
    const auto valor = tabla[clave].value<std::string>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    if (valor->empty()) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] no puede estar vacía");
    }
    return *valor;
}

std::vector<std::string> ConfiguracionCliente::leer_textos(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) const {
    const toml::array& valores = exigir_arreglo(tabla, clave, seccion);

    std::vector<std::string> textos;
    textos.reserve(valores.size());
    for (const auto& valor : valores) {
        const auto texto = valor.value<std::string>();
        if (!texto || texto->empty()) {
            throw std::runtime_error(
                "configuración: la clave '" + clave + "' de [" + seccion +
                "] debe contener textos no vacíos");
        }
        textos.push_back(*texto);
    }

    if (textos.empty()) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] no puede estar vacía");
    }
    return textos;
}

ColorDeFondo ConfiguracionCliente::leer_color(const toml::table& graficos) const {
    const toml::array& valores = exigir_arreglo(graficos, "color_de_fondo", "graficos");
    if (valores.size() != 4) {
        throw std::runtime_error(
            "configuración: 'color_de_fondo' de [graficos] debe tener 4 valores "
            "(rojo, verde, azul, alfa)");
    }

    std::uint8_t componentes[4];
    for (std::size_t i = 0; i < 4; ++i) {
        const auto componente = valores[i].value<int>();
        if (!componente || *componente < 0 || *componente > 255) {
            throw std::runtime_error(
                "configuración: cada valor de 'color_de_fondo' de [graficos] debe estar entre 0 y "
                "255");
        }
        componentes[i] = static_cast<std::uint8_t>(*componente);
    }

    return ColorDeFondo{componentes[0], componentes[1], componentes[2], componentes[3]};
}

std::vector<ConfiguracionTileset> ConfiguracionCliente::leer_tilesets(
    const toml::table& mapa) const {
    const toml::array& valores = exigir_arreglo(mapa, "tilesets", "mapa");

    std::vector<ConfiguracionTileset> tilesets;
    tilesets.reserve(valores.size());
    for (std::size_t i = 0; i < valores.size(); ++i) {
        const toml::table* tileset = valores[i].as_table();
        const std::string seccion = "mapa.tilesets[" + std::to_string(i) + "]";
        if (tileset == nullptr) {
            throw std::runtime_error(
                "configuración: la entrada '" + seccion + "' debe ser una tabla");
        }

        tilesets.push_back(ConfiguracionTileset{
            leer_texto(*tileset, "ruta", seccion),
            leer_entero_positivo(*tileset, "firstgid", seccion),
            leer_entero_positivo(*tileset, "columnas", seccion),
        });
    }

    if (tilesets.empty()) {
        throw std::runtime_error("configuración: [mapa] debe tener al menos un tileset");
    }
    return tilesets;
}

const ConfiguracionConexion& ConfiguracionCliente::servidor() const {
    return configuracion_servidor;
}

const ConfiguracionVista& ConfiguracionCliente::vista() const {
    return configuracion_vista;
}
