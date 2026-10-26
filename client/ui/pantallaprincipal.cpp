#include "pantallaprincipal.h"

#include <QMessageBox>
#include <iostream>

#include "ui_pantallaprincipal.h"

PantallaPrincipal::PantallaPrincipal(
    ConexionConServidor& conexion, EntradaExitosaAPartida& partida_actual, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::PantallaPrincipal),
      conexion(conexion),
      partida_actual(partida_actual) {
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
}

PantallaPrincipal::~PantallaPrincipal() {
    delete ui;
}

void PantallaPrincipal::configurar_ventana() {
    this->setMinimumSize(800, 600);
    this->setMaximumSize(16777215, 16777215);

    ui->centralwidget->setMaximumSize(16777215, 16777215);
    ui->stackedWidget->setMaximumSize(16777215, 16777215);

    ui->stackedWidget->setCurrentIndex(0);
}

void PantallaPrincipal::volver_al_menu() {
    ui->stackedWidget->setCurrentIndex(0);
}

void PantallaPrincipal::confirmar_personaje() {
    QString nombre = ui->input_nombre->text();
    QString granja = ui->input_nombre_granja->text();
    QString cosa_favorita = ui->input_cosa_fav->text();

    if (nombre.isEmpty() || granja.isEmpty() || cosa_favorita.isEmpty()) {
        std::cout << "[Advertencia] Tenés que completar todos los campos para continuar.\n";
        return;
    }

    conexion.enviar_comando(Identificarse{0, nombre.toStdString()});
    conexion.enviar_comando(CrearPartida{granja.toStdString()});

    try {
        EventoServidor evento = conexion.esperar_evento();
        if (std::holds_alternative<EntradaExitosaAPartida>(evento)) {
            const auto& entrada = std::get<EntradaExitosaAPartida>(evento);
            std::cout << "Partida creada! Código: " << entrada.codigo_partida
                      << " | ID Jugador: " << entrada.id_jugador << "\n";
            this->partida_actual = entrada;
            this->close();
        } else if (std::holds_alternative<ErrorServidor>(evento)) {
            const auto& error = std::get<ErrorServidor>(evento);
            std::cerr << "Error al crear partida: " << error.detalle << "\n";
            QMessageBox::warning(this, "Error", QString::fromStdString(error.detalle));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error de conexión: " << e.what() << "\n";
    }
}

void PantallaPrincipal::preparar_nueva_partida() {
    ui->stackedWidget->setCurrentIndex(1);
    std::cout << "Customización de Personaje de prueba\n";
}

void PantallaPrincipal::preparar_retomar_partida() {
    std::cout << "Lista de Partidas Guardadas pruebita\n";
}

void PantallaPrincipal::preparar_unirse_partida() {
    std::cout << "Ingreso de Código para unirsee\n";
}
