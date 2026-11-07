#include "recursos_ui.h"

constexpr const char* RUTA_HOJA_CURSORS = "Assets/LooseSprites/Cursors.png";

// Coordenadas originales de los sprites dentro de Cursors.png
constexpr RecursosUI::RegionSprite REGION_DADO = {381, 361, 10, 10};
constexpr RecursosUI::RegionSprite REGION_BOTON_OK = {128, 256, 64, 64};
constexpr RecursosUI::RegionSprite REGION_GENERO_MASCULINO = {128, 192, 16, 16};
constexpr RecursosUI::RegionSprite REGION_GENERO_FEMENINO = {144, 192, 16, 16};
constexpr RecursosUI::RegionSprite REGION_FLECHA_IZQUIERDA = {352, 495, 12, 11};
constexpr RecursosUI::RegionSprite REGION_FLECHA_DERECHA = {365, 495, 12, 11};

// Dimensiones de visualización en la interfaz
constexpr int TAMANIO_ICONO_DADO = 30;
constexpr int TAMANIO_BOTON_OK = 64;
constexpr int TAMANIO_ICONO_GENERO = 32;
constexpr int ANCHO_BOTON_FLECHA = 28;
constexpr int ALTO_BOTON_FLECHA = 26;

RecursosUI& RecursosUI::instancia() {
    static RecursosUI singleton;
    return singleton;
}

RecursosUI::RecursosUI() {
    hoja_cursors.load(RUTA_HOJA_CURSORS);
    if (!hoja_cursors.isNull()) {
        sprite_dado = recortar_y_escalar(REGION_DADO, TAMANIO_ICONO_DADO, TAMANIO_ICONO_DADO);

        sprite_ok = recortar_y_escalar(REGION_BOTON_OK, TAMANIO_BOTON_OK, TAMANIO_BOTON_OK);

        sprite_masculino =
            recortar_y_escalar(REGION_GENERO_MASCULINO, TAMANIO_ICONO_GENERO, TAMANIO_ICONO_GENERO);

        sprite_femenino =
            recortar_y_escalar(REGION_GENERO_FEMENINO, TAMANIO_ICONO_GENERO, TAMANIO_ICONO_GENERO);

        sprite_flecha_izq =
            recortar_y_escalar(REGION_FLECHA_IZQUIERDA, ANCHO_BOTON_FLECHA, ALTO_BOTON_FLECHA);

        sprite_flecha_der =
            recortar_y_escalar(REGION_FLECHA_DERECHA, ANCHO_BOTON_FLECHA, ALTO_BOTON_FLECHA);
    }
}

QPixmap RecursosUI::recortar_y_escalar(
    const RegionSprite& region, int ancho_destino, int alto_destino) const {
    return hoja_cursors.copy(region.x, region.y, region.ancho, region.alto)
        .scaled(ancho_destino, alto_destino, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

void RecursosUI::configurar_boton_icono(QPushButton* boton, const QPixmap& pixmap) {
    if (boton == nullptr || pixmap.isNull()) {
        return;
    }
    boton->setText("");
    boton->setIcon(QIcon(pixmap));
    boton->setIconSize(pixmap.size());
    boton->setCursor(Qt::PointingHandCursor);
}

void RecursosUI::configurar_boton_dado(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_dado);
}

void RecursosUI::configurar_boton_ok(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_ok);
}

void RecursosUI::configurar_boton_masculino(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_masculino);
}

void RecursosUI::configurar_boton_femenino(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_femenino);
}

void RecursosUI::configurar_boton_flecha_izquierda(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_flecha_izq);
}

void RecursosUI::configurar_boton_flecha_derecha(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_flecha_der);
}
