#ifndef PANTALLAPRINCIPAL_H
#define PANTALLAPRINCIPAL_H

#include <QMainWindow>
#include <string>

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
    void configurar_formulario_granja(bool visible);
    bool procesar_evento_ingreso(const EventoServidor& evento);
    QString describir_error(const ErrorServidor& error) const;

    ConexionConServidor& conexion;
    EntradaExitosaAPartida& partida_actual;
    bool es_modo_unirse = false;
    std::string codigo_partida_a_unirse;
    bool identificado = false;
    uint32_t id_cliente = 0;
};

#endif  // PANTALLAPRINCIPAL_H
