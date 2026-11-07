#ifndef RECURSOS_UI_H
#define RECURSOS_UI_H

#include <QIcon>
#include <QPixmap>
#include <QPushButton>

// Gestiona y centraliza los sprites originales de la UI extraídos de Cursors.png
// (dado, botón OK, flechas y símbolos de género), asegurando que se recorten
// y escalen manteniendo el estilo pixel-art nítido.
class RecursosUI {
public:
    struct RegionSprite {
        int x;
        int y;
        int ancho;
        int alto;
    };

    static RecursosUI& instancia();

    QPixmap icono_dado() const {
        return sprite_dado;
    }
    QPixmap boton_ok() const {
        return sprite_ok;
    }
    QPixmap icono_masculino() const {
        return sprite_masculino;
    }
    QPixmap icono_femenino() const {
        return sprite_femenino;
    }
    QPixmap flecha_izquierda() const {
        return sprite_flecha_izq;
    }
    QPixmap flecha_derecha() const {
        return sprite_flecha_der;
    }

    void configurar_boton_dado(QPushButton* boton);
    void configurar_boton_ok(QPushButton* boton);
    void configurar_boton_masculino(QPushButton* boton);
    void configurar_boton_femenino(QPushButton* boton);
    void configurar_boton_flecha_izquierda(QPushButton* boton);
    void configurar_boton_flecha_derecha(QPushButton* boton);

    void configurar_boton_icono(QPushButton* boton, const QPixmap& pixmap);

private:
    RecursosUI();

    QPixmap recortar_y_escalar(
        const RegionSprite& region, int ancho_destino, int alto_destino) const;

    QPixmap hoja_cursors;
    QPixmap sprite_dado;
    QPixmap sprite_ok;
    QPixmap sprite_masculino;
    QPixmap sprite_femenino;
    QPixmap sprite_flecha_izq;
    QPixmap sprite_flecha_der;
};

#endif  // RECURSOS_UI_H
