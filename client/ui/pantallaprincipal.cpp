#include "pantallaprincipal.h"

#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QTableWidgetItem>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>

#include "ui_pantallaprincipal.h"

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";

struct OpcionColor {
    const char* nombre;
    ColorRGB rgb;
};

constexpr OpcionColor COLORES_PELO[] = {
    {"Castaño", {115, 65, 35}},
    {"Rubio", {220, 185, 85}},
    {"Negro", {40, 35, 35}},
    {"Pelirrojo", {180, 65, 30}},
    {"Azul", {55, 95, 195}},
    {"Rosa", {215, 95, 155}},
    {"Verde", {55, 160, 85}},
    {"Blanco", {230, 230, 230}},
};
constexpr int CANTIDAD_COLORES_PELO = sizeof(COLORES_PELO) / sizeof(COLORES_PELO[0]);

constexpr OpcionColor COLORES_PANTALON[] = {
    {"Azul clásico", {45, 65, 120}},
    {"Negro", {40, 40, 45}},
    {"Marrón", {95, 65, 40}},
    {"Gris", {115, 115, 125}},
    {"Verde oliva", {65, 95, 55}},
    {"Bordó", {125, 45, 45}},
};
constexpr int CANTIDAD_COLORES_PANTALON = sizeof(COLORES_PANTALON) / sizeof(COLORES_PANTALON[0]);

constexpr int CANTIDAD_PEINADOS = 16;
constexpr int CANTIDAD_REMERAS = 16;
constexpr int CANTIDAD_SOMBREROS = 132;
constexpr int CANTIDAD_TIPOS_PANTALON = 6;

static int encontrar_indice_color_cercano(
    const ColorRGB& color, const OpcionColor* opciones, int cantidad) {
    int mejor_indice = 0;
    int menor_diferencia = std::numeric_limits<int>::max();
    for (int i = 0; i < cantidad; ++i) {
        int dr = static_cast<int>(color.rojo) - static_cast<int>(opciones[i].rgb.rojo);
        int dg = static_cast<int>(color.verde) - static_cast<int>(opciones[i].rgb.verde);
        int db = static_cast<int>(color.azul) - static_cast<int>(opciones[i].rgb.azul);
        int diferencia = dr * dr + dg * dg + db * db;
        if (diferencia < menor_diferencia) {
            menor_diferencia = diferencia;
            mejor_indice = i;
        }
    }
    return mejor_indice;
}

PantallaPrincipal::PantallaPrincipal(
    ConexionConServidor& conexion, EntradaExitosaAPartida& partida_actual,
    ConfiguracionJugador& configuracion_jugador, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::PantallaPrincipal),
      conexion(conexion),
      partida_actual(partida_actual),
      configuracion_jugador(configuracion_jugador) {
    ui->setupUi(this);
    configurar_ventana();
    inicializar_selectores_personaje();
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

    logo_original.load("Assets/LooseSprites/logo.png");

    if (!logo_original.isNull()) {
        this->setWindowIcon(QIcon(logo_original));
        logo_escalado =
            logo_original.scaled(660, 363, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_logo->setPixmap(logo_escalado);
        ui->label_logo->setAlignment(Qt::AlignCenter);
    }

    botones_menu.configurar(
        ui->btn_nueva_partida,
        ui->btn_retomar_partida,
        ui->btn_unirse_codigo,
        ui->btn_salir);

    configurar_tabla_partidas();
    ui->stackedWidget->setCurrentIndex(0);

    temporizador_animacion = new QTimer(this);
    connect(
        temporizador_animacion,
        &QTimer::timeout,
        this,
        &PantallaPrincipal::actualizar_animacion);
    temporizador_animacion->start(33);
}

void PantallaPrincipal::actualizar_animacion() {
    tiempo_animacion += 0.05f;
    fondo_animado.actualizar(this->width());

    if (ui->stackedWidget->currentIndex() == 0) {
        int desplazamiento_vertical = static_cast<int>(std::sin(tiempo_animacion) * 5.0f);
        ui->label_logo
            ->setContentsMargins(0, 10 + desplazamiento_vertical, 0, 10 - desplazamiento_vertical);
        update();
    }
}

void PantallaPrincipal::paintEvent(QPaintEvent* evento) {
    QPainter pintor(this);
    fondo_animado.dibujar(pintor, rect(), ui->stackedWidget->currentIndex() == 0);
    QMainWindow::paintEvent(evento);
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

    if (nombre.isEmpty() || cosa_favorita.isEmpty() || (!es_modo_unirse && granja.isEmpty())) {
        QMessageBox::warning(
            this,
            "Campos incompletos",
            "Tenés que completar todos los campos obligatorios para continuar.");
        return;
    }

    configuracion_jugador = obtener_configuracion_actual();

    ui->btn_confirmar_personaje->setEnabled(false);

    if (es_modo_unirse) {
        std::cout << "Uniéndose a partida " << codigo_partida_a_unirse
                  << " con personaje: " << nombre.toStdString() << "\n";
        conexion.enviar_comando(UnirsePartida{codigo_partida_a_unirse, nombre.toStdString()});
    } else {
        std::cout << "Creando personaje: " << nombre.toStdString()
                  << " | Granja: " << granja.toStdString() << "\n";
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

void PantallaPrincipal::inicializar_selectores_personaje() {
    es_personaje_femenino = (configuracion_jugador.ruta_cuerpo == RUTA_CUERPO_FEMENINO);
    configuracion_jugador.offset_y_remera =
        es_personaje_femenino ? OFFSET_Y_REMERA_FEMENINO : OFFSET_Y_REMERA_MASCULINO;
    configuracion_jugador.offset_y_pelo =
        es_personaje_femenino ? OFFSET_Y_PELO_FEMENINO : OFFSET_Y_PELO_MASCULINO;
    indice_pelo_seleccionado = configuracion_jugador.indice_pelo;
    indice_color_pelo_seleccionado = encontrar_indice_color_cercano(
        configuracion_jugador.color_pelo,
        COLORES_PELO,
        CANTIDAD_COLORES_PELO);
    indice_remera_seleccionado = configuracion_jugador.indice_remera;
    indice_pantalon_color_seleccionado = encontrar_indice_color_cercano(
        configuracion_jugador.color_pantalon,
        COLORES_PANTALON,
        CANTIDAD_COLORES_PANTALON);
    indice_tipo_pantalon_seleccionado =
        (configuracion_jugador.indice_pantalon >= 0)
            ? (configuracion_jugador.indice_pantalon % CANTIDAD_TIPOS_PANTALON)
            : 0;
    indice_sombrero_seleccionado = configuracion_jugador.indice_sombrero;
    if (indice_sombrero_seleccionado < -1 || indice_sombrero_seleccionado >= CANTIDAD_SOMBREROS) {
        indice_sombrero_seleccionado = -1;
    }

    connect(ui->btn_genero_masculino, &QPushButton::clicked, this, [this]() {
        es_personaje_femenino = false;
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_genero_femenino, &QPushButton::clicked, this, [this]() {
        es_personaje_femenino = true;
        actualizar_previsualizacion_personaje();
    });

    connect(ui->btn_sombrero_anterior, &QPushButton::clicked, this, [this]() {
        if (indice_sombrero_seleccionado <= -1) {
            indice_sombrero_seleccionado = CANTIDAD_SOMBREROS - 1;
        } else {
            indice_sombrero_seleccionado--;
        }
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_sombrero_siguiente, &QPushButton::clicked, this, [this]() {
        if (indice_sombrero_seleccionado >= CANTIDAD_SOMBREROS - 1) {
            indice_sombrero_seleccionado = -1;
        } else {
            indice_sombrero_seleccionado++;
        }
        actualizar_previsualizacion_personaje();
    });

    connect(ui->btn_pelo_anterior, &QPushButton::clicked, this, [this]() {
        indice_pelo_seleccionado =
            (indice_pelo_seleccionado - 1 + CANTIDAD_PEINADOS) % CANTIDAD_PEINADOS;
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_pelo_siguiente, &QPushButton::clicked, this, [this]() {
        indice_pelo_seleccionado = (indice_pelo_seleccionado + 1) % CANTIDAD_PEINADOS;
        actualizar_previsualizacion_personaje();
    });

    connect(ui->btn_color_pelo_anterior, &QPushButton::clicked, this, [this]() {
        indice_color_pelo_seleccionado =
            (indice_color_pelo_seleccionado - 1 + CANTIDAD_COLORES_PELO) % CANTIDAD_COLORES_PELO;
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_color_pelo_siguiente, &QPushButton::clicked, this, [this]() {
        indice_color_pelo_seleccionado =
            (indice_color_pelo_seleccionado + 1) % CANTIDAD_COLORES_PELO;
        actualizar_previsualizacion_personaje();
    });

    connect(ui->btn_remera_anterior, &QPushButton::clicked, this, [this]() {
        indice_remera_seleccionado =
            (indice_remera_seleccionado - 1 + CANTIDAD_REMERAS) % CANTIDAD_REMERAS;
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_remera_siguiente, &QPushButton::clicked, this, [this]() {
        indice_remera_seleccionado = (indice_remera_seleccionado + 1) % CANTIDAD_REMERAS;
        actualizar_previsualizacion_personaje();
    });

    connect(ui->btn_pantalon_anterior, &QPushButton::clicked, this, [this]() {
        indice_tipo_pantalon_seleccionado =
            (indice_tipo_pantalon_seleccionado - 1 + CANTIDAD_TIPOS_PANTALON) %
            CANTIDAD_TIPOS_PANTALON;
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_pantalon_siguiente, &QPushButton::clicked, this, [this]() {
        indice_tipo_pantalon_seleccionado =
            (indice_tipo_pantalon_seleccionado + 1) % CANTIDAD_TIPOS_PANTALON;
        actualizar_previsualizacion_personaje();
    });

    connect(ui->btn_color_pantalon_anterior, &QPushButton::clicked, this, [this]() {
        indice_pantalon_color_seleccionado =
            (indice_pantalon_color_seleccionado - 1 + CANTIDAD_COLORES_PANTALON) %
            CANTIDAD_COLORES_PANTALON;
        actualizar_previsualizacion_personaje();
    });
    connect(ui->btn_color_pantalon_siguiente, &QPushButton::clicked, this, [this]() {
        indice_pantalon_color_seleccionado =
            (indice_pantalon_color_seleccionado + 1) % CANTIDAD_COLORES_PANTALON;
        actualizar_previsualizacion_personaje();
    });

    actualizar_previsualizacion_personaje();
}

ConfiguracionJugador PantallaPrincipal::obtener_configuracion_actual() const {
    ConfiguracionJugador jugador;
    jugador.ruta_cuerpo = es_personaje_femenino ? RUTA_CUERPO_FEMENINO : RUTA_CUERPO_MASCULINO;
    jugador.offset_y_remera =
        es_personaje_femenino ? OFFSET_Y_REMERA_FEMENINO : OFFSET_Y_REMERA_MASCULINO;
    jugador.offset_y_pelo =
        es_personaje_femenino ? OFFSET_Y_PELO_FEMENINO : OFFSET_Y_PELO_MASCULINO;
    jugador.indice_pelo = indice_pelo_seleccionado;
    jugador.color_pelo = COLORES_PELO[indice_color_pelo_seleccionado].rgb;
    jugador.indice_remera = indice_remera_seleccionado;
    jugador.color_pantalon = COLORES_PANTALON[indice_pantalon_color_seleccionado].rgb;
    jugador.indice_pantalon = indice_tipo_pantalon_seleccionado;
    jugador.indice_sombrero = indice_sombrero_seleccionado;
    return jugador;
}

void PantallaPrincipal::actualizar_previsualizacion_personaje() {
    ui->label_sombrero_valor->setText(
        indice_sombrero_seleccionado < 0 ? "Ninguno"
                                         : QString::number(indice_sombrero_seleccionado + 1));
    ui->label_pelo_valor->setText(QString::number(indice_pelo_seleccionado + 1));
    ui->label_color_pelo_valor->setText(COLORES_PELO[indice_color_pelo_seleccionado].nombre);
    ui->label_remera_valor->setText(QString::number(indice_remera_seleccionado + 1));
    ui->label_pantalon_valor->setText(QString::number(indice_tipo_pantalon_seleccionado + 1));
    ui->label_color_pantalon_valor->setText(
        COLORES_PANTALON[indice_pantalon_color_seleccionado].nombre);

    ui->btn_genero_masculino->setChecked(!es_personaje_femenino);
    ui->btn_genero_femenino->setChecked(es_personaje_femenino);

    ConfiguracionJugador temp = obtener_configuracion_actual();
    QPixmap sprite = compositor_personaje.componer(temp, 4);
    ui->label_previsualizacion_personaje->setPixmap(sprite);
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
            QString("%1/%2").arg(partida.jugadores_totales).arg(partida.jugadores_maximos);
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
