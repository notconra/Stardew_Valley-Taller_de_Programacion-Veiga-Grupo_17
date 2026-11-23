#include "paleta_de_herramientas.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QString>
#include <QVBoxLayout>
#include <cstdint>
#include <vector>

#include "catalogo_de_estampas.h"
#include "catalogo_de_terrenos.h"
#include "tipo_herramienta_editor.h"

static constexpr int TAMANO_BOTON_TILE_PX = 38;
static constexpr int TAMANO_ICONO_TILE_PX = 32;
static constexpr int COLUMNAS_GRILLA_TILES = 3;
static constexpr int MARGEN_LAYOUT_PX = 8;
static constexpr int ESPACIADO_LAYOUT_PX = 8;
static constexpr int ESPACIADO_MODOS_PX = 4;
static constexpr int ESPACIADO_TILES_PX = 4;
static constexpr int ESPACIADO_ESTAMPAS_PX = 4;

static constexpr const char* ESTILO_BOTON_MODO =
    "QPushButton { background-color: #383838; color: #EEEEEE; font-weight: bold; padding: 6px; "
    "border-radius: 4px; border: 1px solid #555555; }"
    "QPushButton:hover { background-color: #4A4A4A; }"
    "QPushButton:checked { background-color: #2E6B38; color: white; border: 1px solid #4CAF50; }";

static constexpr const char* ESTILO_BOTON_TILE_CUADRO =
    "QPushButton { background-color: #2D2D2D; border: 1px solid #444444; border-radius: 4px; "
    "padding: 2px; }"
    "QPushButton:hover { background-color: #3D3D3D; border: 1px solid #888888; }"
    "QPushButton:checked { background-color: #2E6B38; border: 2px solid #55FF55; }";

static constexpr const char* ESTILO_BOTON_ESTAMPA =
    "QPushButton { background-color: #2B3D4F; color: white; font-weight: bold; padding: 5px 8px; "
    "border-radius: 4px; border: 1px solid #3A536B; text-align: left; font-size: 11px; }"
    "QPushButton:hover { background-color: #385068; }"
    "QPushButton:checked { background-color: #1E6B9B; border: 2px solid #00D2FF; }";

static constexpr const char* ESTILO_BOTON_PERIMETRO =
    "QPushButton { background-color: #5A3A1A; color: white; font-weight: bold; padding: 6px; "
    "border-radius: 4px; border: 1px solid #7A4F23; font-size: 11px; }"
    "QPushButton:hover { background-color: #7A4F23; }";

PaletaDeHerramientas::PaletaDeHerramientas(QWidget* padre)
    : QWidget(padre),
      layout_principal(nullptr),
      grupo_modos(nullptr),
      boton_modo_terreno(nullptr),
      boton_modo_estampa(nullptr),
      boton_modo_colocar(nullptr),
      boton_modo_mover(nullptr),
      boton_modo_borrar(nullptr),
      grupo_tiles(nullptr),
      layout_tiles(nullptr),
      botones_tiles(),
      caja_estampas(nullptr),
      grupo_estampas(nullptr),
      layout_estampas(nullptr),
      botones_estampas(),
      boton_muro_perimetral(nullptr) {
    layout_principal = new QVBoxLayout(this);
    layout_principal->setContentsMargins(
        MARGEN_LAYOUT_PX,
        MARGEN_LAYOUT_PX,
        MARGEN_LAYOUT_PX,
        MARGEN_LAYOUT_PX);
    layout_principal->setSpacing(ESPACIADO_LAYOUT_PX);

    // 1. Grupo de Modos de Edición
    QGroupBox* caja_modos = new QGroupBox("Modo de Edicion", this);
    QVBoxLayout* layout_modos = new QVBoxLayout(caja_modos);
    layout_modos->setSpacing(ESPACIADO_MODOS_PX);
    grupo_modos = new QButtonGroup(this);

    boton_modo_terreno = crear_boton_modo("Pintar Superficie", ModoEditor::TERRENO);
    boton_modo_estampa = crear_boton_modo("Estampar Bloque", ModoEditor::ESTAMPAR);
    boton_modo_colocar = crear_boton_modo("Colocar Entidad", ModoEditor::COLOCAR_ENTIDAD);
    boton_modo_mover = crear_boton_modo("Mover Entidad", ModoEditor::SELECCIONAR_MOVER);
    boton_modo_borrar = crear_boton_modo("Borrar Entidad", ModoEditor::BORRAR);

    boton_modo_terreno->setChecked(true);
    layout_modos->addWidget(boton_modo_terreno);
    layout_modos->addWidget(boton_modo_estampa);
    layout_modos->addWidget(boton_modo_colocar);
    layout_modos->addWidget(boton_modo_mover);
    layout_modos->addWidget(boton_modo_borrar);
    layout_principal->addWidget(caja_modos);

    // 2. Grupo de Superficies del Sector (Tiles continuos)
    QGroupBox* caja_tiles = new QGroupBox("Superficies (Tiles)", this);
    layout_tiles = new QGridLayout(caja_tiles);
    layout_tiles->setContentsMargins(0, ESPACIADO_TILES_PX, 0, ESPACIADO_TILES_PX);
    layout_tiles->setSpacing(ESPACIADO_TILES_PX);

    grupo_tiles = new QButtonGroup(this);
    layout_principal->addWidget(caja_tiles);

    // 3. Grupo de Estampas y Bloques Prefabricados
    caja_estampas = new QGroupBox("Bloques y Estanques", this);
    layout_estampas = new QVBoxLayout(caja_estampas);
    layout_estampas->setSpacing(ESPACIADO_ESTAMPAS_PX);

    grupo_estampas = new QButtonGroup(this);

    boton_muro_perimetral = new QPushButton("Cerrar Perimetro con Vallas", caja_estampas);
    boton_muro_perimetral->setStyleSheet(ESTILO_BOTON_PERIMETRO);
    connect(
        boton_muro_perimetral,
        &QPushButton::clicked,
        this,
        &PaletaDeHerramientas::generar_perimetro_solicitado);
    layout_estampas->addWidget(boton_muro_perimetral);

    layout_principal->addWidget(caja_estampas);
    layout_principal->addStretch();

    actualizar_sector(SectorActual::GRANJA);
}

QPushButton* PaletaDeHerramientas::crear_boton_modo(const QString& texto, ModoEditor modo) {
    QPushButton* boton = new QPushButton(texto, this);
    boton->setCheckable(true);
    boton->setStyleSheet(ESTILO_BOTON_MODO);
    grupo_modos->addButton(boton, static_cast<int>(modo));
    connect(boton, &QPushButton::clicked, this, [this, modo]() { emit modo_cambiado(modo); });
    return boton;
}

void PaletaDeHerramientas::limpiar_botones_de_tiles() {
    size_t indice = 0;
    while (indice < botones_tiles.size()) {
        QPushButton* boton = botones_tiles[indice];
        grupo_tiles->removeButton(boton);
        layout_tiles->removeWidget(boton);
        delete boton;
        indice++;
    }
    botones_tiles.clear();
}

void PaletaDeHerramientas::limpiar_botones_de_estampas() {
    size_t indice = 0;
    while (indice < botones_estampas.size()) {
        QPushButton* boton = botones_estampas[indice];
        grupo_estampas->removeButton(boton);
        layout_estampas->removeWidget(boton);
        delete boton;
        indice++;
    }
    botones_estampas.clear();
}

void PaletaDeHerramientas::actualizar_sector(SectorActual sector) {
    limpiar_botones_de_tiles();
    limpiar_botones_de_estampas();

    // 1. Cargar tiles de superficie
    DefinicionTilesetTMX tileset = CatalogoDeTerrenos::obtener_tileset_sector(sector);
    QPixmap pixmap_tileset(QString::fromStdString(tileset.ruta_imagen));

    const std::vector<DefinicionTile>& catalogo_tiles =
        CatalogoDeTerrenos::obtener_catalogo_por_sector(sector);

    size_t indice_tile = 0;
    while (indice_tile < catalogo_tiles.size()) {
        const DefinicionTile& definicion = catalogo_tiles[indice_tile];
        uint32_t indice_local =
            definicion.identificador_tile_global - tileset.primer_identificador_global;
        int columna_sprite =
            static_cast<int>(indice_local % static_cast<uint32_t>(tileset.cantidad_de_columnas));
        int fila_sprite =
            static_cast<int>(indice_local / static_cast<uint32_t>(tileset.cantidad_de_columnas));

        QPixmap sprite_recortado = pixmap_tileset.copy(
            columna_sprite * tileset.ancho_tile,
            fila_sprite * tileset.alto_tile,
            tileset.ancho_tile,
            tileset.alto_tile);

        QPushButton* boton = new QPushButton(this);
        boton->setFixedSize(TAMANO_BOTON_TILE_PX, TAMANO_BOTON_TILE_PX);
        boton->setCheckable(true);
        boton->setStyleSheet(ESTILO_BOTON_TILE_CUADRO);
        boton->setToolTip(QString::fromStdString(definicion.nombre));
        boton->setIcon(QIcon(sprite_recortado.scaled(
            TAMANO_ICONO_TILE_PX,
            TAMANO_ICONO_TILE_PX,
            Qt::KeepAspectRatio,
            Qt::FastTransformation)));
        boton->setIconSize(QSize(TAMANO_ICONO_TILE_PX, TAMANO_ICONO_TILE_PX));

        grupo_tiles->addButton(boton, static_cast<int>(indice_tile));

        int fila_grilla = static_cast<int>(indice_tile / COLUMNAS_GRILLA_TILES);
        int columna_grilla = static_cast<int>(indice_tile % COLUMNAS_GRILLA_TILES);
        layout_tiles->addWidget(boton, fila_grilla, columna_grilla);

        uint32_t id_global = definicion.identificador_tile_global;
        uint8_t prop = definicion.propiedades;
        connect(boton, &QPushButton::clicked, this, [this, id_global, prop]() {
            boton_modo_terreno->setChecked(true);
            emit modo_cambiado(ModoEditor::TERRENO);
            emit tile_seleccionado(id_global, prop);
        });

        botones_tiles.push_back(boton);
        indice_tile++;
    }

    if (!botones_tiles.empty() && !catalogo_tiles.empty()) {
        botones_tiles[0]->setChecked(true);
        emit tile_seleccionado(
            catalogo_tiles[0].identificador_tile_global,
            catalogo_tiles[0].propiedades);
    }

    // 2. Cargar estampas y bloques
    std::vector<DefinicionEstampa> lista_estampas =
        CatalogoDeEstampas::listar_estampas_por_sector(sector);

    caja_estampas->setVisible(!lista_estampas.empty() || sector == SectorActual::GRANJA);
    boton_muro_perimetral->setVisible(sector == SectorActual::GRANJA);

    size_t indice_estampa = 0;
    while (indice_estampa < lista_estampas.size()) {
        const DefinicionEstampa& estampa = lista_estampas[indice_estampa];
        TipoEstampa tipo = estampa.tipo;
        QPushButton* boton_estampa =
            new QPushButton(QString::fromStdString(estampa.nombre), caja_estampas);
        boton_estampa->setCheckable(true);
        boton_estampa->setStyleSheet(ESTILO_BOTON_ESTAMPA);
        grupo_estampas->addButton(boton_estampa, static_cast<int>(tipo));
        layout_estampas->insertWidget(static_cast<int>(indice_estampa), boton_estampa);

        connect(boton_estampa, &QPushButton::clicked, this, [this, tipo]() {
            boton_modo_estampa->setChecked(true);
            emit modo_cambiado(ModoEditor::ESTAMPAR);
            emit estampa_seleccionada(tipo);
        });

        botones_estampas.push_back(boton_estampa);
        indice_estampa++;
    }
}
