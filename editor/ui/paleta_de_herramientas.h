#ifndef PALETA_DE_HERRAMIENTAS_H
#define PALETA_DE_HERRAMIENTAS_H

#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdint>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "tipo_entidad.h"
#include "tipo_herramienta_editor.h"

class PaletaDeHerramientas : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout* layout_principal;
    QButtonGroup* grupo_modos;
    QButtonGroup* grupo_tiles;
    QWidget* contenedor_tiles;
    QVBoxLayout* layout_tiles;
    std::vector<QPushButton*> botones_tiles;

    QButtonGroup* grupo_entidades;
    QWidget* contenedor_entidades_granja;
    QWidget* contenedor_entidades_pueblo;
    QWidget* contenedor_entidades_caverna;
    QPushButton* boton_eliminar_entidad;

    QPushButton* crear_boton_modo(const QString& texto, ModoEditor modo);
    QPushButton* crear_boton_entidad(
        QWidget* contenedor, const QString& texto, TipoEntidad entidad);
    void inicializar_botones_de_tiles();
    void inicializar_botones_de_entidades();

public:
    explicit PaletaDeHerramientas(QWidget* padre = nullptr);

    void actualizar_sector(SectorActual sector);
    void habilitar_boton_eliminar(bool habilitado);

signals:
    void modo_cambiado(ModoEditor modo);
    void tile_seleccionado(uint32_t id_tile, uint8_t propiedades);
    void entidad_cambiada(TipoEntidad entidad);
    void eliminar_solicitado();
};

#endif  // PALETA_DE_HERRAMIENTAS_H
