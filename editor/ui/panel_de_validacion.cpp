#include "panel_de_validacion.h"

#include <QColor>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <string>
#include <vector>

#include "validador_de_mundo.h"

PanelDeValidacion::PanelDeValidacion(QWidget* padre)
    : QWidget(padre),
      layout_principal(nullptr),
      etiqueta_titulo(nullptr),
      etiqueta_estado_general(nullptr),
      lista_requisitos(nullptr),
      lista_errores(nullptr),
      boton_revalidar(nullptr) {
    layout_principal = new QVBoxLayout(this);
    layout_principal->setContentsMargins(8, 8, 8, 8);
    layout_principal->setSpacing(8);

    etiqueta_titulo = new QLabel("Validacion del Mundo", this);
    QFont fuente_titulo = etiqueta_titulo->font();
    fuente_titulo.setBold(true);
    fuente_titulo.setPointSize(12);
    etiqueta_titulo->setFont(fuente_titulo);
    layout_principal->addWidget(etiqueta_titulo);

    etiqueta_estado_general = new QLabel("Pendiente de validacion", this);
    etiqueta_estado_general->setAlignment(Qt::AlignCenter);
    etiqueta_estado_general->setStyleSheet(
        "background-color: #333333; color: white; padding: 8px; border-radius: 4px; font-weight: "
        "bold;");
    layout_principal->addWidget(etiqueta_estado_general);

    QGroupBox* grupo_requisitos = new QGroupBox("Requisitos del Enunciado", this);
    QVBoxLayout* layout_requisitos = new QVBoxLayout(grupo_requisitos);
    lista_requisitos = new QListWidget(grupo_requisitos);
    lista_requisitos->setSelectionMode(QAbstractItemView::NoSelection);
    layout_requisitos->addWidget(lista_requisitos);
    layout_principal->addWidget(grupo_requisitos);

    QGroupBox* grupo_errores = new QGroupBox("Observaciones y Errores", this);
    QVBoxLayout* layout_errores = new QVBoxLayout(grupo_errores);
    lista_errores = new QListWidget(grupo_errores);
    layout_errores->addWidget(lista_errores);
    layout_principal->addWidget(grupo_errores);

    boton_revalidar = new QPushButton("Revalidar Mundo", this);
    boton_revalidar->setStyleSheet(
        "QPushButton { background-color: #2E6B38; color: white; font-weight: bold; padding: 8px; "
        "border-radius: 4px; }"
        "QPushButton:hover { background-color: #3B8C4A; }");
    connect(
        boton_revalidar,
        &QPushButton::clicked,
        this,
        &PanelDeValidacion::validacion_solicitada);
    layout_principal->addWidget(boton_revalidar);
}

void PanelDeValidacion::agregar_item_requisito(const QString& descripcion, bool cumplido) {
    QString texto = (cumplido ? "[OK] " : "[FALTA] ") + descripcion;
    QListWidgetItem* item = new QListWidgetItem(texto, lista_requisitos);
    if (cumplido) {
        item->setForeground(QColor(80, 200, 80));
    } else {
        item->setForeground(QColor(230, 80, 80));
    }
}

void PanelDeValidacion::actualizar_resultado(const ResultadoValidacion& resultado) {
    lista_requisitos->clear();
    lista_errores->clear();

    if (resultado.es_valido) {
        etiqueta_estado_general->setText("MUNDO VALIDO Y COMPLETO");
        etiqueta_estado_general->setStyleSheet(
            "background-color: #1E6B2A; color: white; padding: 8px; border-radius: 4px; "
            "font-weight: bold;");
    } else {
        etiqueta_estado_general->setText("MUNDO INVALIDO (TIENE ERRORES)");
        etiqueta_estado_general->setStyleSheet(
            "background-color: #8B2500; color: white; padding: 8px; border-radius: 4px; "
            "font-weight: bold;");
    }

    size_t indice_error = 0;
    while (indice_error < resultado.errores.size()) {
        std::string mensaje = resultado.errores[indice_error];
        QListWidgetItem* item_error =
            new QListWidgetItem(QString::fromStdString(mensaje), lista_errores);
        item_error->setForeground(QColor(255, 100, 100));
        indice_error++;
    }

    if (resultado.errores.empty()) {
        QListWidgetItem* item_exito = new QListWidgetItem(
            "Todos los requisitos fueron verificados correctamente.",
            lista_errores);
        item_exito->setForeground(QColor(150, 220, 150));
    }
}
