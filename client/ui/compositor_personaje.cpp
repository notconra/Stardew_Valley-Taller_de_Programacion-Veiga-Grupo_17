#include "compositor_personaje.h"

#include <QPainter>

constexpr int ANCHO_BASE_SPRITE = 16;
constexpr int ALTO_BASE_SPRITE = 32;

constexpr int ANCHO_CELDA_PELO = 16;
constexpr int ALTO_BLOQUE_PELO = 96;

constexpr int ANCHO_CELDA_REMERA = 8;
constexpr int COLUMNAS_REMERA = 16;
constexpr int ALTO_BLOQUE_REMERA = 32;

constexpr int ANCHO_CELDA_SOMBRERO = 20;
constexpr int ALTO_BLOQUE_SOMBRERO = 80;

constexpr int COLUMNAS_HOJA_PANTALON = 10;
constexpr int ANCHO_ESTILO_PANTALON = 192;
constexpr int ALTO_ESTILO_PANTALON = 688;

constexpr int MARGEN_X_SOMBRERO = 2;
constexpr int MARGEN_Y_SOMBRERO = 2;

constexpr int ANCHO_LIENZO = ANCHO_BASE_SPRITE + 2 * MARGEN_X_SOMBRERO;
constexpr int ALTO_LIENZO = ALTO_BASE_SPRITE + MARGEN_Y_SOMBRERO;

constexpr int POSICION_X_REMERA = 4;
constexpr int COLUMNA_BRAZOS = 6;

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";
constexpr const char* RUTA_PANTALONES = "Assets/Characters/Farmer/pants.png";
constexpr const char* RUTA_PEINADOS = "Assets/Characters/Farmer/hairstyles.png";
constexpr const char* RUTA_REMERAS = "Assets/Characters/Farmer/shirts.png";
constexpr const char* RUTA_SOMBREROS = "Assets/Characters/Farmer/hats.png";
constexpr const char* RUTA_PIELES = "Assets/Characters/Farmer/skinColors.png";
constexpr const char* RUTA_FONDO_DIA = "Assets/LooseSprites/daybg.png";

constexpr int ANCHO_RECUADRO_PREVIEW = 116;
constexpr int ALTO_RECUADRO_PREVIEW = 156;

const QColor COLOR_BASE_OJOS(104, 43, 15);

CompositorPersonaje::CompositorPersonaje()
    : hoja_cuerpo_masculino(RUTA_CUERPO_MASCULINO),
      hoja_cuerpo_femenino(RUTA_CUERPO_FEMENINO),
      hoja_pantalones(RUTA_PANTALONES),
      hoja_peinados(RUTA_PEINADOS),
      hoja_remeras(RUTA_REMERAS),
      hoja_sombreros(RUTA_SOMBREROS),
      hoja_pieles(RUTA_PIELES),
      hoja_fondo_dia(RUTA_FONDO_DIA) {
}

QImage CompositorPersonaje::aplicar_tinte(const QImage& original, const ColorRGB& color) const {
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

bool CompositorPersonaje::mismo_rgb(QRgb primer_color, QRgb segundo_color) const {
    return qRed(primer_color) == qRed(segundo_color) &&
           qGreen(primer_color) == qGreen(segundo_color) &&
           qBlue(primer_color) == qBlue(segundo_color);
}

QImage CompositorPersonaje::aplicar_piel(const QImage& original, int indice_piel) const {
    if (hoja_pieles.isNull() || indice_piel <= 0 || indice_piel >= hoja_pieles.height()) {
        return original;
    }
    QRgb original_sombra = hoja_pieles.pixel(0, 0);
    QRgb original_medio = hoja_pieles.pixel(1, 0);
    QRgb original_luz = hoja_pieles.pixel(2, 0);

    QRgb destino_sombra = hoja_pieles.pixel(0, indice_piel);
    QRgb destino_medio = hoja_pieles.pixel(1, indice_piel);
    QRgb destino_luz = hoja_pieles.pixel(2, indice_piel);

    QImage resultado = original.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < resultado.height(); ++y) {
        QRgb* fila = reinterpret_cast<QRgb*>(resultado.scanLine(y));
        for (int x = 0; x < resultado.width(); ++x) {
            QRgb pixel = fila[x];
            int alfa = qAlpha(pixel);
            if (alfa == 0) {
                continue;
            }
            if (mismo_rgb(pixel, original_sombra)) {
                fila[x] = qRgba(
                    qRed(destino_sombra),
                    qGreen(destino_sombra),
                    qBlue(destino_sombra),
                    alfa);
            } else if (mismo_rgb(pixel, original_medio)) {
                fila[x] =
                    qRgba(qRed(destino_medio), qGreen(destino_medio), qBlue(destino_medio), alfa);
            } else if (mismo_rgb(pixel, original_luz)) {
                fila[x] = qRgba(qRed(destino_luz), qGreen(destino_luz), qBlue(destino_luz), alfa);
            }
        }
    }
    return resultado;
}

QImage CompositorPersonaje::aplicar_color_ojos(
    const QImage& original, const ColorRGB& color) const {
    QImage resultado = original.convertToFormat(QImage::Format_ARGB32);
    QRgb rgb_base_ojos = COLOR_BASE_OJOS.rgb();
    for (int y = 0; y < resultado.height(); ++y) {
        QRgb* fila = reinterpret_cast<QRgb*>(resultado.scanLine(y));
        for (int x = 0; x < resultado.width(); ++x) {
            QRgb pixel = fila[x];
            int alfa = qAlpha(pixel);
            if (alfa == 0) {
                continue;
            }
            if (mismo_rgb(pixel, rgb_base_ojos)) {
                fila[x] = qRgba(color.rojo, color.verde, color.azul, alfa);
            }
        }
    }
    return resultado;
}

QPixmap CompositorPersonaje::componer(
    const ConfiguracionJugador& jugador, int factor_escala, bool incluir_fondo_dia) const {
    bool es_femenino = (jugador.ruta_cuerpo == RUTA_CUERPO_FEMENINO);
    const QImage& hoja_cuerpo = es_femenino ? hoja_cuerpo_femenino : hoja_cuerpo_masculino;

    if (hoja_cuerpo.isNull()) {
        return {};
    }

    QImage lienzo(ANCHO_LIENZO, ALTO_LIENZO, QImage::Format_ARGB32_Premultiplied);
    lienzo.fill(Qt::transparent);

    QPainter pintor(&lienzo);

    QImage sprite_cuerpo = aplicar_color_ojos(
        aplicar_piel(
            hoja_cuerpo.copy(0, 0, ANCHO_BASE_SPRITE, ALTO_BASE_SPRITE),
            jugador.indice_piel),
        jugador.color_ojos);
    pintor.drawImage(MARGEN_X_SOMBRERO, MARGEN_Y_SOMBRERO, sprite_cuerpo);

    if (!hoja_pantalones.isNull()) {
        int columna_estilo = jugador.indice_pantalon % COLUMNAS_HOJA_PANTALON;
        int fila_estilo = jugador.indice_pantalon / COLUMNAS_HOJA_PANTALON;
        int origen_x = columna_estilo * ANCHO_ESTILO_PANTALON;
        int origen_y = fila_estilo * ALTO_ESTILO_PANTALON;

        QImage recorte_pantalon =
            hoja_pantalones.copy(origen_x, origen_y, ANCHO_BASE_SPRITE, ALTO_BASE_SPRITE);
        QImage pantalon_tintado = aplicar_tinte(recorte_pantalon, jugador.color_pantalon);
        pintor.drawImage(MARGEN_X_SOMBRERO, MARGEN_Y_SOMBRERO, pantalon_tintado);
    }

    QImage sprite_brazos = aplicar_piel(
        hoja_cuerpo
            .copy(COLUMNA_BRAZOS * ANCHO_BASE_SPRITE, 0, ANCHO_BASE_SPRITE, ALTO_BASE_SPRITE),
        jugador.indice_piel);
    pintor.drawImage(MARGEN_X_SOMBRERO, MARGEN_Y_SOMBRERO, sprite_brazos);

    if (!hoja_remeras.isNull()) {
        int columna_r = jugador.indice_remera % COLUMNAS_REMERA;
        int pagina_r = jugador.indice_remera / COLUMNAS_REMERA;

        QImage recorte_remera = hoja_remeras.copy(
            columna_r * ANCHO_CELDA_REMERA,
            pagina_r * ALTO_BLOQUE_REMERA,
            ANCHO_CELDA_REMERA,
            ANCHO_CELDA_REMERA);
        int desvio_y_remera = es_femenino ? OFFSET_Y_REMERA_FEMENINO : OFFSET_Y_REMERA_MASCULINO;
        pintor.drawImage(
            MARGEN_X_SOMBRERO + POSICION_X_REMERA,
            MARGEN_Y_SOMBRERO + desvio_y_remera,
            recorte_remera);
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
        pintor.drawImage(MARGEN_X_SOMBRERO, MARGEN_Y_SOMBRERO + desvio_y_pelo, pelo_tintado);
    }

    if (jugador.indice_sombrero >= 0 && !hoja_sombreros.isNull()) {
        int columnas_sombrero = hoja_sombreros.width() / ANCHO_CELDA_SOMBRERO;
        if (columnas_sombrero <= 0) {
            columnas_sombrero = 1;
        }
        int columna_s = jugador.indice_sombrero % columnas_sombrero;
        int pagina_s = jugador.indice_sombrero / columnas_sombrero;

        QImage recorte_sombrero = hoja_sombreros.copy(
            columna_s * ANCHO_CELDA_SOMBRERO,
            pagina_s * ALTO_BLOQUE_SOMBRERO,
            ANCHO_CELDA_SOMBRERO,
            ANCHO_CELDA_SOMBRERO);
        int desvio_y_sombrero =
            es_femenino ? OFFSET_Y_SOMBRERO_FEMENINO : OFFSET_Y_SOMBRERO_MASCULINO;
        pintor.drawImage(0, desvio_y_sombrero, recorte_sombrero);
    }

    pintor.end();

    QImage escalada = lienzo.scaled(
        ANCHO_LIENZO * factor_escala,
        ALTO_LIENZO * factor_escala,
        Qt::IgnoreAspectRatio,
        Qt::FastTransformation);

    if (incluir_fondo_dia && !hoja_fondo_dia.isNull()) {
        QImage retrato = hoja_fondo_dia.scaled(
            ANCHO_RECUADRO_PREVIEW,
            ALTO_RECUADRO_PREVIEW,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation);
        QPainter pintor_retrato(&retrato);
        int pos_x = (ANCHO_RECUADRO_PREVIEW - escalada.width()) / 2;
        int pos_y = ALTO_RECUADRO_PREVIEW - escalada.height() - 6;
        pintor_retrato.drawImage(pos_x, pos_y, escalada);
        pintor_retrato.end();
        return QPixmap::fromImage(retrato);
    }

    return QPixmap::fromImage(escalada);
}
