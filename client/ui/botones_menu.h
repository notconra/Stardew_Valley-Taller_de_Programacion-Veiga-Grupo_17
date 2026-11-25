#ifndef BOTONES_MENU_H
#define BOTONES_MENU_H

#include <QEvent>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <unordered_map>

// Configura los botones de la pantalla principal con los sprites pixel-art originales
// de Stardew Valley y maneja sus estados visuales (normal, resaltado al pasar el ratón,
// presionado).
class BotonesMenu : public QObject {
    Q_OBJECT

public:
    explicit BotonesMenu(QObject* padre = nullptr);

    // Configura los 4 botones principales con sus respectivos sprites y eventos.
    void configurar(
        QPushButton* boton_nueva_partida, QPushButton* boton_retomar_partida,
        QPushButton* boton_unirse_partida, QPushButton* boton_salir);

protected:
    bool eventFilter(QObject* objeto_observado, QEvent* evento) override;

private:
    struct EstadoBoton {
        QPixmap normal;
        QPixmap resaltado;
        QPixmap presionado;
    };

    std::unordered_map<QObject*, EstadoBoton> estados_botones;

    void configurar_boton(
        QPushButton* boton, const char* texto_de_ayuda, int columna_sprite,
        const QPixmap& hoja_de_botones);
};

#endif  // BOTONES_MENU_H
