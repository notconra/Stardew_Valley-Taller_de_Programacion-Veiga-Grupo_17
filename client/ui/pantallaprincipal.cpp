#include "pantallaprincipal.h"

#include <iostream>

#include "ui_pantallaprincipal.h"

PantallaPrincipal::PantallaPrincipal(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::PantallaPrincipal) {
    ui->setupUi(this);
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
}

PantallaPrincipal::~PantallaPrincipal() {
    delete ui;
}

void PantallaPrincipal::preparar_nueva_partida() {
    std::cout << "Customización de Personaje de prueba\n";
}

void PantallaPrincipal::preparar_retomar_partida() {
    std::cout << "Lista de Partidas Guardadas pruebita\n";
}

void PantallaPrincipal::preparar_unirse_partida() {
    std::cout << "Ingreso de Código para unirsee\n";
}
