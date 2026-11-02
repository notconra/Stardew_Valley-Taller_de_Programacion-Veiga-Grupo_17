#ifndef PANTALLAPRINCIPAL_H
#define PANTALLAPRINCIPAL_H

#include <QEvent>
#include <QMainWindow>
#include <QPaintEvent>
#include <QPixmap>
#include <QTimer>
#include <string>
#include <unordered_map>
#include <vector>

#include "../red/conexion_con_servidor.h"

namespace Ui {
class PantallaPrincipal;
}

class PantallaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    PantallaPrincipal(
        ConexionConServidor& conexion, EntradaExitosaAPartida& partida_actual, uint32_t id_cliente,
        QWidget* parent = nullptr);
    ~PantallaPrincipal();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void preparar_nueva_partida();
    void preparar_retomar_partida();
    void preparar_unirse_partida();

    void volver_al_menu();
    void confirmar_personaje();
    void confirmar_retomar_partida();
    void actualizar_seleccion_partida();
    void actualizar_animacion();

private:
    struct Nube {
        float x = 0.0f;
        float y = 0.0f;
        float velocidad = 0.0f;
        float escala = 1.0f;
        int tipo = 0;
    };

    struct EstadoBoton {
        QPixmap normal;
        QPixmap hover;
        QPixmap pressed;
    };

    struct Estrella {
        int x_pct = 0;
        int y_pct = 0;
        int tamano = 1;
        QColor color;
    };

    Ui::PantallaPrincipal* ui;
    void configurar_ventana();
    void configurar_tabla_partidas();
    void configurar_formulario_granja(bool visible);
    void llenar_lista_partidas(const ListaPartidas& lista);
    bool procesar_evento_ingreso(const EventoServidor& evento);
    QString describir_error(const ErrorServidor& error) const;

    void inicializar_estrellas();
    void inicializar_nubes();
    void inicializar_botones_menu();

    ConexionConServidor& conexion;
    EntradaExitosaAPartida& partida_actual;
    bool es_modo_unirse = false;
    std::string codigo_partida_a_unirse;
    uint32_t id_cliente = 0;

    std::vector<Estrella> estrellas;
    std::vector<Nube> nubes;
    std::vector<QPixmap> sprites_nubes;
    std::unordered_map<QObject*, EstadoBoton> botones_menu;
    QPixmap pixmap_logo;
    QPixmap pixmap_logo_escalado;
    QTimer* timer_animacion = nullptr;
    float tiempo_animacion = 0.0f;
};

#endif  // PANTALLAPRINCIPAL_H
