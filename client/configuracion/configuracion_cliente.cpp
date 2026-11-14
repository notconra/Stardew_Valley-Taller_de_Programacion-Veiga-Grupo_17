#include "configuracion_cliente.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../../common/configuracion/lector_toml.h"
#include "../persistencia/identidad_cliente.h"

namespace {
// Nombres con los que cada herramienta y cada enemigo se escriben en [sonido.*] del toml.
const std::pair<const char*, TipoItem> HERRAMIENTAS_CON_SONIDO[] = {
    {"espada", TipoItem::ESPADA},
    {"pico", TipoItem::PICO},
    {"pala", TipoItem::PALA},
    {"regadera", TipoItem::REGADERA},
};
const std::pair<const char*, TipoEnemigo> ENEMIGOS_CON_SONIDO[] = {
    {"esqueleto", TipoEnemigo::ESQUELETO},
    {"orco", TipoEnemigo::ORCO},
    {"vampira", TipoEnemigo::VAMPIRA},
    {"vampiro", TipoEnemigo::VAMPIRO},
};
}  // namespace

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
    const toml::table archivo_toml = lector_toml::cargar_archivo(ruta);

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
    cargar_sonido(archivo_toml);
    cargar_tierra(archivo_toml);
    cargar_chat(archivo_toml);
}

void ConfiguracionCliente::cargar_servidor(const toml::table& archivo_toml) {
    const toml::table& servidor = lector_toml::exigir_tabla(archivo_toml, "servidor");

    configuracion_servidor.host = lector_toml::leer_texto(servidor, "host", "servidor");
    configuracion_servidor.puerto = lector_toml::leer_texto(servidor, "puerto", "servidor");
}

void ConfiguracionCliente::cargar_ventana(const toml::table& archivo_toml) {
    const toml::table& ventana = lector_toml::exigir_tabla(archivo_toml, "ventana");

    configuracion_vista.ventana.titulo = lector_toml::leer_texto(ventana, "titulo", "ventana");
    configuracion_vista.ventana.ancho =
        lector_toml::leer_entero_positivo(ventana, "ancho", "ventana");
    configuracion_vista.ventana.alto =
        lector_toml::leer_entero_positivo(ventana, "alto", "ventana");
    configuracion_vista.ventana.pantalla_completa =
        lector_toml::leer_booleano_opcional(ventana, "pantalla_completa", true);
    configuracion_vista.ventana.redimensionable =
        lector_toml::leer_booleano_opcional(ventana, "redimensionable", false);
}

void ConfiguracionCliente::cargar_sdl(const toml::table& archivo_toml) {
    const toml::table& sdl = lector_toml::exigir_tabla(archivo_toml, "sdl");

    configuracion_vista.sdl.acelerado = lector_toml::leer_booleano(sdl, "acelerado", "sdl");
    configuracion_vista.sdl.vsync = lector_toml::leer_booleano(sdl, "vsync", "sdl");
    configuracion_vista.sdl.fps_maximos =
        lector_toml::leer_entero_opcional(sdl, "fps_maximos", "sdl", 60, 0, 1000);
}

void ConfiguracionCliente::cargar_graficos(const toml::table& archivo_toml) {
    const toml::table& graficos = lector_toml::exigir_tabla(archivo_toml, "graficos");

    configuracion_vista.graficos.tamano_de_tile =
        lector_toml::leer_entero_positivo(graficos, "tamano_de_tile", "graficos");
    configuracion_vista.graficos.escala_de_tiles =
        lector_toml::leer_entero_en_rango(graficos, "escala_de_tiles", "graficos", 1, 8);
    configuracion_vista.graficos.escala_maxima_de_tiles =
        lector_toml::leer_entero_opcional(graficos, "escala_maxima_de_tiles", "graficos", 8, 1, 8);
    configuracion_vista.graficos.zoom_automatico =
        lector_toml::leer_booleano_opcional(graficos, "zoom_automatico", true);
    if (configuracion_vista.graficos.escala_maxima_de_tiles <
        configuracion_vista.graficos.escala_de_tiles) {
        throw std::runtime_error(
            "configuración: 'escala_maxima_de_tiles' no puede ser menor que 'escala_de_tiles'");
    }

    configuracion_vista.graficos.zoom_extra = graficos["zoom_extra"].value_or(1.0);
    if (configuracion_vista.graficos.zoom_extra < 1.0 ||
        configuracion_vista.graficos.zoom_extra > 3.0) {
        throw std::runtime_error(
            "configuración: 'zoom_extra' de [graficos] debe estar entre 1.0 y 3.0");
    }
    configuracion_vista.graficos.color_de_fondo = leer_color(graficos);
    configuracion_vista.graficos.radio_vision_mazmorra =
        lector_toml::leer_entero_opcional(graficos, "radio_vision_mazmorra", 0);
    configuracion_vista.graficos.tamano_pixel_vision_mazmorra =
        lector_toml::leer_entero_opcional(graficos, "tamano_pixel_vision_mazmorra", 8);
    configuracion_vista.graficos.ancho_difuminado_vision_mazmorra =
        lector_toml::leer_entero_opcional(graficos, "ancho_difuminado_vision_mazmorra", 24);
}

void ConfiguracionCliente::cargar_mapas(const toml::table& archivo_toml) {
    const toml::array& mapas = lector_toml::exigir_arreglo(archivo_toml, "mapas", "raíz");
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
        configuracion_mapa.id = lector_toml::leer_texto(*mapa, "id", seccion);
        configuracion_mapa.ruta = lector_toml::leer_texto(*mapa, "ruta", seccion);

        const bool id_repetido = std::any_of(
            configuracion_vista.mapas.begin(),
            configuracion_vista.mapas.end(),
            [&configuracion_mapa](const ConfiguracionMapa& existente) {
                return existente.id == configuracion_mapa.id;
            });
        if (id_repetido) {
            throw std::runtime_error(
                "configuración: el id de mapa '" + configuracion_mapa.id + "' está repetido");
        }
        configuracion_mapa.capas_bajo_los_jugadores =
            lector_toml::leer_textos(*mapa, "capas_bajo_los_jugadores", seccion);
        configuracion_mapa.capas_sobre_los_jugadores =
            lector_toml::leer_textos(*mapa, "capas_sobre_los_jugadores", seccion);
        configuracion_mapa.tilesets = leer_tilesets(*mapa, seccion);
        configuracion_mapa.es_mazmorra =
            lector_toml::leer_booleano_opcional(*mapa, "es_mazmorra", false);
        configuracion_mapa.ruta_ambiente =
            lector_toml::leer_texto_opcional(*mapa, "ruta_ambiente", seccion, "");
        configuracion_mapa.ruta_caminar =
            lector_toml::leer_texto_opcional(*mapa, "ruta_caminar", seccion, "");

        configuracion_vista.mapas.push_back(configuracion_mapa);
    }

    configuracion_vista.mapa_inicial =
        lector_toml::leer_texto(archivo_toml, "mapa_inicial", "raíz");
    const bool existe = std::any_of(
        configuracion_vista.mapas.begin(),
        configuracion_vista.mapas.end(),
        [this](const ConfiguracionMapa& m) { return m.id == configuracion_vista.mapa_inicial; });
    if (!existe) {
        throw std::runtime_error(
            "configuración: 'mapa_inicial' (" + configuracion_vista.mapa_inicial +
            ") no está entre los [[mapas]]");
    }
}

void ConfiguracionCliente::cargar_jugador(const toml::table& archivo_toml) {
    const toml::table& jugador = lector_toml::exigir_tabla(archivo_toml, "jugador");

    configuracion_vista.jugador.ruta_cuerpo =
        lector_toml::leer_texto(jugador, "ruta_cuerpo", "jugador");
    configuracion_vista.jugador.ruta_pantalon =
        lector_toml::leer_texto(jugador, "ruta_pantalon", "jugador");
    configuracion_vista.jugador.ruta_pelo =
        lector_toml::leer_texto(jugador, "ruta_pelo", "jugador");
    configuracion_vista.jugador.ruta_sombrero =
        lector_toml::leer_texto(jugador, "ruta_sombrero", "jugador");
    configuracion_vista.jugador.ruta_remera =
        lector_toml::leer_texto(jugador, "ruta_remera", "jugador");

    configuracion_vista.jugador.indice_pelo =
        lector_toml::leer_entero(jugador, "indice_pelo", "jugador");
    configuracion_vista.jugador.indice_sombrero =
        lector_toml::leer_entero(jugador, "indice_sombrero", "jugador");
    configuracion_vista.jugador.indice_remera =
        lector_toml::leer_entero(jugador, "indice_remera", "jugador");
    configuracion_vista.jugador.indice_pantalon =
        lector_toml::leer_entero(jugador, "indice_pantalon", "jugador");

    configuracion_vista.jugador.color_pelo =
        leer_color_rgb(lector_toml::exigir_tabla(jugador, "color_pelo"), "jugador.color_pelo");
    configuracion_vista.jugador.color_pantalon = leer_color_rgb(
        lector_toml::exigir_tabla(jugador, "color_pantalon"),
        "jugador.color_pantalon");

    configuracion_vista.jugador.ms_por_cuadro_caminando =
        lector_toml::leer_entero_positivo(jugador, "ms_por_cuadro_caminando", "jugador");
    configuracion_vista.jugador.offset_y_remera =
        lector_toml::leer_entero(jugador, "offset_y_remera", "jugador");
}

void ConfiguracionCliente::cargar_sonido(const toml::table& archivo_toml) {
    const toml::table& sonido = lector_toml::exigir_tabla(archivo_toml, "sonido");

    configuracion_vista.sonido.ruta_ambiente =
        lector_toml::leer_texto(sonido, "ruta_ambiente", "sonido");
    configuracion_vista.sonido.ruta_caminar =
        lector_toml::leer_texto(sonido, "ruta_caminar", "sonido");
    configuracion_vista.sonido.ruta_accion =
        lector_toml::leer_texto(sonido, "ruta_accion", "sonido");

    if (const toml::table* herramientas = sonido["herramientas"].as_table()) {
        for (const auto& [clave, tipo] : HERRAMIENTAS_CON_SONIDO) {
            const std::string ruta =
                lector_toml::leer_texto_opcional(*herramientas, clave, "sonido.herramientas", "");
            if (!ruta.empty()) {
                configuracion_vista.sonido.rutas_de_uso[tipo] = ruta;
            }
        }
    }
    if (const toml::table* pico = sonido["pico"].as_table()) {
        configuracion_vista.sonido.ruta_pico_golpe =
            lector_toml::leer_texto_opcional(*pico, "golpe", "sonido.pico", "");
        configuracion_vista.sonido.ruta_pico_rotura =
            lector_toml::leer_texto_opcional(*pico, "rotura", "sonido.pico", "");
    }
    if (const toml::table* trampas = sonido["trampas"].as_table()) {
        configuracion_vista.sonido.ruta_trampa_activada =
            lector_toml::leer_texto_opcional(*trampas, "activar", "sonido.trampas", "");
    }
    if (const toml::table* jugador = sonido["jugador"].as_table()) {
        configuracion_vista.sonido.ruta_jugador_danio =
            lector_toml::leer_texto_opcional(*jugador, "danio", "sonido.jugador", "");
    }
    if (auto* animales = sonido["animales"].as_table()) {
        configuracion_vista.sonido.ruta_vaca =
            lector_toml::leer_texto_opcional(*animales, "vaca", "sonido.animales", "");
        configuracion_vista.sonido.ruta_gallina =
            lector_toml::leer_texto_opcional(*animales, "gallina", "sonido.animales", "");
        configuracion_vista.sonido.ruta_ordeniar_vaca =
            lector_toml::leer_texto_opcional(*animales, "ordeniar", "sonido.animales", "");
    }
    if (auto* dinero = sonido["dinero"].as_table()) {
        configuracion_vista.sonido.ruta_dinero =
            lector_toml::leer_texto_opcional(*dinero, "contador", "sonido.dinero", "");
    }
    if (const toml::table* enemigos = sonido["enemigos"].as_table()) {
        for (const auto& [clave, tipo] : ENEMIGOS_CON_SONIDO) {
            const toml::table* enemigo = (*enemigos)[clave].as_table();
            if (enemigo == nullptr) {
                continue;
            }
            const std::string seccion = std::string("sonido.enemigos.") + clave;
            configuracion_vista.sonido.sonidos_de_enemigos[tipo] = SonidosDeEnemigo{
                lector_toml::leer_texto_opcional(*enemigo, "golpe", seccion, ""),
                lector_toml::leer_texto_opcional(*enemigo, "danio", seccion, ""),
                lector_toml::leer_texto_opcional(*enemigo, "muerte", seccion, ""),
                lector_toml::leer_texto_opcional(*enemigo, "aparicion", seccion, "")};
        }
    }
}

void ConfiguracionCliente::cargar_tierra(const toml::table& archivo_toml) {
    const toml::table& tierra = lector_toml::exigir_tabla(archivo_toml, "tierra");

    configuracion_vista.tierra.ruta_sprite =
        lector_toml::leer_texto(tierra, "ruta_sprite", "tierra");
}

void ConfiguracionCliente::cargar_chat(const toml::table& archivo_toml) {
    const toml::table& chat = lector_toml::exigir_tabla(archivo_toml, "chat");

    configuracion_vista.chat.ruta_fuente = lector_toml::leer_texto(chat, "ruta_fuente", "chat");
    configuracion_vista.chat.tamano_de_fuente =
        lector_toml::leer_entero_en_rango(chat, "tamano_de_fuente", "chat", 8, 48);
    configuracion_vista.chat.maximo_de_mensajes =
        lector_toml::leer_entero_opcional(chat, "maximo_de_mensajes", "chat", 100, 10, 1000);
    configuracion_vista.chat.ruta_icono = lector_toml::leer_texto(chat, "ruta_icono", "chat");
    configuracion_vista.chat.ruta_carpeta_de_emojis =
        lector_toml::leer_texto(chat, "ruta_carpeta_de_emojis", "chat");
    configuracion_vista.chat.ruta_carpeta_de_emotes =
        lector_toml::leer_texto(chat, "ruta_carpeta_de_emotes", "chat");
}

ColorDeFondo ConfiguracionCliente::leer_color(const toml::table& graficos) const {
    const toml::array& valores =
        lector_toml::exigir_arreglo(graficos, "color_de_fondo", "graficos");
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

ColorRGB ConfiguracionCliente::leer_color_rgb(
    const toml::table& color, const std::string& seccion) const {
    ColorRGB resultado;
    const auto r = color["r"].value<int>();
    const auto g = color["g"].value<int>();
    const auto b = color["b"].value<int>();
    if (!r || !g || !b || *r < 0 || *r > 255 || *g < 0 || *g > 255 || *b < 0 || *b > 255) {
        throw std::runtime_error(
            "configuración: '" + seccion + "' debe tener r, g, b entre 0 y 255");
    }
    resultado.rojo = static_cast<std::uint8_t>(*r);
    resultado.verde = static_cast<std::uint8_t>(*g);
    resultado.azul = static_cast<std::uint8_t>(*b);
    return resultado;
}

std::vector<ConfiguracionTileset> ConfiguracionCliente::leer_tilesets(
    const toml::table& mapa, const std::string& seccion) const {
    const toml::array& valores = lector_toml::exigir_arreglo(mapa, "tilesets", seccion);

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
            lector_toml::leer_texto(*tileset, "ruta", seccion_tileset),
            lector_toml::leer_entero_positivo(*tileset, "firstgid", seccion_tileset),
            lector_toml::leer_entero_positivo(*tileset, "columnas", seccion_tileset),
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

ConfiguracionJugador& ConfiguracionCliente::jugador() {
    return configuracion_vista.jugador;
}
