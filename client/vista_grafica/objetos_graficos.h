#ifndef OBJETOS_GRAFICOS_H
#define OBJETOS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes_partida.h"
#include "../../common/tipo_objeto.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "iconos_de_items.h"
#include "textura.h"

// Tres momentos de dibujado. Un mineral o el tronco de un árbol van DEBAJO de los jugadores, la
// copa de un árbol o un techo van sobre el jugador. Los objetos altos que ocupan más de una casilla
// hacia arriba, como el cofre, van CON_LOS_JUGADORES, VistaPartida los ordena por su pie junto con
// los jugadores para que se pueda pasar por detrás.
enum class CapaDeObjeto : uint8_t {
    DEBAJO_DE_LOS_JUGADORES,
    CON_LOS_JUGADORES,
    SOBRE_LOS_JUGADORES,
};

// Una forma de dibujar un objeto: qué imagen usa y con qué frecuencia relativa sale.
struct VarianteDeAspecto {
    std::string ruta_de_textura;
    // Zona de la textura que se dibuja. Si el ancho es 0 se usa la textura completa.
    SDL_Rect origen{0, 0, 0, 0};
    // Frecuencia relativa de esta variante entre las del mismo aspecto. No tiene que sumar 100:
    // la probabilidad es el peso sobre la suma de los pesos de todas las variantes.
    uint32_t peso = 1;
};

// Cómo se dibuja un tipo de objeto del mundo.
struct AspectoDeObjeto {
    std::vector<VarianteDeAspecto> variantes;  // al menos una, con peso mayor a cero
    int ancho_en_casillas = 1;
    double alto_en_casillas = 1.0;
    int desplazamiento_x = 0;
    int desplazamiento_y = 0;
    // Si es true, el objeto se apoya en el borde inferior de su casilla en vez de en el superior.
    // Así un objeto más alto que una casilla crece hacia arriba en vez de hacia abajo.
    bool anclar_al_pie = false;
    CapaDeObjeto capa = CapaDeObjeto::DEBAJO_DE_LOS_JUGADORES;
    // Filas del dibujo que van por encima de la casilla del objeto (la cabecera de la cama). Están
    // contadas dentro de alto_en_casillas, no bloquean en el servidor y se dibujan sobre los
    // jugadores, así se puede pasar por detrás.
    int casillas_por_encima = 0;
};

// Dibuja los objetos del mundo que manda el servidor. Es genérico, sólo conoce un mapa de
// TipoObjeto a AspectoDeObjeto. Esto dibuja el objeto en el mapa. El ícono del ítem que deja al
// romperse lo dibuja IconosDeItems a partir de su TipoItem. Por eso un objeto puede verse distinto
// de su ítem, el aspecto se define acá y el ícono en IconosDeItems.
class ObjetosGraficos {
public:
    // Texturas de los objetos tal como se ven en el mapa. Las de los ítems (inventario, drops)
    // son otras y las maneja IconosDeItems.
    static constexpr const char* RUTA_ASSET_PIEDRA_1 = "Assets/Caverns/Mining/piedra1.png";
    static constexpr const char* RUTA_ASSET_PIEDRA_2 = "Assets/Caverns/Mining/piedra2.png";
    static constexpr const char* RUTA_ASSET_PIEDRA_3 = "Assets/Caverns/Mining/piedra3.png";
    static constexpr const char* RUTA_ASSET_MENA_CARBON = "Assets/Caverns/Mining/mena_carbon.png";
    static constexpr const char* RUTA_ASSET_MENA_LAPISLAZULI =
        "Assets/Caverns/Mining/mena_lapislazuli.png";
    static constexpr const char* RUTA_ASSET_MENA_PLATA = "Assets/Caverns/Mining/mena_plata.png";
    static constexpr const char* RUTA_ASSET_MENA_ORO = "Assets/Caverns/Mining/mena_oro.png";
    static constexpr const char* RUTA_ASSET_MENA_ORO = "Assets/Caverns/Mining/mena_oro.png";
    static constexpr const char* RUTA_ASSET_COFRE = "Assets/items/cofre.png";
    static constexpr const char* RUTA_ASSET_MUEBLES = "Assets/TileSheets/furniture_2.png";

    // Frecuencia relativa de cada piedra para q la 1 salga más y la 2 y la 3 salgan menos.
    static constexpr uint32_t PESO_PIEDRA_1 = 60;
    static constexpr uint32_t PESO_PIEDRA_2 = 25;
    static constexpr uint32_t PESO_PIEDRA_3 = 15;

    // Carga todas las texturas de los aspectos conocidos. Lanza std::runtime_error si falta
    // alguna imagen, o std::invalid_argument si un aspecto no tiene variantes con peso.
    ObjetosGraficos(
        SDL_Renderer* renderer, const ConfiguracionGraficos& graficos,
        const IconosDeItems& iconos_de_items);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla. Cada método dibuja solo
    // la capa que le corresponde.
    void dibujar_debajo_de_los_jugadores(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
        int cam_y) const;
    void dibujar_sobre_los_jugadores(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
        int cam_y) const;

    // Dibuja un único objeto, sin importar su capa. VistaPartida lo usa para intercalar los objetos
    // de la capa CON_LOS_JUGADORES con los jugadores, ordenados por su pie.
    void dibujar_objeto(
        SDL_Renderer* renderer, const EstadoObjetoDelMundo& objeto, int cam_x, int cam_y) const;

    // La capa en la que se dibuja el objeto, o std::nullopt si su tipo no tiene aspecto.
    std::optional<CapaDeObjeto> capa_de(const EstadoObjetoDelMundo& objeto) const;

    void dibujar_drops(
        SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const;

private:
    std::map<TipoObjeto, AspectoDeObjeto> aspectos;
    std::map<std::string, Textura> texturas;
    int tamano_de_tile = 16;
    int escala_de_tiles = 1;
    const IconosDeItems& iconos;

    // Aspectos de los objetos que ya existen.
    static std::map<TipoObjeto, AspectoDeObjeto> crear_aspectos_conocidos();

    // Dibuja las filas [primera_fila, ultima_fila) del aspecto, contadas desde la de arriba de su
    // dibujo. La fila 0 del dibujo cae casillas_por_encima filas arriba de la casilla del objeto.
    void dibujar_filas(
        SDL_Renderer* renderer, const EstadoObjetoDelMundo& objeto, const AspectoDeObjeto& aspecto,
        int primera_fila, int ultima_fila, int cam_x, int cam_y) const;

    void dibujar_capa(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa, int cam_x,
        int cam_y, CapaDeObjeto capa) const;

    void dibujar_con_aspecto(
        SDL_Renderer* renderer, const EstadoObjetoDelMundo& objeto, const AspectoDeObjeto& aspecto,
        int cam_x, int cam_y, int ancho_de_pantalla, int alto_de_pantalla) const;
};

#endif
