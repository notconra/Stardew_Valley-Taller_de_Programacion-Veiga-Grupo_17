// Comprueba que Qt5 Widgets esta instalado y enlaza.
// Abre una ventana con un texto.
// Después de hacer make build o make test se corre con ./build/check_qt

#include <QApplication>
#include <QLabel>
#include <QTimer>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QLabel label("Qt funciona :p");
    label.resize(320, 120);
    label.setAlignment(Qt::AlignCenter);
    label.show();

    return app.exec();
}
