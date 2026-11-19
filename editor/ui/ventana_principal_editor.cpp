#include "ventana_principal_editor.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStatusBar>
#include <QString>
#include <QToolBar>
#include <string>

#include "capa_del_mapa.h"
#include "cargador_de_mundo_tmx.h"
#include "definicion_de_mundo.h"
#include "exportador_de_mundo_tmx.h"
#include "propiedad_de_casilla.h"
#include "tipo_herramienta_editor.h"
#include "validador_de_mundo.h"

static constexpr int DURACION_MENSAJE_ESTADO_MS = 2000;

VentanaPrincipalEditor::VentanaPrincipalEditor(QWidget* padre)
    : QMainWindow(padre),
      mundo_actual(DefinicionDeMundo::crear_mundo_por_defecto("Mundo Inicial")),
      ruta_archivo_actual(""),
      lienzo(nullptr),
      paleta(nullptr),
      panel_validacion(nullptr),
      area_desplazamiento(nullptr),
      combo_sectores(nullptr),
      etiqueta_coordenadas(nullptr),
      etiqueta_nombre_mundo(nullptr),
      etiqueta_dimensiones(nullptr) {
    setWindowTitle("Stardew Valley - Editor de Mapas y Mundos");
    resize(1280, 800);

    inicializar_interfaz();
    inicializar_menu();
    inicializar_barra_de_herramientas();
    inicializar_barra_de_estado();
    conectar_senales();

    actualizar_sector_en_pantalla();
    ejecutar_validacion();
}

void VentanaPrincipalEditor::inicializar_interfaz() {
    lienzo = new LienzoDeEdicion(this);

    area_desplazamiento = new QScrollArea(this);
    area_desplazamiento->setWidget(lienzo);
    area_desplazamiento->setWidgetResizable(false);
    area_desplazamiento->setStyleSheet("background-color: #222222;");
    setCentralWidget(area_desplazamiento);

    // Dock lateral izquierdo: Paleta de Herramientas
    QDockWidget* dock_paleta = new QDockWidget("Paleta de Herramientas", this);
    dock_paleta->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    paleta = new PaletaDeHerramientas(dock_paleta);
    dock_paleta->setWidget(paleta);
    addDockWidget(Qt::LeftDockWidgetArea, dock_paleta);

    // Dock lateral derecho: Panel de Validación
    QDockWidget* dock_validacion = new QDockWidget("Panel de Validacion", this);
    dock_validacion->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    panel_validacion = new PanelDeValidacion(dock_validacion);
    dock_validacion->setWidget(panel_validacion);
    addDockWidget(Qt::RightDockWidgetArea, dock_validacion);
}

void VentanaPrincipalEditor::inicializar_menu() {
    QMenuBar* barra_menu = menuBar();

    QMenu* menu_archivo = barra_menu->addMenu("&Archivo");
    QAction* accion_nuevo = menu_archivo->addAction("&Nuevo Mundo");
    accion_nuevo->setShortcut(QKeySequence::New);
    connect(accion_nuevo, &QAction::triggered, this, &VentanaPrincipalEditor::nuevo_mundo);

    QAction* accion_abrir = menu_archivo->addAction("&Abrir Mundo...");
    accion_abrir->setShortcut(QKeySequence::Open);
    connect(accion_abrir, &QAction::triggered, this, &VentanaPrincipalEditor::abrir_mundo);

    QAction* accion_guardar = menu_archivo->addAction("&Guardar Mundo");
    accion_guardar->setShortcut(QKeySequence::Save);
    connect(accion_guardar, &QAction::triggered, this, &VentanaPrincipalEditor::guardar_mundo);

    QAction* accion_guardar_como = menu_archivo->addAction("Guardar Mundo &Como...");
    accion_guardar_como->setShortcut(QKeySequence::SaveAs);
    connect(
        accion_guardar_como,
        &QAction::triggered,
        this,
        &VentanaPrincipalEditor::guardar_mundo_como);

    menu_archivo->addSeparator();
    QAction* accion_salir = menu_archivo->addAction("&Salir");
    accion_salir->setShortcut(QKeySequence::Quit);
    connect(accion_salir, &QAction::triggered, this, &QWidget::close);

    QMenu* menu_ver = barra_menu->addMenu("&Ver");
    QAction* accion_zoom_in = menu_ver->addAction("Acercar Zoom (+)");
    accion_zoom_in->setShortcut(QKeySequence::ZoomIn);
    connect(accion_zoom_in, &QAction::triggered, this, &VentanaPrincipalEditor::aumentar_zoom);

    QAction* accion_zoom_out = menu_ver->addAction("Alejar Zoom (-)");
    accion_zoom_out->setShortcut(QKeySequence::ZoomOut);
    connect(accion_zoom_out, &QAction::triggered, this, &VentanaPrincipalEditor::disminuir_zoom);

    QMenu* menu_herramientas = barra_menu->addMenu("&Herramientas");
    QAction* accion_redimensionar = menu_herramientas->addAction("&Redimensionar Sector...");
    accion_redimensionar->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(
        accion_redimensionar,
        &QAction::triggered,
        this,
        &VentanaPrincipalEditor::abrir_dialogo_redimensionar);

    QAction* accion_validar = menu_herramientas->addAction("&Validar Mundo");
    accion_validar->setShortcut(QKeySequence(Qt::Key_F5));
    connect(
        accion_validar,
        &QAction::triggered,
        this,
        &VentanaPrincipalEditor::ejecutar_validacion);
}

void VentanaPrincipalEditor::inicializar_barra_de_herramientas() {
    QToolBar* barra = addToolBar("Herramientas Principales");
    barra->setMovable(false);

    etiqueta_nombre_mundo = new QLabel(
        QString("Mundo: <b>%1</b>").arg(QString::fromStdString(mundo_actual.obtener_nombre())),
        barra);
    etiqueta_nombre_mundo->setStyleSheet("padding: 0 10px; font-size: 13px;");
    barra->addWidget(etiqueta_nombre_mundo);

    QPushButton* boton_renombrar = new QPushButton("Renombrar", barra);
    connect(boton_renombrar, &QPushButton::clicked, this, &VentanaPrincipalEditor::renombrar_mundo);
    barra->addWidget(boton_renombrar);

    barra->addSeparator();

    QLabel* etiqueta_sector = new QLabel(" Sector Activo: ", barra);
    barra->addWidget(etiqueta_sector);

    combo_sectores = new QComboBox(barra);
    combo_sectores->addItem("Granja");
    combo_sectores->addItem("Pueblo");
    int nivel = 1;
    while (nivel <= 10) {
        combo_sectores->addItem(QString("Caverna - Nivel %1").arg(nivel));
        nivel++;
    }
    connect(
        combo_sectores,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &VentanaPrincipalEditor::cambiar_sector);
    barra->addWidget(combo_sectores);

    barra->addSeparator();

    combo_capas = new QComboBox(barra);
    combo_capas->addItem("Capa: Suelo (Back)");
    combo_capas->addItem("Capa: Estructuras (Buildings)");
    combo_capas->addItem("Capa: Techos (Front)");
    connect(
        combo_capas,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &VentanaPrincipalEditor::cambiar_capa);
    barra->addWidget(combo_capas);

    barra->addSeparator();

    etiqueta_dimensiones = new QLabel(" Dimensiones: <b>35 x 35</b> ", barra);
    etiqueta_dimensiones->setStyleSheet("padding: 0 8px; font-size: 13px;");
    barra->addWidget(etiqueta_dimensiones);

    QPushButton* boton_redimensionar = new QPushButton("Cambiar Tamano...", barra);
    boton_redimensionar->setStyleSheet(
        "QPushButton { background-color: #3C5A78; color: white; font-weight: bold; padding: 4px "
        "10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #4C7298; }");
    connect(
        boton_redimensionar,
        &QPushButton::clicked,
        this,
        &VentanaPrincipalEditor::abrir_dialogo_redimensionar);
    barra->addWidget(boton_redimensionar);

    barra->addSeparator();

    QPushButton* boton_zoom_menos = new QPushButton(" Zoom - ", barra);
    connect(boton_zoom_menos, &QPushButton::clicked, this, &VentanaPrincipalEditor::disminuir_zoom);
    barra->addWidget(boton_zoom_menos);

    QPushButton* boton_zoom_mas = new QPushButton(" Zoom + ", barra);
    connect(boton_zoom_mas, &QPushButton::clicked, this, &VentanaPrincipalEditor::aumentar_zoom);
    barra->addWidget(boton_zoom_mas);

    barra->addSeparator();

    QPushButton* boton_guardar_rapido = new QPushButton("Guardar", barra);
    boton_guardar_rapido->setStyleSheet(
        "QPushButton { background-color: #2E6B38; color: white; font-weight: bold; padding: 4px "
        "10px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3B8C4A; }");
    connect(
        boton_guardar_rapido,
        &QPushButton::clicked,
        this,
        &VentanaPrincipalEditor::guardar_mundo);
    barra->addWidget(boton_guardar_rapido);
}

void VentanaPrincipalEditor::inicializar_barra_de_estado() {
    QStatusBar* barra_estado = statusBar();
    etiqueta_coordenadas = new QLabel("Columna: 0 | Fila: 0", barra_estado);
    barra_estado->addPermanentWidget(etiqueta_coordenadas);
    barra_estado->showMessage("Listo para editar el mundo.", 3000);
}

void VentanaPrincipalEditor::conectar_senales() {
    connect(
        lienzo,
        &LienzoDeEdicion::casilla_apuntada,
        this,
        &VentanaPrincipalEditor::actualizar_coordenadas);
    connect(
        lienzo,
        &LienzoDeEdicion::mapa_modificado,
        this,
        &VentanaPrincipalEditor::ejecutar_validacion);
    connect(lienzo, &LienzoDeEdicion::entidad_seleccionada_cambio, this, [this](int id) {
        paleta->habilitar_boton_eliminar(id >= 0);
    });

    connect(
        paleta,
        &PaletaDeHerramientas::modo_cambiado,
        lienzo,
        &LienzoDeEdicion::establecer_modo);
    connect(
        paleta,
        &PaletaDeHerramientas::tile_seleccionado,
        lienzo,
        &LienzoDeEdicion::establecer_tile);
    connect(
        paleta,
        &PaletaDeHerramientas::entidad_cambiada,
        lienzo,
        &LienzoDeEdicion::establecer_entidad_a_colocar);
    connect(
        paleta,
        &PaletaDeHerramientas::eliminar_solicitado,
        lienzo,
        &LienzoDeEdicion::eliminar_entidad_seleccionada);

    connect(
        panel_validacion,
        &PanelDeValidacion::validacion_solicitada,
        this,
        &VentanaPrincipalEditor::ejecutar_validacion);
}

void VentanaPrincipalEditor::actualizar_sector_en_pantalla() {
    int indice = combo_sectores->currentIndex();
    if (indice == 0) {
        lienzo->establecer_mapa(
            &mundo_actual.obtener_sector_granja_modificable(),
            SectorActual::GRANJA);
        paleta->actualizar_sector(SectorActual::GRANJA);
    } else if (indice == 1) {
        lienzo->establecer_mapa(
            &mundo_actual.obtener_sector_pueblo_modificable(),
            SectorActual::PUEBLO);
        paleta->actualizar_sector(SectorActual::PUEBLO);
    } else if (indice >= 2) {
        size_t indice_mazmorra = static_cast<size_t>(indice - 2);
        lienzo->establecer_mapa(
            &mundo_actual.obtener_nivel_mazmorra_modificable(indice_mazmorra),
            SectorActual::CAVERNA);
        paleta->actualizar_sector(SectorActual::CAVERNA);
    }
    actualizar_etiqueta_dimensiones();
}

void VentanaPrincipalEditor::nuevo_mundo() {
    bool aceptado = false;
    QString nombre = QInputDialog::getText(
        this,
        "Nuevo Mundo",
        "Ingrese el nombre del nuevo mundo:",
        QLineEdit::Normal,
        "Nuevo Mundo",
        &aceptado);
    if (aceptado && !nombre.trimmed().isEmpty()) {
        mundo_actual = DefinicionDeMundo::crear_mundo_por_defecto(nombre.toStdString());
        ruta_archivo_actual = "";
        etiqueta_nombre_mundo->setText(QString("Mundo: <b>%1</b>").arg(nombre));
        combo_sectores->setCurrentIndex(0);
        actualizar_sector_en_pantalla();
        ejecutar_validacion();
        statusBar()->showMessage("Mundo creado correctamente.", 3000);
    }
}

void VentanaPrincipalEditor::abrir_mundo() {
    std::filesystem::create_directories("mundos");
    QString directorio_mundos = QDir::current().absoluteFilePath("mundos");
    QString ruta_directorio = QFileDialog::getExistingDirectory(
        this,
        "Seleccionar Carpeta de Mundo TMX",
        directorio_mundos,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!ruta_directorio.isEmpty()) {
        try {
            mundo_actual = CargadorDeMundoTMX::cargar_mundo(ruta_directorio.toStdString());
            ruta_archivo_actual = ruta_directorio.toStdString();
            etiqueta_nombre_mundo->setText(
                QString("Mundo: <b>%1</b>")
                    .arg(QString::fromStdString(mundo_actual.obtener_nombre())));
            combo_sectores->setCurrentIndex(0);
            actualizar_sector_en_pantalla();
            ejecutar_validacion();
            statusBar()->showMessage("Mundo TMX cargado exitosamente.", 3000);
        } catch (const std::runtime_error& error) {
            QMessageBox::critical(
                this,
                "Error al Cargar Mundo",
                QString::fromStdString(error.what()));
        }
    }
}

void VentanaPrincipalEditor::guardar_mundo() {
    if (ruta_archivo_actual.empty()) {
        guardar_mundo_como();
        return;
    }
    try {
        ExportadorDeMundoTMX::exportar_mundo_completo(ruta_archivo_actual, mundo_actual);
        statusBar()->showMessage(
            QString("Mundo guardado en %1").arg(QString::fromStdString(ruta_archivo_actual)),
            4000);
    } catch (const std::runtime_error& error) {
        QMessageBox::critical(this, "Error al Guardar Mundo", QString::fromStdString(error.what()));
    }
}

void VentanaPrincipalEditor::guardar_mundo_como() {
    std::filesystem::create_directories("mundos");
    bool aceptado = false;
    QString nombre_sugerido = QString::fromStdString(mundo_actual.obtener_nombre());
    QString nombre_carpeta = QInputDialog::getText(
        this,
        "Guardar Mundo TMX",
        "Nombre de la carpeta del mundo (dentro de mundos/):",
        QLineEdit::Normal,
        nombre_sugerido,
        &aceptado);

    if (aceptado && !nombre_carpeta.trimmed().isEmpty()) {
        std::filesystem::path ruta_destino =
            std::filesystem::path("mundos") / nombre_carpeta.trimmed().toStdString();
        ruta_archivo_actual = ruta_destino.string();
        guardar_mundo();
    }
}

void VentanaPrincipalEditor::renombrar_mundo() {
    bool aceptado = false;
    QString nombre = QInputDialog::getText(
        this,
        "Renombrar Mundo",
        "Nuevo nombre para el mundo:",
        QLineEdit::Normal,
        QString::fromStdString(mundo_actual.obtener_nombre()),
        &aceptado);
    if (aceptado && !nombre.trimmed().isEmpty()) {
        mundo_actual.establecer_nombre(nombre.toStdString());
        etiqueta_nombre_mundo->setText(QString("Mundo: <b>%1</b>").arg(nombre));
        ejecutar_validacion();
    }
}

void VentanaPrincipalEditor::cambiar_sector(int indice) {
    (void)indice;
    actualizar_sector_en_pantalla();
}

void VentanaPrincipalEditor::cambiar_capa(int indice) {
    lienzo->establecer_capa_activa(static_cast<TipoCapa>(indice));
    statusBar()->showMessage(
        QString("Editando %1").arg(combo_capas->currentText()),
        DURACION_MENSAJE_ESTADO_MS);
}

void VentanaPrincipalEditor::aumentar_zoom() {
    int tamano = lienzo->obtener_tamano_casilla();
    lienzo->establecer_tamano_casilla(tamano + 4);
}

void VentanaPrincipalEditor::disminuir_zoom() {
    int tamano = lienzo->obtener_tamano_casilla();
    lienzo->establecer_tamano_casilla(tamano - 4);
}

void VentanaPrincipalEditor::actualizar_coordenadas(int columna, int fila) {
    etiqueta_coordenadas->setText(QString("Columna: %1 | Fila: %2").arg(columna).arg(fila));
}

void VentanaPrincipalEditor::ejecutar_validacion() {
    ResultadoValidacion resultado = ValidadorDeMundo::validar(mundo_actual);
    panel_validacion->actualizar_resultado(resultado);
}

MapaEditable* VentanaPrincipalEditor::obtener_mapa_activo() {
    int indice = combo_sectores->currentIndex();
    if (indice == 0) {
        return &mundo_actual.obtener_sector_granja_modificable();
    }
    if (indice == 1) {
        return &mundo_actual.obtener_sector_pueblo_modificable();
    }
    if (indice >= 2) {
        size_t indice_mazmorra = static_cast<size_t>(indice - 2);
        return &mundo_actual.obtener_nivel_mazmorra_modificable(indice_mazmorra);
    }
    return nullptr;
}

void VentanaPrincipalEditor::actualizar_etiqueta_dimensiones() {
    MapaEditable* mapa = obtener_mapa_activo();
    if (mapa != nullptr && etiqueta_dimensiones != nullptr) {
        etiqueta_dimensiones->setText(QString(" Dimensiones: <b>%1 x %2</b> ")
                                          .arg(mapa->obtener_cantidad_de_columnas())
                                          .arg(mapa->obtener_cantidad_de_filas()));
    }
}

void VentanaPrincipalEditor::abrir_dialogo_redimensionar() {
    MapaEditable* mapa_actual_sector = obtener_mapa_activo();
    if (mapa_actual_sector == nullptr) {
        return;
    }

    int columnas_actuales = mapa_actual_sector->obtener_cantidad_de_columnas();
    int filas_actuales = mapa_actual_sector->obtener_cantidad_de_filas();

    QDialog dialogo(this);
    dialogo.setWindowTitle("Redimensionar Mapa del Sector");
    dialogo.setMinimumWidth(320);

    QVBoxLayout* layout_dialogo = new QVBoxLayout(&dialogo);

    QString nombre_sector = combo_sectores->currentText();
    QLabel* etiqueta_info =
        new QLabel(QString("<b>Sector actual:</b> %1").arg(nombre_sector), &dialogo);
    layout_dialogo->addWidget(etiqueta_info);

    QFormLayout* layout_formulario = new QFormLayout();

    QSpinBox* spin_columnas = new QSpinBox(&dialogo);
    spin_columnas->setRange(10, 80);
    spin_columnas->setValue(columnas_actuales);
    spin_columnas->setSuffix(" casillas");
    layout_formulario->addRow("Ancho (Columnas):", spin_columnas);

    QSpinBox* spin_filas = new QSpinBox(&dialogo);
    spin_filas->setRange(10, 80);
    spin_filas->setValue(filas_actuales);
    spin_filas->setSuffix(" casillas");
    layout_formulario->addRow("Alto (Filas):", spin_filas);

    layout_dialogo->addLayout(layout_formulario);

    QLabel* etiqueta_aviso = new QLabel(
        "<small style='color: #AAAAAA;'>Nota: Al reducir el mapa, las casillas y entidades que "
        "queden fuera se recortaran.</small>",
        &dialogo);
    layout_dialogo->addWidget(etiqueta_aviso);

    QDialogButtonBox* botones_dialogo =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialogo);
    connect(botones_dialogo, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
    connect(botones_dialogo, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
    layout_dialogo->addWidget(botones_dialogo);

    if (dialogo.exec() == QDialog::Accepted) {
        int nuevas_columnas = spin_columnas->value();
        int nuevas_filas = spin_filas->value();

        if (nuevas_columnas != columnas_actuales || nuevas_filas != filas_actuales) {
            uint32_t identificador_tile_relleno = 0;
            uint8_t propiedades_relleno = PropiedadDeCasilla::DECORABLE;
            int indice_sector = combo_sectores->currentIndex();
            if (indice_sector == 0) {
                propiedades_relleno = PropiedadDeCasilla::CULTIVABLE |
                                      PropiedadDeCasilla::DECORABLE |
                                      PropiedadDeCasilla::ADMITE_HIERBA;
            } else if (indice_sector >= 2) {
                propiedades_relleno =
                    PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL;
            }

            mapa_actual_sector->redimensionar(
                nuevas_columnas,
                nuevas_filas,
                identificador_tile_relleno,
                propiedades_relleno);
            lienzo->actualizar_tamano_del_lienzo();
            lienzo->update();
            actualizar_etiqueta_dimensiones();
            ejecutar_validacion();
            statusBar()->showMessage(
                QString("Dimensiones actualizadas a %1 x %2 casillas.")
                    .arg(nuevas_columnas)
                    .arg(nuevas_filas),
                4000);
        }
    }
}
