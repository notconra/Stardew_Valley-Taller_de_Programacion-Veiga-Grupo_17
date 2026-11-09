#ifndef TARJETA_PARTIDA_H
#define TARJETA_PARTIDA_H

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <string>

#include "../../common/mensajes/mensajes_lobby.h"
#include "compositor_personaje.h"

// Representa una ranura de guardado interactiva en el menú de "Retomar Partida".
class TarjetaPartida : public QFrame {
    Q_OBJECT

public:
    TarjetaPartida(
        const InfoPartida& info, int numero_slot, const CompositorPersonaje& compositor,
        QWidget* padre = nullptr);

    void establecer_seleccionada(bool seleccionada);
    bool esta_seleccionada() const;
    std::string obtener_codigo_partida() const;

signals:
    void partidaSeleccionada(const QString& codigo_partida);
    void partidaEliminarSolicitada(const QString& codigo_partida);
    void partidaDobleClic(const QString& codigo_partida);

protected:
    void mousePressEvent(QMouseEvent* evento) override;
    void mouseDoubleClickEvent(QMouseEvent* evento) override;
    bool eventFilter(QObject* objeto, QEvent* evento) override;

private slots:
    void al_hacer_clic_eliminar();

private:
    void inicializar_interfaz(const CompositorPersonaje& compositor);
    ConfiguracionJugador crear_configuracion_desde_apariencia(
        const AparienciaPersonaje& apariencia) const;
    QImage recortar_silueta(const QImage& imagen) const;
    QString formatear_fecha(uint32_t dia) const;
    QString formatear_tiempo_jugado(uint32_t segundos) const;

    InfoPartida info_partida;
    int numero_slot;
    bool seleccionada;

    QHBoxLayout layout_principal;
    QLabel label_slot;
    QLabel label_avatar;
    QVBoxLayout layout_info;
    QLabel label_nombre_personaje;
    QLabel label_fecha;
    QLabel label_estado;
    QHBoxLayout layout_derecha;
    QVBoxLayout layout_textos_derecha;
    QLabel label_nombre_granja;
    QPushButton btn_eliminar;
    QHBoxLayout layout_tiempo_jugado;
    QLabel label_icono_reloj;
    QLabel label_tiempo_jugado;
};

#endif  // TARJETA_PARTIDA_H
