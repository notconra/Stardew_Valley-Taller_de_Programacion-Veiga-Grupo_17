#ifndef LIENZO_DE_EDICION_H
#define LIENZO_DE_EDICION_H

#include <QWidget>

#include "capa_del_mapa.h"
#include "mapa_editable.h"
#include "tipo_herramienta_editor.h"

class LienzoDeEdicion : public QWidget {
    Q_OBJECT

private:
    MapaEditable* mapa_actual;
    SectorActual sector_actual;
    TipoCapa capa_activa;
    int tamano_casilla_px;
    ModoEditor modo_actual;
    TipoTerrenoPaleta terreno_actual;
    TipoEntidad entidad_a_colocar;

    int identificador_entidad_seleccionada;
    bool arrastrando_entidad;
    int columna_arrastre_actual;
    int fila_arrastre_actual;

    int columna_hover;
    int fila_hover;
    bool mouse_presionado;

    void obtener_coordenadas_desde_posicion(const QPoint& posicion, int& columna, int& fila) const;
    void pintar_casilla_en_posicion(int columna, int fila);

protected:
    void paintEvent(QPaintEvent* evento) override;
    void mousePressEvent(QMouseEvent* evento) override;
    void mouseMoveEvent(QMouseEvent* evento) override;
    void mouseReleaseEvent(QMouseEvent* evento) override;
    void leaveEvent(QEvent* evento) override;

public:
    explicit LienzoDeEdicion(QWidget* padre = nullptr);

    void establecer_mapa(MapaEditable* mapa, SectorActual sector);
    void establecer_tamano_casilla(int nuevo_tamano);
    int obtener_tamano_casilla() const;
    void actualizar_tamano_del_lienzo();

    void establecer_modo(ModoEditor nuevo_modo);
    void establecer_terreno(TipoTerrenoPaleta nuevo_terreno);
    void establecer_entidad_a_colocar(TipoEntidad nueva_entidad);
    void establecer_capa_activa(TipoCapa nueva_capa);
    TipoCapa obtener_capa_activa() const;

    void deseleccionar_entidad();
    bool eliminar_entidad_seleccionada();
    int obtener_identificador_entidad_seleccionada() const;

signals:
    void casilla_apuntada(int columna, int fila);
    void mapa_modificado();
    void entidad_seleccionada_cambio(int identificador_entidad);
};

#endif  // LIENZO_DE_EDICION_H
