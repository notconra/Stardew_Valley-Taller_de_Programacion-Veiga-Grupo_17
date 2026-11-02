#include "pantallaprincipal.h"

#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QTableWidgetItem>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>

#include "ui_pantallaprincipal.h"

PantallaPrincipal::PantallaPrincipal(
    ConexionConServidor& conexion, EntradaExitosaAPartida& partida_actual, uint32_t id_cliente,
    QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::PantallaPrincipal),
      conexion(conexion),
      partida_actual(partida_actual),
      id_cliente(id_cliente) {
    ui->setupUi(this);
    configurar_ventana();
    connect(ui->btn_salir, &QPushButton::clicked, this, &PantallaPrincipal::close);
    connect(
        ui->btn_nueva_partida,
        &QPushButton::clicked,
        this,
        &PantallaPrincipal::preparar_nueva_partida);
    connect(
        ui->btn_retomar_partida,
        &QPushButton::clicked,
        this,
        &PantallaPrincipal::preparar_retomar_partida);
    connect(
        ui->btn_unirse_codigo,
        &QPushButton::clicked,
        this,
        &PantallaPrincipal::preparar_unirse_partida);
    connect(ui->btn_volver_menu, &QPushButton::clicked, this, &PantallaPrincipal::volver_al_menu);
    connect(
        ui->btn_confirmar_personaje,
        &QPushButton::clicked,
        this,
        &PantallaPrincipal::confirmar_personaje);
    connect(
        ui->btn_volver_retomar,
        &QPushButton::clicked,
        this,
        &PantallaPrincipal::volver_al_menu);
    connect(
        ui->btn_jugar_retomar,
        &QPushButton::clicked,
        this,
        &PantallaPrincipal::confirmar_retomar_partida);
    connect(
        ui->tabla_partidas,
        &QTableWidget::itemSelectionChanged,
        this,
        &PantallaPrincipal::actualizar_seleccion_partida);
    connect(
        ui->tabla_partidas,
        &QTableWidget::cellDoubleClicked,
        this,
        &PantallaPrincipal::confirmar_retomar_partida);
}

PantallaPrincipal::~PantallaPrincipal() {
    delete ui;
}

void PantallaPrincipal::configurar_ventana() {
    this->setMinimumSize(800, 600);
    this->resize(1024, 768);
    this->setMaximumSize(16777215, 16777215);
    this->setWindowTitle("Stardew Valley");

    ui->centralwidget->setMaximumSize(16777215, 16777215);
    ui->stackedWidget->setMaximumSize(16777215, 16777215);

    pixmap_logo.load("Assets/LooseSprites/logo.png");

    if (!pixmap_logo.isNull()) {
        this->setWindowIcon(QIcon(pixmap_logo));
        pixmap_logo_escalado =
            pixmap_logo.scaled(660, 363, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label->setPixmap(pixmap_logo_escalado);
        ui->label->setAlignment(Qt::AlignCenter);
    }

    inicializar_estrellas();
    inicializar_nubes();
    inicializar_botones_menu();

    configurar_tabla_partidas();
    ui->stackedWidget->setCurrentIndex(0);

    timer_animacion = new QTimer(this);
    connect(timer_animacion, &QTimer::timeout, this, &PantallaPrincipal::actualizar_animacion);
    timer_animacion->start(33);
}

void PantallaPrincipal::inicializar_estrellas() {
    estrellas.clear();
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist_x(10, 990);
    std::uniform_int_distribution<int> dist_y(10, 990);
    std::uniform_int_distribution<int> dist_tam(1, 2);
    std::uniform_int_distribution<int> dist_brillo(180, 255);

    for (int i = 0; i < 45; ++i) {
        int b = dist_brillo(gen);
        estrellas.push_back(Estrella{
            dist_x(gen),
            dist_y(gen),
            dist_tam(gen),
            QColor(b, b, b, b),
        });
    }
}

void PantallaPrincipal::inicializar_nubes() {
    QPixmap hoja_nubes("Assets/Minigames/Clouds.png");
    if (hoja_nubes.isNull()) {
        return;
    }

    static const struct {
        int x;
        int y;
        int w;
        int h;
    } RECTANGULOS_NUBES[] = {
        {260, 336, 168, 116},
        {115, 349, 123, 82},
        {152, 447, 123, 53},
        {20, 414, 82, 46},
        {22, 471, 104, 29},
        {410, 467, 64, 33},
    };

    sprites_nubes.clear();
    for (const auto& r : RECTANGULOS_NUBES) {
        sprites_nubes.push_back(hoja_nubes.copy(r.x, r.y, r.w, r.h));
    }

    nubes = {
        Nube{-100.0f, 60.0f, 0.35f, 2.8f, 0},
        Nube{450.0f, 130.0f, 0.22f, 3.2f, 1},
        Nube{850.0f, 260.0f, 0.45f, 2.5f, 2},
        Nube{100.0f, 300.0f, 0.18f, 2.5f, 3},
        Nube{750.0f, 70.0f, 0.50f, 2.6f, 4},
        Nube{-60.0f, 380.0f, 0.28f, 2.4f, 5},
    };
}

void PantallaPrincipal::inicializar_botones_menu() {
    QPixmap hoja_botones("Assets/Minigames/TitleButtons.es-ES.png");
    if (hoja_botones.isNull()) {
        hoja_botones.load("Assets/Minigames/TitleButtons.png");
    }
    if (hoja_botones.isNull()) {
        return;
    }

    QPushButton* botones[4] = {
        ui->btn_nueva_partida,
        ui->btn_retomar_partida,
        ui->btn_unirse_codigo,
        ui->btn_salir,
    };

    const char* tooltips[4] = {
        "Nueva Partida",
        "Cargar Partida",
        "Coop.",
        "Salir",
    };

    constexpr int ANCHO_BOTON = 74;
    constexpr int ALTO_BOTON = 58;
    constexpr int ANCHO_ESCALADO = 118;
    constexpr int ALTO_ESCALADO = 93;
    constexpr int Y_NORMAL = 187;
    constexpr int Y_HOVER = 245;
    constexpr int Y_PRESSED = 303;

    for (int i = 0; i < 4; ++i) {
        QPushButton* btn = botones[i];
        btn->setText("");
        btn->setToolTip(tooltips[i]);
        btn->setCursor(Qt::PointingHandCursor);

        QPixmap normal = hoja_botones.copy(i * ANCHO_BOTON, Y_NORMAL, ANCHO_BOTON, ALTO_BOTON)
                             .scaled(
                                 ANCHO_ESCALADO,
                                 ALTO_ESCALADO,
                                 Qt::IgnoreAspectRatio,
                                 Qt::FastTransformation);
        QPixmap hover = hoja_botones.copy(i * ANCHO_BOTON, Y_HOVER, ANCHO_BOTON, ALTO_BOTON)
                            .scaled(
                                ANCHO_ESCALADO,
                                ALTO_ESCALADO,
                                Qt::IgnoreAspectRatio,
                                Qt::FastTransformation);
        QPixmap pressed = hoja_botones.copy(i * ANCHO_BOTON, Y_PRESSED, ANCHO_BOTON, ALTO_BOTON)
                              .scaled(
                                  ANCHO_ESCALADO,
                                  ALTO_ESCALADO,
                                  Qt::IgnoreAspectRatio,
                                  Qt::FastTransformation);

        btn->setIcon(QIcon(normal));
        btn->setIconSize(QSize(ANCHO_ESCALADO, ALTO_ESCALADO));
        btn->installEventFilter(this);

        botones_menu[btn] = EstadoBoton{normal, hover, pressed};
    }
}

bool PantallaPrincipal::eventFilter(QObject* watched, QEvent* event) {
    auto it = botones_menu.find(watched);
    if (it != botones_menu.end()) {
        auto* btn = static_cast<QPushButton*>(watched);
        switch (event->type()) {
            case QEvent::Enter:
                btn->setIcon(QIcon(it->second.hover));
                break;
            case QEvent::Leave:
                btn->setIcon(QIcon(it->second.normal));
                break;
            case QEvent::MouseButtonPress:
                btn->setIcon(QIcon(it->second.pressed));
                break;
            case QEvent::MouseButtonRelease:
                btn->setIcon(QIcon(it->second.hover));
                break;
            default:
                break;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void PantallaPrincipal::actualizar_animacion() {
    tiempo_animacion += 0.05f;

    for (auto& nube : nubes) {
        nube.x += nube.velocidad;
        float ancho_nube = 100.0f;
        if (nube.tipo >= 0 && nube.tipo < static_cast<int>(sprites_nubes.size())) {
            ancho_nube = sprites_nubes[nube.tipo].width() * nube.escala;
        }
        if (nube.x > this->width()) {
            nube.x = -ancho_nube;
        }
    }

    if (ui->stackedWidget->currentIndex() == 0) {
        int offset_y = static_cast<int>(std::sin(tiempo_animacion) * 5.0f);
        ui->label->setContentsMargins(0, 10 + offset_y, 0, 10 - offset_y);
        update();
    }
}

void PantallaPrincipal::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    // Gradiente de cielo idéntico al juego original (noche estrellada superior -> día celeste)
    QLinearGradient gradiente_cielo(0, 0, 0, height());
    gradiente_cielo.setColorAt(0.00, QColor(8, 30, 80));      // Noche estrellada superior (#081e50)
    gradiente_cielo.setColorAt(0.12, QColor(22, 105, 195));   // Crepúsculo
    gradiente_cielo.setColorAt(0.40, QColor(58, 175, 245));   // Celeste cielo brillante Stardew
    gradiente_cielo.setColorAt(1.00, QColor(135, 215, 255));  // Celeste claro horizonte
    painter.fillRect(rect(), gradiente_cielo);

    // Estrellas en la franja nocturna superior
    painter.setPen(Qt::NoPen);
    for (const auto& estrella : estrellas) {
        int sx = (estrella.x_pct * width()) / 1000;
        int sy = static_cast<int>((estrella.y_pct * height() * 0.13f) / 1000.0f);
        painter.fillRect(sx, sy, estrella.tamano, estrella.tamano, estrella.color);
    }

    if (ui->stackedWidget->currentIndex() == 0) {
        for (const auto& nube : nubes) {
            if (nube.tipo >= 0 && nube.tipo < static_cast<int>(sprites_nubes.size())) {
                const QPixmap& sprite = sprites_nubes[nube.tipo];
                int w = static_cast<int>(sprite.width() * nube.escala);
                int h = static_cast<int>(sprite.height() * nube.escala);
                painter
                    .drawPixmap(static_cast<int>(nube.x), static_cast<int>(nube.y), w, h, sprite);
            }
        }
    }

    QMainWindow::paintEvent(event);
}

void PantallaPrincipal::configurar_tabla_partidas() {
    ui->tabla_partidas->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tabla_partidas->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tabla_partidas->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}

void PantallaPrincipal::volver_al_menu() {
    ui->stackedWidget->setCurrentIndex(0);
}

void PantallaPrincipal::confirmar_personaje() {
    QString nombre = ui->input_nombre->text().trimmed();
    QString cosa_favorita = ui->input_cosa_fav->text().trimmed();
    QString granja = ui->input_nombre_granja->text().trimmed();
    QString mundo = ui->combo_mundo->currentText();
    QString peinado = ui->combo_peinado->currentText();
    QString color_pelo = ui->combo_color_pelo->currentText();

    if (nombre.isEmpty() || cosa_favorita.isEmpty() || (!es_modo_unirse && granja.isEmpty())) {
        QMessageBox::warning(
            this,
            "Campos incompletos",
            "Tenés que completar todos los campos obligatorios para continuar.");
        return;
    }

    ui->btn_confirmar_personaje->setEnabled(false);

    if (es_modo_unirse) {
        std::cout << "Uniéndose a partida " << codigo_partida_a_unirse
                  << " con personaje: " << nombre.toStdString()
                  << " | Pelo: " << peinado.toStdString() << " (" << color_pelo.toStdString()
                  << ")\n";
    } else {
        std::cout << "Creando personaje: " << nombre.toStdString()
                  << " | Granja: " << granja.toStdString() << " (" << mundo.toStdString() << ")"
                  << " | Pelo: " << peinado.toStdString() << " (" << color_pelo.toStdString()
                  << ")\n";
    }

    if (es_modo_unirse) {
        conexion.enviar_comando(UnirsePartida{codigo_partida_a_unirse, nombre.toStdString()});
    } else {
        conexion.enviar_comando(CrearPartida{granja.toStdString(), nombre.toStdString()});
    }

    try {
        while (!procesar_evento_ingreso(conexion.esperar_evento())) {
        }
    } catch (const std::exception& e) {
        std::cerr << "Error de conexión: " << e.what() << "\n";
        QMessageBox::critical(this, "Error de conexión", QString::fromStdString(e.what()));
        ui->btn_confirmar_personaje->setEnabled(true);
    }
}

QString PantallaPrincipal::describir_error(const ErrorServidor& error) const {
    static const std::unordered_map<CodigoError, QString> MENSAJES_ERROR = {
        {CodigoError::PARTIDA_INEXISTENTE, "El código de partida no existe o es inválido."},
        {CodigoError::PARTIDA_LLENA, "La partida está llena (máximo de jugadores alcanzado)."},
        {CodigoError::NOMBRE_JUGADOR_REPETIDO,
         "Ya existe un jugador con ese nombre en la partida."},
        {CodigoError::NOMBRE_PARTIDA_REPETIDO, "Ya existe una partida con ese nombre."},
        {CodigoError::PARTIDA_CERRADA, "La partida ya no está disponible."}};

    auto it = MENSAJES_ERROR.find(error.codigo);
    if (it != MENSAJES_ERROR.end()) {
        return it->second;
    }

    if (error.codigo == CodigoError::NO_PERMITIDO) {
        if (!error.detalle.empty()) {
            return QString("Operación no permitida: %1").arg(QString::fromStdString(error.detalle));
        }
        return "Operación no permitida.";
    }

    return error.detalle.empty() ? "Error desconocido del servidor."
                                 : QString::fromStdString(error.detalle);
}

bool PantallaPrincipal::procesar_evento_ingreso(const EventoServidor& evento) {
    if (std::holds_alternative<EntradaExitosaAPartida>(evento)) {
        const auto& entrada = std::get<EntradaExitosaAPartida>(evento);
        std::cout << (es_modo_unirse ? "Unido a la partida! Código: " : "Partida creada! Código: ")
                  << entrada.codigo_partida << " | ID Jugador: " << entrada.id_jugador << "\n";
        this->partida_actual = entrada;
        this->close();
        return true;
    }

    if (std::holds_alternative<ErrorServidor>(evento)) {
        const auto& error = std::get<ErrorServidor>(evento);
        std::cerr << "Error del servidor: " << error.detalle << "\n";
        QMessageBox::warning(this, "Error", describir_error(error));
        ui->btn_confirmar_personaje->setEnabled(true);
        ui->btn_jugar_retomar->setEnabled(true);
        return true;
    }

    return false;
}

void PantallaPrincipal::configurar_formulario_granja(bool visible) {
    ui->label_3->setVisible(visible);
    ui->input_nombre_granja->setVisible(visible);
    ui->label_mundo->setVisible(visible);
    ui->combo_mundo->setVisible(visible);
}

void PantallaPrincipal::preparar_nueva_partida() {
    es_modo_unirse = false;
    codigo_partida_a_unirse.clear();
    configurar_formulario_granja(true);
    ui->btn_confirmar_personaje->setEnabled(true);
    ui->stackedWidget->setCurrentIndex(1);
}

void PantallaPrincipal::preparar_retomar_partida() {
    conexion.enviar_comando(ListarPartidas{});
    ui->tabla_partidas->setRowCount(0);
    ui->btn_jugar_retomar->setEnabled(false);

    try {
        bool lista_recibida = false;
        while (!lista_recibida) {
            EventoServidor evento = conexion.esperar_evento();
            if (std::holds_alternative<ListaPartidas>(evento)) {
                llenar_lista_partidas(std::get<ListaPartidas>(evento));
                lista_recibida = true;
            } else if (std::holds_alternative<ErrorServidor>(evento)) {
                const auto& error = std::get<ErrorServidor>(evento);
                std::cerr << "Error del servidor al listar partidas: " << error.detalle << "\n";
                QMessageBox::warning(this, "Error", describir_error(error));
                return;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error de conexión al listar partidas: " << e.what() << "\n";
        QMessageBox::critical(this, "Error de conexión", "Se perdió la conexión con el servidor.");
        return;
    }

    ui->stackedWidget->setCurrentIndex(2);
}

void PantallaPrincipal::llenar_lista_partidas(const ListaPartidas& lista) {
    ui->tabla_partidas->setRowCount(0);
    ui->tabla_partidas->setRowCount(static_cast<int>(lista.partidas.size()));

    for (int i = 0; i < static_cast<int>(lista.partidas.size()); ++i) {
        const auto& partida = lista.partidas[i];

        auto* item_nombre = new QTableWidgetItem(QString::fromStdString(partida.nombre_partida));
        auto* item_codigo = new QTableWidgetItem(QString::fromStdString(partida.codigo_partida));
        item_codigo->setTextAlignment(Qt::AlignCenter);

        QString jugadores_str =
            QString("%1/%2").arg(partida.jugadores_actuales).arg(partida.jugadores_maximos);
        auto* item_jugadores = new QTableWidgetItem(jugadores_str);
        item_jugadores->setTextAlignment(Qt::AlignCenter);

        ui->tabla_partidas->setItem(i, 0, item_nombre);
        ui->tabla_partidas->setItem(i, 1, item_codigo);
        ui->tabla_partidas->setItem(i, 2, item_jugadores);
    }
}

void PantallaPrincipal::actualizar_seleccion_partida() {
    bool hay_seleccion = !ui->tabla_partidas->selectedItems().isEmpty();
    ui->btn_jugar_retomar->setEnabled(hay_seleccion);
}

void PantallaPrincipal::confirmar_retomar_partida() {
    int fila = ui->tabla_partidas->currentRow();
    if (fila < 0) {
        return;
    }

    QTableWidgetItem* item_codigo = ui->tabla_partidas->item(fila, 1);
    if (!item_codigo) {
        return;
    }

    std::string codigo = item_codigo->text().trimmed().toStdString();
    if (codigo.empty()) {
        return;
    }

    es_modo_unirse = true;
    ui->btn_jugar_retomar->setEnabled(false);

    try {
        conexion.enviar_comando(RetomarPartida{codigo});

        bool procesado = false;
        while (!procesado) {
            EventoServidor evento = conexion.esperar_evento();
            procesado = procesar_evento_ingreso(evento);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al retomar la partida: " << e.what() << "\n";
        QMessageBox::critical(this, "Error de conexión", "Se perdió la conexión con el servidor.");
        ui->btn_jugar_retomar->setEnabled(true);
    }
}

void PantallaPrincipal::preparar_unirse_partida() {
    bool acepto = false;
    QString codigo = QInputDialog::getText(
        this,
        "Unirse a partida",
        "Ingresá el código de invitación de la partida (ej: P0):",
        QLineEdit::Normal,
        "",
        &acepto);

    if (!acepto || codigo.trimmed().isEmpty()) {
        return;
    }

    es_modo_unirse = true;
    codigo_partida_a_unirse = codigo.trimmed().toStdString();
    configurar_formulario_granja(false);
    ui->btn_confirmar_personaje->setEnabled(true);
    ui->stackedWidget->setCurrentIndex(1);
}
