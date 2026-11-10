#ifndef ENEMIGOS_GRAFICOS_H
#define ENEMIGOS_GRAFICOS_H

#include <SDL2/SDL.h>

#include <array>
#include <cstdint>
#include <map>
#include <string>

#include "../../common/mensajes/mensajes_partida.h"
#include "../../common/tipo_enemigo.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "textura.h"

// Dibuja a los enemigos que manda el servidor, a partir de sus hojas de sprites.
class EnemigosGraficos {
public:
    // Lo que tarda en cruzar una casilla si el servidor no lo manda.
    static constexpr uint32_t MS_POR_CASILLA = 500;

    // Una animación: una fila de cuadros por dirección. Ataque y muerte se reproducen una sola
    // vez (duran lo que dice el servidor); el resto se repite.
    struct Animacion {
        std::string ruta;  // vacía: el tipo no tiene esta animación
        // Fila de la imagen por Direccion (ARRIBA, ABAJO, IZQUIERDA, DERECHA). -1: no hay.
        std::array<int, 4> filas{-1, -1, -1, -1};
        int cuadros = 1;
        uint32_t ms_por_cuadro = 1000;

        bool existe() const {
            return !ruta.empty();
        }
    };

    // Cómo se dibuja un tipo de enemigo.
    //  - Si no hay animación de muerte, el cuadro de quieto se desvanece.
    //  - Si muerte_en_huesos, la fila de muerte son huesos sueltos que quedan tirados en el piso.
    struct Hoja {
        int frame_ancho = 0;
        int frame_alto = 0;
        // Fila del cuadro (en píxeles de la imagen) que se apoya en el piso, y la de arriba del
        // cuerpo (para la barra de vida).
        int pies_y = 0;
        int cabeza_y = 0;
        float escala = 1.0f;  // extra, sobre la escala de los tiles
        Animacion quieto;
        Animacion caminar;
        Animacion ataque;
        Animacion muerte;
        Animacion correr;  // opcional: reemplaza a caminar mientras está en frenesí
        bool muerte_en_huesos = false;
    };

    // Lanza std::runtime_error si falta alguna imagen.
    EnemigosGraficos(SDL_Renderer* renderer, const ConfiguracionGraficos& graficos);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla. Dibuja los enemigos vivos
    // o muriendo; un cadáver que ya terminó de caer no se dibuja hasta que reviva.
    void dibujar(SDL_Renderer* renderer, const EstadoJuego& estado, int cam_x, int cam_y) const;

private:
    // Lo que el cliente recuerda de cada enemigo entre cuadros: el paso que está dando (el
    // servidor manda casillas enteras y acá se desliza el dibujo) y cuándo empezó a atacar o morir.
    struct Recorrido {
        float columna_origen = 0;
        float fila_origen = 0;
        uint32_t columna_destino = 0;
        uint32_t fila_destino = 0;
        uint32_t desde_ms = 0;
        uint32_t ataque_desde_ms = 0;  // 0: no está atacando
        uint32_t muerte_desde_ms = 0;  // 0: no está muriendo
    };

    // Dónde dibujarlo, en casillas (con decimales mientras camina).
    struct Posicion {
        float columna;
        float fila;
        bool caminando;
    };

    // Qué cuadro de qué imagen dibujar, y cómo.
    struct Cuadro {
        const Animacion* animacion;
        int columna;
        uint8_t opacidad = 255;
    };

    std::map<TipoEnemigo, Hoja> hojas;
    std::map<std::string, Textura> texturas;  // por ruta
    mutable std::map<uint32_t, Recorrido> recorridos;
    int tamano_de_tile = 16;
    int escala_de_tiles = 1;

    Posicion avanzar(Recorrido& recorrido, const EstadoEnemigo& enemigo, uint32_t ahora) const;
    Cuadro elegir_cuadro(const Hoja& hoja, const EstadoEnemigo& enemigo, Recorrido& recorrido,
        bool caminando, uint32_t ahora) const;

    void dibujar_enemigo(SDL_Renderer* renderer, const Hoja& hoja, const EstadoEnemigo& enemigo,
        uint32_t ahora, int cam_x, int cam_y) const;

    // Los huesos tirados en el piso alrededor de "centro_x", "pies_y" (píxeles de pantalla),
    // desvaneciéndose.
    void dibujar_huesos(SDL_Renderer* renderer, const Hoja& hoja, uint32_t ms_desde_la_muerte,
        int centro_x, int pies_y) const;

    void dibujar_barra_de_vida(SDL_Renderer* renderer, const SDL_Rect& sobre_el_enemigo,
        uint16_t vida, uint16_t vida_maxima) const;
};

#endif
