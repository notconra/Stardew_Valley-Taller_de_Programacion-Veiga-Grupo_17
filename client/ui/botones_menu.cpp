#include "botones_menu.h"

#include <QIcon>

constexpr int ANCHO_BOTON_ORIGINAL = 74;
constexpr int ALTO_BOTON_ORIGINAL = 58;
constexpr int ANCHO_BOTON_ESCALADO = 118;
constexpr int ALTO_BOTON_ESCALADO = 93;
constexpr int FILA_Y_NORMAL = 187;
constexpr int FILA_Y_RESALTADO = 245;

constexpr int COLUMNA_NUEVA_PARTIDA = 0;
constexpr int COLUMNA_RETOMAR_PARTIDA = 1;
constexpr int COLUMNA_UNIRSE_PARTIDA = 2;
constexpr int COLUMNA_SALIR = 3;

const char* const TEXTO_AYUDA_NUEVA_PARTIDA = "Nueva Partida";
const char* const TEXTO_AYUDA_RETOMAR_PARTIDA = "Cargar Partida";
const char* const TEXTO_AYUDA_UNIRSE_PARTIDA = "Coop.";
const char* const TEXTO_AYUDA_SALIR = "Salir";

BotonesMenu::BotonesMenu(QObject* padre) : QObject(padre) {
}

void BotonesMenu::configurar(
    QPushButton* boton_nueva_partida, QPushButton* boton_retomar_partida,
    QPushButton* boton_unirse_partida, QPushButton* boton_salir) {
    QPixmap hoja_de_botones("Assets/Minigames/TitleButtons.es-ES.png");
    if (hoja_de_botones.isNull()) {
        hoja_de_botones.load("Assets/Minigames/TitleButtons.png");
    }
    if (hoja_de_botones.isNull()) {
        return;
    }

    configurar_boton(
        boton_nueva_partida,
        TEXTO_AYUDA_NUEVA_PARTIDA,
        COLUMNA_NUEVA_PARTIDA,
        hoja_de_botones);
    configurar_boton(
        boton_retomar_partida,
        TEXTO_AYUDA_RETOMAR_PARTIDA,
        COLUMNA_RETOMAR_PARTIDA,
        hoja_de_botones);
    configurar_boton(
        boton_unirse_partida,
        TEXTO_AYUDA_UNIRSE_PARTIDA,
        COLUMNA_UNIRSE_PARTIDA,
        hoja_de_botones);
    configurar_boton(boton_salir, TEXTO_AYUDA_SALIR, COLUMNA_SALIR, hoja_de_botones);
}

void BotonesMenu::configurar_boton(
    QPushButton* boton, const char* texto_de_ayuda, int columna_sprite,
    const QPixmap& hoja_de_botones) {
    boton->setText("");
    boton->setToolTip(texto_de_ayuda);
    boton->setCursor(Qt::PointingHandCursor);

    int origen_x = columna_sprite * ANCHO_BOTON_ORIGINAL;

    auto recortar_sprite = [&](int fila_y) {
        return hoja_de_botones.copy(origen_x, fila_y, ANCHO_BOTON_ORIGINAL, ALTO_BOTON_ORIGINAL)
            .scaled(
                ANCHO_BOTON_ESCALADO,
                ALTO_BOTON_ESCALADO,
                Qt::IgnoreAspectRatio,
                Qt::FastTransformation);
    };

    QPixmap sprite_normal = recortar_sprite(FILA_Y_NORMAL);
    QPixmap sprite_resaltado = recortar_sprite(FILA_Y_RESALTADO);
    QPixmap sprite_presionado = sprite_resaltado;

    boton->setIcon(QIcon(sprite_normal));
    boton->setIconSize(QSize(ANCHO_BOTON_ESCALADO, ALTO_BOTON_ESCALADO));
    boton->installEventFilter(this);

    estados_botones[boton] = EstadoBoton{sprite_normal, sprite_resaltado, sprite_presionado};
}

bool BotonesMenu::eventFilter(QObject* objeto_observado, QEvent* evento) {
    auto iterador = estados_botones.find(objeto_observado);
    if (iterador != estados_botones.end()) {
        auto* boton = static_cast<QPushButton*>(objeto_observado);
        switch (evento->type()) {
            case QEvent::Enter:
                boton->setIcon(QIcon(iterador->second.resaltado));
                break;
            case QEvent::Leave:
                boton->setIcon(QIcon(iterador->second.normal));
                break;
            case QEvent::MouseButtonPress:
                boton->setIcon(QIcon(iterador->second.presionado));
                break;
            case QEvent::MouseButtonRelease:
                boton->setIcon(QIcon(iterador->second.resaltado));
                break;
            default:
                break;
        }
    }
    return QObject::eventFilter(objeto_observado, evento);
}
