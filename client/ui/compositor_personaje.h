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
    // indicado.
    QPixmap componer(const ConfiguracionJugador& jugador, int factor_escala) const;

private:
    QImage hoja_cuerpo_masculino;
    QImage hoja_cuerpo_femenino;
    QImage hoja_pantalones;
    QImage hoja_peinados;
    QImage hoja_remeras;
    QImage hoja_sombreros;

    static QImage aplicar_tinte(const QImage& original, const ColorRGB& color);
};

#endif  // COMPOSITOR_PERSONAJE_H
