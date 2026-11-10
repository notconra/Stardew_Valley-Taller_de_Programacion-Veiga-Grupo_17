#include "enemigos_graficos.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

#include "color_de_vida.h"

namespace {

using Hoja = EnemigosGraficos::Hoja;
using Animacion = EnemigosGraficos::Animacion;

constexpr std::array<int, 4> FILAS_CRAFTPIX{1, 0, 2, 3};

Hoja hoja_de_esqueleto() {
    const std::string imagen = "Assets/Characters/Monsters/Skeleton.png";
    Hoja hoja;
    hoja.frame_ancho = 16;
    hoja.frame_alto = 32;
    hoja.pies_y = 32;
    hoja.cabeza_y = 3;
    hoja.quieto = {imagen, {2, 0, 3, 1}, 1, 1000};
    hoja.caminar = {imagen, {2, 0, 3, 1}, 4, 120};
    hoja.ataque = {imagen, {5, 5, 5, 5}, 4, 100};
    hoja.muerte = {imagen, {4, 4, 4, 4}, 4, 900};
    hoja.muerte_en_huesos = true;
    return hoja;
}

Hoja hoja_de_orco() {
    const std::string carpeta = "Assets/Caverns/Enemies/Orc1/";
    Hoja hoja;
    hoja.frame_ancho = 64;
    hoja.frame_alto = 64;
    hoja.pies_y = 46;
    hoja.cabeza_y = 14;
    hoja.quieto = {carpeta + "Orc1_idle/orc1_idle_full.png", FILAS_CRAFTPIX, 4, 150};
    hoja.caminar = {carpeta + "Orc1_walk/orc1_walk_full.png", FILAS_CRAFTPIX, 6, 80};
    hoja.ataque = {carpeta + "Orc1_attack/orc1_attack_full.png", FILAS_CRAFTPIX, 8, 50};
    hoja.muerte = {carpeta + "Orc1_death/orc1_death_full.png", FILAS_CRAFTPIX, 8, 110};
    return hoja;
}

Hoja hoja_de_vampiro(const std::string& carpeta, const std::string& prefijo) {
    const auto imagen = [&](const std::string& animacion) {
        return "Assets/Caverns/Enemies/" + carpeta + "/" + prefijo + "_" + animacion +
               "_with_shadow.png";
    };
    Hoja hoja;
    hoja.frame_ancho = 64;
    hoja.frame_alto = 64;
    hoja.pies_y = 48;
    hoja.cabeza_y = 14;
    hoja.quieto = {imagen("Idle"), FILAS_CRAFTPIX, 4, 150};
    hoja.caminar = {imagen("Walk"), FILAS_CRAFTPIX, 6, 80};
    hoja.ataque = {imagen("Attack"), FILAS_CRAFTPIX, 12, 33};
    hoja.muerte = {imagen("Death"), FILAS_CRAFTPIX, 11, 80};
    hoja.correr = {imagen("Run"), FILAS_CRAFTPIX, 8, 60};
    return hoja;
}

int cuadro_una_vez(const Animacion& animacion, uint32_t desde, uint32_t ahora) {
    return std::min(static_cast<int>((ahora - desde) / animacion.ms_por_cuadro), animacion.cuadros - 1);
}

int cuadro_en_bucle(const Animacion& animacion, uint32_t ahora) {
    return static_cast<int>((ahora / animacion.ms_por_cuadro) % animacion.cuadros);
}

void marcar_inicio(bool activo, uint32_t& desde, uint32_t ahora) {
    if (!activo) {
        desde = 0;
    } else if (desde == 0) {
        desde = ahora | 1;
    }
}

float interpolar(float desde, float hasta, float t) {
    return desde + (hasta - desde) * t;
}

}  // namespace

EnemigosGraficos::EnemigosGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : tamano_de_tile(graficos.tamano_de_tile), escala_de_tiles(graficos.escala_de_tiles) {
    hojas.emplace(TipoEnemigo::ESQUELETO, hoja_de_esqueleto());
    hojas.emplace(TipoEnemigo::ORCO, hoja_de_orco());
    hojas.emplace(TipoEnemigo::VAMPIRA, hoja_de_vampiro("vampira", "Vampires3"));
    hojas.emplace(TipoEnemigo::VAMPIRO, hoja_de_vampiro("vampiro", "Vampires2"));

    for (const auto& [tipo, hoja] : hojas) {
        for (const Animacion* animacion :
             {&hoja.quieto, &hoja.caminar, &hoja.ataque, &hoja.muerte, &hoja.correr}) {
            if (animacion->existe()) {
                texturas.try_emplace(animacion->ruta, animacion->ruta, renderer);
            }
        }
    }
}

void EnemigosGraficos::dibujar(
    SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const {
    const uint32_t ahora = SDL_GetTicks();
    for (const EstadoEnemigo& enemigo : estado.enemigos) {
        const auto hoja = hojas.find(enemigo.tipo);
        const bool cadaver = enemigo.vida == 0 && !enemigo.muriendo;
        if (hoja != hojas.end() && !cadaver) {
            dibujar_enemigo(renderer, hoja->second, enemigo, ahora, cam_x, cam_y);
        }
    }

    std::erase_if(recorridos, [&estado](const auto& entrada) {
        return std::none_of(estado.enemigos.begin(), estado.enemigos.end(),
            [&entrada](const EstadoEnemigo& e) { return e.id == entrada.first; });
    });
}

EnemigosGraficos::Posicion EnemigosGraficos::avanzar(
    Recorrido& recorrido, const EstadoEnemigo& enemigo, uint32_t ahora) const {
    const uint32_t ms_del_paso = enemigo.ms_por_casilla > 0 ? enemigo.ms_por_casilla : MS_POR_CASILLA;
    const auto avance = [&]() {
        return std::min(1.0f, static_cast<float>(ahora - recorrido.desde_ms) / ms_del_paso);
    };

    const int salto = std::abs(static_cast<int>(enemigo.columna) - static_cast<int>(recorrido.columna_destino)) +
                      std::abs(static_cast<int>(enemigo.fila) - static_cast<int>(recorrido.fila_destino));
    if (recorrido.desde_ms == 0 || salto > 1) {
        recorrido.columna_origen = static_cast<float>(enemigo.columna);
        recorrido.fila_origen = static_cast<float>(enemigo.fila);
        recorrido.columna_destino = enemigo.columna;
        recorrido.fila_destino = enemigo.fila;
        recorrido.desde_ms = ahora - ms_del_paso;
    }

    if (recorrido.columna_destino != enemigo.columna || recorrido.fila_destino != enemigo.fila) {
        const float t = avance();
        recorrido.columna_origen = interpolar(recorrido.columna_origen, recorrido.columna_destino, t);
        recorrido.fila_origen = interpolar(recorrido.fila_origen, recorrido.fila_destino, t);
        recorrido.columna_destino = enemigo.columna;
        recorrido.fila_destino = enemigo.fila;
        recorrido.desde_ms = ahora;
    }

    const float t = avance();
    return Posicion{
        interpolar(recorrido.columna_origen, recorrido.columna_destino, t),
        interpolar(recorrido.fila_origen, recorrido.fila_destino, t),
        t < 1.0f};
}

EnemigosGraficos::Cuadro EnemigosGraficos::elegir_cuadro(const Hoja& hoja,
    const EstadoEnemigo& enemigo, Recorrido& recorrido, bool caminando, uint32_t ahora) const {
    marcar_inicio(enemigo.atacando && !enemigo.muriendo, recorrido.ataque_desde_ms, ahora);
    marcar_inicio(enemigo.muriendo, recorrido.muerte_desde_ms, ahora);

    if (enemigo.muriendo) {
        if (hoja.muerte.existe()) {
            return Cuadro{&hoja.muerte, cuadro_una_vez(hoja.muerte, recorrido.muerte_desde_ms, ahora)};
        }
        const float avance = std::min(1.0f,
            static_cast<float>(ahora - recorrido.muerte_desde_ms) / hoja.muerte.ms_por_cuadro);
        Cuadro cuadro{&hoja.quieto, 0};
        cuadro.opacidad = static_cast<uint8_t>(255 * (1.0f - avance));
        return cuadro;
    }

    if (enemigo.atacando) {
        return Cuadro{&hoja.ataque, cuadro_una_vez(hoja.ataque, recorrido.ataque_desde_ms, ahora)};
    }

    if (!caminando) {
        return Cuadro{&hoja.quieto, cuadro_en_bucle(hoja.quieto, ahora)};
    }
    const Animacion& paso = (enemigo.frenesi && hoja.correr.existe()) ? hoja.correr : hoja.caminar;
    return Cuadro{&paso, cuadro_en_bucle(paso, ahora)};
}

void EnemigosGraficos::dibujar_enemigo(SDL_Renderer* renderer, const Hoja& hoja,
    const EstadoEnemigo& enemigo, uint32_t ahora, int cam_x, int cam_y) const {
    const int casilla = tamano_de_tile * escala_de_tiles;
    const float escala = static_cast<float>(escala_de_tiles) * hoja.escala;
    const int ancho_px = static_cast<int>(hoja.frame_ancho * escala);
    const int alto_px = static_cast<int>(hoja.frame_alto * escala);

    Recorrido& recorrido = recorridos[enemigo.id];
    const Posicion posicion = avanzar(recorrido, enemigo, ahora);
    const int centro_x = static_cast<int>((posicion.columna + 0.5f) * casilla) - cam_x;
    const int piso_y = static_cast<int>((posicion.fila + 1) * casilla) - cam_y;

    const Cuadro cuadro = elegir_cuadro(hoja, enemigo, recorrido, posicion.caminando, ahora);

    if (enemigo.muriendo && hoja.muerte_en_huesos) {
        dibujar_huesos(renderer, hoja, ahora - recorrido.muerte_desde_ms, centro_x, piso_y);
        return;
    }

    const auto textura = texturas.find(cuadro.animacion->ruta);
    const int fila = cuadro.animacion->filas[static_cast<int>(enemigo.mirando)];
    if (textura == texturas.end() || fila < 0) {
        return;
    }
    SDL_Texture* imagen = textura->second.ptr();

    const SDL_Rect origen{
        cuadro.columna * hoja.frame_ancho, fila * hoja.frame_alto, hoja.frame_ancho, hoja.frame_alto};
    const SDL_Rect destino{
        centro_x - ancho_px / 2,
        piso_y - static_cast<int>(hoja.pies_y * escala),
        ancho_px,
        alto_px};

    SDL_SetTextureAlphaMod(imagen, cuadro.opacidad);
    if (enemigo.frenesi) {
        SDL_SetTextureColorMod(imagen, 255, 110, 110);
    }
    SDL_RenderCopy(renderer, imagen, &origen, &destino);
    SDL_SetTextureColorMod(imagen, 255, 255, 255);
    SDL_SetTextureAlphaMod(imagen, 255);

    if (!enemigo.muriendo) {
        const int ancho_barra = std::min(ancho_px, casilla);
        const SDL_Rect cuerpo{
            centro_x - ancho_barra / 2,
            destino.y + static_cast<int>(hoja.cabeza_y * escala),
            ancho_barra,
            static_cast<int>((hoja.pies_y - hoja.cabeza_y) * escala)};
        dibujar_barra_de_vida(renderer, cuerpo, enemigo.vida, enemigo.vida_maxima);
    }
}

void EnemigosGraficos::dibujar_huesos(SDL_Renderer* renderer, const Hoja& hoja,
    uint32_t ms_desde_la_muerte, int centro_x, int pies_y) const {
    const uint32_t duracion = hoja.muerte.ms_por_cuadro;
    const auto textura = texturas.find(hoja.muerte.ruta);
    if (ms_desde_la_muerte >= duracion || textura == texturas.end()) {
        return;
    }
    SDL_Texture* imagen = textura->second.ptr();

    constexpr int DESPLAZAMIENTO_X[] = {-10, 9, -3, 6};
    constexpr int DESPLAZAMIENTO_Y[] = {-2, -4, 2, 3};
    const float escala = static_cast<float>(escala_de_tiles) * hoja.escala;
    const int lado = hoja.frame_ancho;
    const int tamano = static_cast<int>(lado * escala);

    SDL_SetTextureAlphaMod(imagen, static_cast<uint8_t>(255 - 255 * ms_desde_la_muerte / duracion));
    for (int i = 0; i < std::min(hoja.muerte.cuadros, 4); ++i) {
        const SDL_Rect origen{i * lado, hoja.muerte.filas[0] * hoja.frame_alto, lado, lado};
        const SDL_Rect destino{
            centro_x + static_cast<int>(DESPLAZAMIENTO_X[i] * escala) - tamano / 2,
            pies_y + static_cast<int>(DESPLAZAMIENTO_Y[i] * escala) - tamano,
            tamano,
            tamano};
        SDL_RenderCopy(renderer, imagen, &origen, &destino);
    }
    SDL_SetTextureAlphaMod(imagen, 255);
}

void EnemigosGraficos::dibujar_barra_de_vida(SDL_Renderer* renderer,
    const SDL_Rect& sobre_el_enemigo, uint16_t vida, uint16_t vida_maxima) const {
    if (vida_maxima == 0 || vida >= vida_maxima) {
        return;
    }

    const int alto = std::max(2, sobre_el_enemigo.h / 10);
    const SDL_Rect fondo{sobre_el_enemigo.x, sobre_el_enemigo.y - alto - 2, sobre_el_enemigo.w, alto};
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &fondo);

    const double fraccion = std::clamp(static_cast<double>(vida) / vida_maxima, 0.0, 1.0);
    const SDL_Rect relleno{fondo.x, fondo.y, static_cast<int>(fondo.w * fraccion), fondo.h};
    const SDL_Color color = color_de_vida(fraccion);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &relleno);
}
