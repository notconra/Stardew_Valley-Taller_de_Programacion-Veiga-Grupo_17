#ifndef COMPOSITOR_PERSONAJE_H
#define COMPOSITOR_PERSONAJE_H

#include <QImage>
#include <QPixmap>
#include <string>

#include "../configuracion/configuracion_cliente_datos.h"

// Compone las capas del personaje de Stardew Valley (cuerpo, pantalón coloreado, brazos,
// remera y peinado coloreado) para generar el sprite visual de previsualización en Qt.
class CompositorPersonaje {
public:
    CompositorPersonaje();

    // Genera el sprite del personaje compuesto mirando al frente, escalado según el factor
    // indicado, con opción de incluir el fondo de día de Stardew Valley.
    QPixmap componer(
        const ConfiguracionJugador& jugador, int factor_escala,
        bool incluir_fondo_dia = false) const;

private:
    QImage hoja_cuerpo_masculino;
    QImage hoja_cuerpo_femenino;
    QImage hoja_pantalones;
    QImage hoja_peinados;
    QImage hoja_remeras;
    QImage hoja_sombreros;
    QImage hoja_pieles;
    QImage hoja_fondo_dia;

    QImage aplicar_tinte(const QImage& original, const ColorRGB& color) const;
    QImage aplicar_piel(const QImage& original, int indice_piel) const;
    QImage aplicar_color_ojos(const QImage& original, const ColorRGB& color) const;
    bool mismo_rgb(QRgb primer_color, QRgb segundo_color) const;
};

#endif  // COMPOSITOR_PERSONAJE_H
