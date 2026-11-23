#include "panel_de_validacion.h"

#include <QButtonGroup>
#include <QColor>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <map>
#include <string>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "tipo_entidad.h"
#include "validador_de_mundo.h"

static constexpr const char* ESTILO_BOTON_ENTIDAD =
    "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; border-radius: 4px; }"
    "QPushButton:checked { background-color: #4A6E82; color: white; font-weight: bold; }";

static constexpr const char* ESTILO_BOTON_ELIMINAR =
    "QPushButton { background-color: #B22222; color: white; font-weight: bold; padding: 6px; "
    "border-radius: 4px; }"
    "QPushButton:hover { background-color: #DC143C; }"
    "QPushButton:disabled { background-color: #555555; color: #888888; }";

struct ConfiguracionBotonEntidad {
    TipoEntidad entidad;
    const char* texto;
};

static const std::map<SectorActual, std::vector<ConfiguracionBotonEntidad>> ENTIDADES_POR_SECTOR = {
    {SectorActual::GRANJA,
     {{TipoEntidad::CABANIA, "Cabania del Jugador (9x9)"},
      {TipoEntidad::ENTRADA_CAVERNA, "Entrada Mina (5x7)"},
      {TipoEntidad::TRANSICION_PUEBLO, "Salida a Pueblo (2x2)"}}},
    {SectorActual::PUEBLO,
     {{TipoEntidad::CENTRO_COMUNITARIO, "Centro Comunitario (12x10)"},
      {TipoEntidad::MERCADO, "Mercado (10x10)"},
      {TipoEntidad::TRANSICION_GRANJA, "Salida a Granja (2x2)"}}},
    {SectorActual::MERCADO,
     {{TipoEntidad::PUESTO_MARNIE, "Puesto Marnie (2x2)"},
      {TipoEntidad::PUESTO_CLINT, "Puesto Clint (2x2)"},
      {TipoEntidad::PUESTO_PIERRE, "Puesto Pierre (2x2)"},
      {TipoEntidad::PUESTO_ROBIN, "Puesto Robin (2x2)"}}},
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
      lista_errores(nullptr),
      boton_revalidar(nullptr) {
    layout_principal = new QVBoxLayout(this);
    layout_principal->setContentsMargins(8, 8, 8, 8);
    layout_principal->setSpacing(8);

    // 1. Sección dinámica de Entidades del Sector
    inicializar_seccion_entidades();

    // 2. Título de Validación
    etiqueta_titulo = new QLabel("Validacion del Mundo", this);
    QFont fuente_titulo = etiqueta_titulo->font();
    fuente_titulo.setBold(true);
    fuente_titulo.setPointSize(12);
    etiqueta_titulo->setFont(fuente_titulo);
    layout_principal->addWidget(etiqueta_titulo);

    // 3. Etiqueta de Estado General (Válido / Inválido)
    etiqueta_estado_general = new QLabel("Pendiente de validacion", this);
    etiqueta_estado_general->setAlignment(Qt::AlignCenter);
    etiqueta_estado_general->setStyleSheet(
        "background-color: #333333; color: white; padding: 8px; border-radius: 4px; font-weight: "
        "bold;");
    layout_principal->addWidget(etiqueta_estado_general);

    // 4. Lista de Observaciones y Errores
    QGroupBox* grupo_errores = new QGroupBox("Observaciones y Errores", this);
    QVBoxLayout* layout_errores = new QVBoxLayout(grupo_errores);
    lista_errores = new QListWidget(grupo_errores);
    layout_errores->addWidget(lista_errores);
    layout_principal->addWidget(grupo_errores);

    // 5. Botón de Revalidar Mundo
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

    actualizar_sector(SectorActual::GRANJA);
}

void PanelDeValidacion::inicializar_seccion_entidades() {
    caja_entidades = new QGroupBox("Entidades del Sector", this);
    layout_botones_entidades = new QVBoxLayout(caja_entidades);
    layout_botones_entidades->setSpacing(4);

    grupo_entidades = new QButtonGroup(this);

    boton_eliminar_entidad = new QPushButton("Eliminar Seleccionada", caja_entidades);
    boton_eliminar_entidad->setStyleSheet(ESTILO_BOTON_ELIMINAR);
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
        boton->setStyleSheet(ESTILO_BOTON_ENTIDAD);
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
        etiqueta_estado_general->setText("MUNDO VALIDO Y COMPLETO");
        etiqueta_estado_general->setStyleSheet(
            "background-color: #2E6B38; color: white; padding: 8px; border-radius: 4px; "
            "font-weight: bold;");
        lista_errores->addItem("Todos los requisitos fueron verificados correctamente.");
        if (lista_errores->count() > 0) {
            lista_errores->item(0)->setForeground(QColor(100, 255, 100));
        }
    } else {
        etiqueta_estado_general->setText("MUNDO INVALIDO (TIENE ERRORES)");
        etiqueta_estado_general->setStyleSheet(
            "background-color: #8B0000; color: white; padding: 8px; border-radius: 4px; "
            "font-weight: bold;");
        size_t indice_error = 0;
        while (indice_error < resultado.errores.size()) {
            lista_errores->addItem(QString::fromStdString(resultado.errores[indice_error]));
            int fila_item = static_cast<int>(indice_error);
            if (lista_errores->item(fila_item) != nullptr) {
                lista_errores->item(fila_item)->setForeground(QColor(255, 120, 120));
            }
            indice_error++;
        }
    }
}
