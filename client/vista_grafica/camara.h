#ifndef CAMARA_H
#define CAMARA_H

// Calcula la esquina de la cámara (en píxeles de pantalla) para dejar a un jugador en el centro
// de la ventana, sin salirse del mapa. La posición del jugador viene en píxeles del mapa.
class Camara {
private:
    int ancho_de_ventana;
    int alto_de_ventana;
    int ancho_del_mapa;
    int alto_del_mapa;
    int esquina_x = 0;
    int esquina_y = 0;

    static int calcular_esquina(
        int posicion_en_mapa, int tamanio_del_sprite, int tamanio_de_ventana, int tamanio_del_mapa);

public:
    Camara(int ancho_de_ventana, int alto_de_ventana, int ancho_del_mapa, int alto_del_mapa);

    void centrar_en(int x_en_mapa, int y_en_mapa, int ancho_del_sprite, int alto_del_sprite);

    // Vuelve a la esquina superior izquierda (por ejemplo al empezar una partida nueva).
    void reiniciar();

    int x() const;
    int y() const;
};

#endif
