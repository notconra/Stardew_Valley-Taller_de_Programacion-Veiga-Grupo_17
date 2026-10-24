#ifndef PANTALLAPRINCIPAL_H
#define PANTALLAPRINCIPAL_H

#include <QMainWindow>

namespace Ui {
class PantallaPrincipal;
}

class PantallaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    explicit PantallaPrincipal(QWidget* parent = nullptr);
    ~PantallaPrincipal();

private slots:
    // Agregás tus propios métodos para manejar los clics
    void preparar_nueva_partida();
    void preparar_retomar_partida();
    void preparar_unirse_partida();

private:
    Ui::PantallaPrincipal* ui;
};

#endif  // PANTALLAPRINCIPAL_H
