#ifndef PANEL_DE_VALIDACION_H
#define PANEL_DE_VALIDACION_H

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "validador_de_mundo.h"

class PanelDeValidacion : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout* layout_principal;
    QLabel* etiqueta_titulo;
    QLabel* etiqueta_estado_general;
    QListWidget* lista_requisitos;
    QListWidget* lista_errores;
    QPushButton* boton_revalidar;

    void agregar_item_requisito(const QString& descripcion, bool cumplido);

public:
    explicit PanelDeValidacion(QWidget* padre = nullptr);

    void actualizar_resultado(const ResultadoValidacion& resultado);

signals:
    void validacion_solicitada();
};

#endif  // PANEL_DE_VALIDACION_H
