#include "tarjeta_partida.h"

#include <QDialog>
#include <QStyle>
#include <string>

#include "recursos_ui.h"

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";
constexpr int ALTO_TARJETA = 104;
constexpr int FACTOR_ESCALA_AVATAR = 4;
constexpr int ANCHO_AVATAR_MOSTRAR = 40;
constexpr int ALTO_AVATAR_MOSTRAR = 68;

TarjetaPartida::TarjetaPartida(
    const InfoPartida& info, int slot, const CompositorPersonaje& compositor, QWidget* padre)
    : QFrame(padre),
      info_partida(info),
      numero_slot(slot),
      seleccionada(false),
      layout_principal(this),
      label_slot(this),
      label_avatar(this),
      label_nombre_personaje(this),
      label_estado(this),
      layout_derecha(),
      layout_superior_derecha(),
      label_nombre_granja(this),
      btn_eliminar(this) {
    inicializar_interfaz(compositor);
}

void TarjetaPartida::inicializar_interfaz(const CompositorPersonaje& compositor) {
    setFixedHeight(ALTO_TARJETA);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setProperty("class", "tarjeta_partida");
    setProperty("seleccionada", false);

    layout_principal.setContentsMargins(20, 8, 8, 8);
    layout_principal.setSpacing(14);

    // 1. Ranura: "1.", "2.", etc.
    label_slot.setText(QString("%1.").arg(numero_slot));
    label_slot.setFixedWidth(30);
    label_slot.setAlignment(Qt::AlignTop | Qt::AlignLeft);
    label_slot.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_slot.setProperty("class", "label_partida_slot");
    layout_principal.addWidget(&label_slot);

    // 2. Avatar centrado con silueta recortada
    label_avatar.setFixedSize(ANCHO_AVATAR_MOSTRAR, ALTO_AVATAR_MOSTRAR);
    label_avatar.setAlignment(Qt::AlignCenter);
    label_avatar.setAttribute(Qt::WA_TransparentForMouseEvents);
    ConfiguracionJugador config_jugador =
        crear_configuracion_desde_apariencia(info_partida.apariencia);
    QPixmap sprite_original = compositor.componer(config_jugador, FACTOR_ESCALA_AVATAR, false);
    if (!sprite_original.isNull()) {
        QImage silueta = recortar_silueta(sprite_original.toImage());
        QPixmap sprite_centrado = QPixmap::fromImage(silueta).scaled(
            ANCHO_AVATAR_MOSTRAR,
            ALTO_AVATAR_MOSTRAR,
            Qt::KeepAspectRatio,
            Qt::FastTransformation);
        label_avatar.setPixmap(sprite_centrado);
    }
    layout_principal.addWidget(&label_avatar);

    // 3. Columna izquierda de datos: Nombre y Estado
    layout_info.setSpacing(6);
    layout_info.setAlignment(Qt::AlignTop);

    label_nombre_personaje.setText(QString::fromStdString(info_partida.nombre_personaje));
    label_nombre_personaje.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_nombre_personaje.setProperty("class", "label_partida_nombre");
    layout_info.addWidget(&label_nombre_personaje);

    QString texto_estado =
        info_partida.multijugador
            ? QString("Multijugador: %1 conectados").arg(info_partida.jugadores_conectados)
            : "Un Jugador";
    label_estado.setText(texto_estado);
    label_estado.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_estado.setProperty("class", "label_partida_estado");
    layout_info.addWidget(&label_estado);

    layout_principal.addLayout(&layout_info);
    layout_principal.addStretch();

    // 4. Columna derecha: Fila superior con "Granja..." y botón eliminar en la punta
    layout_derecha.setAlignment(Qt::AlignTop | Qt::AlignRight);
    layout_derecha.setSpacing(0);

    layout_superior_derecha.setSpacing(12);
    layout_superior_derecha.setAlignment(Qt::AlignTop | Qt::AlignRight);

    label_nombre_granja.setText(
        QString("Granja %1").arg(QString::fromStdString(info_partida.nombre_partida)));
    label_nombre_granja.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_nombre_granja.setProperty("class", "label_partida_granja");
    label_nombre_granja.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    layout_superior_derecha.addWidget(&label_nombre_granja);

    RecursosUI::instancia().configurar_boton_eliminar(&btn_eliminar);
    btn_eliminar.setToolTip("Eliminar partida guardada");
    btn_eliminar.installEventFilter(this);
    connect(&btn_eliminar, &QPushButton::clicked, this, &TarjetaPartida::al_hacer_clic_eliminar);
    layout_superior_derecha.addWidget(&btn_eliminar);

    layout_derecha.addLayout(&layout_superior_derecha);
    layout_principal.addLayout(&layout_derecha);
}

void TarjetaPartida::al_hacer_clic_eliminar() {
    QDialog dialogo(window());
    dialogo.setObjectName("dialogo_confirmacion");
    dialogo.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialogo.setAttribute(Qt::WA_TranslucentBackground, true);

    QVBoxLayout layout_dialogo(&dialogo);
    layout_dialogo.setContentsMargins(0, 0, 0, 0);

    QFrame marco(&dialogo);
    marco.setObjectName("marco_dialogo_confirmacion");

    QVBoxLayout layout_interior(&marco);
    layout_interior.setContentsMargins(24, 24, 24, 24);
    layout_interior.setSpacing(18);

    QLabel label_texto(&marco);
    label_texto.setObjectName("label_texto_confirmacion");
    label_texto.setAlignment(Qt::AlignCenter);
    label_texto.setWordWrap(true);
    label_texto.setText(
        QString("Seguro que queres eliminar la partida \"%1\"?\nEsta accion no se puede deshacer.")
            .arg(QString::fromStdString(info_partida.nombre_partida)));
    layout_interior.addWidget(&label_texto);

    QHBoxLayout layout_botones;
    layout_botones.setSpacing(24);
    layout_botones.setAlignment(Qt::AlignCenter);

    QPushButton btn_no("No", &marco);
    btn_no.setObjectName("btn_confirmar_no");
    btn_no.setCursor(Qt::PointingHandCursor);
    connect(&btn_no, &QPushButton::clicked, &dialogo, &QDialog::reject);
    layout_botones.addWidget(&btn_no);

    QPushButton btn_si("Si", &marco);
    btn_si.setObjectName("btn_confirmar_si");
    btn_si.setCursor(Qt::PointingHandCursor);
    connect(&btn_si, &QPushButton::clicked, &dialogo, &QDialog::accept);
    layout_botones.addWidget(&btn_si);

    layout_interior.addLayout(&layout_botones);
    layout_dialogo.addWidget(&marco);

    if (dialogo.exec() == QDialog::Accepted) {
        emit partidaEliminarSolicitada(QString::fromStdString(info_partida.codigo_partida));
    }
}

void TarjetaPartida::mousePressEvent(QMouseEvent* evento) {
    if (evento->button() == Qt::LeftButton) {
        emit partidaSeleccionada(QString::fromStdString(info_partida.codigo_partida));
    }
    QFrame::mousePressEvent(evento);
}

void TarjetaPartida::mouseDoubleClickEvent(QMouseEvent* evento) {
    if (evento->button() == Qt::LeftButton) {
        emit partidaDobleClic(QString::fromStdString(info_partida.codigo_partida));
    }
    QFrame::mouseDoubleClickEvent(evento);
}

bool TarjetaPartida::eventFilter(QObject* objeto, QEvent* evento) {
    if (objeto == &btn_eliminar) {
        if (evento->type() == QEvent::Enter) {
            btn_eliminar.setIcon(RecursosUI::instancia().boton_eliminar_hover());
        } else if (evento->type() == QEvent::Leave) {
            btn_eliminar.setIcon(RecursosUI::instancia().boton_eliminar());
        }
    }
    return QFrame::eventFilter(objeto, evento);
}

void TarjetaPartida::establecer_seleccionada(bool valor) {
    if (seleccionada != valor) {
        seleccionada = valor;
        setProperty("seleccionada", valor);
        style()->unpolish(this);
        style()->polish(this);
    }
}

bool TarjetaPartida::esta_seleccionada() const {
    return seleccionada;
}

std::string TarjetaPartida::obtener_codigo_partida() const {
    return info_partida.codigo_partida;
}

ConfiguracionJugador TarjetaPartida::crear_configuracion_desde_apariencia(
    const AparienciaPersonaje& apariencia) const {
    ConfiguracionJugador config;
    config.ruta_cuerpo = apariencia.es_femenino ? RUTA_CUERPO_FEMENINO : RUTA_CUERPO_MASCULINO;
    config.offset_y_remera =
        apariencia.es_femenino ? OFFSET_Y_REMERA_FEMENINO : OFFSET_Y_REMERA_MASCULINO;
    config.offset_y_pelo =
        apariencia.es_femenino ? OFFSET_Y_PELO_FEMENINO : OFFSET_Y_PELO_MASCULINO;
    config.offset_y_sombrero =
        apariencia.es_femenino ? OFFSET_Y_SOMBRERO_FEMENINO : OFFSET_Y_SOMBRERO_MASCULINO;
    config.indice_pelo = apariencia.indice_pelo;
    config.color_ojos = apariencia.color_ojos;
    config.color_pelo = apariencia.color_pelo;
    config.indice_remera = apariencia.indice_remera;
    config.color_pantalon = apariencia.color_pantalon;
    config.indice_pantalon = apariencia.indice_pantalon;
    config.indice_sombrero = apariencia.indice_sombrero;
    config.indice_piel = apariencia.indice_piel;
    return config;
}

QImage TarjetaPartida::recortar_silueta(const QImage& imagen) const {
    if (imagen.isNull()) {
        return imagen;
    }
    int min_x = imagen.width();
    int min_y = imagen.height();
    int max_x = -1;
    int max_y = -1;

    for (int y = 0; y < imagen.height(); ++y) {
        const QRgb* fila = reinterpret_cast<const QRgb*>(imagen.scanLine(y));
        for (int x = 0; x < imagen.width(); ++x) {
            if (qAlpha(fila[x]) > 0) {
                if (x < min_x) {
                    min_x = x;
                }
                if (x > max_x) {
                    max_x = x;
                }
                if (y < min_y) {
                    min_y = y;
                }
                if (y > max_y) {
                    max_y = y;
                }
            }
        }
    }

    if (max_x < min_x || max_y < min_y) {
        return imagen;
    }
    return imagen.copy(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
}
