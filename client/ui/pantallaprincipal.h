#ifndef PANTALLAPRINCIPAL_H
#define PANTALLAPRINCIPAL_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QPixmap>
#include <QTimer>
#include <string>

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

protected:
    void paintEvent(QPaintEvent* evento) override;

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
    Ui::PantallaPrincipal* ui;
    void configurar_ventana();
    void configurar_tabla_partidas();
    void configurar_formulario_granja(bool visible);
    void inicializar_selectores_personaje();
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
    int indice_pelo_seleccionado = 0;
    int indice_color_pelo_seleccionado = 0;
    int indice_remera_seleccionado = 0;
    int indice_pantalon_color_seleccionado = 0;
    int indice_tipo_pantalon_seleccionado = 0;
    int indice_sombrero_seleccionado = -1;
    bool es_personaje_femenino = false;

    FondoAnimado fondo_animado;
    BotonesMenu botones_menu;
    QPixmap logo_original;
    QPixmap logo_escalado;
    QTimer* temporizador_animacion = nullptr;
    float tiempo_animacion = 0.0f;
};

#endif  // PANTALLAPRINCIPAL_H
