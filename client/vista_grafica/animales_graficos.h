#ifndef ANIMALES_GRAFICOS_H
#define ANIMALES_GRAFICOS_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../common/direccion.h"
#include "../../common/mensajes/mensajes_partida.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

class AnimalesGraficos {
public:
    static constexpr const char* SECTOR_GRANJA = "granja";

    static constexpr const char* RUTA_VACA_BLANCA = "Assets/Animals/White Cow.png";
    static constexpr const char* RUTA_VACA_MARRON = "Assets/Animals/Brown Cow.png";

    static constexpr const char* RUTA_GALLINA_BLANCA = "Assets/Animals/White Chicken.png";
    static constexpr const char* RUTA_GALLINA_MARRON = "Assets/Animals/Brown Chicken.png";
    static constexpr const char* RUTA_GALLINA_AZUL = "Assets/Animals/Blue Chicken.png";
    static constexpr const char* RUTA_GALLINA_DORADA = "Assets/Animals/Golden Chicken.png";
    static constexpr const char* RUTA_GALLINA_VOID = "Assets/Animals/Void Chicken.png";

    static constexpr const char* RUTA_ICONO_LECHE = "Assets/items/leche.png";
    static constexpr const char* RUTA_BURBUJA_EMOTE = "Assets/TileSheets/emotes.png";

    static constexpr int VACA_ANCHO = 32;
    static constexpr int VACA_ALTO = 32;
    static constexpr int GALLINA_ANCHO = 16;
    static constexpr int GALLINA_ALTO = 16;
    static constexpr int ICONO_LECHE_ANCHO = 10;
    static constexpr int ICONO_LECHE_ALTO = 12;

    static constexpr int BURBUJA_ORIGEN_X = 48;
    static constexpr int BURBUJA_ORIGEN_Y = 0;
    static constexpr int BURBUJA_ANCHO = 16;
    static constexpr int BURBUJA_ALTO = 16;
    static constexpr int BURBUJA_ESCALA_NUMERADOR = 5;
    static constexpr int BURBUJA_ESCALA_DENOMINADOR = 4;

    static constexpr int FILA_ABAJO = 0;
    static constexpr int FILA_LATERAL = 1;
    static constexpr int FILA_ARRIBA = 2;
    static constexpr int FILA_DORMIDA_VACA = 3;
    static constexpr int FILA_DORMIDA_GALLINA_ABAJO = 4;
    static constexpr int FILA_DORMIDA_GALLINA_LATERAL = 4;
    static constexpr int FILA_DORMIDA_GALLINA_ARRIBA = 5;

    static constexpr int CUADROS_CAMINANDO = 4;
    static constexpr uint32_t MS_POR_CUADRO_CAMINANDO = 160;

    static constexpr double PERIODO_FLOTACION_MS = 200.0;
    static constexpr double AMPLITUD_FLOTACION = 2.0;
    static constexpr int AJUSTE_ALTURA_BURBUJA_VACA = 7;
    static constexpr int MS_POR_PASO = 400;

    AnimalesGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos);

    void dibujar(
        SDL_Renderer* renderer, const EstadoJuego& estado, const std::string& mapa_actual,
        int cam_x, int cam_y) const;

    void dibujar_un_animal(
        SDL_Renderer* renderer, const EstadoJuego& estado, const EstadoAnimal& animal, int cam_x,
        int cam_y) const;

private:
    struct DisposicionFrame {
        int fila = 0;
        int columna = 0;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
    };

    struct Recorrido {
        float columna_origen = 0.0f;
        float fila_origen = 0.0f;
        uint32_t columna_destino = 0;
        uint32_t fila_destino = 0;
        uint32_t desde_ms = 0;
    };

    struct Posicion {
        float columna = 0.0f;
        float fila = 0.0f;
        bool caminando = false;
    };

    mutable std::map<uint32_t, Recorrido> recorridos;
    int tamano_de_tile = 16;
    int escala_de_tiles = 1;

    std::vector<Textura> texturas_vacas;
    std::vector<Textura> texturas_gallinas;
    Textura icono_leche;
    Textura textura_burbuja;
    std::map<Direccion, DisposicionFrame> disposicion_por_direccion;
    std::map<Direccion, DisposicionFrame> disposicion_dormida_vaca;
    std::map<Direccion, DisposicionFrame> disposicion_dormida_gallina;

    SDL_Texture* obtener_textura(TipoAnimal tipo, uint8_t variante) const;
    DisposicionFrame obtener_disposicion(
        TipoAnimal tipo, Direccion direccion, bool durmiendo) const;

    Posicion avanzar(Recorrido& recorrido, const EstadoAnimal& animal, uint32_t ahora) const;

    void dibujar_animal(
        SDL_Renderer* renderer, const EstadoAnimal& animal, bool tiene_leche, int cam_x, int cam_y,
        int ancho_pantalla, int alto_pantalla, uint32_t ahora) const;

    void dibujar_notificacion_ordenie(
        SDL_Renderer* renderer, const SDL_Rect& destino_vaca, uint32_t ahora) const;

    bool esta_en_pantalla(const SDL_Rect& rect, int ancho_pantalla, int alto_pantalla) const;
};

#endif
