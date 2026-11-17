#include "paleta_de_herramientas.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>

#include "tipo_entidad.h"
#include "tipo_herramienta_editor.h"

PaletaDeHerramientas::PaletaDeHerramientas(QWidget* padre)
    : QWidget(padre),
      layout_principal(nullptr),
      grupo_modos(nullptr),
      grupo_terrenos(nullptr),
      botones_terrenos(),
      grupo_entidades(nullptr),
      contenedor_entidades(nullptr),
      layout_entidades(nullptr),
      boton_eliminar_entidad(nullptr) {
    layout_principal = new QVBoxLayout(this);
    layout_principal->setContentsMargins(8, 8, 8, 8);
    layout_principal->setSpacing(10);

    // 1. Grupo de Modos
    QGroupBox* caja_modos = new QGroupBox("Modo de Edicion", this);
    QVBoxLayout* layout_modos = new QVBoxLayout(caja_modos);
    grupo_modos = new QButtonGroup(this);

    QPushButton* boton_terreno = crear_boton_modo("Pintar Terreno", ModoEditor::TERRENO);
    QPushButton* boton_colocar = crear_boton_modo("Colocar Entidad", ModoEditor::COLOCAR_ENTIDAD);
    QPushButton* boton_mover = crear_boton_modo("Mover Entidad", ModoEditor::SELECCIONAR_MOVER);
    QPushButton* boton_borrar = crear_boton_modo("Borrar Entidad", ModoEditor::BORRAR);

    boton_terreno->setChecked(true);
    layout_modos->addWidget(boton_terreno);
    layout_modos->addWidget(boton_colocar);
    layout_modos->addWidget(boton_mover);
    layout_modos->addWidget(boton_borrar);
    layout_principal->addWidget(caja_modos);

    // 2. Grupo de Terrenos
    QGroupBox* caja_terrenos = new QGroupBox("Tipo de Terreno", this);
    QVBoxLayout* layout_terrenos = new QVBoxLayout(caja_terrenos);
    grupo_terrenos = new QButtonGroup(this);

    QPushButton* boton_pasto = crear_boton_terreno("Pasto", TipoTerrenoPaleta::PASTO, "#5C8E32");
    QPushButton* boton_cultivable =
        crear_boton_terreno("Tierra Cultivable", TipoTerrenoPaleta::TIERRA_CULTIVABLE, "#8B5A2B");
    QPushButton* boton_camino = crear_boton_terreno("Camino", TipoTerrenoPaleta::CAMINO, "#D3B88C");
    QPushButton* boton_agua = crear_boton_terreno("Agua", TipoTerrenoPaleta::AGUA, "#3A75C4");
    QPushButton* boton_muro =
        crear_boton_terreno("Muro / Roca", TipoTerrenoPaleta::MURO, "#4A4A4A");

    botones_terrenos.push_back(boton_pasto);
    botones_terrenos.push_back(boton_cultivable);
    botones_terrenos.push_back(boton_camino);
    botones_terrenos.push_back(boton_agua);
    botones_terrenos.push_back(boton_muro);

    boton_pasto->setChecked(true);
    layout_terrenos->addWidget(boton_pasto);
    layout_terrenos->addWidget(boton_cultivable);
    layout_terrenos->addWidget(boton_camino);
    layout_terrenos->addWidget(boton_agua);
    layout_terrenos->addWidget(boton_muro);
    layout_principal->addWidget(caja_terrenos);

    // 3. Grupo de Entidades
    QGroupBox* caja_entidades = new QGroupBox("Entidades del Sector", this);
    QVBoxLayout* layout_caja_entidades = new QVBoxLayout(caja_entidades);
    contenedor_entidades = new QWidget(caja_entidades);
    layout_entidades = new QVBoxLayout(contenedor_entidades);
    layout_entidades->setContentsMargins(0, 0, 0, 0);
    layout_caja_entidades->addWidget(contenedor_entidades);

    grupo_entidades = new QButtonGroup(this);

    boton_eliminar_entidad = new QPushButton("Eliminar Seleccionada", caja_entidades);
    boton_eliminar_entidad->setStyleSheet(
        "QPushButton { background-color: #B22222; color: white; font-weight: bold; padding: 6px; "
        "border-radius: 4px; }"
        "QPushButton:hover { background-color: #DC143C; }"
        "QPushButton:disabled { background-color: #555555; color: #888888; }");
    boton_eliminar_entidad->setEnabled(false);
    connect(
        boton_eliminar_entidad,
        &QPushButton::clicked,
        this,
        &PaletaDeHerramientas::eliminar_solicitado);
    layout_caja_entidades->addWidget(boton_eliminar_entidad);

    layout_principal->addWidget(caja_entidades);
    layout_principal->addStretch();

    actualizar_sector(SectorActual::GRANJA);
}

QPushButton* PaletaDeHerramientas::crear_boton_modo(const QString& texto, ModoEditor modo) {
    QPushButton* boton = new QPushButton(texto, this);
    boton->setCheckable(true);
    boton->setStyleSheet(
        "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; border-radius: 4px; }"
        "QPushButton:checked { background-color: #2E6B38; color: white; font-weight: bold; }");
    grupo_modos->addButton(boton, static_cast<int>(modo));
    connect(boton, &QPushButton::clicked, this, [this, modo]() { emit modo_cambiado(modo); });
    return boton;
}

QPushButton* PaletaDeHerramientas::crear_boton_terreno(
    const QString& texto, TipoTerrenoPaleta terreno, const QString& color_hex) {
    QPushButton* boton = new QPushButton(texto, this);
    boton->setCheckable(true);
    configurar_estilo_boton_terreno(boton, texto, color_hex);
    grupo_terrenos->addButton(boton, static_cast<int>(terreno));
    connect(boton, &QPushButton::clicked, this, [this, terreno]() {
        emit terreno_cambiado(terreno);
    });
    return boton;
}

void PaletaDeHerramientas::configurar_estilo_boton_terreno(
    QPushButton* boton, const QString& texto, const QString& color_hex) {
    if (boton == nullptr) {
        return;
    }
    boton->setText(texto);
    QString estilo = QString(
                         "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; "
                         "border-left: 8px solid %1; border-radius: 4px; }"
                         "QPushButton:checked { background-color: #3A3A3A; font-weight: bold; "
                         "border-left: 12px solid %1; }")
                         .arg(color_hex);
    boton->setStyleSheet(estilo);
}

QPushButton* PaletaDeHerramientas::crear_boton_entidad(const QString& texto, TipoEntidad entidad) {
    QPushButton* boton = new QPushButton(texto, contenedor_entidades);
    boton->setCheckable(true);
    boton->setStyleSheet(
        "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; border-radius: 4px; }"
        "QPushButton:checked { background-color: #4A6E82; color: white; font-weight: bold; }");
    grupo_entidades->addButton(boton, static_cast<int>(entidad));
    connect(boton, &QPushButton::clicked, this, [this, entidad]() {
        emit entidad_cambiada(entidad);
    });
    return boton;
}

void PaletaDeHerramientas::limpiar_botones_de_entidades() {
    QList<QAbstractButton*> botones = grupo_entidades->buttons();
    int indice = 0;
    while (indice < botones.size()) {
        grupo_entidades->removeButton(botones[indice]);
        delete botones[indice];
        indice++;
    }
}

void PaletaDeHerramientas::actualizar_sector(SectorActual sector) {
    // 1. Actualizar estilos y textos temáticos de la paleta de terrenos
    if (botones_terrenos.size() == 5) {
        if (sector == SectorActual::GRANJA) {
            configurar_estilo_boton_terreno(botones_terrenos[0], "Pasto", "#5C8E32");
            configurar_estilo_boton_terreno(botones_terrenos[1], "Tierra Cultivable", "#8B5A2B");
            configurar_estilo_boton_terreno(botones_terrenos[2], "Camino", "#D3B88C");
            configurar_estilo_boton_terreno(botones_terrenos[3], "Agua", "#3A75C4");
            configurar_estilo_boton_terreno(botones_terrenos[4], "Muro / Roca", "#4A4A4A");
        } else if (sector == SectorActual::PUEBLO) {
            configurar_estilo_boton_terreno(botones_terrenos[0], "Pasto", "#4E7D2B");
            configurar_estilo_boton_terreno(botones_terrenos[1], "Piso", "#BA8C53");
            configurar_estilo_boton_terreno(botones_terrenos[2], "Adoquin", "#808790");
            configurar_estilo_boton_terreno(botones_terrenos[3], "Agua", "#2A60A0");
            configurar_estilo_boton_terreno(botones_terrenos[4], "Muro / Edificio", "#4D382C");
        } else if (sector == SectorActual::CAVERNA) {
            configurar_estilo_boton_terreno(botones_terrenos[0], "Piso de Cueva", "#4A4850");
            configurar_estilo_boton_terreno(botones_terrenos[1], "Tierra de Cueva", "#594638");
            configurar_estilo_boton_terreno(botones_terrenos[2], "Sendero de Cueva", "#6E6675");
            configurar_estilo_boton_terreno(botones_terrenos[3], "Agua de Cueva", "#1F4366");
            configurar_estilo_boton_terreno(botones_terrenos[4], "Muro", "#201F24");
        }
    }

    // 2. Actualizar botones de entidades
    limpiar_botones_de_entidades();

    QPushButton* primer_boton = nullptr;

    if (sector == SectorActual::GRANJA) {
        primer_boton = crear_boton_entidad("Cabania del Jugador (3x3)", TipoEntidad::CABANIA);
        layout_entidades->addWidget(primer_boton);
        QPushButton* boton_caverna =
            crear_boton_entidad("Entrada Caverna (2x2)", TipoEntidad::ENTRADA_CAVERNA);
        layout_entidades->addWidget(boton_caverna);
        QPushButton* boton_transicion_pueblo =
            crear_boton_entidad("Salida a Pueblo (2x2)", TipoEntidad::TRANSICION_PUEBLO);
        layout_entidades->addWidget(boton_transicion_pueblo);
    } else if (sector == SectorActual::PUEBLO) {
        primer_boton = crear_boton_entidad("Puesto Marnie (2x2)", TipoEntidad::PUESTO_MARNIE);
        layout_entidades->addWidget(primer_boton);
        QPushButton* boton_clint =
            crear_boton_entidad("Puesto Clint (2x2)", TipoEntidad::PUESTO_CLINT);
        layout_entidades->addWidget(boton_clint);
        QPushButton* boton_pierre =
            crear_boton_entidad("Puesto Pierre (2x2)", TipoEntidad::PUESTO_PIERRE);
        layout_entidades->addWidget(boton_pierre);
        QPushButton* boton_robin =
            crear_boton_entidad("Puesto Robin (2x2)", TipoEntidad::PUESTO_ROBIN);
        layout_entidades->addWidget(boton_robin);
        QPushButton* boton_centro =
            crear_boton_entidad("Centro Comunitario (4x3)", TipoEntidad::CENTRO_COMUNITARIO);
        layout_entidades->addWidget(boton_centro);
        QPushButton* boton_transicion_granja =
            crear_boton_entidad("Salida a Granja (2x2)", TipoEntidad::TRANSICION_GRANJA);
        layout_entidades->addWidget(boton_transicion_granja);
    } else if (sector == SectorActual::CAVERNA) {
        primer_boton = crear_boton_entidad("Escalera Mazmorra (1x1)", TipoEntidad::ESCALERA_NIVEL);
        layout_entidades->addWidget(primer_boton);
        QPushButton* boton_salida =
            crear_boton_entidad("Salida Caverna (1x1)", TipoEntidad::SALIDA_CAVERNA);
        layout_entidades->addWidget(boton_salida);
    }

    if (primer_boton != nullptr) {
        primer_boton->setChecked(true);
        if (sector == SectorActual::GRANJA) {
            emit entidad_cambiada(TipoEntidad::CABANIA);
        } else if (sector == SectorActual::PUEBLO) {
            emit entidad_cambiada(TipoEntidad::PUESTO_MARNIE);
        } else if (sector == SectorActual::CAVERNA) {
            emit entidad_cambiada(TipoEntidad::ESCALERA_NIVEL);
        }
    }
}

void PaletaDeHerramientas::habilitar_boton_eliminar(bool habilitado) {
    if (boton_eliminar_entidad != nullptr) {
        boton_eliminar_entidad->setEnabled(habilitado);
    }
}
