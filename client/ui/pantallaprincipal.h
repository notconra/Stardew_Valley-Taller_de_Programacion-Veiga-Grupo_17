#ifndef PANTALLAPRINCIPAL_H
#define PANTALLAPRINCIPAL_H

#include <QMainWindow>

#include "../red/conexion_con_servidor.h"

namespace Ui {
class PantallaPrincipal;
}

class PantallaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    PantallaPrincipal(
        ConexionConServidor& conexion, EntradaExitosaAPartida& partida_actual,
        QWidget* parent = nullptr);
    ~PantallaPrincipal();

private slots:
    void preparar_nueva_partida();
    void preparar_retomar_partida();
    void preparar_unirse_partida();

    void volver_al_menu();
    void confirmar_personaje();

private:
    Ui::PantallaPrincipal* ui;
    void configurar_ventana();
    ConexionConServidor& conexion;
    EntradaExitosaAPartida& partida_actual;
};

#endif  // PANTALLAPRINCIPAL_H
