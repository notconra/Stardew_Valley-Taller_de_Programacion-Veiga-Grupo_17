#ifndef VISION_MAZMORRA_H
#define VISION_MAZMORRA_H

#include <SDL2/SDL.h>

// El efecto de "linterna" de las mazmorras, estilo Pokémon: tapa la pantalla con negro salvo un
// círculo alrededor del jugador, armado con cuadraditos (sin degradado ni texturas) para que se
// vea pixelado en lugar de un círculo liso.
//
// Si el radio es 0 o negativo el efecto queda desactivado y dibujar() no hace nada.
class VisionMazmorra {
public:
    // radio_visible_px es el radio del círculo, en píxeles de pantalla. tamanio_de_bloque_px es
    // el lado de cada cuadradito de la grilla del borde: cuanto más chico, más redondeado se ve
    // el círculo (y más cuadraditos hay que dibujar por cuadro). ancho_difuminado_px es el
    // ancho (también en píxeles) de la franja alrededor del borde que se dibuja semitransparente
    // en lugar de negro sólido, para difuminar la transición; 0 deja el corte seco de antes.
    VisionMazmorra(int radio_visible_px, int tamanio_de_bloque_px, int ancho_difuminado_px);

    bool activa() const;

    // Dibuja la oscuridad alrededor de (centro_x, centro_y): coordenadas de pantalla (ya
    // restada la cámara), el mismo sistema que usa Personaje::dibujar. ancho_ventana y
    // alto_ventana son el tamaño de la ventana. No hace nada si el efecto está desactivado.
    // Se llama después de dibujar el mapa y los personajes, y antes del HUD, para que el HUD
    // no quede oscurecido.
    void dibujar(
        SDL_Renderer* renderer, int centro_x, int centro_y, int ancho_ventana,
        int alto_ventana) const;

private:
    // Alfa (0 = invisible, 255 = negro sólido) para un punto a esta distancia del centro.
    int calcular_alfa(double distancia) const;

    int radio_visible;
    int tamanio_de_bloque;
    int ancho_difuminado;
};

#endif
