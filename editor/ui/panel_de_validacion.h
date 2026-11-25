#ifndef PANEL_DE_VALIDACION_H
#define PANEL_DE_VALIDACION_H

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "tipo_entidad.h"
#include "validador_de_mundo.h"

class PanelDeValidacion : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout* layout_principal;

    QGroupBox* caja_entidades;
    QVBoxLayout* layout_botones_entidades;
    QButtonGroup* grupo_entidades;
    std::vector<QPushButton*> botones_entidades_actuales;
    QPushButton* boton_eliminar_entidad;

    QLabel* etiqueta_titulo;
    QLabel* etiqueta_estado_general;
    QListWidget* lista_errores;

    void inicializar_seccion_entidades();
    void limpiar_botones_entidades();

public:
    explicit PanelDeValidacion(QWidget* padre = nullptr);

    void actualizar_sector(SectorActual sector);
    void habilitar_boton_eliminar(bool habilitado);
    void actualizar_resultado(const ResultadoValidacion& resultado);

signals:
    void entidad_cambiada(TipoEntidad entidad);
    void eliminar_solicitado();
    void sector_solicitado(int indice_sector);
};

#endif  // PANEL_DE_VALIDACION_H
