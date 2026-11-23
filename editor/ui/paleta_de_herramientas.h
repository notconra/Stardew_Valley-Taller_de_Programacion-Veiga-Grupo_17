#ifndef PALETA_DE_HERRAMIENTAS_H
#define PALETA_DE_HERRAMIENTAS_H

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdint>
#include <vector>

#include "catalogo_de_estampas.h"
#include "catalogo_de_terrenos.h"
#include "tipo_herramienta_editor.h"

class PaletaDeHerramientas : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout* layout_principal;
    QButtonGroup* grupo_modos;
    QPushButton* boton_modo_terreno;
    QPushButton* boton_modo_estampa;
    QPushButton* boton_modo_colocar;
    QPushButton* boton_modo_mover;
    QPushButton* boton_modo_borrar;

    QButtonGroup* grupo_tiles;
    QGridLayout* layout_tiles;
    std::vector<QPushButton*> botones_tiles;

    QGroupBox* caja_estampas;
    QButtonGroup* grupo_estampas;
    QVBoxLayout* layout_estampas;
    std::vector<QPushButton*> botones_estampas;
    QPushButton* boton_muro_perimetral;

    QPixmap pixmap_tileset;
    DefinicionTilesetTMX datos_tileset;
    TipoEstampa estampa_recto_activa;
    TipoEstampa estampa_union_t_activa;
    TipoEstampa estampa_esquina_activa;
    TipoEstampa estampa_mostrador_activa;
    TipoEstampa estampa_puesto_mercado_activa;
    TipoEstampa estampa_estanteria_activa;
    uint32_t id_tile_borde_activo;
    uint32_t id_tile_esquina_exterior_activa;
    uint32_t id_tile_esquina_interior_activa;

    QPushButton* crear_boton_modo(const QString& texto, ModoEditor modo);
    void limpiar_botones_de_tiles();
    void limpiar_botones_de_estampas();

public:
    explicit PaletaDeHerramientas(QWidget* padre = nullptr);

    void actualizar_sector(SectorActual sector);
    void sincronizar_rotacion_estampa(TipoEstampa estampa);
    void sincronizar_rotacion_tile(uint32_t id_tile);

signals:
    void modo_cambiado(ModoEditor modo);
    void tile_seleccionado(uint32_t id_tile, uint8_t propiedades);
    void estampa_seleccionada(TipoEstampa estampa);
    void generar_perimetro_solicitado();
};

#endif  // PALETA_DE_HERRAMIENTAS_H
