#include <QApplication>
#include <QString>

#include "ui/ventana_principal_editor.h"

int main(int cantidad_argumentos, char* argumentos[]) {
    QApplication aplicacion(cantidad_argumentos, argumentos);

    // Estilo visual moderno y oscuro para el editor
    QString estilo =
        "QMainWindow { background-color: #2D2D2D; color: #FFFFFF; }"
        "QWidget { background-color: #2D2D2D; color: #EEEEEE; font-family: sans-serif; }"
        "QDockWidget { titlebar-close-icon: none; titlebar-normal-icon: none; font-weight: bold; }"
        "QDockWidget::title { background: #383838; padding: 6px; border-radius: 3px; }"
        "QGroupBox { border: 1px solid #4A4A4A; border-radius: 4px; margin-top: 10px; font-weight: "
        "bold; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 "
        "4px; color: #A0D468; }"
        "QToolBar { background: #333333; border-bottom: 1px solid #444444; spacing: 6px; padding: "
        "4px; }"
        "QMenuBar { background-color: #242424; color: #EEEEEE; border-bottom: 1px solid #3A3A3A; }"
        "QMenuBar::item:selected { background-color: #3A6B35; }"
        "QMenu { background-color: #2E2E2E; color: #EEEEEE; border: 1px solid #444444; }"
        "QMenu::item:selected { background-color: #3A6B35; }"
        "QStatusBar { background-color: #242424; color: #BBBBBB; border-top: 1px solid #3A3A3A; }"
        "QListWidget { background-color: #1F1F1F; border: 1px solid #3E3E3E; border-radius: 4px; "
        "padding: 4px; }"
        "QComboBox { background-color: #3C3C3C; border: 1px solid #555555; border-radius: 4px; "
        "padding: 4px 8px; color: white; }"
        "QPushButton { background-color: #3E3E3E; border: 1px solid #555555; border-radius: 4px; "
        "padding: 5px 10px; color: white; }"
        "QPushButton:hover { background-color: #4E4E4E; }"
        "QPushButton:pressed { background-color: #2E2E2E; }";
    aplicacion.setStyleSheet(estilo);

    VentanaPrincipalEditor ventana;
    ventana.show();

    return aplicacion.exec();
}
