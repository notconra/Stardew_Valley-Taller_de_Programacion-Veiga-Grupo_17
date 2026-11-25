#include "panel_de_validacion.h"

#include <QButtonGroup>
#include <QColor>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QStyle>
#include <QVBoxLayout>
#include <map>
#include <string>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "tipo_entidad.h"
#include "validador_de_mundo.h"

struct ConfiguracionBotonEntidad {
    TipoEntidad entidad;
    const char* texto;
};

static const std::map<SectorActual, std::vector<ConfiguracionBotonEntidad>> ENTIDADES_POR_SECTOR = {
    {SectorActual::GRANJA,
     {{TipoEntidad::CABANIA, "Cabaña del Jugador (9x9)"},
      {TipoEntidad::ENTRADA_CAVERNA, "Entrada Mina (5x7)"},
      {TipoEntidad::TRANSICION_PUEBLO, "Salida a Pueblo (Borde 1x2 / 2x1)"}}},
    {SectorActual::PUEBLO,
     {{TipoEntidad::CENTRO_COMUNITARIO, "Centro Comunitario (12x10)"},
      {TipoEntidad::MERCADO, "Mercado (10x10)"},
      {TipoEntidad::TRANSICION_GRANJA, "Salida a Granja (Borde 1x2 / 2x1)"}}},
    {SectorActual::MERCADO,
     {{TipoEntidad::PUESTO_MARNIE, "Puesto Marnie (1x1)"},
      {TipoEntidad::PUESTO_CLINT, "Puesto Clint (1x1)"},
      {TipoEntidad::PUESTO_PIERRE, "Puesto Pierre (1x1)"},
      {TipoEntidad::PUESTO_ROBIN, "Puesto Robin (1x1)"}}},
    {SectorActual::CAVERNA,
     {{TipoEntidad::ESCALERA_NIVEL, "Escalera Mazmorra (1x1)"},
      {TipoEntidad::SALIDA_CAVERNA, "Salida Caverna (1x1)"}}}};

PanelDeValidacion::PanelDeValidacion(QWidget* padre)
    : QWidget(padre),
      layout_principal(nullptr),
      caja_entidades(nullptr),
      layout_botones_entidades(nullptr),
      grupo_entidades(nullptr),
      botones_entidades_actuales(),
      boton_eliminar_entidad(nullptr),
      etiqueta_titulo(nullptr),
      etiqueta_estado_general(nullptr),
      lista_errores(nullptr) {
    layout_principal = new QVBoxLayout(this);
    layout_principal->setContentsMargins(8, 8, 8, 8);
    layout_principal->setSpacing(8);

    // 1. Sección dinámica de Entidades del Sector
    inicializar_seccion_entidades();

    // 2. Título de Validación
    etiqueta_titulo = new QLabel("Validación del Mundo", this);
    QFont fuente_titulo = etiqueta_titulo->font();
    fuente_titulo.setBold(true);
    fuente_titulo.setPointSize(12);
    etiqueta_titulo->setFont(fuente_titulo);
    layout_principal->addWidget(etiqueta_titulo);

    // 3. Etiqueta de Estado General (Válido / Inválido)
    etiqueta_estado_general = new QLabel("Pendiente de validación", this);
    etiqueta_estado_general->setObjectName("etiqueta_estado_general");
    etiqueta_estado_general->setAlignment(Qt::AlignCenter);
    layout_principal->addWidget(etiqueta_estado_general);

    // 4. Lista de Observaciones y Errores
    QGroupBox* grupo_errores = new QGroupBox("Observaciones y Errores", this);
    QVBoxLayout* layout_errores = new QVBoxLayout(grupo_errores);
    lista_errores = new QListWidget(grupo_errores);
    lista_errores->setWordWrap(true);
    lista_errores->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lista_errores->setTextElideMode(Qt::ElideNone);
    lista_errores->setSpacing(4);
    connect(lista_errores, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (item == nullptr) {
            return;
        }
        QVariant dato = item->data(Qt::UserRole);
        if (!dato.isValid()) {
            return;
        }
        int indice_sector = dato.toInt();
        if (indice_sector >= 0) {
            emit sector_solicitado(indice_sector);
        }
    });
    layout_errores->addWidget(lista_errores);
    layout_principal->addWidget(grupo_errores);

    actualizar_sector(SectorActual::GRANJA);
}

void PanelDeValidacion::inicializar_seccion_entidades() {
    caja_entidades = new QGroupBox("Entidades del Sector", this);
    layout_botones_entidades = new QVBoxLayout(caja_entidades);
    layout_botones_entidades->setSpacing(4);

    grupo_entidades = new QButtonGroup(this);

    boton_eliminar_entidad = new QPushButton("Eliminar Seleccionada", caja_entidades);
    boton_eliminar_entidad->setObjectName("boton_eliminar_entidad");
    boton_eliminar_entidad->setEnabled(false);
    connect(
        boton_eliminar_entidad,
        &QPushButton::clicked,
        this,
        &PanelDeValidacion::eliminar_solicitado);
    layout_botones_entidades->addWidget(boton_eliminar_entidad);

    layout_principal->addWidget(caja_entidades);
}

void PanelDeValidacion::limpiar_botones_entidades() {
    size_t indice_boton = 0;
    while (indice_boton < botones_entidades_actuales.size()) {
        QPushButton* boton = botones_entidades_actuales[indice_boton];
        grupo_entidades->removeButton(boton);
        layout_botones_entidades->removeWidget(boton);
        delete boton;
        indice_boton++;
    }
    botones_entidades_actuales.clear();
}

void PanelDeValidacion::actualizar_sector(SectorActual sector) {
    limpiar_botones_entidades();

    auto iterador = ENTIDADES_POR_SECTOR.find(sector);
    if (iterador == ENTIDADES_POR_SECTOR.end()) {
        return;
    }

    const std::vector<ConfiguracionBotonEntidad>& lista_configs = iterador->second;
    size_t indice_config = 0;
    while (indice_config < lista_configs.size()) {
        TipoEntidad tipo_entidad = lista_configs[indice_config].entidad;
        QString texto_boton = QString::fromUtf8(lista_configs[indice_config].texto);
        QPushButton* boton = new QPushButton(texto_boton, caja_entidades);
        boton->setCheckable(true);
        boton->setProperty("rol", "entidad");
        grupo_entidades->addButton(boton, static_cast<int>(tipo_entidad));
        connect(boton, &QPushButton::clicked, this, [this, tipo_entidad]() {
            emit entidad_cambiada(tipo_entidad);
        });
        layout_botones_entidades->insertWidget(static_cast<int>(indice_config), boton);
        botones_entidades_actuales.push_back(boton);
        indice_config++;
    }

    if (!botones_entidades_actuales.empty()) {
        botones_entidades_actuales[0]->setChecked(true);
        emit entidad_cambiada(lista_configs[0].entidad);
    }
}

void PanelDeValidacion::habilitar_boton_eliminar(bool habilitado) {
    if (boton_eliminar_entidad != nullptr) {
        boton_eliminar_entidad->setEnabled(habilitado);
    }
}

void PanelDeValidacion::actualizar_resultado(const ResultadoValidacion& resultado) {
    lista_errores->clear();

    if (resultado.es_valido) {
        etiqueta_estado_general->setText("MUNDO VÁLIDO Y COMPLETO");
        etiqueta_estado_general->setProperty("estado", "valido");
        etiqueta_estado_general->style()->unpolish(etiqueta_estado_general);
        etiqueta_estado_general->style()->polish(etiqueta_estado_general);
        lista_errores->addItem("Todos los requisitos fueron verificados correctamente.");
        if (lista_errores->count() > 0) {
            lista_errores->item(0)->setForeground(QColor(100, 255, 100));
        }
    } else {
        etiqueta_estado_general->setText("MUNDO INVÁLIDO (TIENE ERRORES)");
        etiqueta_estado_general->setProperty("estado", "invalido");
        etiqueta_estado_general->style()->unpolish(etiqueta_estado_general);
        etiqueta_estado_general->style()->polish(etiqueta_estado_general);
        size_t indice_error = 0;
        while (indice_error < resultado.errores.size()) {
            QListWidgetItem* item = new QListWidgetItem(
                QString::fromStdString(resultado.errores[indice_error].mensaje));
            item->setForeground(QColor(255, 120, 120));
            item->setData(Qt::UserRole, resultado.errores[indice_error].indice_sector);
            if (resultado.errores[indice_error].indice_sector >= 0) {
                item->setToolTip("Doble clic para ir al sector");
            }
            lista_errores->addItem(item);
            indice_error++;
        }
    }
}
