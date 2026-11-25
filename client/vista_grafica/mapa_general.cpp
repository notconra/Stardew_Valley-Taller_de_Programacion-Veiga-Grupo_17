#include "mapa_general.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../../common/editor/cargador_de_mundo_tmx.h"
#include "../../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "../../common/llegadas.h"
#include "mapa_grafico.h"

// La parte del valle de map.png (la de arriba; el resto son otras zonas de Stardew).
#define ANCHO_DEL_VALLE 300
#define ALTO_DEL_VALLE 180

// En un mundo del editor, el espacio entre la granja y el pueblo, en píxeles del plano.
#define SEPARACION_DE_LOS_MAPAS 32
// La capa de los TMX con las llegadas.
#define CAPA_DE_TRANSICIONES "Transiciones"

#define PUNTOS_DEL_TITULO 26
#define PUNTOS_DE_LA_LETRA 18
#define MARGEN 16
#define ANCHO_DE_LA_LISTA 270

#define GROSOR_DEL_MARCO 12
#define BORDE_EN_EL_PNG 12
#define LADO_DEL_PNG 56
#define RUTA_DEL_MARCO "Assets/LooseSprites/menu_box.png"
#define RADIO_DE_UN_LUGAR 8
#define RADIO_DE_UN_JUGADOR 8
#define RADIO_DEL_PROPIO 10
#define OPACIDAD_DEL_OSCURECIDO 120

namespace {

// Dónde está cada cosa en el dibujo del valle (píxeles de map.png).
const SDL_Point CASA{106, 70};
const SDL_Point ENTRADA_A_LA_MAZMORRA{90, 63};
const SDL_Point MERCADO{176, 84};
const SDL_Point CENTRO_COMUNITARIO{183, 57};
const SDL_Point COLECTIVO{125, 56};
const SDL_Point GRANJA{94, 92};

const std::pair<Estacion, const char*> RUTAS_DE_LOS_FONDOS[] = {
    {Estacion::PRIMAVERA, "Assets/LooseSprites/map.png"},
    {Estacion::VERANO, "Assets/LooseSprites/map_summer.png"},
    {Estacion::OTONO, "Assets/LooseSprites/map_fall.png"},
    {Estacion::INVIERNO, "Assets/LooseSprites/map_winter.png"},
};

// El propio va en celeste; los demás, en el orden de la partida, con estos colores.
const SDL_Color COLOR_PROPIO{70, 200, 255, 255};
const SDL_Color COLORES_DE_LOS_DEMAS[] = {
    {240, 90, 90, 255},
    {120, 220, 110, 255},
    {220, 120, 250, 255},
    {250, 160, 60, 255},
    {250, 120, 190, 255},
    {240, 230, 90, 255},
};
const SDL_Color COLOR_DE_UN_LUGAR{250, 210, 60, 255};
const SDL_Color COLOR_DEL_BORDE{40, 20, 10, 255};
const SDL_Color COLOR_DEL_TEXTO{255, 240, 200, 255};
const SDL_Color COLOR_DEL_NOMBRE{255, 255, 255, 255};
const SDL_Color COLOR_DE_LA_LISTA{60, 30, 10, 255};

}  // namespace

MapaGeneral::MapaGeneral(SDL_Renderer* renderer, const std::string& ruta_fuente)
    : fuente_del_titulo(renderer, ruta_fuente, PUNTOS_DEL_TITULO),
      fuente(renderer, ruta_fuente, PUNTOS_DE_LA_LETRA),
      marco_del_menu(RUTA_DEL_MARCO, renderer) {
    usar_el_valle(renderer);
}

void MapaGeneral::usar_el_valle(SDL_Renderer* renderer) {
    mundo_propio.reset();
    ancho_del_plano = ANCHO_DEL_VALLE;
    alto_del_plano = ALTO_DEL_VALLE;
    // Su tamaño en casillas es el de su .tmx.
    zonas = {
        {"granja", "Granja", {70, 60, 48, 52}, 80, 65},
        {"BusStop", "Parada del bus", {118, 48, 27, 14}, 65, 30},
        {"Town", "Pueblo", {145, 42, 95, 85}, 130, 110},
    };
    interiores = {
        {"FarmHouse2", "Casa", CASA},
        {"mercado", "Mercado", MERCADO},
        {"centro_comunitario", "Centro comunitario", CENTRO_COMUNITARIO},
        {"centro_comunitario_arreglado", "Centro comunitario", CENTRO_COMUNITARIO},
        {"Bus_Interior", "Colectivo", COLECTIVO},
    };
    lugares = {
        {"", "Granja", GRANJA},
        {"", "Casa", CASA},
        {"", "Entrada a la mazmorra", ENTRADA_A_LA_MAZMORRA},
        {"", "Mercado", MERCADO},
        {"", "Centro comunitario", CENTRO_COMUNITARIO},
    };
    entrada_a_la_mazmorra = ENTRADA_A_LA_MAZMORRA;
    fondos.clear();
    for (const auto& [estacion, ruta] : RUTAS_DE_LOS_FONDOS) {
        std::vector<Pieza> piezas;
        piezas.push_back(Pieza{
            Textura(ruta, renderer),
            SDL_Rect{0, 0, ANCHO_DEL_VALLE, ALTO_DEL_VALLE},
            SDL_Rect{0, 0, ANCHO_DEL_VALLE, ALTO_DEL_VALLE}});
        fondos.emplace(estacion, std::move(piezas));
    }
}

void MapaGeneral::usar_mundo(
    const std::string& nombre_mundo, const ConfiguracionMapa& granja,
    const ConfiguracionMapa& pueblo, const ConfiguracionGraficos& graficos) {
    const std::string ruta_granja =
        CargadorDeMundoTMX::resolver_ruta_sector(nombre_mundo, "granja");
    const std::string ruta_pueblo = CargadorDeMundoTMX::resolver_ruta_sector(nombre_mundo, "Town");
    if (ruta_granja.empty() || ruta_pueblo.empty()) {
        return;
    }
    const LectorDeMapasTMX lector_granja(ruta_granja);
    const LectorDeMapasTMX lector_pueblo(ruta_pueblo);
    const TamanioMapa tamanio_granja = lector_granja.leer_tamanio();
    const TamanioMapa tamanio_pueblo = lector_pueblo.leer_tamanio();
    if (tamanio_granja.ancho_en_casillas <= 0 || tamanio_pueblo.ancho_en_casillas <= 0) {
        std::cerr << "[mapa general] no se pudo leer el mundo " << nombre_mundo << "\n";
        return;
    }

    const int ancho_granja = tamanio_granja.ancho_en_casillas * tamanio_granja.ancho_de_casilla;
    const int alto_granja = tamanio_granja.alto_en_casillas * tamanio_granja.alto_de_casilla;
    const int ancho_pueblo = tamanio_pueblo.ancho_en_casillas * tamanio_pueblo.ancho_de_casilla;
    const int alto_pueblo = tamanio_pueblo.alto_en_casillas * tamanio_pueblo.alto_de_casilla;
    ancho_del_plano = ancho_granja + SEPARACION_DE_LOS_MAPAS + ancho_pueblo;
    alto_del_plano = std::max(alto_granja, alto_pueblo);
    bool pueblo_a_la_izquierda = false;
    for (const TransicionTMX& transicion :
         lector_granja.cargar_transiciones(CAPA_DE_TRANSICIONES)) {
        if (transicion.mapa_destino == "Town") {
            pueblo_a_la_izquierda = transicion.x + transicion.ancho / 2 < ancho_granja / 2;
        }
    }
    const SDL_Rect en_la_granja{
        pueblo_a_la_izquierda ? ancho_pueblo + SEPARACION_DE_LOS_MAPAS : 0,
        (alto_del_plano - alto_granja) / 2,
        ancho_granja,
        alto_granja};
    const SDL_Rect en_el_pueblo{
        pueblo_a_la_izquierda ? 0 : ancho_granja + SEPARACION_DE_LOS_MAPAS,
        (alto_del_plano - alto_pueblo) / 2,
        ancho_pueblo,
        alto_pueblo};
    zonas = {
        {"granja",
         "Granja",
         en_la_granja,
         tamanio_granja.ancho_en_casillas,
         tamanio_granja.alto_en_casillas},
        {"Town",
         "Pueblo",
         en_el_pueblo,
         tamanio_pueblo.ancho_en_casillas,
         tamanio_pueblo.alto_en_casillas},
    };

    // El medio de la casilla de la llegada, o el medio del mapa si no la tiene.
    const std::vector<LlegadaTMX> llegadas_granja =
        lector_granja.cargar_llegadas(CAPA_DE_TRANSICIONES);
    const std::vector<LlegadaTMX> llegadas_pueblo =
        lector_pueblo.cargar_llegadas(CAPA_DE_TRANSICIONES);
    const auto punto_de = [](const std::vector<LlegadaTMX>& llegadas,
                             const SDL_Rect& zona,
                             const TamanioMapa& tamanio,
                             const std::string& nombre) {
        for (const LlegadaTMX& llegada : llegadas) {
            if (llegada.nombre == nombre) {
                return SDL_Point{
                    zona.x + llegada.x + tamanio.ancho_de_casilla / 2,
                    zona.y + llegada.y + tamanio.alto_de_casilla / 2};
            }
        }
        return SDL_Point{zona.x + zona.w / 2, zona.y + zona.h / 2};
    };
    const SDL_Point casa =
        punto_de(llegadas_granja, en_la_granja, tamanio_granja, LLEGADA_PUERTA_DE_LA_CASA);
    const SDL_Point cueva = punto_de(llegadas_granja, en_la_granja, tamanio_granja, LLEGADA_CUEVA);
    const SDL_Point mercado =
        punto_de(llegadas_pueblo, en_el_pueblo, tamanio_pueblo, LLEGADA_PUERTA_DEL_MERCADO);
    const SDL_Point centro =
        punto_de(llegadas_pueblo, en_el_pueblo, tamanio_pueblo, LLEGADA_PUERTA_DEL_CENTRO);
    interiores = {
        {"FarmHouse2", "Casa", casa},
        {"mercado", "Mercado", mercado},
        {"centro_comunitario", "Centro comunitario", centro},
        {"centro_comunitario_arreglado", "Centro comunitario", centro},
    };
    lugares = {
        {"", "Granja", SDL_Point{en_la_granja.x + MARGEN, en_la_granja.y + MARGEN}},
        {"", "Pueblo", SDL_Point{en_el_pueblo.x + MARGEN, en_el_pueblo.y + MARGEN}},
        {"", "Casa", casa},
        {"", "Entrada a la mazmorra", cueva},
        {"", "Mercado", mercado},
        {"", "Centro comunitario", centro},
    };
    entrada_a_la_mazmorra = cueva;
    mundo_propio = MundoPropio{nombre_mundo, granja, pueblo, graficos};
    fondos.clear();
}

Textura MapaGeneral::dibujar_en_una_imagen(
    SDL_Renderer* renderer, const ConfiguracionMapa& configuracion, Estacion estacion) const {
    ConfiguracionGraficos graficos = mundo_propio->graficos;
    graficos.escala_de_tiles = 1;
    const MapaGrafico mapa(renderer, configuracion, graficos, estacion, mundo_propio->nombre);
    SDL_Texture* imagen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        mapa.ancho_en_casillas() * graficos.tamano_de_tile,
        mapa.alto_en_casillas() * graficos.tamano_de_tile);
    if (imagen == nullptr) {
        throw std::runtime_error(
            std::string("no se pudo crear la imagen del mapa: ") + SDL_GetError());
    }
    Textura textura(imagen);
    SDL_SetTextureBlendMode(imagen, SDL_BLENDMODE_BLEND);
    SDL_Texture* anterior = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, imagen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    mapa.dibujar_bajo_los_jugadores(renderer, 0, 0);
    mapa.dibujar_sobre_los_jugadores(renderer, 0, 0);
    SDL_SetRenderTarget(renderer, anterior);
    return textura;
}

void MapaGeneral::preparar(SDL_Renderer* renderer, Estacion estacion) {
    if (!mundo_propio.has_value() || fondos.count(estacion) > 0) {
        return;
    }
    try {
        std::vector<Pieza> piezas;
        const ConfiguracionMapa* mapas[] = {&mundo_propio->granja, &mundo_propio->pueblo};
        for (size_t indice = 0; indice < zonas.size() && indice < std::size(mapas); ++indice) {
            const SDL_Rect& zona = zonas[indice].en_el_plano;
            piezas.push_back(Pieza{
                dibujar_en_una_imagen(renderer, *mapas[indice], estacion),
                SDL_Rect{0, 0, zona.w, zona.h},
                zona});
        }
        fondos.emplace(estacion, std::move(piezas));
    } catch (const std::exception& error) {
        std::cerr << "[mapa general] no se pudo dibujar el mundo " << mundo_propio->nombre << ": "
                  << error.what() << "\n";
        usar_el_valle(renderer);
    }
}

void MapaGeneral::alternar() {
    abierto = !abierto;
}

void MapaGeneral::cerrar() {
    abierto = false;
}

bool MapaGeneral::esta_abierto() const {
    return abierto;
}

std::optional<SDL_Point> MapaGeneral::punto_en_el_plano(
    const PosicionDeJugador& posicion, int lado) const {
    if (posicion.piso_de_mazmorra > 0) {
        return entrada_a_la_mazmorra;
    }
    for (const Punto& interior : interiores) {
        if (posicion.mapa == interior.clave) {
            return interior.en_el_plano;
        }
    }
    for (const Zona& zona : zonas) {
        if (posicion.mapa != zona.mapa) {
            continue;
        }
        // En proporción dentro del rectángulo de la zona, sin salirse de él.
        const double en_x = static_cast<double>(posicion.x) / (zona.columnas * lado);
        const double en_y = static_cast<double>(posicion.y) / (zona.filas * lado);
        return SDL_Point{
            zona.en_el_plano.x + static_cast<int>(std::clamp(en_x, 0.0, 1.0) * zona.en_el_plano.w),
            zona.en_el_plano.y + static_cast<int>(std::clamp(en_y, 0.0, 1.0) * zona.en_el_plano.h)};
    }
    return std::nullopt;
}

std::string MapaGeneral::lugar_de(const PosicionDeJugador& posicion) const {
    if (posicion.piso_de_mazmorra > 0) {
        return "Mazmorra, piso " + std::to_string(posicion.piso_de_mazmorra);
    }
    for (const Punto& interior : interiores) {
        if (posicion.mapa == interior.clave) {
            return interior.nombre;
        }
    }
    for (const Zona& zona : zonas) {
        if (posicion.mapa == zona.mapa) {
            return zona.nombre;
        }
    }
    return posicion.mapa;
}

std::string MapaGeneral::nombre_de(const JugadoresDeLaPartida& jugadores, uint32_t id) const {
    for (const JugadorDeLaPartida& jugador : jugadores.jugadores) {
        if (jugador.id_jugador == id) {
            return jugador.nombre;
        }
    }
    return "Jugador " + std::to_string(id);
}

SDL_Color MapaGeneral::color_de(const Datos& datos, uint32_t id_jugador) const {
    if (id_jugador == datos.id_propio) {
        return COLOR_PROPIO;
    }
    // Según su lugar entre los demás de la partida, así cada uno tiene siempre el mismo color.
    size_t indice = 0;
    for (const JugadorDeLaPartida& jugador : datos.jugadores.jugadores) {
        if (jugador.id_jugador == id_jugador) {
            break;
        }
        if (jugador.id_jugador != datos.id_propio) {
            ++indice;
        }
    }
    return COLORES_DE_LOS_DEMAS[indice % std::size(COLORES_DE_LOS_DEMAS)];
}

void MapaGeneral::dibujar_circulo(
    SDL_Renderer* renderer, int centro_x, int centro_y, int radio, SDL_Color color) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int dy = -radio; dy <= radio; ++dy) {
        int dx = 0;
        while ((dx + 1) * (dx + 1) + dy * dy <= radio * radio) {
            ++dx;
        }
        SDL_RenderDrawLine(renderer, centro_x - dx, centro_y + dy, centro_x + dx, centro_y + dy);
    }
}

void MapaGeneral::escribir(
    SDL_Renderer* renderer, const FuenteDelJuego& letra, const std::string& texto, int x, int y,
    SDL_Color color) const {
    for (const SDL_Point& corrido :
         {SDL_Point{-2, 0}, SDL_Point{2, 0}, SDL_Point{0, -2}, SDL_Point{0, 2}}) {
        letra.dibujar(renderer, texto, x + corrido.x, y + corrido.y, COLOR_DEL_BORDE);
    }
    letra.dibujar(renderer, texto, x, y, color);
}

void MapaGeneral::dibujar_marco(SDL_Renderer* renderer, const SDL_Rect& exterior) const {
    const int borde = std::max(1, std::min({GROSOR_DEL_MARCO, exterior.w / 2, exterior.h / 2}));
    const int b = BORDE_EN_EL_PNG;
    const int medio = LADO_DEL_PNG - 2 * b;
    const int x = exterior.x;
    const int y = exterior.y;
    const int ancho_del_medio = exterior.w - 2 * borde;
    const int alto_del_medio = exterior.h - 2 * borde;

    // los 9 pedazos pars cada recorte del PNG con el lugar que ocupa en pantalla
    const std::pair<SDL_Rect, SDL_Rect> pedazos[] = {
        // Esquinas.
        {{0, 0, b, b}, {x, y, borde, borde}},
        {{b + medio, 0, b, b}, {x + exterior.w - borde, y, borde, borde}},
        {{0, b + medio, b, b}, {x, y + exterior.h - borde, borde, borde}},
        {{b + medio, b + medio, b, b},
         {x + exterior.w - borde, y + exterior.h - borde, borde, borde}},

        {{b, 0, medio, b}, {x + borde, y, ancho_del_medio, borde}},
        {{b, b + medio, medio, b}, {x + borde, y + exterior.h - borde, ancho_del_medio, borde}},
        {{0, b, b, medio}, {x, y + borde, borde, alto_del_medio}},
        {{b + medio, b, b, medio}, {x + exterior.w - borde, y + borde, borde, alto_del_medio}},

        {{b, b, medio, medio}, {x + borde, y + borde, ancho_del_medio, alto_del_medio}},
    };
    for (const auto& [recorte, destino] : pedazos) {
        if (destino.w > 0 && destino.h > 0) {
            SDL_RenderCopy(renderer, marco_del_menu.ptr(), &recorte, &destino);
        }
    }
}

void MapaGeneral::dibujar_lista(
    SDL_Renderer* renderer, const Datos& datos, const SDL_Rect& lugar) const {
    dibujar_marco(renderer, lugar);
    const SDL_Rect papel{
        lugar.x + GROSOR_DEL_MARCO,
        lugar.y + GROSOR_DEL_MARCO,
        lugar.w - 2 * GROSOR_DEL_MARCO,
        lugar.h - 2 * GROSOR_DEL_MARCO};

    const int renglon = fuente.alto_de_renglon() + MARGEN / 2;
    int y = papel.y + MARGEN / 2;
    escribir(renderer, fuente_del_titulo, "Jugadores", papel.x + MARGEN, y, COLOR_DEL_TEXTO);
    y += fuente_del_titulo.alto_de_renglon() + MARGEN / 2;
    for (const PosicionDeJugador& posicion : datos.posiciones.jugadores) {
        if (y + renglon > papel.y + papel.h) {
            break;
        }
        const int medio = y + fuente.alto_de_renglon() / 2;
        dibujar_circulo(
            renderer,
            papel.x + MARGEN + RADIO_DE_UN_JUGADOR,
            medio,
            RADIO_DE_UN_JUGADOR + 2,
            COLOR_DEL_BORDE);
        dibujar_circulo(
            renderer,
            papel.x + MARGEN + RADIO_DE_UN_JUGADOR,
            medio,
            RADIO_DE_UN_JUGADOR,
            color_de(datos, posicion.id_jugador));
        fuente.dibujar(
            renderer,
            nombre_de(datos.jugadores, posicion.id_jugador) + " - " + lugar_de(posicion),
            papel.x + MARGEN + 2 * RADIO_DE_UN_JUGADOR + MARGEN / 2,
            y,
            COLOR_DE_LA_LISTA);
        y += renglon;
    }
}

void MapaGeneral::dibujar(SDL_Renderer* renderer, const Datos& datos, const SDL_Rect& area) const {
    if (!abierto) {
        return;
    }
    const int alto_del_titulo = fuente_del_titulo.alto_de_renglon() + MARGEN + GROSOR_DEL_MARCO;
    const int alto_de_la_lista_abajo = fuente_del_titulo.alto_de_renglon() + MARGEN * 3 / 2 +
                                       GROSOR_DEL_MARCO * 2 +
                                       static_cast<int>(datos.posiciones.jugadores.size()) *
                                           (fuente.alto_de_renglon() + MARGEN / 2);
    // El plano se agranda en números enteros si entra más grande que su tamaño (el valle, que es
    // pixel art) y si no se achica lo que haga falta (un mundo del editor). La lista va al costado
    // o, si así el plano queda más grande (por ejemplo con el chat abierto), abajo.
    const double escala_al_costado = std::min(
        static_cast<double>(area.w - ANCHO_DE_LA_LISTA - 2 * GROSOR_DEL_MARCO - 3 * MARGEN) /
            ancho_del_plano,
        static_cast<double>(area.h - alto_del_titulo - GROSOR_DEL_MARCO - 2 * MARGEN) /
            alto_del_plano);
    const double escala_abajo = std::min(
        static_cast<double>(area.w - 2 * GROSOR_DEL_MARCO - 2 * MARGEN) / ancho_del_plano,
        static_cast<double>(
            area.h - alto_del_titulo - GROSOR_DEL_MARCO - alto_de_la_lista_abajo - 3 * MARGEN) /
            alto_del_plano);
    const bool lista_abajo = escala_abajo > escala_al_costado;
    double escala = lista_abajo ? escala_abajo : escala_al_costado;
    escala = escala >= 1.0 ? std::floor(escala) : std::max(escala, 0.1);
    const int ancho = static_cast<int>(ancho_del_plano * escala);
    const int alto = static_cast<int>(alto_del_plano * escala);
    // con el marco el mapa ocupa "ancho + 2 * GROSOR_DEL_MARCO" de ancho (y lo mismo de alto)
    const int ancho_con_marco = ancho + 2 * GROSOR_DEL_MARCO;
    const int ancho_total =
        lista_abajo ? ancho_con_marco : ancho_con_marco + MARGEN + ANCHO_DE_LA_LISTA;
    const int alto_total = alto_del_titulo + alto + GROSOR_DEL_MARCO +
                           (lista_abajo ? MARGEN + alto_de_la_lista_abajo : 0);
    const SDL_Rect dibujo{
        area.x + std::max(0, (area.w - ancho_total) / 2) + GROSOR_DEL_MARCO,
        area.y + std::max(0, (area.h - alto_total) / 2) + alto_del_titulo,
        ancho,
        alto};
    const SDL_Rect marco{
        dibujo.x - GROSOR_DEL_MARCO,
        dibujo.y - GROSOR_DEL_MARCO,
        ancho_con_marco,
        alto + 2 * GROSOR_DEL_MARCO};
    const SDL_Rect lugar_de_la_lista =
        lista_abajo ? SDL_Rect{marco.x, marco.y + marco.h + MARGEN, marco.w, alto_de_la_lista_abajo}
                    : SDL_Rect{marco.x + marco.w + MARGEN, marco.y, ANCHO_DE_LA_LISTA, marco.h};

    // Se oscurece lo de atrás, pero se sigue viendo el juego.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, OPACIDAD_DEL_OSCURECIDO);
    SDL_RenderFillRect(renderer, &area);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    escribir(
        renderer,
        fuente_del_titulo,
        mundo_propio.has_value() ? "Mapa de " + mundo_propio->nombre + "  (Tab o M para cerrar)"
                                 : std::string("Mapa del valle  (Tab o M para cerrar)"),
        marco.x,
        dibujo.y - alto_del_titulo,
        COLOR_DEL_TEXTO);

    dibujar_marco(renderer, marco);
    const auto en_pantalla = [&](const SDL_Point& punto) {
        return SDL_Point{
            dibujo.x + static_cast<int>(punto.x * escala),
            dibujo.y + static_cast<int>(punto.y * escala)};
    };
    const auto fondo = fondos.find(datos.estacion);
    if (fondo != fondos.end()) {
        for (const Pieza& pieza : fondo->second) {
            const SDL_Point esquina = en_pantalla({pieza.en_el_plano.x, pieza.en_el_plano.y});
            const SDL_Rect destino{
                esquina.x,
                esquina.y,
                static_cast<int>(pieza.en_el_plano.w * escala),
                static_cast<int>(pieza.en_el_plano.h * escala)};
            SDL_RenderCopy(renderer, pieza.imagen.ptr(), &pieza.recorte, &destino);
        }
    }

    for (const Punto& lugar : lugares) {
        const SDL_Point punto = en_pantalla(lugar.en_el_plano);
        dibujar_circulo(renderer, punto.x, punto.y, RADIO_DE_UN_LUGAR + 2, COLOR_DEL_BORDE);
        dibujar_circulo(renderer, punto.x, punto.y, RADIO_DE_UN_LUGAR, COLOR_DE_UN_LUGAR);
        escribir(
            renderer,
            fuente,
            lugar.nombre,
            punto.x + RADIO_DE_UN_LUGAR + 4,
            punto.y - fuente.alto_de_renglon() / 2,
            COLOR_DEL_TEXTO);
    }

    // El propio al final, así queda encima de los demás.
    std::vector<const PosicionDeJugador*> en_orden;
    for (const PosicionDeJugador& posicion : datos.posiciones.jugadores) {
        if (posicion.id_jugador != datos.id_propio) {
            en_orden.push_back(&posicion);
        }
    }
    for (const PosicionDeJugador& posicion : datos.posiciones.jugadores) {
        if (posicion.id_jugador == datos.id_propio) {
            en_orden.push_back(&posicion);
        }
    }
    for (const PosicionDeJugador* posicion : en_orden) {
        const std::optional<SDL_Point> punto =
            punto_en_el_plano(*posicion, std::max(1, datos.tamano_de_tile));
        if (!punto.has_value()) {
            continue;
        }
        const SDL_Point centro = en_pantalla(*punto);
        const bool es_el_propio = posicion->id_jugador == datos.id_propio;
        const int radio = es_el_propio ? RADIO_DEL_PROPIO : RADIO_DE_UN_JUGADOR;
        dibujar_circulo(
            renderer,
            centro.x,
            centro.y,
            radio + 3,
            es_el_propio ? COLOR_DEL_NOMBRE : COLOR_DEL_BORDE);
        dibujar_circulo(renderer, centro.x, centro.y, radio, color_de(datos, posicion->id_jugador));
        const std::string nombre = nombre_de(datos.jugadores, posicion->id_jugador);
        escribir(
            renderer,
            fuente,
            nombre,
            centro.x - fuente.ancho(nombre) / 2,
            centro.y + radio + 2,
            COLOR_DEL_NOMBRE);
    }

    dibujar_lista(renderer, datos, lugar_de_la_lista);
}
