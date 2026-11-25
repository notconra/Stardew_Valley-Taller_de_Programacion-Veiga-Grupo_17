#ifndef FONDO_ANIMADO_H
#define FONDO_ANIMADO_H

#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <vector>

// Representa el fondo animado del juego: cielo con gradiente suave (noche estrellada superior
// hacia día celeste en el horizonte), estrellas titilantes y nubes pixel-art que flotan en bucle.
class FondoAnimado {
public:
    FondoAnimado();

    // Actualiza la posición de las nubes según su velocidad.
    void actualizar(int ancho_pantalla);

    // Dibuja el gradiente del cielo, las estrellas y opcionalmente las nubes flotantes.
    void dibujar(QPainter& pintor, const QRect& area_pantalla, bool dibujar_nubes = true) const;

private:
    struct Nube {
        float posicion_x = 0.0f;
        float posicion_y = 0.0f;
        float velocidad = 0.0f;
        float escala = 1.0f;
        int indice_sprite = 0;
    };

    struct Estrella {
        int porcentaje_x = 0;
        int porcentaje_y = 0;
        int tamanio = 1;
        QColor color;
    };

    std::vector<Nube> nubes;
    std::vector<QPixmap> sprites_nubes;
    std::vector<Estrella> estrellas;

    void inicializar_estrellas();
    void inicializar_nubes();
};

#endif  // FONDO_ANIMADO_H
