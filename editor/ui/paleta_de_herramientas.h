#ifndef PALETA_DE_HERRAMIENTAS_H
#define PALETA_DE_HERRAMIENTAS_H

#include <QButtonGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <vector>

#include "tipo_entidad.h"
#include "tipo_herramienta_editor.h"

class PaletaDeHerramientas : public QWidget {
    Q_OBJECT

private:
    QVBoxLayout* layout_principal;
    QButtonGroup* grupo_modos;
    QButtonGroup* grupo_terrenos;
    std::vector<QPushButton*> botones_terrenos;
    QButtonGroup* grupo_entidades;
    QWidget* contenedor_entidades;
    QVBoxLayout* layout_entidades;
    QPushButton* boton_eliminar_entidad;

    QPushButton* crear_boton_modo(const QString& texto, ModoEditor modo);
    QPushButton* crear_boton_terreno(
        const QString& texto, TipoTerrenoPaleta terreno, const QString& color_hex);
    QPushButton* crear_boton_entidad(const QString& texto, TipoEntidad entidad);
    void configurar_estilo_boton_terreno(
        QPushButton* boton, const QString& texto, const QString& color_hex);

    void limpiar_botones_de_entidades();

public:
    explicit PaletaDeHerramientas(QWidget* padre = nullptr);

    void actualizar_sector(SectorActual sector);
    void habilitar_boton_eliminar(bool habilitado);

signals:
    void modo_cambiado(ModoEditor modo);
    void terreno_cambiado(TipoTerrenoPaleta terreno);
    void entidad_cambiada(TipoEntidad entidad);
    void eliminar_solicitado();
};

#endif  // PALETA_DE_HERRAMIENTAS_H
