#include "compositor_personaje.h"

#include <QPainter>

constexpr int ANCHO_BASE_SPRITE = 16;
constexpr int ALTO_BASE_SPRITE = 32;

constexpr int ANCHO_CELDA_PELO = 16;
constexpr int ALTO_BLOQUE_PELO = 96;

constexpr int ANCHO_CELDA_REMERA = 8;
constexpr int ALTO_BLOQUE_REMERA = 24;

constexpr int POSICION_X_REMERA = 4;
constexpr int COLUMNA_BRAZOS = 6;

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";
constexpr const char* RUTA_PANTALONES = "Assets/Characters/Farmer/pants.png";
constexpr const char* RUTA_PEINADOS = "Assets/Characters/Farmer/hairstyles.png";
constexpr const char* RUTA_REMERAS = "Assets/Characters/Farmer/shirts.png";

CompositorPersonaje::CompositorPersonaje()
    : hoja_cuerpo_masculino(RUTA_CUERPO_MASCULINO),
      hoja_cuerpo_femenino(RUTA_CUERPO_FEMENINO),
      hoja_pantalones(RUTA_PANTALONES),
      hoja_peinados(RUTA_PEINADOS),
      hoja_remeras(RUTA_REMERAS) {
}

QImage CompositorPersonaje::aplicar_tinte(const QImage& original, const ColorRGB& color) {
    QImage resultado = original.convertToFormat(QImage::Format_ARGB32);
    int factor_r = color.rojo;
    int factor_g = color.verde;
    int factor_b = color.azul;

    for (int y = 0; y < resultado.height(); ++y) {
        QRgb* fila = reinterpret_cast<QRgb*>(resultado.scanLine(y));
        for (int x = 0; x < resultado.width(); ++x) {
            QRgb pixel = fila[x];
            int alfa = qAlpha(pixel);
            if (alfa == 0) {
                continue;
            }
            int r = (qRed(pixel) * factor_r) / 255;
            int g = (qGreen(pixel) * factor_g) / 255;
            int b = (qBlue(pixel) * factor_b) / 255;
            fila[x] = qRgba(r, g, b, alfa);
        }
    }
    return resultado;
}

QPixmap CompositorPersonaje::componer(
    const ConfiguracionJugador& jugador, int factor_escala) const {
    bool es_femenino = (jugador.ruta_cuerpo == RUTA_CUERPO_FEMENINO);
    const QImage& hoja_cuerpo = es_femenino ? hoja_cuerpo_femenino : hoja_cuerpo_masculino;

    if (hoja_cuerpo.isNull()) {
        return {};
    }

    QImage lienzo(ANCHO_BASE_SPRITE, ALTO_BASE_SPRITE, QImage::Format_ARGB32_Premultiplied);
    lienzo.fill(Qt::transparent);

    QPainter pintor(&lienzo);

    QImage sprite_cuerpo = hoja_cuerpo.copy(0, 0, ANCHO_BASE_SPRITE, ALTO_BASE_SPRITE);
    pintor.drawImage(0, 0, sprite_cuerpo);

    if (!hoja_pantalones.isNull()) {
        int ancho_cuerpo = hoja_cuerpo.width() > 0 ? hoja_cuerpo.width() : 288;
        int columnas_pantalon = hoja_pantalones.width() / ancho_cuerpo;
        if (columnas_pantalon <= 0) {
            columnas_pantalon = 1;
        }
        int columna = jugador.indice_pantalon % columnas_pantalon;
        int origen_x = columna * ancho_cuerpo;

        QImage recorte_pantalon =
            hoja_pantalones.copy(origen_x, 0, ANCHO_BASE_SPRITE, ALTO_BASE_SPRITE);
        QImage pantalon_tintado = aplicar_tinte(recorte_pantalon, jugador.color_pantalon);
        pintor.drawImage(0, 0, pantalon_tintado);
    }

    QImage sprite_brazos = hoja_cuerpo.copy(
        COLUMNA_BRAZOS * ANCHO_BASE_SPRITE,
        0,
        ANCHO_BASE_SPRITE,
        ALTO_BASE_SPRITE);
    pintor.drawImage(0, 0, sprite_brazos);

    if (!hoja_remeras.isNull()) {
        int columnas_remera = hoja_remeras.width() / ANCHO_CELDA_REMERA;
        if (columnas_remera <= 0) {
            columnas_remera = 1;
        }
        int columna_r = jugador.indice_remera % columnas_remera;
        int pagina_r = jugador.indice_remera / columnas_remera;

        QImage recorte_remera = hoja_remeras.copy(
            columna_r * ANCHO_CELDA_REMERA,
            pagina_r * ALTO_BLOQUE_REMERA,
            ANCHO_CELDA_REMERA,
            ANCHO_CELDA_REMERA);
        int desvio_y_remera = es_femenino ? OFFSET_Y_REMERA_FEMENINO : OFFSET_Y_REMERA_MASCULINO;
        pintor.drawImage(POSICION_X_REMERA, desvio_y_remera, recorte_remera);
    }

    if (!hoja_peinados.isNull()) {
        int columnas_pelo = hoja_peinados.width() / ANCHO_CELDA_PELO;
        if (columnas_pelo <= 0) {
            columnas_pelo = 1;
        }
        int columna_p = jugador.indice_pelo % columnas_pelo;
        int pagina_p = jugador.indice_pelo / columnas_pelo;

        QImage recorte_pelo = hoja_peinados.copy(
            columna_p * ANCHO_CELDA_PELO,
            pagina_p * ALTO_BLOQUE_PELO,
            ANCHO_CELDA_PELO,
            ALTO_BASE_SPRITE);
        QImage pelo_tintado = aplicar_tinte(recorte_pelo, jugador.color_pelo);
        int desvio_y_pelo = es_femenino ? OFFSET_Y_PELO_FEMENINO : OFFSET_Y_PELO_MASCULINO;
        pintor.drawImage(0, desvio_y_pelo, pelo_tintado);
    }

    pintor.end();

    QImage escalada = lienzo.scaled(
        ANCHO_BASE_SPRITE * factor_escala,
        ALTO_BASE_SPRITE * factor_escala,
        Qt::IgnoreAspectRatio,
        Qt::FastTransformation);

    return QPixmap::fromImage(escalada);
}
