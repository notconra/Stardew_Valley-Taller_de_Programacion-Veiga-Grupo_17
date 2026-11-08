#ifndef PANTALLAPRINCIPAL_H
#define PANTALLAPRINCIPAL_H

#include <QButtonGroup>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QPaintEvent>
#include <QPixmap>
#include <QSlider>
#include <QTimer>
#include <random>
#include <string>
#include <vector>

#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "botones_menu.h"
#include "compositor_personaje.h"
#include "fondo_animado.h"

namespace Ui {
class PantallaPrincipal;
}

class PantallaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    PantallaPrincipal(
        ConexionConServidor& conexion, EntradaExitosaAPartida& partida_actual,
        ConfiguracionJugador& configuracion_jugador, QWidget* parent = nullptr);
    ~PantallaPrincipal();

    std::string mapa_seleccionado() const {
        return mapa_granja_seleccionado;
    }
    int tipo_granja_seleccionado() const {
        return indice_granja_seleccionada;
    }

protected:
    void paintEvent(QPaintEvent* evento) override;
    void keyPressEvent(QKeyEvent* evento) override;

private slots:
    void preparar_nueva_partida();
    void preparar_retomar_partida();
    void preparar_unirse_partida();

    void volver_al_menu();
    void confirmar_personaje();
    void confirmar_retomar_partida();
    void actualizar_seleccion_partida();
    void actualizar_animacion();
    void randomizar_personaje();
    void actualizar_color_ojos();
    void actualizar_color_pelo();
    void actualizar_color_pantalon();
    void al_seleccionar_tipo_granja(int indice);

private:
    Ui::PantallaPrincipal* ui;
    void configurar_ventana();
    void configurar_tabla_partidas();
    void configurar_formulario_granja(bool visible);
    void inicializar_selectores_personaje();
    void inicializar_selector_granjas();
    void seleccionar_tipo_granja(int indice);
    void sincronizar_sliders_desde_colores();
    void sincronizar_canal_slider(QSlider* slider, QLabel* etiqueta_valor, int valor);
    ColorRGB convertir_hsv_a_rgb(
        int matiz_porcentaje, int saturacion_porcentaje, int luminosidad_porcentaje) const;
    ColorRGB leer_color_desde_sliders(
        const QSlider* slider_matiz, const QSlider* slider_saturacion,
        const QSlider* slider_luminosidad, QLabel* etiqueta_matiz, QLabel* etiqueta_saturacion,
        QLabel* etiqueta_luminosidad);
    void actualizar_previsualizacion_personaje();
    ConfiguracionJugador obtener_configuracion_actual() const;
    void llenar_lista_partidas(const ListaPartidas& lista);
    bool procesar_evento_ingreso(const EventoServidor& evento);
    QString describir_error(const ErrorServidor& error) const;

    ConexionConServidor& conexion;
    EntradaExitosaAPartida& partida_actual;
    bool es_modo_unirse = false;
    std::string codigo_partida_a_unirse;
    ConfiguracionJugador& configuracion_jugador;
    CompositorPersonaje compositor_personaje;
    int indice_piel_seleccionada = 0;
    int indice_pelo_seleccionado = 0;
    int indice_remera_seleccionado = 0;
    int indice_tipo_pantalon_seleccionado = 0;
    int indice_sombrero_seleccionado = -1;
    ColorRGB color_ojos_actual = {104, 43, 15};
    ColorRGB color_pelo_actual = {193, 90, 33};
    ColorRGB color_pantalon_actual = {45, 60, 140};
    bool es_personaje_femenino = false;
    int indice_granja_seleccionada = 0;
    std::string mapa_granja_seleccionado = "Farm";
    QButtonGroup grupo_botones_granja{this};
    std::vector<QPushButton*> botones_tipo_granja;
    std::mt19937 generador_azar{std::random_device{}()};

    FondoAnimado fondo_animado;
    BotonesMenu botones_menu;
    QPixmap logo_original;
    QPixmap logo_escalado;
    QTimer* temporizador_animacion = nullptr;
    float tiempo_animacion = 0.0f;
};

#endif  // PANTALLAPRINCIPAL_H
