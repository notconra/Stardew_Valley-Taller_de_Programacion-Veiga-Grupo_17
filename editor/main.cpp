#include <QApplication>
#include <QFile>
#include <QString>
#include <QTimer>

#include "ui/ventana_principal_editor.h"

int main(int cantidad_argumentos, char* argumentos[]) {
    QApplication aplicacion(cantidad_argumentos, argumentos);

    QFile archivo_estilos("editor/ui/estilos_editor.qss");
    if (!archivo_estilos.exists()) {
        archivo_estilos.setFileName("../editor/ui/estilos_editor.qss");
    }
    if (archivo_estilos.open(QIODevice::ReadOnly | QIODevice::Text)) {
        aplicacion.setStyleSheet(QString::fromUtf8(archivo_estilos.readAll()));
    }

    VentanaPrincipalEditor ventana;
    ventana.show();
    QTimer::singleShot(50, &ventana, &QWidget::showMaximized);

    return aplicacion.exec();
}
