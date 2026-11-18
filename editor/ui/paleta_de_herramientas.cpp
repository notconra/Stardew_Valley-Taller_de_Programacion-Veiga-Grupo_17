#include "paleta_de_herramientas.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QIcon>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QVBoxLayout>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "catalogo_de_terrenos.h"
#include "tipo_entidad.h"
#include "tipo_herramienta_editor.h"

static constexpr int TAMANO_ICONO_TILE_PX = 24;
static constexpr size_t CANTIDAD_TILES_POR_SECTOR = 5;

static constexpr const char* ESTILO_BOTON_MODO =
    "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; border-radius: 4px; }"
    "QPushButton:checked { background-color: #2E6B38; color: white; font-weight: bold; }";

static constexpr const char* ESTILO_BOTON_TILE =
    "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; border-radius: 4px; }"
    "QPushButton:checked { background-color: #2E6B38; color: white; font-weight: bold; }";

static constexpr const char* ESTILO_BOTON_ENTIDAD =
    "QPushButton { text-align: left; padding: 6px 10px; font-size: 13px; border-radius: 4px; }"
    "QPushButton:checked { background-color: #4A6E82; color: white; font-weight: bold; }";

static constexpr const char* ESTILO_BOTON_ELIMINAR =
    "QPushButton { background-color: #B22222; color: white; font-weight: bold; padding: 6px; "
    "border-radius: 4px; }"
    "QPushButton:hover { background-color: #DC143C; }"
    "QPushButton:disabled { background-color: #555555; color: #888888; }";

PaletaDeHerramientas::PaletaDeHerramientas(QWidget* padre)
    : QWidget(padre),
      layout_principal(nullptr),
      grupo_modos(nullptr),
      grupo_tiles(nullptr),
      contenedor_tiles(nullptr),
      layout_tiles(nullptr),
      botones_tiles(),
      grupo_entidades(nullptr),
      contenedor_entidades_granja(nullptr),
      contenedor_entidades_pueblo(nullptr),
      contenedor_entidades_caverna(nullptr),
      boton_eliminar_entidad(nullptr) {
    layout_principal = new QVBoxLayout(this);
    layout_principal->setContentsMargins(8, 8, 8, 8);
    layout_principal->setSpacing(10);

    // 1. Grupo de Modos
    QGroupBox* caja_modos = new QGroupBox("Modo de Edicion", this);
    QVBoxLayout* layout_modos = new QVBoxLayout(caja_modos);
    grupo_modos = new QButtonGroup(this);

    QPushButton* boton_terreno = crear_boton_modo("Pintar Tile", ModoEditor::TERRENO);
    QPushButton* boton_colocar = crear_boton_modo("Colocar Entidad", ModoEditor::COLOCAR_ENTIDAD);
    QPushButton* boton_mover = crear_boton_modo("Mover Entidad", ModoEditor::SELECCIONAR_MOVER);
    QPushButton* boton_borrar = crear_boton_modo("Borrar Entidad", ModoEditor::BORRAR);

    boton_terreno->setChecked(true);
    layout_modos->addWidget(boton_terreno);
    layout_modos->addWidget(boton_colocar);
    layout_modos->addWidget(boton_mover);
    layout_modos->addWidget(boton_borrar);
    layout_principal->addWidget(caja_modos);

    // 2. Grupo de Tiles del Sector
    QGroupBox* caja_tiles = new QGroupBox("Tiles del Sector", this);
    QVBoxLayout* layout_caja_tiles = new QVBoxLayout(caja_tiles);
    contenedor_tiles = new QWidget(caja_tiles);
    layout_tiles = new QVBoxLayout(contenedor_tiles);
    layout_tiles->setContentsMargins(0, 0, 0, 0);
    layout_caja_tiles->addWidget(contenedor_tiles);
    grupo_tiles = new QButtonGroup(this);
    layout_principal->addWidget(caja_tiles);
    inicializar_botones_de_tiles();

    // 3. Grupo de Entidades
    QGroupBox* caja_entidades = new QGroupBox("Entidades del Sector", this);
    QVBoxLayout* layout_caja_entidades = new QVBoxLayout(caja_entidades);
    grupo_entidades = new QButtonGroup(this);

    contenedor_entidades_granja = new QWidget(caja_entidades);
    contenedor_entidades_pueblo = new QWidget(caja_entidades);
    contenedor_entidades_caverna = new QWidget(caja_entidades);
    layout_caja_entidades->addWidget(contenedor_entidades_granja);
    layout_caja_entidades->addWidget(contenedor_entidades_pueblo);
    layout_caja_entidades->addWidget(contenedor_entidades_caverna);
    inicializar_botones_de_entidades();

    boton_eliminar_entidad = new QPushButton("Eliminar Seleccionada", caja_entidades);
    boton_eliminar_entidad->setStyleSheet(ESTILO_BOTON_ELIMINAR);
    boton_eliminar_entidad->setEnabled(false);
    connect(
        boton_eliminar_entidad,
        &QPushButton::clicked,
        this,
        &PaletaDeHerramientas::eliminar_solicitado);
    layout_caja_entidades->addWidget(boton_eliminar_entidad);

    layout_principal->addWidget(caja_entidades);
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

QPushButton* PaletaDeHerramientas::crear_boton_entidad(
    QWidget* contenedor, const QString& texto, TipoEntidad entidad) {
    QPushButton* boton = new QPushButton(texto, contenedor);
    boton->setCheckable(true);
    boton->setStyleSheet(ESTILO_BOTON_ENTIDAD);
    grupo_entidades->addButton(boton, static_cast<int>(entidad));
    connect(boton, &QPushButton::clicked, this, [this, entidad]() {
        emit entidad_cambiada(entidad);
    });
    return boton;
}

void PaletaDeHerramientas::inicializar_botones_de_tiles() {
    size_t indice = 0;
    while (indice < CANTIDAD_TILES_POR_SECTOR) {
        QPushButton* boton = new QPushButton(contenedor_tiles);
        boton->setCheckable(true);
        boton->setStyleSheet(ESTILO_BOTON_TILE);
        boton->setIconSize(QSize(TAMANO_ICONO_TILE_PX, TAMANO_ICONO_TILE_PX));
        grupo_tiles->addButton(boton, static_cast<int>(indice));
        layout_tiles->addWidget(boton);
        botones_tiles.push_back(boton);
        indice++;
    }
}

void PaletaDeHerramientas::inicializar_botones_de_entidades() {
    QVBoxLayout* layout_granja = new QVBoxLayout(contenedor_entidades_granja);
    layout_granja->setContentsMargins(0, 0, 0, 0);
    layout_granja->addWidget(crear_boton_entidad(
        contenedor_entidades_granja,
        "Cabania del Jugador (3x3)",
        TipoEntidad::CABANIA));
    layout_granja->addWidget(crear_boton_entidad(
        contenedor_entidades_granja,
        "Entrada Caverna (2x2)",
        TipoEntidad::ENTRADA_CAVERNA));
    layout_granja->addWidget(crear_boton_entidad(
        contenedor_entidades_granja,
        "Salida a Pueblo (2x2)",
        TipoEntidad::TRANSICION_PUEBLO));

    QVBoxLayout* layout_pueblo = new QVBoxLayout(contenedor_entidades_pueblo);
    layout_pueblo->setContentsMargins(0, 0, 0, 0);
    layout_pueblo->addWidget(crear_boton_entidad(
        contenedor_entidades_pueblo,
        "Puesto Marnie (2x2)",
        TipoEntidad::PUESTO_MARNIE));
    layout_pueblo->addWidget(crear_boton_entidad(
        contenedor_entidades_pueblo,
        "Puesto Clint (2x2)",
        TipoEntidad::PUESTO_CLINT));
    layout_pueblo->addWidget(crear_boton_entidad(
        contenedor_entidades_pueblo,
        "Puesto Pierre (2x2)",
        TipoEntidad::PUESTO_PIERRE));
    layout_pueblo->addWidget(crear_boton_entidad(
        contenedor_entidades_pueblo,
        "Puesto Robin (2x2)",
        TipoEntidad::PUESTO_ROBIN));
    layout_pueblo->addWidget(crear_boton_entidad(
        contenedor_entidades_pueblo,
        "Centro Comunitario (4x3)",
        TipoEntidad::CENTRO_COMUNITARIO));
    layout_pueblo->addWidget(crear_boton_entidad(
        contenedor_entidades_pueblo,
        "Salida a Granja (2x2)",
        TipoEntidad::TRANSICION_GRANJA));

    QVBoxLayout* layout_caverna = new QVBoxLayout(contenedor_entidades_caverna);
    layout_caverna->setContentsMargins(0, 0, 0, 0);
    layout_caverna->addWidget(crear_boton_entidad(
        contenedor_entidades_caverna,
        "Escalera Mazmorra (1x1)",
        TipoEntidad::ESCALERA_NIVEL));
    layout_caverna->addWidget(crear_boton_entidad(
        contenedor_entidades_caverna,
        "Salida Caverna (1x1)",
        TipoEntidad::SALIDA_CAVERNA));
}

void PaletaDeHerramientas::actualizar_sector(SectorActual sector) {
    DefinicionTilesetTMX tileset = CatalogoDeTerrenos::obtener_tileset_sector(sector);
    QPixmap pixmap_tileset(QString::fromStdString(tileset.ruta_imagen));

    const std::vector<DefinicionTile>& catalogo_tiles =
        CatalogoDeTerrenos::obtener_catalogo_por_sector(sector);

    size_t indice_tile = 0;
    while (indice_tile < botones_tiles.size() && indice_tile < catalogo_tiles.size()) {
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

        QPushButton* boton = botones_tiles[indice_tile];
        boton->setText(QString::fromStdString(definicion.nombre));
        boton->setIcon(QIcon(sprite_recortado.scaled(
            TAMANO_ICONO_TILE_PX,
            TAMANO_ICONO_TILE_PX,
            Qt::KeepAspectRatio,
            Qt::FastTransformation)));

        disconnect(boton, nullptr, nullptr, nullptr);
        uint32_t id_global = definicion.identificador_tile_global;
        uint8_t prop = definicion.propiedades;
        connect(boton, &QPushButton::clicked, this, [this, id_global, prop]() {
            emit tile_seleccionado(id_global, prop);
        });

        indice_tile++;
    }

    if (!botones_tiles.empty() && !catalogo_tiles.empty()) {
        botones_tiles[0]->setChecked(true);
        emit tile_seleccionado(
            catalogo_tiles[0].identificador_tile_global,
            catalogo_tiles[0].propiedades);
    }

    contenedor_entidades_granja->setVisible(sector == SectorActual::GRANJA);
    contenedor_entidades_pueblo->setVisible(sector == SectorActual::PUEBLO);
    contenedor_entidades_caverna->setVisible(sector == SectorActual::CAVERNA);

    if (sector == SectorActual::GRANJA) {
        emit entidad_cambiada(TipoEntidad::CABANIA);
    } else if (sector == SectorActual::PUEBLO) {
        emit entidad_cambiada(TipoEntidad::PUESTO_MARNIE);
    } else if (sector == SectorActual::CAVERNA) {
        emit entidad_cambiada(TipoEntidad::ESCALERA_NIVEL);
    }
}

void PaletaDeHerramientas::habilitar_boton_eliminar(bool habilitado) {
    if (boton_eliminar_entidad != nullptr) {
        boton_eliminar_entidad->setEnabled(habilitado);
    }
}
