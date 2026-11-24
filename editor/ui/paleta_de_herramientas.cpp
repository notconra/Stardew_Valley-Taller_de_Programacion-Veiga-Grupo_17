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
      boton_muro_perimetral(nullptr),
      pixmap_tileset(QString::fromStdString(
          CatalogoDeTerrenos::obtener_tileset_sector(SectorActual::GRANJA).ruta_imagen)),
      datos_tileset(CatalogoDeTerrenos::obtener_tileset_sector(SectorActual::GRANJA)),
      estampa_recto_activa(TipoEstampa::CAMINO_RECTO_HORIZONTAL),
      estampa_union_t_activa(TipoEstampa::CAMINO_UNION_T_NORTE),
      estampa_esquina_activa(TipoEstampa::CAMINO_ESQUINA_NOROESTE),
      estampa_mostrador_activa(TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO),
      estampa_puesto_mercado_activa(TipoEstampa::MERCADO_PUESTO_IZQUIERDO),
      estampa_estanteria_activa(TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA),
      id_tile_borde_activo(CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_NORTE),
      id_tile_esquina_exterior_activa(
          CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NOROESTE),
      id_tile_esquina_interior_activa(
          CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NOROESTE) {
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

    datos_tileset = CatalogoDeTerrenos::obtener_tileset_sector(sector);
    pixmap_tileset.load(QString::fromStdString(datos_tileset.ruta_imagen));
    estampa_recto_activa = TipoEstampa::CAMINO_RECTO_HORIZONTAL;
    estampa_union_t_activa = TipoEstampa::CAMINO_UNION_T_NORTE;
    estampa_esquina_activa = TipoEstampa::CAMINO_ESQUINA_NOROESTE;
    estampa_mostrador_activa = TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO;
    estampa_puesto_mercado_activa = TipoEstampa::MERCADO_PUESTO_IZQUIERDO;
    estampa_estanteria_activa = TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA;
    id_tile_borde_activo = CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_NORTE;
    id_tile_esquina_exterior_activa =
        CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NOROESTE;
    id_tile_esquina_interior_activa =
        CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NOROESTE;

    QPixmap pixmap_paredes_suelos;
    if (sector == SectorActual::MERCADO) {
        pixmap_paredes_suelos.load("Assets/Maps/walls_and_floors.png");
    }

    // 1. Cargar tiles de superficie
    auto& catalogo_tiles = CatalogoDeTerrenos::obtener_catalogo_por_sector(sector);

    size_t indice_tile = 0;
    while (indice_tile < catalogo_tiles.size()) {
        auto& definicion = catalogo_tiles[indice_tile];
        QPixmap sprite_recortado;
        if (definicion.identificador_tile_global >= 4353 && !pixmap_paredes_suelos.isNull()) {
            uint32_t indice_local = definicion.identificador_tile_global - 4353;
            int columna_sprite = static_cast<int>(indice_local % 16);
            int fila_sprite = static_cast<int>(indice_local / 16);
            sprite_recortado =
                pixmap_paredes_suelos.copy(columna_sprite * 16, fila_sprite * 16, 16, 16);
        } else {
            uint32_t indice_local =
                definicion.identificador_tile_global - datos_tileset.primer_identificador_global;
            int columna_sprite = static_cast<int>(
                indice_local % static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            int fila_sprite = static_cast<int>(
                indice_local / static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            sprite_recortado = pixmap_tileset.copy(
                columna_sprite * datos_tileset.ancho_tile,
                fila_sprite * datos_tileset.alto_tile,
                datos_tileset.ancho_tile,
                datos_tileset.alto_tile);
        }

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
            uint32_t id_a_emitir = id_global;
            if (id_global == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_NORTE) {
                id_a_emitir = id_tile_borde_activo;
            } else if (
                id_global == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NOROESTE) {
                id_a_emitir = id_tile_esquina_exterior_activa;
            } else if (
                id_global == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NOROESTE) {
                id_a_emitir = id_tile_esquina_interior_activa;
            }
            emit tile_seleccionado(id_a_emitir, prop);
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

    caja_estampas->setVisible(
        !lista_estampas.empty() || sector == SectorActual::GRANJA ||
        sector == SectorActual::PUEBLO || sector == SectorActual::MERCADO);
    boton_muro_perimetral->setVisible(
        sector == SectorActual::GRANJA || sector == SectorActual::PUEBLO ||
        sector == SectorActual::MERCADO);
    if (sector == SectorActual::MERCADO) {
        boton_muro_perimetral->setText("Restaurar Paredes Perimetrales");
    } else {
        boton_muro_perimetral->setText("Cerrar Perimetro con Vallas");
    }

    size_t indice_estampa = 0;
    while (indice_estampa < lista_estampas.size()) {
        auto& estampa = lista_estampas[indice_estampa];
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
            TipoEstampa estampa_a_emitir = tipo;
            if (tipo == TipoEstampa::CAMINO_RECTO_HORIZONTAL) {
                estampa_a_emitir = estampa_recto_activa;
            } else if (tipo == TipoEstampa::CAMINO_UNION_T_NORTE) {
                estampa_a_emitir = estampa_union_t_activa;
            } else if (tipo == TipoEstampa::CAMINO_ESQUINA_NOROESTE) {
                estampa_a_emitir = estampa_esquina_activa;
            } else if (tipo == TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO) {
                estampa_a_emitir = estampa_mostrador_activa;
            } else if (tipo == TipoEstampa::MERCADO_PUESTO_IZQUIERDO) {
                estampa_a_emitir = estampa_puesto_mercado_activa;
            } else if (tipo == TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA) {
                estampa_a_emitir = estampa_estanteria_activa;
            }
            emit estampa_seleccionada(estampa_a_emitir);
        });

        botones_estampas.push_back(boton_estampa);
        indice_estampa++;
    }
}

void PaletaDeHerramientas::sincronizar_rotacion_estampa(TipoEstampa estampa) {
    size_t indice = 0;
    while (indice < botones_estampas.size()) {
        QPushButton* boton = botones_estampas[indice];
        if (estampa == TipoEstampa::CAMINO_RECTO_HORIZONTAL ||
            estampa == TipoEstampa::CAMINO_RECTO_VERTICAL) {
            estampa_recto_activa = estampa;
            if (grupo_estampas->id(boton) ==
                static_cast<int>(TipoEstampa::CAMINO_RECTO_HORIZONTAL)) {
                boton->setChecked(true);
                QString texto = (estampa == TipoEstampa::CAMINO_RECTO_HORIZONTAL)
                                    ? "Camino Recto (Horizontal) [R]"
                                    : "Camino Recto (Vertical) [R]";
                boton->setText(texto);
            }
        } else if (
            estampa == TipoEstampa::CAMINO_UNION_T_NORTE ||
            estampa == TipoEstampa::CAMINO_UNION_T_ESTE ||
            estampa == TipoEstampa::CAMINO_UNION_T_SUR ||
            estampa == TipoEstampa::CAMINO_UNION_T_OESTE) {
            estampa_union_t_activa = estampa;
            if (grupo_estampas->id(boton) == static_cast<int>(TipoEstampa::CAMINO_UNION_T_NORTE)) {
                boton->setChecked(true);
                QString texto = "Union T (Norte) [R]";
                if (estampa == TipoEstampa::CAMINO_UNION_T_ESTE) {
                    texto = "Union T (Este) [R]";
                } else if (estampa == TipoEstampa::CAMINO_UNION_T_SUR) {
                    texto = "Union T (Sur) [R]";
                } else if (estampa == TipoEstampa::CAMINO_UNION_T_OESTE) {
                    texto = "Union T (Oeste) [R]";
                }
                boton->setText(texto);
            }
        } else if (
            estampa == TipoEstampa::CAMINO_ESQUINA_NOROESTE ||
            estampa == TipoEstampa::CAMINO_ESQUINA_NORESTE ||
            estampa == TipoEstampa::CAMINO_ESQUINA_SUDESTE ||
            estampa == TipoEstampa::CAMINO_ESQUINA_SUDOESTE) {
            estampa_esquina_activa = estampa;
            if (grupo_estampas->id(boton) ==
                static_cast<int>(TipoEstampa::CAMINO_ESQUINA_NOROESTE)) {
                boton->setChecked(true);
                QString texto = "Esquina (Noroeste) [R]";
                if (estampa == TipoEstampa::CAMINO_ESQUINA_NORESTE) {
                    texto = "Esquina (Noreste) [R]";
                } else if (estampa == TipoEstampa::CAMINO_ESQUINA_SUDESTE) {
                    texto = "Esquina (Sudeste) [R]";
                } else if (estampa == TipoEstampa::CAMINO_ESQUINA_SUDOESTE) {
                    texto = "Esquina (Sudoeste) [R]";
                }
                boton->setText(texto);
            }
        } else if (
            estampa == TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO ||
            estampa == TipoEstampa::MERCADO_MOSTRADOR_DERECHO) {
            estampa_mostrador_activa = estampa;
            if (grupo_estampas->id(boton) ==
                static_cast<int>(TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO)) {
                boton->setChecked(true);
                QString texto = (estampa == TipoEstampa::MERCADO_MOSTRADOR_IZQUIERDO)
                                    ? "Mostrador Principal (6x2) [R]"
                                    : "Mostrador Principal (Derecha) [R]";
                boton->setText(texto);
            }
        } else if (
            estampa == TipoEstampa::MERCADO_PUESTO_IZQUIERDO ||
            estampa == TipoEstampa::MERCADO_PUESTO_DERECHO) {
            estampa_puesto_mercado_activa = estampa;
            if (grupo_estampas->id(boton) ==
                static_cast<int>(TipoEstampa::MERCADO_PUESTO_IZQUIERDO)) {
                boton->setChecked(true);
                QString texto = (estampa == TipoEstampa::MERCADO_PUESTO_IZQUIERDO)
                                    ? "Puesto de Atencion (3x4) [R]"
                                    : "Puesto de Atencion (Derecha) [R]";
                boton->setText(texto);
            }
        } else if (
            estampa == TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA ||
            estampa == TipoEstampa::MERCADO_ESTANTERIA_DERECHA) {
            estampa_estanteria_activa = estampa;
            if (grupo_estampas->id(boton) ==
                static_cast<int>(TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA)) {
                boton->setChecked(true);
                QString texto = (estampa == TipoEstampa::MERCADO_ESTANTERIA_IZQUIERDA)
                                    ? "Estanteria Comercial (6x3) [R]"
                                    : "Estanteria Comercial (Derecha) [R]";
                boton->setText(texto);
            }
        }
        indice++;
    }
}

void PaletaDeHerramientas::sincronizar_rotacion_tile(uint32_t id_tile) {
    if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_NORTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_ESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_SUR ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_OESTE) {
        id_tile_borde_activo = id_tile;
        if (botones_tiles.size() >= 3) {
            QPushButton* boton_borde = botones_tiles[2];
            boton_borde->setChecked(true);
            uint32_t indice_local = id_tile - datos_tileset.primer_identificador_global;
            int columna_sprite = static_cast<int>(
                indice_local % static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            int fila_sprite = static_cast<int>(
                indice_local / static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            QPixmap sprite_recortado = pixmap_tileset.copy(
                columna_sprite * datos_tileset.ancho_tile,
                fila_sprite * datos_tileset.alto_tile,
                datos_tileset.ancho_tile,
                datos_tileset.alto_tile);
            boton_borde->setIcon(QIcon(sprite_recortado.scaled(
                TAMANO_ICONO_TILE_PX,
                TAMANO_ICONO_TILE_PX,
                Qt::KeepAspectRatio,
                Qt::FastTransformation)));
            QString direccion_borde = "Norte";
            if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_ESTE) {
                direccion_borde = "Este";
            } else if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_SUR) {
                direccion_borde = "Sur";
            } else if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_BORDE_OESTE) {
                direccion_borde = "Oeste";
            }
            boton_borde->setToolTip("Borde de Camino (" + direccion_borde + ") [R]");
        }
    } else if (
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NOROESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NORESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_SUDESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_SUDOESTE) {
        id_tile_esquina_exterior_activa = id_tile;
        if (botones_tiles.size() >= 4) {
            QPushButton* boton_esquina = botones_tiles[3];
            boton_esquina->setChecked(true);
            uint32_t indice_local = id_tile - datos_tileset.primer_identificador_global;
            int columna_sprite = static_cast<int>(
                indice_local % static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            int fila_sprite = static_cast<int>(
                indice_local / static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            QPixmap sprite_recortado = pixmap_tileset.copy(
                columna_sprite * datos_tileset.ancho_tile,
                fila_sprite * datos_tileset.alto_tile,
                datos_tileset.ancho_tile,
                datos_tileset.alto_tile);
            boton_esquina->setIcon(QIcon(sprite_recortado.scaled(
                TAMANO_ICONO_TILE_PX,
                TAMANO_ICONO_TILE_PX,
                Qt::KeepAspectRatio,
                Qt::FastTransformation)));
            QString orientacion = "Noroeste";
            if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_NORESTE) {
                orientacion = "Noreste";
            } else if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_SUDESTE) {
                orientacion = "Sudeste";
            } else if (
                id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_EXTERIOR_SUDOESTE) {
                orientacion = "Sudoeste";
            }
            boton_esquina->setToolTip("Esquina Exterior (" + orientacion + ") [R]");
        }
    } else if (
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NOROESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NORESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_SUDESTE ||
        id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_SUDOESTE) {
        id_tile_esquina_interior_activa = id_tile;
        if (botones_tiles.size() >= 5) {
            QPushButton* boton_interior = botones_tiles[4];
            boton_interior->setChecked(true);
            uint32_t indice_local = id_tile - datos_tileset.primer_identificador_global;
            int columna_sprite = static_cast<int>(
                indice_local % static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            int fila_sprite = static_cast<int>(
                indice_local / static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
            QPixmap sprite_recortado = pixmap_tileset.copy(
                columna_sprite * datos_tileset.ancho_tile,
                fila_sprite * datos_tileset.alto_tile,
                datos_tileset.ancho_tile,
                datos_tileset.alto_tile);
            boton_interior->setIcon(QIcon(sprite_recortado.scaled(
                TAMANO_ICONO_TILE_PX,
                TAMANO_ICONO_TILE_PX,
                Qt::KeepAspectRatio,
                Qt::FastTransformation)));
            QString orientacion = "Noroeste";
            if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_NORESTE) {
                orientacion = "Noreste";
            } else if (id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_SUDESTE) {
                orientacion = "Sudeste";
            } else if (
                id_tile == CatalogoDeEstampas::TILE_PUEBLO_CAMINO_ESQUINA_INTERIOR_SUDOESTE) {
                orientacion = "Sudoeste";
            }
            boton_interior->setToolTip("Esquina Interior (" + orientacion + ") [R]");
        }
    }
}
