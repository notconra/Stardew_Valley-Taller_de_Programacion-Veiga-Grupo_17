#include "ventana_principal_editor.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
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
static constexpr int MARGEN_ANCHO_PANEL_PX = 16;
static constexpr size_t MAXIMO_HISTORIAL_DESHACER = 20;
static constexpr int TAMANO_CASILLA_BASE_ZOOM_PX = 28;

VentanaPrincipalEditor::VentanaPrincipalEditor(QWidget* padre)
    : QMainWindow(padre),
      mundo_actual(DefinicionDeMundo::crear_mundo_por_defecto("Mundo Inicial")),
      estado_actual_respaldo(mundo_actual),
      historial_deshacer(),
      historial_rehacer(),
      ruta_archivo_actual(""),
      hay_cambios_sin_guardar(false),
      accion_deshacer(nullptr),
      accion_rehacer(nullptr),
      lienzo(nullptr),
      paleta(nullptr),
      panel_validacion(nullptr),
      dock_paleta(nullptr),
      dock_validacion(nullptr),
      area_desplazamiento(nullptr),
      combo_sectores(nullptr),
      etiqueta_coordenadas(nullptr),
      etiqueta_nombre_mundo(nullptr),
      etiqueta_dimensiones(nullptr),
      etiqueta_zoom(nullptr) {
    setWindowTitle("Stardew Valley - Editor de Mapas y Mundos");
    setMinimumSize(960, 600);

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
    area_desplazamiento->setObjectName("area_desplazamiento");
    area_desplazamiento->setWidget(lienzo);
    area_desplazamiento->setWidgetResizable(false);
    area_desplazamiento->setAlignment(Qt::AlignCenter);
    setCentralWidget(area_desplazamiento);

    // Dock lateral izquierdo: Paleta de Herramientas
    dock_paleta = new QDockWidget("Paleta de Herramientas", this);
    dock_paleta->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    paleta = new PaletaDeHerramientas(dock_paleta);
    dock_paleta->setWidget(paleta);
    addDockWidget(Qt::LeftDockWidgetArea, dock_paleta);

    // Dock lateral derecho: Panel de Validación
    dock_validacion = new QDockWidget("Panel de Validación", this);
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

    QMenu* menu_editar = barra_menu->addMenu("&Editar");
    accion_deshacer = menu_editar->addAction("&Deshacer");
    accion_deshacer->setShortcut(QKeySequence::Undo);
    accion_deshacer->setEnabled(false);
    connect(accion_deshacer, &QAction::triggered, this, &VentanaPrincipalEditor::deshacer);

    accion_rehacer = menu_editar->addAction("&Rehacer");
    accion_rehacer->setShortcut(QKeySequence::Redo);
    accion_rehacer->setEnabled(false);
    connect(accion_rehacer, &QAction::triggered, this, &VentanaPrincipalEditor::rehacer);

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
}

void VentanaPrincipalEditor::inicializar_barra_de_herramientas() {
    QToolBar* barra = addToolBar("Herramientas Principales");
    barra->setMovable(false);

    etiqueta_nombre_mundo = new QLabel(barra);
    etiqueta_nombre_mundo->setObjectName("etiqueta_nombre_mundo");
    actualizar_etiqueta_nombre_mundo();
    barra->addWidget(etiqueta_nombre_mundo);

    barra->addSeparator();

    QLabel* etiqueta_sector = new QLabel(" Sector Activo: ", barra);
    barra->addWidget(etiqueta_sector);

    combo_sectores = new QComboBox(barra);
    combo_sectores->addItem("Granja");
    combo_sectores->addItem("Pueblo");
    combo_sectores->addItem("Mercado");
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

    etiqueta_dimensiones = new QLabel(" Dimensiones: <b>35 x 35</b> ", barra);
    etiqueta_dimensiones->setObjectName("etiqueta_dimensiones");
    barra->addWidget(etiqueta_dimensiones);

    QPushButton* boton_redimensionar = new QPushButton("Cambiar Tamaño...", barra);
    boton_redimensionar->setObjectName("boton_redimensionar");
    connect(
        boton_redimensionar,
        &QPushButton::clicked,
        this,
        &VentanaPrincipalEditor::abrir_dialogo_redimensionar);
    barra->addWidget(boton_redimensionar);

    barra->addSeparator();

    QPushButton* boton_zoom_menos = new QPushButton(" - ", barra);
    connect(boton_zoom_menos, &QPushButton::clicked, this, &VentanaPrincipalEditor::disminuir_zoom);
    barra->addWidget(boton_zoom_menos);

    etiqueta_zoom = new QLabel("100%", barra);
    etiqueta_zoom->setAlignment(Qt::AlignCenter);
    etiqueta_zoom->setMinimumWidth(44);
    barra->addWidget(etiqueta_zoom);

    QPushButton* boton_zoom_mas = new QPushButton(" + ", barra);
    connect(boton_zoom_mas, &QPushButton::clicked, this, &VentanaPrincipalEditor::aumentar_zoom);
    barra->addWidget(boton_zoom_mas);

    barra->addSeparator();

    QPushButton* boton_guardar_rapido = new QPushButton("Guardar", barra);
    boton_guardar_rapido->setObjectName("boton_guardar_rapido");
    connect(
        boton_guardar_rapido,
        &QPushButton::clicked,
        this,
        &VentanaPrincipalEditor::guardar_mundo);
    barra->addWidget(boton_guardar_rapido);

    QPushButton* boton_guardar_como = new QPushButton("Guardar como...", barra);
    connect(
        boton_guardar_como,
        &QPushButton::clicked,
        this,
        &VentanaPrincipalEditor::guardar_mundo_como);
    barra->addWidget(boton_guardar_como);
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
    connect(lienzo, &LienzoDeEdicion::mapa_modificado, this, [this]() {
        marcar_cambios_sin_guardar();
        if (!lienzo->esta_pintando_con_mouse()) {
            registrar_estado_en_historial();
        }
        ejecutar_validacion();
    });
    connect(lienzo, &LienzoDeEdicion::trazo_finalizado, this, [this]() {
        registrar_estado_en_historial();
    });
    connect(
        lienzo,
        &LienzoDeEdicion::mapa_expandido,
        this,
        [this](int columnas_izquierda, int filas_arriba) {
            int paso = lienzo->obtener_tamano_casilla();
            if (columnas_izquierda > 0 && area_desplazamiento->horizontalScrollBar() != nullptr) {
                QScrollBar* barra_horizontal = area_desplazamiento->horizontalScrollBar();
                barra_horizontal->setValue(barra_horizontal->value() + columnas_izquierda * paso);
            }
            if (filas_arriba > 0 && area_desplazamiento->verticalScrollBar() != nullptr) {
                QScrollBar* barra_vertical = area_desplazamiento->verticalScrollBar();
                barra_vertical->setValue(barra_vertical->value() + filas_arriba * paso);
            }
            actualizar_etiqueta_dimensiones();
        });
    connect(lienzo, &LienzoDeEdicion::entidad_seleccionada_cambio, this, [this](int id) {
        panel_validacion->habilitar_boton_eliminar(id >= 0);
    });

    connect(paleta, &PaletaDeHerramientas::modo_cambiado, this, [this](ModoEditor modo) {
        lienzo->establecer_modo(modo);
        panel_validacion->desmarcar_entidades();
    });
    connect(
        paleta,
        &PaletaDeHerramientas::tile_seleccionado,
        lienzo,
        &LienzoDeEdicion::establecer_tile);
    connect(
        paleta,
        &PaletaDeHerramientas::estampa_seleccionada,
        lienzo,
        &LienzoDeEdicion::establecer_estampa_a_colocar);
    connect(
        paleta,
        &PaletaDeHerramientas::generar_perimetro_solicitado,
        this,
        &VentanaPrincipalEditor::generar_muro_perimetral_granja);
    connect(panel_validacion, &PanelDeValidacion::entidad_cambiada, this, [this](TipoEntidad tipo) {
        lienzo->establecer_entidad_a_colocar(tipo);
        lienzo->establecer_modo(ModoEditor::COLOCAR_ENTIDAD);
        paleta->desmarcar_todo();
    });
    connect(
        panel_validacion,
        &PanelDeValidacion::eliminar_solicitado,
        lienzo,
        &LienzoDeEdicion::eliminar_entidad_seleccionada);
    connect(
        panel_validacion,
        &PanelDeValidacion::sector_solicitado,
        combo_sectores,
        &QComboBox::setCurrentIndex);

    QShortcut* atajo_rotar = new QShortcut(QKeySequence(Qt::Key_R), this);
    connect(atajo_rotar, &QShortcut::activated, lienzo, &LienzoDeEdicion::rotar_elemento_actual);

    connect(
        lienzo,
        &LienzoDeEdicion::estampa_rotada,
        paleta,
        &PaletaDeHerramientas::sincronizar_rotacion_estampa);
    connect(
        lienzo,
        &LienzoDeEdicion::tile_rotado,
        paleta,
        &PaletaDeHerramientas::sincronizar_rotacion_tile);
}

void VentanaPrincipalEditor::actualizar_sector_en_pantalla() {
    int indice = combo_sectores->currentIndex();
    if (indice == 0) {
        lienzo->establecer_mapa(
            &mundo_actual.obtener_sector_granja_modificable(),
            SectorActual::GRANJA);
        paleta->actualizar_sector(SectorActual::GRANJA);
        panel_validacion->actualizar_sector(SectorActual::GRANJA);
    } else if (indice == 1) {
        lienzo->establecer_mapa(
            &mundo_actual.obtener_sector_pueblo_modificable(),
            SectorActual::PUEBLO);
        paleta->actualizar_sector(SectorActual::PUEBLO);
        panel_validacion->actualizar_sector(SectorActual::PUEBLO);
    } else if (indice == 2) {
        lienzo->establecer_mapa(
            &mundo_actual.obtener_sector_mercado_modificable(),
            SectorActual::MERCADO);
        paleta->actualizar_sector(SectorActual::MERCADO);
        panel_validacion->actualizar_sector(SectorActual::MERCADO);
    } else if (indice >= 3) {
        size_t indice_mazmorra = static_cast<size_t>(indice - 3);
        lienzo->establecer_mapa(
            &mundo_actual.obtener_nivel_mazmorra_modificable(indice_mazmorra),
            SectorActual::CAVERNA);
        paleta->actualizar_sector(SectorActual::CAVERNA);
        panel_validacion->actualizar_sector(SectorActual::CAVERNA);
    }
    actualizar_etiqueta_dimensiones();
    ajustar_ancho_paneles();
}

void VentanaPrincipalEditor::ajustar_ancho_paneles() {
    if (paleta == nullptr || panel_validacion == nullptr || dock_paleta == nullptr ||
        dock_validacion == nullptr) {
        return;
    }
    if (paleta->layout() != nullptr) {
        paleta->layout()->activate();
    }
    if (panel_validacion->layout() != nullptr) {
        panel_validacion->layout()->activate();
    }
    int ancho_paleta = paleta->sizeHint().width() + MARGEN_ANCHO_PANEL_PX;
    int ancho_validacion = panel_validacion->sizeHint().width() + MARGEN_ANCHO_PANEL_PX;
    resizeDocks({dock_paleta, dock_validacion}, {ancho_paleta, ancho_validacion}, Qt::Horizontal);
}

void VentanaPrincipalEditor::nuevo_mundo() {
    if (!confirmar_descartar_cambios_si_es_necesario()) {
        return;
    }
    mundo_actual = DefinicionDeMundo::crear_mundo_por_defecto("Nuevo Mundo");
    ruta_archivo_actual = "";
    hay_cambios_sin_guardar = false;
    limpiar_historial();
    actualizar_etiqueta_nombre_mundo();
    combo_sectores->setCurrentIndex(0);
    actualizar_sector_en_pantalla();
    ejecutar_validacion();
    statusBar()->showMessage("Nuevo mundo creado.", 3000);
}

void VentanaPrincipalEditor::abrir_mundo() {
    if (!confirmar_descartar_cambios_si_es_necesario()) {
        return;
    }
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
            hay_cambios_sin_guardar = false;
            limpiar_historial();
            actualizar_etiqueta_nombre_mundo();
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
    if (!validar_antes_de_guardar()) {
        return;
    }
    if (ruta_archivo_actual.empty()) {
        guardar_mundo_como();
        return;
    }
    try {
        ExportadorDeMundoTMX::exportar_mundo_completo(ruta_archivo_actual, mundo_actual);
        hay_cambios_sin_guardar = false;
        actualizar_etiqueta_nombre_mundo();
        statusBar()->showMessage(
            QString("Mundo guardado en %1").arg(QString::fromStdString(ruta_archivo_actual)),
            4000);
    } catch (const std::runtime_error& error) {
        QMessageBox::critical(this, "Error al Guardar Mundo", QString::fromStdString(error.what()));
    }
}

void VentanaPrincipalEditor::guardar_mundo_como() {
    if (!validar_antes_de_guardar()) {
        return;
    }
    std::filesystem::create_directories("mundos");
    QString nombre_sugerido = QString::fromStdString(mundo_actual.obtener_nombre());
    QInputDialog dialogo(this);
    dialogo.setWindowTitle("Guardar Mundo TMX");
    dialogo.setLabelText("Nombre de la carpeta del mundo (dentro de mundos/):");
    dialogo.setTextValue(nombre_sugerido);
    dialogo.setOkButtonText("Aceptar");
    dialogo.setCancelButtonText("Cancelar");

    if (dialogo.exec() == QDialog::Accepted && !dialogo.textValue().trimmed().isEmpty()) {
        std::string nombre_carpeta_limpio = dialogo.textValue().trimmed().toStdString();
        std::filesystem::path ruta_destino =
            std::filesystem::path("mundos") / nombre_carpeta_limpio;
        ruta_archivo_actual = ruta_destino.string();
        mundo_actual.establecer_nombre(nombre_carpeta_limpio);
        actualizar_etiqueta_nombre_mundo();
        guardar_mundo();
    }
}

void VentanaPrincipalEditor::actualizar_etiqueta_nombre_mundo() {
    QString nombre = QString::fromStdString(mundo_actual.obtener_nombre());
    if (ruta_archivo_actual.empty()) {
        etiqueta_nombre_mundo->setText(QString("Mundo: <b>%1 (sin guardar)</b>").arg(nombre));
        return;
    }
    etiqueta_nombre_mundo->setText(QString("Mundo: <b>%1</b>").arg(nombre));
}

void VentanaPrincipalEditor::marcar_cambios_sin_guardar() {
    hay_cambios_sin_guardar = true;
}

void VentanaPrincipalEditor::registrar_estado_en_historial() {
    if (historial_deshacer.size() >= MAXIMO_HISTORIAL_DESHACER) {
        historial_deshacer.erase(historial_deshacer.begin());
    }
    historial_deshacer.push_back(estado_actual_respaldo);
    historial_rehacer.clear();
    estado_actual_respaldo = mundo_actual;
    actualizar_acciones_historial();
}

void VentanaPrincipalEditor::limpiar_historial() {
    historial_deshacer.clear();
    historial_rehacer.clear();
    estado_actual_respaldo = mundo_actual;
    actualizar_acciones_historial();
}

void VentanaPrincipalEditor::actualizar_acciones_historial() {
    if (accion_deshacer != nullptr) {
        accion_deshacer->setEnabled(!historial_deshacer.empty());
    }
    if (accion_rehacer != nullptr) {
        accion_rehacer->setEnabled(!historial_rehacer.empty());
    }
}

void VentanaPrincipalEditor::deshacer() {
    if (historial_deshacer.empty()) {
        return;
    }
    historial_rehacer.push_back(mundo_actual);
    mundo_actual = historial_deshacer.back();
    historial_deshacer.pop_back();
    estado_actual_respaldo = mundo_actual;
    marcar_cambios_sin_guardar();
    actualizar_sector_en_pantalla();
    ejecutar_validacion();
    actualizar_acciones_historial();
    statusBar()->showMessage("Acción deshecha.", DURACION_MENSAJE_ESTADO_MS);
}

void VentanaPrincipalEditor::rehacer() {
    if (historial_rehacer.empty()) {
        return;
    }
    historial_deshacer.push_back(mundo_actual);
    mundo_actual = historial_rehacer.back();
    historial_rehacer.pop_back();
    estado_actual_respaldo = mundo_actual;
    marcar_cambios_sin_guardar();
    actualizar_sector_en_pantalla();
    ejecutar_validacion();
    actualizar_acciones_historial();
    statusBar()->showMessage("Acción rehecha.", DURACION_MENSAJE_ESTADO_MS);
}

bool VentanaPrincipalEditor::confirmar_descartar_cambios_si_es_necesario() {
    if (!hay_cambios_sin_guardar) {
        return true;
    }
    QMessageBox dialogo(this);
    dialogo.setIcon(QMessageBox::Question);
    dialogo.setWindowTitle("Cambios sin guardar");
    dialogo.setText(
        "Hay cambios sin guardar en el mundo actual. ¿Querés guardarlos antes de continuar?");
    QPushButton* boton_guardar = dialogo.addButton("Guardar", QMessageBox::AcceptRole);
    QPushButton* boton_descartar = dialogo.addButton("Descartar", QMessageBox::DestructiveRole);
    QPushButton* boton_cancelar = dialogo.addButton("Cancelar", QMessageBox::RejectRole);
    dialogo.setDefaultButton(boton_guardar);
    dialogo.exec();

    QAbstractButton* boton_presionado = dialogo.clickedButton();
    if (boton_presionado == nullptr || boton_presionado == boton_cancelar) {
        return false;
    }
    if (boton_presionado == boton_descartar) {
        return true;
    }
    guardar_mundo();
    return !hay_cambios_sin_guardar;
}

void VentanaPrincipalEditor::closeEvent(QCloseEvent* evento) {
    if (!confirmar_descartar_cambios_si_es_necesario()) {
        evento->ignore();
        return;
    }
    evento->accept();
}

bool VentanaPrincipalEditor::validar_antes_de_guardar() {
    ResultadoValidacion resultado = ValidadorDeMundo::validar(mundo_actual);
    panel_validacion->actualizar_resultado(resultado);
    if (resultado.es_valido) {
        return true;
    }
    QString detalle_errores = "No se puede guardar un mundo inválido.\n\nErrores encontrados:";
    for (const ErrorValidacion& error : resultado.errores) {
        detalle_errores += QString("\n- %1").arg(QString::fromStdString(error.mensaje));
    }
    QMessageBox::warning(this, "Mundo Inválido", detalle_errores);
    return false;
}

void VentanaPrincipalEditor::cambiar_sector(int indice) {
    (void)indice;
    actualizar_sector_en_pantalla();
}

void VentanaPrincipalEditor::aumentar_zoom() {
    int tamano = lienzo->obtener_tamano_casilla();
    lienzo->establecer_tamano_casilla(tamano + 4);
    actualizar_etiqueta_zoom();
}

void VentanaPrincipalEditor::disminuir_zoom() {
    int tamano = lienzo->obtener_tamano_casilla();
    lienzo->establecer_tamano_casilla(tamano - 4);
    actualizar_etiqueta_zoom();
}

void VentanaPrincipalEditor::actualizar_etiqueta_zoom() {
    if (etiqueta_zoom != nullptr && lienzo != nullptr) {
        int porcentaje = (lienzo->obtener_tamano_casilla() * 100) / TAMANO_CASILLA_BASE_ZOOM_PX;
        etiqueta_zoom->setText(QString("%1%").arg(porcentaje));
    }
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
    if (indice == 2) {
        return &mundo_actual.obtener_sector_mercado_modificable();
    }
    if (indice >= 3) {
        size_t indice_mazmorra = static_cast<size_t>(indice - 3);
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
    int indice_sector = combo_sectores->currentIndex();
    bool es_sector_granja = (indice_sector == 0);
    bool es_sector_mercado = (indice_sector == 2);

    QDialog dialogo(this);
    dialogo.setWindowTitle("Redimensionar Terreno del Sector");
    dialogo.setMinimumWidth(380);

    QVBoxLayout* layout_dialogo = new QVBoxLayout(&dialogo);

    QString nombre_sector = combo_sectores->currentText();
    QLabel* etiqueta_info =
        new QLabel(QString("<b>Sector actual:</b> %1").arg(nombre_sector), &dialogo);
    layout_dialogo->addWidget(etiqueta_info);

    QFormLayout* layout_formulario = new QFormLayout();

    int ancho_util_actual = columnas_actuales;
    int alto_util_actual = filas_actuales;
    int minimo_columnas = 10;
    int minimo_filas = 10;

    if (es_sector_granja) {
        ancho_util_actual = columnas_actuales - (MapaEditable::ANCHO_VALLA_OESTE_CASILLAS +
                                                 MapaEditable::ANCHO_VALLA_ESTE_CASILLAS);
        alto_util_actual = filas_actuales - (MapaEditable::ALTO_VALLA_NORTE_CASILLAS +
                                             MapaEditable::ALTO_VALLA_SUR_CASILLAS);
        if (ancho_util_actual < MapaEditable::MINIMO_ANCHO_UTIL_GRANJA) {
            ancho_util_actual = MapaEditable::MINIMO_ANCHO_UTIL_GRANJA;
        }
        if (alto_util_actual < MapaEditable::MINIMO_ALTO_UTIL_GRANJA) {
            alto_util_actual = MapaEditable::MINIMO_ALTO_UTIL_GRANJA;
        }

        minimo_columnas = MapaEditable::MINIMO_ANCHO_UTIL_GRANJA;
        minimo_filas = MapaEditable::MINIMO_ALTO_UTIL_GRANJA;

        int columna_maxima_ocupada = mapa_actual_sector->obtener_columna_maxima_ocupada();
        int fila_maxima_ocupada = mapa_actual_sector->obtener_fila_maxima_ocupada();
        if (columna_maxima_ocupada > minimo_columnas) {
            minimo_columnas = columna_maxima_ocupada;
        }
        if (fila_maxima_ocupada > minimo_filas) {
            minimo_filas = fila_maxima_ocupada;
        }
    } else if (es_sector_mercado) {
        minimo_columnas = MapaEditable::MINIMO_COLUMNAS_MERCADO;
        minimo_filas = MapaEditable::MINIMO_FILAS_MERCADO;

        int columna_maxima_ocupada = mapa_actual_sector->obtener_columna_maxima_ocupada();
        int fila_maxima_ocupada = mapa_actual_sector->obtener_fila_maxima_ocupada();
        if (columna_maxima_ocupada > minimo_columnas) {
            minimo_columnas = columna_maxima_ocupada;
        }
        if (fila_maxima_ocupada > minimo_filas) {
            minimo_filas = fila_maxima_ocupada;
        }
    }

    QSpinBox* spin_columnas = new QSpinBox(&dialogo);
    spin_columnas->setRange(minimo_columnas, es_sector_granja ? 78 : 80);
    spin_columnas->setValue(ancho_util_actual);

    QSpinBox* spin_filas = new QSpinBox(&dialogo);
    spin_filas->setRange(minimo_filas, es_sector_granja ? 77 : 80);
    spin_filas->setValue(alto_util_actual);

    QHBoxLayout* layout_tamano = new QHBoxLayout();
    layout_tamano->addWidget(spin_columnas);
    layout_tamano->addWidget(new QLabel("x", &dialogo));
    layout_tamano->addWidget(spin_filas);
    layout_formulario->addRow(es_sector_granja ? "Tamaño útil:" : "Tamaño:", layout_tamano);

    layout_dialogo->addLayout(layout_formulario);

    if (es_sector_granja) {
        QLabel* etiqueta_total = new QLabel(&dialogo);
        auto actualizar_etiqueta_total = [spin_columnas, spin_filas, etiqueta_total]() {
            int ancho_util = spin_columnas->value();
            int alto_util = spin_filas->value();
            int total_columnas = ancho_util + (MapaEditable::ANCHO_VALLA_OESTE_CASILLAS +
                                               MapaEditable::ANCHO_VALLA_ESTE_CASILLAS);
            int total_filas = alto_util + (MapaEditable::ALTO_VALLA_NORTE_CASILLAS +
                                           MapaEditable::ALTO_VALLA_SUR_CASILLAS);
            etiqueta_total->setText(
                QString(
                    "<div style='background-color: #2D3139; padding: 8px; border-radius: 4px;'>"
                    "<b>Tamaño total con vallas perimetrales:</b> %1 x %2<br>"
                    "<small style='color: #88C0D0;'>(+2 columnas para vallas laterales este/oeste, "
                    "+2 filas para valla norte, +1 fila para base de valla sur)</small></div>")
                    .arg(total_columnas)
                    .arg(total_filas));
        };
        connect(
            spin_columnas,
            QOverload<int>::of(&QSpinBox::valueChanged),
            &dialogo,
            actualizar_etiqueta_total);
        connect(
            spin_filas,
            QOverload<int>::of(&QSpinBox::valueChanged),
            &dialogo,
            actualizar_etiqueta_total);
        actualizar_etiqueta_total();
        layout_dialogo->addWidget(etiqueta_total);
    }

    QLabel* etiqueta_aviso = new QLabel(
        "<small style='color: #AAAAAA;'>Nota: El límite inferior garantiza que todas las "
        "estructuras obligatorias y entidades ubicadas quepan sin solaparse ni recortarse.</small>",
        &dialogo);
    etiqueta_aviso->setWordWrap(true);
    layout_dialogo->addWidget(etiqueta_aviso);

    QHBoxLayout* layout_botones = new QHBoxLayout();
    layout_botones->addStretch();
    QPushButton* boton_cancelar = new QPushButton("Cancelar", &dialogo);
    QPushButton* boton_aceptar = new QPushButton("Aceptar", &dialogo);
    boton_aceptar->setDefault(true);
    connect(boton_cancelar, &QPushButton::clicked, &dialogo, &QDialog::reject);
    connect(boton_aceptar, &QPushButton::clicked, &dialogo, &QDialog::accept);
    layout_botones->addWidget(boton_cancelar);
    layout_botones->addWidget(boton_aceptar);
    layout_dialogo->addLayout(layout_botones);

    if (dialogo.exec() == QDialog::Accepted) {
        int nuevas_columnas = spin_columnas->value();
        int nuevas_filas = spin_filas->value();

        if (es_sector_granja) {
            nuevas_columnas +=
                (MapaEditable::ANCHO_VALLA_OESTE_CASILLAS +
                 MapaEditable::ANCHO_VALLA_ESTE_CASILLAS);
            nuevas_filas +=
                (MapaEditable::ALTO_VALLA_NORTE_CASILLAS + MapaEditable::ALTO_VALLA_SUR_CASILLAS);
        }

        if (nuevas_columnas != columnas_actuales || nuevas_filas != filas_actuales) {
            uint32_t identificador_tile_relleno = 0;
            uint8_t propiedades_relleno = PropiedadDeCasilla::DECORABLE;
            if (es_sector_granja) {
                identificador_tile_relleno =
                    CatalogoDeTerrenos::obtener_tile_base(SectorActual::GRANJA);
                propiedades_relleno = PropiedadDeCasilla::CULTIVABLE |
                                      PropiedadDeCasilla::DECORABLE |
                                      PropiedadDeCasilla::ADMITE_HIERBA;

                // Limpiar vallas y pasto perimetral viejo en este y sur antes de redimensionar
                int columna_este_vieja = columnas_actuales - 1;
                int fila_limpiar = 0;
                while (fila_limpiar < filas_actuales) {
                    if (mapa_actual_sector
                            ->obtener_casilla_en_capa(
                                TipoCapa::SUELO,
                                columna_este_vieja,
                                fila_limpiar)
                            .obtener_id_tile() == CatalogoDeEstampas::TILE_PASTO_PERIMETRAL) {
                        mapa_actual_sector->pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_este_vieja,
                            fila_limpiar,
                            identificador_tile_relleno,
                            propiedades_relleno);
                    }
                    mapa_actual_sector->borrar_en_capa(
                        TipoCapa::ESTRUCTURAS,
                        columna_este_vieja,
                        fila_limpiar);
                    mapa_actual_sector->borrar_en_capa(
                        TipoCapa::FRENTE,
                        columna_este_vieja,
                        fila_limpiar);
                    fila_limpiar++;
                }

                int columna_limpiar = 0;
                while (columna_limpiar < columnas_actuales) {
                    int fila_sur_base_vieja = filas_actuales - 1;
                    if (mapa_actual_sector
                            ->obtener_casilla_en_capa(
                                TipoCapa::SUELO,
                                columna_limpiar,
                                fila_sur_base_vieja)
                            .obtener_id_tile() == CatalogoDeEstampas::TILE_PASTO_PERIMETRAL) {
                        mapa_actual_sector->pintar_en_capa(
                            TipoCapa::SUELO,
                            columna_limpiar,
                            fila_sur_base_vieja,
                            identificador_tile_relleno,
                            propiedades_relleno);
                    }
                    mapa_actual_sector->borrar_en_capa(
                        TipoCapa::ESTRUCTURAS,
                        columna_limpiar,
                        fila_sur_base_vieja);
                    mapa_actual_sector->borrar_en_capa(
                        TipoCapa::FRENTE,
                        columna_limpiar,
                        fila_sur_base_vieja);

                    int fila_sur_frente_vieja = filas_actuales - 2;
                    mapa_actual_sector->borrar_en_capa(
                        TipoCapa::FRENTE,
                        columna_limpiar,
                        fila_sur_frente_vieja);
                    columna_limpiar++;
                }
            } else if (es_sector_mercado) {
                identificador_tile_relleno =
                    CatalogoDeTerrenos::obtener_tile_base(SectorActual::MERCADO);
                propiedades_relleno = PropiedadDeCasilla::DECORABLE;
            } else if (indice_sector >= 3) {
                propiedades_relleno = PropiedadDeCasilla::BLOQUEA;
            }

            mapa_actual_sector->redimensionar(
                nuevas_columnas,
                nuevas_filas,
                identificador_tile_relleno,
                propiedades_relleno);

            if (es_sector_granja) {
                mapa_actual_sector->generar_muro_perimetral_granja();
            } else if (indice_sector == 1) {
                mapa_actual_sector->generar_muro_perimetral_pueblo();
            } else if (es_sector_mercado) {
                mapa_actual_sector->reconstruir_perimetro_mercado();
            }

            lienzo->actualizar_tamano_del_lienzo();
            lienzo->update();
            actualizar_etiqueta_dimensiones();
            marcar_cambios_sin_guardar();
            registrar_estado_en_historial();
            ejecutar_validacion();
            statusBar()->showMessage(
                QString("Dimensiones actualizadas a %1 x %2 casillas.")
                    .arg(nuevas_columnas)
                    .arg(nuevas_filas),
                4000);
        }
    }
}

void VentanaPrincipalEditor::generar_muro_perimetral_granja() {
    int indice_sector = combo_sectores->currentIndex();
    MapaEditable* mapa_actual_sector = obtener_mapa_activo();
    if (mapa_actual_sector == nullptr) {
        return;
    }
    if (indice_sector == 0) {
        mapa_actual_sector->generar_muro_perimetral_granja();
        lienzo->update();
        marcar_cambios_sin_guardar();
        registrar_estado_en_historial();
        ejecutar_validacion();
        statusBar()->showMessage("Perímetro de muros de troncos generado con éxito.", 3000);
    } else if (indice_sector == 1) {
        mapa_actual_sector->generar_muro_perimetral_pueblo();
        lienzo->update();
        marcar_cambios_sin_guardar();
        registrar_estado_en_historial();
        ejecutar_validacion();
        statusBar()->showMessage("Perímetro de vallas urbanas generado con éxito.", 3000);
    } else if (indice_sector == 2) {
        mapa_actual_sector->reconstruir_perimetro_mercado();
        lienzo->update();
        marcar_cambios_sin_guardar();
        registrar_estado_en_historial();
        ejecutar_validacion();
        statusBar()->showMessage("Perímetro del mercado restaurado con éxito.", 3000);
    } else if (indice_sector >= 3) {
        mapa_actual_sector->generar_muros_caverna();
        lienzo->actualizar_tamano_del_lienzo();
        lienzo->update();
        actualizar_etiqueta_dimensiones();
        marcar_cambios_sin_guardar();
        registrar_estado_en_historial();
        ejecutar_validacion();
        statusBar()->showMessage("Muros de mina generados con éxito.", 3000);
    }
}
