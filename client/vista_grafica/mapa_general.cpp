#include "mapa_general.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

// La parte del valle de map.png (la de arriba; el resto son otras zonas de Stardew).
#define ANCHO_DEL_VALLE 300
#define ALTO_DEL_VALLE 180

#define PUNTOS_DEL_TITULO 26
#define PUNTOS_DE_LA_LETRA 18
#define MARGEN 16
#define ANCHO_DE_LA_LISTA 270
#define GROSOR_DEL_MARCO 4
#define RADIO_DE_UN_LUGAR 8
#define RADIO_DE_UN_JUGADOR 8
#define RADIO_DEL_PROPIO 10
#define OPACIDAD_DEL_OSCURECIDO 120

namespace {

struct ZonaDelValle {
    const char* mapa;
    const char* nombre;
    SDL_Rect en_el_dibujo;
    int columnas;
    int filas;
};

const ZonaDelValle ZONAS[] = {
    {"granja", "Granja", {70, 60, 48, 52}, 80, 65},
    {"BusStop", "Parada del bus", {118, 48, 27, 14}, 65, 30},
    {"Town", "Pueblo", {145, 42, 95, 85}, 130, 110},
};

struct Interior {
    const char* mapa;
    const char* nombre;
    SDL_Point en_el_dibujo;
};

const SDL_Point CASA{106, 70};
const SDL_Point ENTRADA_A_LA_MAZMORRA{90, 63};
const SDL_Point MERCADO{176, 84};
const SDL_Point CENTRO_COMUNITARIO{183, 57};
const SDL_Point COLECTIVO{125, 56};

const Interior INTERIORES[] = {
    {"FarmHouse2", "Casa", CASA},
    {"mercado", "Mercado", MERCADO},
    {"centro_comunitario", "Centro comunitario", CENTRO_COMUNITARIO},
    {"centro_comunitario_arreglado", "Centro comunitario", CENTRO_COMUNITARIO},
    {"Bus_Interior", "Colectivo", COLECTIVO},
};

struct LugarImportante {
    const char* nombre;
    SDL_Point en_el_dibujo;
};

const LugarImportante LUGARES[] = {
    {"Granja", {94, 92}},
    {"Casa", CASA},
    {"Entrada a la mazmorra", ENTRADA_A_LA_MAZMORRA},
    {"Mercado", MERCADO},
    {"Centro comunitario", CENTRO_COMUNITARIO},
};

const std::pair<Estacion, const char*> RUTAS_DE_LOS_FONDOS[] = {
    {Estacion::PRIMAVERA, "Assets/LooseSprites/map.png"},
    {Estacion::VERANO, "Assets/LooseSprites/map_summer.png"},
    {Estacion::OTONO, "Assets/LooseSprites/map_fall.png"},
    {Estacion::INVIERNO, "Assets/LooseSprites/map_winter.png"},
};

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
const SDL_Color COLOR_DEL_PAPEL{250, 210, 140, 255};
const SDL_Color COLOR_DEL_MARCO{120, 60, 20, 255};
const SDL_Color COLOR_DE_LA_LISTA{60, 30, 10, 255};

}  // namespace

MapaGeneral::MapaGeneral(SDL_Renderer* renderer, const std::string& ruta_fuente)
    : fuente_del_titulo(renderer, ruta_fuente, PUNTOS_DEL_TITULO),
      fuente(renderer, ruta_fuente, PUNTOS_DE_LA_LETRA) {
    for (const auto& [estacion, ruta] : RUTAS_DE_LOS_FONDOS) {
        fondos.try_emplace(estacion, ruta, renderer);
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

std::optional<SDL_Point> MapaGeneral::punto_en_el_dibujo(
    const PosicionDeJugador& posicion, int lado) const {
    if (posicion.piso_de_mazmorra > 0) {
        return ENTRADA_A_LA_MAZMORRA;
    }
    for (const Interior& interior : INTERIORES) {
        if (posicion.mapa == interior.mapa) {
            return interior.en_el_dibujo;
        }
    }
    for (const ZonaDelValle& zona : ZONAS) {
        if (posicion.mapa != zona.mapa) {
            continue;
        }
        const double en_x = static_cast<double>(posicion.x) / (zona.columnas * lado);
        const double en_y = static_cast<double>(posicion.y) / (zona.filas * lado);
        return SDL_Point{
            zona.en_el_dibujo.x +
                static_cast<int>(std::clamp(en_x, 0.0, 1.0) * zona.en_el_dibujo.w),
            zona.en_el_dibujo.y +
                static_cast<int>(std::clamp(en_y, 0.0, 1.0) * zona.en_el_dibujo.h)};
    }
    return std::nullopt;
}

std::string MapaGeneral::lugar_de(const PosicionDeJugador& posicion) const {
    if (posicion.piso_de_mazmorra > 0) {
        return "Mazmorra, piso " + std::to_string(posicion.piso_de_mazmorra);
    }
    for (const Interior& interior : INTERIORES) {
        if (posicion.mapa == interior.mapa) {
            return interior.nombre;
        }
    }
    for (const ZonaDelValle& zona : ZONAS) {
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

void MapaGeneral::dibujar_lista(
    SDL_Renderer* renderer, const Datos& datos, const SDL_Rect& lugar) const {
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_DEL_MARCO.r,
        COLOR_DEL_MARCO.g,
        COLOR_DEL_MARCO.b,
        COLOR_DEL_MARCO.a);
    SDL_RenderFillRect(renderer, &lugar);
    const SDL_Rect papel{
        lugar.x + GROSOR_DEL_MARCO,
        lugar.y + GROSOR_DEL_MARCO,
        lugar.w - 2 * GROSOR_DEL_MARCO,
        lugar.h - 2 * GROSOR_DEL_MARCO};
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_DEL_PAPEL.r,
        COLOR_DEL_PAPEL.g,
        COLOR_DEL_PAPEL.b,
        COLOR_DEL_PAPEL.a);
    SDL_RenderFillRect(renderer, &papel);

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
    const int alto_del_titulo = fuente_del_titulo.alto_de_renglon() + MARGEN;
    const int alto_de_la_lista_abajo = fuente_del_titulo.alto_de_renglon() + MARGEN * 3 / 2 +
                                       GROSOR_DEL_MARCO * 2 +
                                       static_cast<int>(datos.posiciones.jugadores.size()) *
                                           (fuente.alto_de_renglon() + MARGEN / 2);
    const int escala_al_costado = std::min(
        (area.w - ANCHO_DE_LA_LISTA - 3 * MARGEN) / ANCHO_DEL_VALLE,
        (area.h - alto_del_titulo - 2 * MARGEN) / ALTO_DEL_VALLE);
    const int escala_abajo = std::min(
        (area.w - 2 * MARGEN) / ANCHO_DEL_VALLE,
        (area.h - alto_del_titulo - alto_de_la_lista_abajo - 3 * MARGEN) / ALTO_DEL_VALLE);
    const bool lista_abajo = escala_abajo > escala_al_costado;
    const int escala = std::max(1, lista_abajo ? escala_abajo : escala_al_costado);
    const int ancho = ANCHO_DEL_VALLE * escala;
    const int alto = ALTO_DEL_VALLE * escala;
    const int ancho_total = lista_abajo ? ancho : ancho + MARGEN + ANCHO_DE_LA_LISTA;
    const int alto_total =
        alto_del_titulo + alto + (lista_abajo ? MARGEN + alto_de_la_lista_abajo : 0);
    const SDL_Rect dibujo{
        area.x + std::max(0, (area.w - ancho_total) / 2),
        area.y + std::max(0, (area.h - alto_total) / 2) + alto_del_titulo,
        ancho,
        alto};
    const SDL_Rect lugar_de_la_lista =
        lista_abajo
            ? SDL_Rect{dibujo.x, dibujo.y + dibujo.h + MARGEN, ancho, alto_de_la_lista_abajo}
            : SDL_Rect{dibujo.x + dibujo.w + MARGEN, dibujo.y, ANCHO_DE_LA_LISTA, dibujo.h};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, OPACIDAD_DEL_OSCURECIDO);
    SDL_RenderFillRect(renderer, &area);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    escribir(
        renderer,
        fuente_del_titulo,
        "Mapa del valle  (Tab o M para cerrar)",
        dibujo.x,
        dibujo.y - alto_del_titulo,
        COLOR_DEL_TEXTO);
    const SDL_Rect marco{
        dibujo.x - GROSOR_DEL_MARCO,
        dibujo.y - GROSOR_DEL_MARCO,
        dibujo.w + 2 * GROSOR_DEL_MARCO,
        dibujo.h + 2 * GROSOR_DEL_MARCO};
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_DEL_MARCO.r,
        COLOR_DEL_MARCO.g,
        COLOR_DEL_MARCO.b,
        COLOR_DEL_MARCO.a);
    SDL_RenderFillRect(renderer, &marco);
    const auto fondo = fondos.find(datos.estacion);
    const SDL_Rect valle{0, 0, ANCHO_DEL_VALLE, ALTO_DEL_VALLE};
    SDL_RenderCopy(
        renderer,
        (fondo != fondos.end() ? fondo->second : fondos.at(Estacion::PRIMAVERA)).ptr(),
        &valle,
        &dibujo);

    const auto en_pantalla = [&](const SDL_Point& punto) {
        return SDL_Point{dibujo.x + punto.x * escala, dibujo.y + punto.y * escala};
    };
    for (const LugarImportante& lugar : LUGARES) {
        const SDL_Point punto = en_pantalla(lugar.en_el_dibujo);
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
            punto_en_el_dibujo(*posicion, std::max(1, datos.tamano_de_tile));
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
