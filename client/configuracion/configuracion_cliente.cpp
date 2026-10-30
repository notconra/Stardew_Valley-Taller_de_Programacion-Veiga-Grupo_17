#include "configuracion_cliente.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>

#include "../persistencia/identidad_cliente.h"

ConfiguracionCliente::ConfiguracionCliente(int argc, char* argv[]) {
    if (argc != 2 && argc != 4) {
        throw std::invalid_argument("cantidad inválida de argumentos");
    }

    const std::string ruta = argv[1];
    uint32_t id_override = 0;

    if (argc == 4) {
        if (std::string_view(argv[2]) != "--id") {
            throw std::invalid_argument(
                std::string("argumento inesperado: '") + argv[2] + "', se esperaba '--id <N>'");
        }
        id_override = validar_id(argv[3]);
    }

    inicializar(ruta, id_override);
}

uint32_t ConfiguracionCliente::validar_id(const std::string& id_en_texto) const {
    if (id_en_texto.empty()) {
        throw std::invalid_argument("el ID de cliente no puede estar vacío");
    }
    for (char caracter : id_en_texto) {
        if (caracter < '0' || caracter > '9') {
            throw std::invalid_argument(
                "el ID de cliente debe ser un número entero positivo: " + id_en_texto);
        }
    }
    try {
        unsigned long id_numerico = std::stoul(id_en_texto);
        if (id_numerico == 0 || id_numerico > std::numeric_limits<uint32_t>::max()) {
            throw std::invalid_argument(
                "el ID de cliente debe ser mayor a 0 y menor a " +
                std::to_string(std::numeric_limits<uint32_t>::max()));
        }
        return static_cast<uint32_t>(id_numerico);
    } catch (const std::out_of_range&) {
        throw std::invalid_argument("valor de ID fuera de rango: " + id_en_texto);
    }
}

void ConfiguracionCliente::inicializar(const std::string& ruta, uint32_t id_override) {
    toml::table archivo_toml;
    try {
        archivo_toml = toml::parse_file(ruta);
    } catch (const toml::parse_error& error) {
        throw std::runtime_error(
            "no se pudo leer la configuración '" + ruta + "': " + std::string(error.description()) +
            " (línea " + std::to_string(error.source().begin.line) + ")");
    }

    cargar_toml(archivo_toml);
    id_del_cliente = (id_override > 0) ? id_override : IdentidadCliente::obtener_o_crear();
}

void ConfiguracionCliente::cargar_toml(const toml::table& archivo_toml) {
    cargar_servidor(archivo_toml);
    cargar_ventana(archivo_toml);
    cargar_sdl(archivo_toml);
    cargar_graficos(archivo_toml);
    cargar_mapas(archivo_toml);
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

void ConfiguracionCliente::cargar_mapas(const toml::table& archivo_toml) {
    const toml::array& mapas = exigir_arreglo(archivo_toml, "mapas", "raíz");
    if (mapas.empty()) {
        throw std::runtime_error("configuración: [[mapas]] debe tener al menos un mapa");
    }

    configuracion_vista.mapas.clear();
    for (std::size_t i = 0; i < mapas.size(); ++i) {
        const toml::table* mapa = mapas[i].as_table();
        const std::string seccion = "mapas[" + std::to_string(i) + "]";
        if (mapa == nullptr) {
            throw std::runtime_error(
                "configuración: la entrada '" + seccion + "' debe ser una tabla");
        }

        ConfiguracionMapa configuracion_mapa;
        configuracion_mapa.id = leer_texto(*mapa, "id", seccion);
        configuracion_mapa.ruta = leer_texto(*mapa, "ruta", seccion);
        configuracion_mapa.capas_bajo_los_jugadores =
            leer_textos(*mapa, "capas_bajo_los_jugadores", seccion);
        configuracion_mapa.capas_sobre_los_jugadores =
            leer_textos(*mapa, "capas_sobre_los_jugadores", seccion);
        configuracion_mapa.tilesets = leer_tilesets(*mapa, seccion);

        configuracion_vista.mapas.push_back(configuracion_mapa);
    }

    configuracion_vista.mapa_inicial = leer_texto(archivo_toml, "mapa_inicial", "raíz");
    const bool existe = std::any_of(
        configuracion_vista.mapas.begin(), configuracion_vista.mapas.end(),
        [this](const ConfiguracionMapa& m) { return m.id == configuracion_vista.mapa_inicial; });
    if (!existe) {
        throw std::runtime_error(
            "configuración: 'mapa_inicial' (" + configuracion_vista.mapa_inicial +
            ") no está entre los [[mapas]]");
    }
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
    const toml::table& mapa, const std::string& seccion) const {
    const toml::array& valores = exigir_arreglo(mapa, "tilesets", seccion);

    std::vector<ConfiguracionTileset> tilesets;
    tilesets.reserve(valores.size());
    for (std::size_t i = 0; i < valores.size(); ++i) {
        const toml::table* tileset = valores[i].as_table();
        const std::string seccion_tileset = seccion + ".tilesets[" + std::to_string(i) + "]";
        if (tileset == nullptr) {
            throw std::runtime_error(
                "configuración: la entrada '" + seccion_tileset + "' debe ser una tabla");
        }

        tilesets.push_back(ConfiguracionTileset{
            leer_texto(*tileset, "ruta", seccion_tileset),
            leer_entero_positivo(*tileset, "firstgid", seccion_tileset),
            leer_entero_positivo(*tileset, "columnas", seccion_tileset),
        });
    }

    if (tilesets.empty()) {
        throw std::runtime_error("configuración: " + seccion + " debe tener al menos un tileset");
    }
    return tilesets;
}

const ConfiguracionConexion& ConfiguracionCliente::servidor() const {
    return configuracion_servidor;
}

const ConfiguracionVista& ConfiguracionCliente::vista() const {
    return configuracion_vista;
}

uint32_t ConfiguracionCliente::id_cliente() const {
    return id_del_cliente;
}
