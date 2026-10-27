#include "camara.h"

#include <algorithm>

#include "config.h"

Camara::Camara(int ancho_de_ventana, int alto_de_ventana, int ancho_del_mapa, int alto_del_mapa)
    : ancho_de_ventana(ancho_de_ventana),
      alto_de_ventana(alto_de_ventana),
      ancho_del_mapa(ancho_del_mapa),
      alto_del_mapa(alto_del_mapa) {
}

int Camara::calcular_esquina(
    int posicion_en_mapa, int tamanio_del_sprite, int tamanio_de_ventana, int tamanio_del_mapa) {
    const int centro_del_sprite = (posicion_en_mapa + tamanio_del_sprite / 2) * TILE_SCALE;
    const int maximo = std::max(0, tamanio_del_mapa - tamanio_de_ventana);
    return std::clamp(centro_del_sprite - tamanio_de_ventana / 2, 0, maximo);
}

void Camara::centrar_en(int x_en_mapa, int y_en_mapa, int ancho_del_sprite, int alto_del_sprite) {
    esquina_x = calcular_esquina(x_en_mapa, ancho_del_sprite, ancho_de_ventana, ancho_del_mapa);
    esquina_y = calcular_esquina(y_en_mapa, alto_del_sprite, alto_de_ventana, alto_del_mapa);
}

void Camara::reiniciar() {
    esquina_x = 0;
    esquina_y = 0;
}

int Camara::x() const {
    return esquina_x;
}

int Camara::y() const {
    return esquina_y;
}
