#ifndef VENTANA_PRINCIPAL_EDITOR_H
#define VENTANA_PRINCIPAL_EDITOR_H

#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <string>

#include "definicion_de_mundo.h"
#include "lienzo_de_edicion.h"
#include "paleta_de_herramientas.h"
#include "panel_de_validacion.h"

class VentanaPrincipalEditor : public QMainWindow {
    Q_OBJECT

private:
    DefinicionDeMundo mundo_actual;
    std::string ruta_archivo_actual;

    LienzoDeEdicion* lienzo;
    PaletaDeHerramientas* paleta;
    PanelDeValidacion* panel_validacion;
    QDockWidget* dock_paleta;
    QDockWidget* dock_validacion;
    QScrollArea* area_desplazamiento;
    QComboBox* combo_sectores;
    QLabel* etiqueta_coordenadas;
    QLabel* etiqueta_nombre_mundo;
    QLabel* etiqueta_dimensiones;

    void inicializar_interfaz();
    void inicializar_menu();
    void inicializar_barra_de_herramientas();
    void inicializar_barra_de_estado();
    void conectar_senales();
    void actualizar_sector_en_pantalla();
    void actualizar_etiqueta_dimensiones();
    void actualizar_etiqueta_nombre_mundo();
    void ajustar_ancho_paneles();
    bool validar_antes_de_guardar();
    MapaEditable* obtener_mapa_activo();

private slots:
    void nuevo_mundo();
    void abrir_mundo();
    void guardar_mundo();
    void guardar_mundo_como();
    void cambiar_sector(int indice);
    void aumentar_zoom();
    void disminuir_zoom();
    void actualizar_coordenadas(int columna, int fila);
    void ejecutar_validacion();
    void abrir_dialogo_redimensionar();
    void generar_muro_perimetral_granja();

public:
    explicit VentanaPrincipalEditor(QWidget* padre = nullptr);
};

#endif  // VENTANA_PRINCIPAL_EDITOR_H
