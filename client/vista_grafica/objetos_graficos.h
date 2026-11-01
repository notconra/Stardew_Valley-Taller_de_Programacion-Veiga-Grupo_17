#ifndef OBJETOS_GRAFICOS_H
#define OBJETOS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <string>

#include "../../common/mensajes/mensajes_partida.h"
#include "../../common/tipo_objeto.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dos momentos de dibujado para que los objetos puedan quedar detrás o delante de los jugadores.
// Un mineral o el tronco de un árbol van DEBAJO, la copa de un árbol o un techo van SOBRE.
enum class CapaDeObjeto : uint8_t {
    DEBAJO_DE_LOS_JUGADORES,
    SOBRE_LOS_JUGADORES,
};

// Cómo se dibuja un tipo de objeto del mundo.
struct AspectoDeObjeto {
    std::string ruta_de_textura;
    // Zona de la textura que se dibuja. Si el ancho es 0 se usa la textura completa.
    SDL_Rect origen{0, 0, 0, 0};
    int ancho_en_casillas = 1;
    int alto_en_casillas = 1;
    int desplazamiento_x = 0;
    int desplazamiento_y = 0;
    CapaDeObjeto capa = CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES;
};

// Dibuja los objetos del mundo que manda el servidor. Es genérico, sólo conoce un mapa de
// TipoObjeto a AspectoDeObjeto. Esto dibuja el objeto en el mapa. El ícono del ítem que deja al
// romperse lo dibuja IconosDeItems a partir de su TipoItem. Por eso un objeto puede verse distinto
// de su ítem, el aspecto se define acá y el ícono en IconosDeItems.
class ObjetosGraficos {
public:
    static constexpr const char* RUTA_ASSET_MENAS = "Assets/Caverns/Mining/ore.png";
    static constexpr const char* RUTA_ASSET_PIEDRA = "Assets/items/piedra.png";
    static constexpr const char* RUTA_ASSET_COFRE = "Assets/Misc/chest.png";

    // Carga todas las texturas de los aspectos conocidos. Lanza std::runtime_error si falta
    // alguna imagen.
    ObjetosGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla. Cada método dibuja solo
    // la capa que le corresponde.
    void dibujar_debajo_de_los_jugadores(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
        int cam_y) const;
    void dibujar_sobre_los_jugadores(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
        int cam_y) const;

private:
    std::map<TipoObjeto, AspectoDeObjeto> aspectos;
    std::map<std::string, Textura> texturas;
    int tamano_de_tile = 16;
    int escala_de_tiles = 1;

    // Aspectos de los objetos que ya existen.
    static std::map<TipoObjeto, AspectoDeObjeto> crear_aspectos_conocidos();

    void dibujar_capa(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
        int cam_y, CapaDeObjeto capa) const;
};

#endif
