#ifndef RECURSOS_UI_H
#define RECURSOS_UI_H

#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <vector>

// Gestiona y centraliza los sprites originales de la UI extraídos de Cursors.png
// (dado, botón OK, flechas, símbolos de género y tipos de granja), asegurando que se
// recorten y escalen manteniendo el estilo pixel-art nítido.
class RecursosUI {
public:
    struct RegionSprite {
        int x;
        int y;
        int ancho;
        int alto;
    };

    struct InfoTipoGranja {
        int id;
        QString nombre;
        QString mapa_id;
        QString descripcion;
    };

    static RecursosUI& instancia();

    QPixmap icono_dado() const;
    QPixmap boton_ok() const;
    QPixmap boton_eliminar() const;
    QPixmap boton_eliminar_hover() const;
    QPixmap icono_reloj() const;
    QPixmap icono_masculino() const;
    QPixmap icono_femenino() const;
    QPixmap flecha_izquierda() const;
    QPixmap flecha_derecha() const;

    QPixmap icono_granja(int indice) const;

    void configurar_boton_dado(QPushButton* boton);
    void configurar_boton_ok(QPushButton* boton);
    void configurar_boton_eliminar(QPushButton* boton);
    void configurar_boton_masculino(QPushButton* boton);
    void configurar_boton_femenino(QPushButton* boton);
    void configurar_boton_flecha_izquierda(QPushButton* boton);
    void configurar_boton_flecha_derecha(QPushButton* boton);
    void configurar_boton_granja(QPushButton* boton, int indice);

    void configurar_boton_icono(QPushButton* boton, const QPixmap& pixmap);

    static const std::vector<InfoTipoGranja>& obtener_tipos_granja();

private:
    RecursosUI();

    QPixmap recortar_y_escalar(
        const QPixmap& hoja, const RegionSprite& region, int ancho_destino, int alto_destino) const;

    QPixmap hoja_cursors;
    QPixmap sprite_dado;
    QPixmap sprite_ok;
    QPixmap sprite_eliminar;
    QPixmap sprite_eliminar_hover;
    QPixmap sprite_reloj;
    QPixmap sprite_masculino;
    QPixmap sprite_femenino;
    QPixmap sprite_flecha_izq;
    QPixmap sprite_flecha_der;
    std::vector<QPixmap> sprite_iconos_granja;
};

#endif  // RECURSOS_UI_H
