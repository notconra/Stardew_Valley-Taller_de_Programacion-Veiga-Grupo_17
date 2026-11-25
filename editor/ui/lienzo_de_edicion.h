#ifndef LIENZO_DE_EDICION_H
#define LIENZO_DE_EDICION_H

#include <QPixmap>
#include <QRect>
#include <QWidget>
#include <cstdint>
#include <map>
#include <vector>

#include "capa_del_mapa.h"
#include "catalogo_de_estampas.h"
#include "catalogo_de_terrenos.h"
#include "mapa_editable.h"
#include "tipo_herramienta_editor.h"

struct DefinicionVisualEntidad {
    const QPixmap* pixmap;
    QRect rectangulo_origen;
};

struct EntradaTilesetLienzo {
    QPixmap pixmap;
    uint32_t primer_identificador_global;
    int cantidad_de_columnas;
    int ancho_tile;
    int alto_tile;
};

class LienzoDeEdicion : public QWidget {
    Q_OBJECT

private:
    MapaEditable* mapa_actual;
    SectorActual sector_actual;
    int tamano_casilla_px;
    ModoEditor modo_actual;
    uint32_t id_tile_actual;
    uint8_t propiedades_actuales;
    TipoEntidad entidad_a_colocar;
    TipoEstampa estampa_a_colocar;
    Direccion orientacion_entidad_a_colocar;

    QPixmap pixmap_tileset;
    DefinicionTilesetTMX datos_tileset;
    std::vector<EntradaTilesetLienzo> lista_tilesets_actuales;

    QPixmap pixmap_cabania;
    QPixmap pixmap_caseta_mina;
    QPixmap pixmap_exteriores_entidades;
    QPixmap pixmap_caverna_entidades;
    QPixmap pixmap_pueblo_entidades;
    QPixmap pixmap_mercado;
    QPixmap pixmap_retrato_marnie;
    QPixmap pixmap_retrato_clint;
    QPixmap pixmap_retrato_pierre;
    QPixmap pixmap_retrato_robin;

    std::map<TipoEntidad, DefinicionVisualEntidad> mapa_visual_entidades;

    int identificador_entidad_seleccionada;
    bool arrastrando_entidad;
    int columna_arrastre_actual;
    int fila_arrastre_actual;

    int columna_hover;
    int fila_hover;
    bool mouse_presionado;

    void inicializar_catalogo_visual_entidades();
    void cargar_tilesets_de_sector(SectorActual sector);
    void dibujar_tile_en_lienzo(QPainter& pintor, uint32_t valor_crudo, int columna, int fila);
    void obtener_coordenadas_desde_posicion(const QPoint& posicion, int& columna, int& fila) const;
    int obtener_margen_visible() const;
    bool asegurar_espacio_en_caverna(int& columna, int& fila, int ancho, int alto);
    void borrar_en_casilla(int columna, int fila);
    bool casilla_es_perimetro_protegido(int columna, int fila) const;
    void pintar_casilla_en_posicion(int columna, int fila);
    void dibujar_capa_de_tiles(
        QPainter& pintor, TipoCapa tipo_capa, int cantidad_columnas, int cantidad_filas);
    void dibujar_visual_entidad(
        QPainter& pintor, TipoEntidad tipo_entidad, int origen_columna, int origen_fila,
        const QRect& rectangulo_destino);
    void dibujar_entidad(
        QPainter& pintor, const EntidadDelMapa& entidad, const QRect& rectangulo_entidad,
        bool es_seleccionada);

protected:
    void paintEvent(QPaintEvent* evento) override;
    void mousePressEvent(QMouseEvent* evento) override;
    void mouseMoveEvent(QMouseEvent* evento) override;
    void mouseReleaseEvent(QMouseEvent* evento) override;
    void leaveEvent(QEvent* evento) override;
    void keyPressEvent(QKeyEvent* evento) override;

public:
    static constexpr const char* RUTA_SPRITE_CABANIA = "Assets/Buildings/houses.png";
    static constexpr const char* RUTA_SPRITE_CASETA_MINA = "Assets/Buildings/Stone Cabin.png";
    static constexpr const char* RUTA_SPRITE_EXTERIORES =
        "Assets/Maps/spring_outdoorsTileSheet.png";
    static constexpr const char* RUTA_SPRITE_CAVERNA = "Assets/Maps/Mines/mine.png";
    static constexpr const char* RUTA_SPRITE_PUEBLO = "Assets/Maps/spring_town.png";
    static constexpr const char* RUTA_RETRATO_MARNIE = "Assets/Portraits/Marnie.png";
    static constexpr const char* RUTA_RETRATO_CLINT = "Assets/Portraits/Clint.png";
    static constexpr const char* RUTA_RETRATO_PIERRE = "Assets/Portraits/Pierre.png";
    static constexpr const char* RUTA_RETRATO_ROBIN = "Assets/Portraits/Robin.png";

    explicit LienzoDeEdicion(QWidget* padre = nullptr);

    void establecer_mapa(MapaEditable* mapa, SectorActual sector);
    void establecer_tamano_casilla(int nuevo_tamano);
    int obtener_tamano_casilla() const;
    void actualizar_tamano_del_lienzo();

    void establecer_modo(ModoEditor nuevo_modo);
    void establecer_tile(uint32_t nuevo_id_tile, uint8_t nuevas_propiedades);
    void establecer_estampa_a_colocar(TipoEstampa nueva_estampa);
    void establecer_entidad_a_colocar(TipoEntidad nueva_entidad);

    void deseleccionar_entidad();
    bool eliminar_entidad_seleccionada();
    int obtener_identificador_entidad_seleccionada() const;

    void rotar_elemento_actual();
    TipoEstampa obtener_estampa_a_colocar() const;
    uint32_t obtener_id_tile_actual() const;

signals:
    void casilla_apuntada(int columna, int fila);
    void mapa_modificado();
    void mapa_expandido(int columnas_agregadas_izquierda, int filas_agregadas_arriba);
    void entidad_seleccionada_cambio(int identificador_entidad);
    void estampa_rotada(TipoEstampa nueva_estampa);
    void tile_rotado(uint32_t nuevo_id_tile);
};

#endif  // LIENZO_DE_EDICION_H
