#include "tarjeta_partida.h"

#include <QMessageBox>
#include <QStyle>
#include <string>

#include "recursos_ui.h"

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";
constexpr int ALTO_TARJETA = 84;
constexpr int FACTOR_ESCALA_AVATAR = 2;

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
      label_nombre_granja(this),
      label_estado(this),
      btn_eliminar(this) {
    inicializar_interfaz(compositor);
}

void TarjetaPartida::inicializar_interfaz(const CompositorPersonaje& compositor) {
    setFixedHeight(ALTO_TARJETA);
    setCursor(Qt::PointingHandCursor);
    setProperty("class", "tarjeta_partida");
    setProperty("seleccionada", false);

    layout_principal.setContentsMargins(14, 6, 14, 6);
    layout_principal.setSpacing(12);

    label_slot.setText(QString("#%1").arg(numero_slot));
    label_slot.setFixedWidth(36);
    label_slot.setAlignment(Qt::AlignCenter);
    label_slot.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_slot.setProperty("class", "label_partida_slot");
    layout_principal.addWidget(&label_slot);

    label_avatar.setFixedSize(48, 70);
    label_avatar.setAlignment(Qt::AlignCenter);
    label_avatar.setAttribute(Qt::WA_TransparentForMouseEvents);
    ConfiguracionJugador config_jugador =
        crear_configuracion_desde_apariencia(info_partida.apariencia);
    QPixmap sprite = compositor.componer(config_jugador, FACTOR_ESCALA_AVATAR);
    if (!sprite.isNull()) {
        label_avatar.setPixmap(sprite);
    }
    layout_principal.addWidget(&label_avatar);

    layout_info.setSpacing(2);
    layout_info.setAlignment(Qt::AlignVCenter);

    label_nombre_personaje.setText(QString::fromStdString(info_partida.nombre_personaje));
    label_nombre_personaje.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_nombre_personaje.setProperty("class", "label_partida_nombre");
    layout_info.addWidget(&label_nombre_personaje);

    label_nombre_granja.setText(
        QString("Granja %1").arg(QString::fromStdString(info_partida.nombre_partida)));
    label_nombre_granja.setAttribute(Qt::WA_TransparentForMouseEvents);
    label_nombre_granja.setProperty("class", "label_partida_granja");
    layout_info.addWidget(&label_nombre_granja);

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

    RecursosUI::instancia().configurar_boton_eliminar(&btn_eliminar);
    btn_eliminar.setToolTip("Eliminar partida guardada");
    connect(&btn_eliminar, &QPushButton::clicked, this, &TarjetaPartida::al_hacer_clic_eliminar);
    layout_principal.addWidget(&btn_eliminar);
}

void TarjetaPartida::al_hacer_clic_eliminar() {
    QMessageBox::StandardButton respuesta = QMessageBox::question(
        this,
        "Eliminar partida",
        QString("¿Estás seguro de que querés eliminar la partida \"%1\"? Esta acción no se puede "
                "deshacer.")
            .arg(QString::fromStdString(info_partida.nombre_partida)),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (respuesta == QMessageBox::Yes) {
        emit partidaEliminarSolicitada(QString::fromStdString(info_partida.codigo_partida));
    }
}

void TarjetaPartida::mousePressEvent(QMouseEvent* evento) {
    if (evento->button() == Qt::LeftButton) {
        emit partidaSeleccionada(QString::fromStdString(info_partida.codigo_partida));
    }
    QFrame::mousePressEvent(evento);
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
