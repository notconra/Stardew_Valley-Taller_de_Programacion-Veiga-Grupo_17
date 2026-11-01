#include "objetos_graficos.h"

#include <SDL2/SDL.h>

#include <map>
#include <string>

ObjetosGraficos::ObjetosGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos)
    : aspectos(crear_aspectos_conocidos()),
      tamano_de_tile(graficos.tamano_de_tile),
      escala_de_tiles(graficos.escala_de_tiles) {
    for (const auto& entrada : aspectos) {
        const AspectoDeObjeto& aspecto = entrada.second;
        if (!texturas.contains(aspecto.ruta_de_textura)) {
            texturas.try_emplace(aspecto.ruta_de_textura, aspecto.ruta_de_textura, renderer);
        }
    }
}

std::map<TipoObjeto, AspectoDeObjeto> ObjetosGraficos::crear_aspectos_conocidos() {
    std::map<TipoObjeto, AspectoDeObjeto> conocidos;

    // Piedra: se dibuja el ícono completo, escalado a una casilla.
    conocidos.emplace(
        TipoObjeto::PIEDRA,
        AspectoDeObjeto{
            RUTA_ASSET_PIEDRA,
            SDL_Rect{0, 0, 0, 0},
            1,
            1,
            0,
            0,
            CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES});

    // Menas
    conocidos.emplace(
        TipoObjeto::PLATA,
        AspectoDeObjeto{
            RUTA_ASSET_MENAS,
            SDL_Rect{4, 2 + (32 * 0), 26, 25},
            1,
            1,
            0,
            0,
            CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES});
    conocidos.emplace(
        TipoObjeto::CARBON,
        AspectoDeObjeto{
            RUTA_ASSET_MENAS,
            SDL_Rect{4, 2 + (32 * 1), 26, 25},
            1,
            1,
            0,
            0,
            CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES});
    conocidos.emplace(
        TipoObjeto::LAPISLAZULI,
        AspectoDeObjeto{
            RUTA_ASSET_MENAS,
            SDL_Rect{4, 2 + (32 * 2), 26, 25},
            1,
            1,
            0,
            0,
            CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES});
    conocidos.emplace(
        TipoObjeto::ORO,
        AspectoDeObjeto{
            RUTA_ASSET_MENAS,
            SDL_Rect{4, 2 + (32 * 3), 26, 25},
            1,
            1,
            0,
            0,
            CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES});

    // Cofre
    conocidos.emplace(
        TipoObjeto::COFRE,
        AspectoDeObjeto{
            RUTA_ASSET_COFRE,
            SDL_Rect{0, 0, 32, 32},
            1,
            1,
            0,
            0,
            CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES});

    return conocidos;
}

void ObjetosGraficos::dibujar_debajo_de_los_jugadores(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
    int cam_y) const {
    dibujar_capa(renderer, estado, mapa, cam_x, cam_y, CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES);
}

void ObjetosGraficos::dibujar_sobre_los_jugadores(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
    int cam_y) const {
    dibujar_capa(renderer, estado, mapa, cam_x, cam_y, CapaDeObjeto::SOBRE_LOS_JUGADORES);
}

void ObjetosGraficos::dibujar_capa(
    SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
    int cam_y, CapaDeObjeto capa) const {
    const int tamano_en_pantalla = tamano_de_tile * escala_de_tiles;

    for (const EstadoObjetoDelMundo& objeto : estado.objetos) {
        if (objeto.mapa != mapa) {
            continue;
        }

        const auto encontrado = aspectos.find(objeto.tipo);
        if (encontrado == aspectos.end() || encontrado->second.capa != capa) {
            continue;
        }
        const AspectoDeObjeto& aspecto = encontrado->second;

        const SDL_Rect destino{
            static_cast<int>(objeto.columna) * tamano_en_pantalla - cam_x +
                aspecto.desplazamiento_x,
            static_cast<int>(objeto.fila) * tamano_en_pantalla - cam_y + aspecto.desplazamiento_y,
            aspecto.ancho_en_casillas * tamano_en_pantalla,
            aspecto.alto_en_casillas * tamano_en_pantalla};
        SDL_RenderCopy(
            renderer,
            texturas.at(aspecto.ruta_de_textura).ptr(),
            aspecto.origen.w > 0 ? &aspecto.origen : nullptr,
            &destino);
    }
}
