#include "lienzo_de_edicion.h"

#include <QColor>
#include <QEvent>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "casilla_del_mapa.h"
#include "entidad_del_mapa.h"
#include "mapa_editable.h"
#include "propiedad_de_casilla.h"
#include "tipo_entidad.h"
#include "tipo_herramienta_editor.h"

static constexpr int ALFA_COLOR_FONDO_ENTIDAD = 45;
static constexpr int ALFA_COLOR_TRANSLUCIDO_ENTIDAD = 95;
static constexpr int ALFA_COLOR_FONDO_ETIQUETA = 175;
static constexpr int ALFA_COLOR_FANTASMA = 110;
static constexpr int ALFA_COLOR_ARRASTRE = 130;
static constexpr int ALFA_COLOR_ESTAMPA = 120;
static constexpr int GROSOR_BORDE_SELECCIONADO = 3;
static constexpr int GROSOR_BORDE_NORMAL = 2;
static constexpr int GROSOR_BORDE_PUERTA = 2;
static constexpr int MARGEN_INFINITO_CAVERNA = 6;
static constexpr int PASO_EXPANSION_CAVERNA = 4;

static constexpr int COLUMNA_PUERTA_CABANIA = 5;
static constexpr int FILA_PUERTA_CABANIA = 7;
static constexpr int COLUMNA_PUERTA_CAVERNA = 2;
static constexpr int FILA_PUERTA_CAVERNA = 6;
static constexpr int COLUMNA_PUERTA_CENTRO_COMUNITARIO = 5;
static constexpr int ANCHO_PUERTA_CENTRO_COMUNITARIO = 2;
static constexpr int COLUMNA_PUERTA_MERCADO = 5;
static constexpr int ANCHO_PUERTA_MERCADO = 2;

LienzoDeEdicion::LienzoDeEdicion(QWidget* padre)
    : QWidget(padre),
      mapa_actual(nullptr),
      sector_actual(SectorActual::GRANJA),
      tamano_casilla_px(28),
      modo_actual(ModoEditor::TERRENO),
      id_tile_actual(CatalogoDeTerrenos::obtener_tile_base(SectorActual::GRANJA)),
      propiedades_actuales(
          PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
          PropiedadDeCasilla::ADMITE_HIERBA),
      entidad_a_colocar(TipoEntidad::CABANIA),
      estampa_a_colocar(TipoEstampa::ESTANQUE_NATURAL),
      orientacion_entidad_a_colocar(Direccion::ABAJO),
      pixmap_tileset(QString::fromStdString(
          CatalogoDeTerrenos::obtener_tileset_sector(SectorActual::GRANJA).ruta_imagen)),
      datos_tileset(CatalogoDeTerrenos::obtener_tileset_sector(SectorActual::GRANJA)),
      pixmap_cabania(QString(RUTA_SPRITE_CABANIA)),
      pixmap_caseta_mina(QString(RUTA_SPRITE_CASETA_MINA)),
      pixmap_exteriores_entidades(QString(RUTA_SPRITE_EXTERIORES)),
      pixmap_caverna_entidades(QString(RUTA_SPRITE_CAVERNA)),
      pixmap_pueblo_entidades(QString(RUTA_SPRITE_PUEBLO)),
      pixmap_mercado(),
      pixmap_retrato_marnie(QString(RUTA_RETRATO_MARNIE)),
      pixmap_retrato_clint(QString(RUTA_RETRATO_CLINT)),
      pixmap_retrato_pierre(QString(RUTA_RETRATO_PIERRE)),
      pixmap_retrato_robin(QString(RUTA_RETRATO_ROBIN)),
      mapa_visual_entidades(),
      identificador_entidad_seleccionada(-1),
      arrastrando_entidad(false),
      columna_arrastre_actual(0),
      fila_arrastre_actual(0),
      columna_hover(-1),
      fila_hover(-1),
      mouse_presionado(false),
      trazo_terreno_activo(false) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    cargar_tilesets_de_sector(SectorActual::GRANJA);
    inicializar_catalogo_visual_entidades();
    actualizar_tamano_del_lienzo();
}

void LienzoDeEdicion::cargar_tilesets_de_sector(SectorActual sector) {
    lista_tilesets_actuales.clear();
    if (sector == SectorActual::MERCADO) {
        EntradaTilesetLienzo tileset_interior;
        tileset_interior.pixmap.load("Assets/Maps/townInterior.png");
        tileset_interior.primer_identificador_global = 1;
        tileset_interior.cantidad_de_columnas = 32;
        tileset_interior.ancho_tile = 16;
        tileset_interior.alto_tile = 16;
        lista_tilesets_actuales.push_back(tileset_interior);

        EntradaTilesetLienzo tileset_interior_secundario;
        tileset_interior_secundario.pixmap = tileset_interior.pixmap;
        tileset_interior_secundario.primer_identificador_global = 2177;
        tileset_interior_secundario.cantidad_de_columnas = 32;
        tileset_interior_secundario.ancho_tile = 16;
        tileset_interior_secundario.alto_tile = 16;
        lista_tilesets_actuales.push_back(tileset_interior_secundario);

        EntradaTilesetLienzo tileset_paredes_suelos;
        tileset_paredes_suelos.pixmap.load("Assets/Maps/walls_and_floors.png");
        tileset_paredes_suelos.primer_identificador_global = 4353;
        tileset_paredes_suelos.cantidad_de_columnas = 16;
        tileset_paredes_suelos.ancho_tile = 16;
        tileset_paredes_suelos.alto_tile = 16;
        lista_tilesets_actuales.push_back(tileset_paredes_suelos);
    } else {
        DefinicionTilesetTMX tileset_base = CatalogoDeTerrenos::obtener_tileset_sector(sector);
        EntradaTilesetLienzo entrada;
        entrada.pixmap.load(QString::fromStdString(tileset_base.ruta_imagen));
        entrada.primer_identificador_global = tileset_base.primer_identificador_global;
        entrada.cantidad_de_columnas = tileset_base.cantidad_de_columnas;
        entrada.ancho_tile = tileset_base.ancho_tile;
        entrada.alto_tile = tileset_base.alto_tile;
        lista_tilesets_actuales.push_back(entrada);
    }
}

void LienzoDeEdicion::dibujar_tile_en_lienzo(
    QPainter& pintor, uint32_t valor_crudo, int columna, int fila) {
    if (valor_crudo == 0) {
        return;
    }
    uint32_t identificador_tile = valor_crudo & 0x1FFFFFFF;
    bool espejo_horizontal = (valor_crudo & 0x80000000) != 0;
    bool espejo_vertical = (valor_crudo & 0x40000000) != 0;

    EntradaTilesetLienzo* tileset_elegido = nullptr;
    size_t indice = 0;
    while (indice < lista_tilesets_actuales.size()) {
        EntradaTilesetLienzo& candidato = lista_tilesets_actuales[indice];
        if (identificador_tile >= candidato.primer_identificador_global) {
            if (tileset_elegido == nullptr || candidato.primer_identificador_global >
                                                  tileset_elegido->primer_identificador_global) {
                tileset_elegido = &candidato;
            }
        }
        indice++;
    }

    if (tileset_elegido == nullptr || tileset_elegido->pixmap.isNull() ||
        tileset_elegido->cantidad_de_columnas <= 0) {
        return;
    }

    uint32_t indice_local = identificador_tile - tileset_elegido->primer_identificador_global;
    int columna_sprite = static_cast<int>(
        indice_local % static_cast<uint32_t>(tileset_elegido->cantidad_de_columnas));
    int fila_sprite = static_cast<int>(
        indice_local / static_cast<uint32_t>(tileset_elegido->cantidad_de_columnas));

    QRect rectangulo_origen(
        columna_sprite * tileset_elegido->ancho_tile,
        fila_sprite * tileset_elegido->alto_tile,
        tileset_elegido->ancho_tile,
        tileset_elegido->alto_tile);
    QRect rectangulo_destino(
        columna * tamano_casilla_px,
        fila * tamano_casilla_px,
        tamano_casilla_px,
        tamano_casilla_px);

    if (!espejo_horizontal && !espejo_vertical) {
        pintor.drawPixmap(rectangulo_destino, tileset_elegido->pixmap, rectangulo_origen);
    } else {
        QImage imagen_espejada = tileset_elegido->pixmap.copy(rectangulo_origen)
                                     .toImage()
                                     .mirrored(espejo_horizontal, espejo_vertical);
        pintor.drawImage(rectangulo_destino, imagen_espejada);
    }
}

void LienzoDeEdicion::establecer_mapa(MapaEditable* mapa, SectorActual sector) {
    mapa_actual = mapa;
    sector_actual = sector;
    datos_tileset = CatalogoDeTerrenos::obtener_tileset_sector(sector);
    pixmap_tileset.load(QString::fromStdString(datos_tileset.ruta_imagen));
    cargar_tilesets_de_sector(sector);
    identificador_entidad_seleccionada = -1;
    arrastrando_entidad = false;
    actualizar_tamano_del_lienzo();
    update();
    emit entidad_seleccionada_cambio(-1);
}

void LienzoDeEdicion::establecer_tamano_casilla(int nuevo_tamano) {
    if (nuevo_tamano < 12) {
        nuevo_tamano = 12;
    } else if (nuevo_tamano > 64) {
        nuevo_tamano = 64;
    }
    tamano_casilla_px = nuevo_tamano;
    actualizar_tamano_del_lienzo();
    update();
}

int LienzoDeEdicion::obtener_tamano_casilla() const {
    return tamano_casilla_px;
}

void LienzoDeEdicion::establecer_modo(ModoEditor nuevo_modo) {
    modo_actual = nuevo_modo;
    if (modo_actual != ModoEditor::SELECCIONAR_MOVER) {
        identificador_entidad_seleccionada = -1;
        arrastrando_entidad = false;
        emit entidad_seleccionada_cambio(-1);
    }
    update();
}

void LienzoDeEdicion::establecer_tile(uint32_t nuevo_id_tile, uint8_t nuevas_propiedades) {
    id_tile_actual = nuevo_id_tile;
    propiedades_actuales = nuevas_propiedades;
}

void LienzoDeEdicion::establecer_estampa_a_colocar(TipoEstampa nueva_estampa) {
    estampa_a_colocar = nueva_estampa;
    modo_actual = ModoEditor::ESTAMPAR;
    update();
}

void LienzoDeEdicion::establecer_entidad_a_colocar(TipoEntidad nueva_entidad) {
    entidad_a_colocar = nueva_entidad;
}

void LienzoDeEdicion::deseleccionar_entidad() {
    identificador_entidad_seleccionada = -1;
    arrastrando_entidad = false;
    update();
    emit entidad_seleccionada_cambio(-1);
}

bool LienzoDeEdicion::eliminar_entidad_seleccionada() {
    if (mapa_actual == nullptr || identificador_entidad_seleccionada < 0) {
        return false;
    }
    uint32_t identificador = static_cast<uint32_t>(identificador_entidad_seleccionada);
    bool eliminado = mapa_actual->retirar_entidad(identificador);
    if (eliminado) {
        identificador_entidad_seleccionada = -1;
        arrastrando_entidad = false;
        update();
        emit entidad_seleccionada_cambio(-1);
        emit mapa_modificado();
    }
    return eliminado;
}

int LienzoDeEdicion::obtener_identificador_entidad_seleccionada() const {
    return identificador_entidad_seleccionada;
}

int LienzoDeEdicion::obtener_margen_visible() const {
    if (sector_actual == SectorActual::CAVERNA) {
        return MARGEN_INFINITO_CAVERNA;
    }
    return 0;
}

void LienzoDeEdicion::obtener_coordenadas_desde_posicion(
    const QPoint& posicion, int& columna, int& fila) const {
    if (tamano_casilla_px <= 0) {
        columna = 0;
        fila = 0;
        return;
    }
    int margen = obtener_margen_visible();
    columna = (posicion.x() / tamano_casilla_px) - margen;
    fila = (posicion.y() / tamano_casilla_px) - margen;
}

void LienzoDeEdicion::actualizar_tamano_del_lienzo() {
    if (mapa_actual == nullptr) {
        setFixedSize(400, 400);
        return;
    }
    int margen = obtener_margen_visible();
    int ancho_total =
        (mapa_actual->obtener_cantidad_de_columnas() + 2 * margen) * tamano_casilla_px;
    int alto_total = (mapa_actual->obtener_cantidad_de_filas() + 2 * margen) * tamano_casilla_px;
    setFixedSize(ancho_total, alto_total);
}

bool LienzoDeEdicion::asegurar_espacio_en_caverna(int& columna, int& fila, int ancho, int alto) {
    if (mapa_actual == nullptr || sector_actual != SectorActual::CAVERNA) {
        return false;
    }
    int cantidad_columnas = mapa_actual->obtener_cantidad_de_columnas();
    int cantidad_filas = mapa_actual->obtener_cantidad_de_filas();

    int agregar_izquierda = 0;
    int agregar_arriba = 0;
    int agregar_derecha = 0;
    int agregar_abajo = 0;

    if (columna < 1) {
        agregar_izquierda = (1 - columna) + PASO_EXPANSION_CAVERNA;
    }
    if (fila < 1) {
        agregar_arriba = (1 - fila) + PASO_EXPANSION_CAVERNA;
    }
    if (columna + ancho > cantidad_columnas - 1) {
        agregar_derecha = (columna + ancho - (cantidad_columnas - 1)) + PASO_EXPANSION_CAVERNA;
    }
    if (fila + alto > cantidad_filas - 1) {
        agregar_abajo = (fila + alto - (cantidad_filas - 1)) + PASO_EXPANSION_CAVERNA;
    }

    if (agregar_izquierda == 0 && agregar_arriba == 0 && agregar_derecha == 0 &&
        agregar_abajo == 0) {
        return false;
    }

    mapa_actual->expandir(
        agregar_izquierda,
        agregar_arriba,
        agregar_derecha,
        agregar_abajo,
        0,
        PropiedadDeCasilla::BLOQUEA);

    columna += agregar_izquierda;
    fila += agregar_arriba;
    columna_hover += agregar_izquierda;
    fila_hover += agregar_arriba;
    columna_arrastre_actual += agregar_izquierda;
    fila_arrastre_actual += agregar_arriba;

    actualizar_tamano_del_lienzo();
    emit mapa_expandido(agregar_izquierda, agregar_arriba);
    return true;
}

void LienzoDeEdicion::borrar_en_casilla(int columna, int fila) {
    if (mapa_actual == nullptr || !mapa_actual->esta_dentro(columna, fila)) {
        return;
    }
    EntidadDelMapa* entidad_tocada = mapa_actual->buscar_entidad_en_casilla(columna, fila);
    if (entidad_tocada != nullptr) {
        mapa_actual->retirar_entidad(entidad_tocada->obtener_identificador());
        identificador_entidad_seleccionada = -1;
        emit entidad_seleccionada_cambio(-1);
        emit mapa_modificado();
        update();
        return;
    }
    if (casilla_es_perimetro_protegido(columna, fila)) {
        return;
    }
    if (mapa_actual->buscar_estampa_en_casilla(columna, fila) != nullptr ||
        mapa_actual->casilla_pertenece_a_estanque(columna, fila)) {
        uint32_t tile_base = CatalogoDeTerrenos::obtener_tile_base(sector_actual);
        uint8_t prop_base =
            (sector_actual == SectorActual::GRANJA)
                ? (PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
                   PropiedadDeCasilla::ADMITE_HIERBA)
                : ((sector_actual == SectorActual::CAVERNA)
                       ? (PropiedadDeCasilla::DECORABLE | PropiedadDeCasilla::ADMITE_MINERAL)
                       : PropiedadDeCasilla::DECORABLE);
        mapa_actual->retirar_estanque_en_casilla(columna, fila, tile_base, prop_base);
    } else if (sector_actual == SectorActual::CAVERNA) {
        mapa_actual->borrar_en_capa(TipoCapa::FRENTE, columna, fila);
        mapa_actual->borrar_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
        mapa_actual->pintar_en_capa(TipoCapa::SUELO, columna, fila, 0, PropiedadDeCasilla::BLOQUEA);
    } else {
        uint32_t tile_base = CatalogoDeTerrenos::obtener_tile_base(sector_actual);
        uint8_t prop_base = (sector_actual == SectorActual::GRANJA)
                                ? (PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
                                   PropiedadDeCasilla::ADMITE_HIERBA)
                                : PropiedadDeCasilla::DECORABLE;
        mapa_actual->borrar_en_capa(TipoCapa::FRENTE, columna, fila);
        mapa_actual->borrar_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
        mapa_actual->pintar_en_capa(TipoCapa::SUELO, columna, fila, tile_base, prop_base);
    }
    emit mapa_modificado();
    update();
}

bool LienzoDeEdicion::casilla_es_perimetro_protegido(int columna, int fila) const {
    if (mapa_actual == nullptr) {
        return false;
    }
    int cantidad_columnas = mapa_actual->obtener_cantidad_de_columnas();
    int cantidad_filas = mapa_actual->obtener_cantidad_de_filas();

    if (sector_actual == SectorActual::MERCADO) {
        if (fila == 0 || fila == cantidad_filas - 1 || columna == 0 ||
            columna == cantidad_columnas - 1) {
            return true;
        }
        return false;
    } else if (sector_actual == SectorActual::GRANJA) {
        if (columna == 0 || columna >= cantidad_columnas - 1 || fila <= 1 ||
            fila >= cantidad_filas - 1) {
            return true;
        }
    } else if (sector_actual == SectorActual::PUEBLO) {
        if (columna == 0 || columna >= cantidad_columnas - 1 || fila <= 1 ||
            fila >= cantidad_filas - 2) {
            return true;
        }
    }
    return false;
}

void LienzoDeEdicion::pintar_casilla_en_posicion(int columna, int fila) {
    if (mapa_actual == nullptr) {
        return;
    }
    asegurar_espacio_en_caverna(columna, fila, 1, 1);
    if (!mapa_actual->esta_dentro(columna, fila)) {
        return;
    }
    if (casilla_es_perimetro_protegido(columna, fila)) {
        return;
    }
    if (mapa_actual->casilla_pertenece_a_estanque(columna, fila)) {
        return;
    }
    mapa_actual
        ->pintar_en_capa(TipoCapa::SUELO, columna, fila, id_tile_actual, propiedades_actuales);
    trazo_terreno_activo = true;
    emit mapa_modificado();
    update();
}

bool LienzoDeEdicion::esta_pintando_con_mouse() const {
    return mouse_presionado && modo_actual == ModoEditor::TERRENO;
}

void LienzoDeEdicion::dibujar_capa_de_tiles(
    QPainter& pintor, TipoCapa tipo_capa, int cantidad_columnas, int cantidad_filas) {
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            CasillaDelMapa casilla = mapa_actual->obtener_casilla_en_capa(tipo_capa, columna, fila);
            uint32_t id_tile = casilla.obtener_id_tile();
            if (id_tile > 0) {
                dibujar_tile_en_lienzo(pintor, id_tile, columna, fila);
            }
            columna++;
        }
        fila++;
    }
}

static const std::map<TipoEntidad, QColor> COLORES_ENTIDADES = {
    {TipoEntidad::CABANIA, QColor(165, 42, 42)},
    {TipoEntidad::ENTRADA_CAVERNA, QColor(47, 79, 79)},
    {TipoEntidad::PUESTO_MARNIE, QColor(46, 139, 87)},
    {TipoEntidad::PUESTO_CLINT, QColor(112, 128, 144)},
    {TipoEntidad::PUESTO_PIERRE, QColor(218, 165, 32)},
    {TipoEntidad::PUESTO_ROBIN, QColor(205, 133, 63)},
    {TipoEntidad::CENTRO_COMUNITARIO, QColor(106, 90, 205)},
    {TipoEntidad::MERCADO, QColor(218, 165, 32)},
    {TipoEntidad::ESCALERA_NIVEL, QColor(139, 69, 19)},
    {TipoEntidad::SALIDA_CAVERNA, QColor(0, 139, 139)},
    {TipoEntidad::TRANSICION_PUEBLO, QColor(30, 144, 255)},
    {TipoEntidad::TRANSICION_GRANJA, QColor(50, 168, 82)}};

void LienzoDeEdicion::inicializar_catalogo_visual_entidades() {
    pixmap_mercado = QPixmap(160, 160);
    pixmap_mercado.fill(Qt::transparent);
    QPainter pintor_mercado(&pixmap_mercado);
    pintor_mercado.drawPixmap(0, 16, pixmap_pueblo_entidades, 5 * 16, 11 * 16, 160, 144);
    pintor_mercado.drawPixmap(0, 0, pixmap_pueblo_entidades, 25 * 16, 10 * 16, 112, 16);
    pintor_mercado.end();

    mapa_visual_entidades = {
        {TipoEntidad::CABANIA, {&pixmap_cabania, QRect(16, 0, 144, 144)}},
        {TipoEntidad::ENTRADA_CAVERNA, {&pixmap_caseta_mina, QRect(80, 0, 80, 112)}},
        {TipoEntidad::CENTRO_COMUNITARIO, {&pixmap_pueblo_entidades, QRect(0, 640, 192, 160)}},
        {TipoEntidad::MERCADO, {&pixmap_mercado, QRect(0, 0, 160, 160)}},
        {TipoEntidad::ESCALERA_NIVEL, {&pixmap_caverna_entidades, QRect(208, 160, 16, 16)}},
        {TipoEntidad::SALIDA_CAVERNA, {&pixmap_caverna_entidades, QRect(48, 112, 16, 16)}},
        {TipoEntidad::PUESTO_MARNIE, {&pixmap_retrato_marnie, QRect(0, 0, 64, 64)}},
        {TipoEntidad::PUESTO_CLINT, {&pixmap_retrato_clint, QRect(0, 0, 64, 64)}},
        {TipoEntidad::PUESTO_PIERRE, {&pixmap_retrato_pierre, QRect(0, 0, 64, 64)}},
        {TipoEntidad::PUESTO_ROBIN, {&pixmap_retrato_robin, QRect(0, 0, 64, 64)}}};
}

void LienzoDeEdicion::dibujar_visual_entidad(
    QPainter& pintor, TipoEntidad tipo_entidad, int origen_columna, int origen_fila,
    const QRect& rectangulo_destino) {
    if (tipo_entidad == TipoEntidad::SALIDA_CAVERNA && !pixmap_caverna_entidades.isNull()) {
        pintor.drawPixmap(rectangulo_destino, pixmap_caverna_entidades, QRect(48, 112, 16, 16));
        int altura_escalera = 0;
        if (mapa_actual != nullptr) {
            bool continuar_subiendo = true;
            while (altura_escalera < 4 && continuar_subiendo) {
                int fila_objetivo = origen_fila - (altura_escalera + 1);
                if (!mapa_actual->esta_dentro(origen_columna, fila_objetivo)) {
                    continuar_subiendo = false;
                } else {
                    uint32_t tile_muro = mapa_actual
                                             ->obtener_casilla_en_capa(
                                                 TipoCapa::ESTRUCTURAS,
                                                 origen_columna,
                                                 fila_objetivo)
                                             .obtener_id_tile();
                    if (MapaEditable::es_tile_muro_caverna(tile_muro)) {
                        altura_escalera++;
                    } else {
                        continuar_subiendo = false;
                    }
                }
            }
        }
        if (altura_escalera == 0) {
            altura_escalera = 3;
        }

        int paso = 1;
        while (paso <= altura_escalera) {
            QRect rectangulo_tramo(
                origen_columna * tamano_casilla_px,
                (origen_fila - paso) * tamano_casilla_px,
                tamano_casilla_px,
                tamano_casilla_px);
            QRect origen_tramo(48, 80, 16, 16);
            if (paso == altura_escalera) {
                origen_tramo = QRect(48, 64, 16, 16);
            } else if (paso % 2 == 1) {
                origen_tramo = QRect(48, 96, 16, 16);
            }
            pintor.drawPixmap(rectangulo_tramo, pixmap_caverna_entidades, origen_tramo);
            paso++;
        }
        return;
    }

    auto iterador_visual = mapa_visual_entidades.find(tipo_entidad);
    if (iterador_visual != mapa_visual_entidades.end() &&
        iterador_visual->second.pixmap != nullptr && !iterador_visual->second.pixmap->isNull()) {
        pintor.drawPixmap(
            rectangulo_destino,
            *iterador_visual->second.pixmap,
            iterador_visual->second.rectangulo_origen);
    }
}

void LienzoDeEdicion::dibujar_entidad(
    QPainter& pintor, const EntidadDelMapa& entidad, const QRect& rectangulo_entidad,
    bool es_seleccionada) {
    TipoEntidad tipo_entidad = entidad.obtener_tipo();
    auto iterador_color = COLORES_ENTIDADES.find(tipo_entidad);
    QColor color_base =
        iterador_color != COLORES_ENTIDADES.end() ? iterador_color->second : QColor(100, 100, 100);

    bool es_puesto_comerciante =
        (tipo_entidad == TipoEntidad::PUESTO_MARNIE || tipo_entidad == TipoEntidad::PUESTO_CLINT ||
         tipo_entidad == TipoEntidad::PUESTO_PIERRE || tipo_entidad == TipoEntidad::PUESTO_ROBIN);
    if (es_puesto_comerciante) {
        auto iterador_visual = mapa_visual_entidades.find(tipo_entidad);
        if (iterador_visual != mapa_visual_entidades.end() &&
            iterador_visual->second.pixmap != nullptr &&
            !iterador_visual->second.pixmap->isNull()) {
            pintor.drawPixmap(
                rectangulo_entidad,
                *iterador_visual->second.pixmap,
                iterador_visual->second.rectangulo_origen);
        } else {
            QColor color_translucido = color_base;
            color_translucido.setAlpha(ALFA_COLOR_TRANSLUCIDO_ENTIDAD);
            pintor.fillRect(rectangulo_entidad, color_translucido);
        }

        pintor.setBrush(Qt::NoBrush);
        if (es_seleccionada) {
            pintor.setPen(QPen(QColor(255, 215, 0), GROSOR_BORDE_SELECCIONADO));
        } else {
            pintor.setPen(QPen(color_base, GROSOR_BORDE_NORMAL));
        }
        pintor.drawRect(rectangulo_entidad);

        // Flecha indicadora de orientación
        Direccion orientacion = entidad.obtener_orientacion();
        QPolygon flecha;
        int centro_x = rectangulo_entidad.center().x();
        int centro_y = rectangulo_entidad.center().y();
        int radio_flecha = tamano_casilla_px / 5;
        if (radio_flecha < 4) {
            radio_flecha = 4;
        }
        if (orientacion == Direccion::ABAJO) {
            int y_borde = rectangulo_entidad.bottom() - 1;
            flecha << QPoint(centro_x - radio_flecha, y_borde - radio_flecha)
                   << QPoint(centro_x + radio_flecha, y_borde - radio_flecha)
                   << QPoint(centro_x, y_borde);
        } else if (orientacion == Direccion::ARRIBA) {
            int y_borde = rectangulo_entidad.top() + 1;
            flecha << QPoint(centro_x - radio_flecha, y_borde + radio_flecha)
                   << QPoint(centro_x + radio_flecha, y_borde + radio_flecha)
                   << QPoint(centro_x, y_borde);
        } else if (orientacion == Direccion::DERECHA) {
            int x_borde = rectangulo_entidad.right() - 1;
            flecha << QPoint(x_borde - radio_flecha, centro_y - radio_flecha)
                   << QPoint(x_borde - radio_flecha, centro_y + radio_flecha)
                   << QPoint(x_borde, centro_y);
        } else {
            int x_borde = rectangulo_entidad.left() + 1;
            flecha << QPoint(x_borde + radio_flecha, centro_y - radio_flecha)
                   << QPoint(x_borde + radio_flecha, centro_y + radio_flecha)
                   << QPoint(x_borde, centro_y);
        }
        pintor.setBrush(QColor(255, 255, 255, 230));
        pintor.setPen(QPen(Qt::black, 1));
        pintor.drawPolygon(flecha);
        pintor.setBrush(Qt::NoBrush);

        // Punto de Venta (una casilla adelante en la dirección de la mirada)
        if (mapa_actual != nullptr) {
            int columna_npc = 0;
            int fila_npc = 0;
            entidad.consultar_origen(columna_npc, fila_npc);
            int columna_interaccion = 0;
            int fila_interaccion = 0;
            int columna_atencion = 0;
            int fila_atencion = 0;
            mapa_actual->calcular_interaccion_puesto(
                columna_npc,
                fila_npc,
                orientacion,
                columna_interaccion,
                fila_interaccion,
                columna_atencion,
                fila_atencion);
            QRect rectangulo_punto_venta(
                columna_interaccion * tamano_casilla_px,
                fila_interaccion * tamano_casilla_px,
                tamano_casilla_px,
                tamano_casilla_px);
            QColor color_venta = es_seleccionada ? QColor(0, 255, 127) : QColor(0, 206, 209);
            pintor.setBrush(Qt::NoBrush);
            pintor.setPen(QPen(color_venta, 2, Qt::DashLine));
            pintor.drawRect(rectangulo_punto_venta);

            int alto_insignia = tamano_casilla_px > 24 ? 12 : 9;
            QRect rectangulo_insignia(
                rectangulo_punto_venta.x(),
                rectangulo_punto_venta.bottom() - alto_insignia + 1,
                rectangulo_punto_venta.width(),
                alto_insignia);
            pintor.fillRect(rectangulo_insignia, QColor(0, 0, 0, 180));
            pintor.setPen(color_venta);
            QFont fuente_venta = pintor.font();
            fuente_venta.setBold(true);
            fuente_venta.setPointSize(tamano_casilla_px > 24 ? 6 : 5);
            pintor.setFont(fuente_venta);
            pintor.drawText(rectangulo_insignia, Qt::AlignCenter, "Venta");
        }
        return;
    }

    bool es_entidad_caverna =
        (tipo_entidad == TipoEntidad::ESCALERA_NIVEL ||
         tipo_entidad == TipoEntidad::SALIDA_CAVERNA);
    if (es_entidad_caverna) {
        int origen_columna = 0;
        int origen_fila = 0;
        entidad.consultar_origen(origen_columna, origen_fila);
        dibujar_visual_entidad(
            pintor,
            tipo_entidad,
            origen_columna,
            origen_fila,
            rectangulo_entidad);
        if (es_seleccionada) {
            pintor.setBrush(Qt::NoBrush);
            pintor.setPen(QPen(QColor(255, 215, 0), GROSOR_BORDE_SELECCIONADO));
            pintor.drawRect(rectangulo_entidad);
        }
        return;
    }

    QColor color_fondo = color_base;
    color_fondo.setAlpha(ALFA_COLOR_FONDO_ENTIDAD);
    pintor.fillRect(rectangulo_entidad, color_fondo);

    auto iterador_visual = mapa_visual_entidades.find(tipo_entidad);
    if (iterador_visual != mapa_visual_entidades.end() &&
        iterador_visual->second.pixmap != nullptr && !iterador_visual->second.pixmap->isNull()) {
        pintor.drawPixmap(
            rectangulo_entidad,
            *iterador_visual->second.pixmap,
            iterador_visual->second.rectangulo_origen);
    } else {
        QColor color_translucido = color_base;
        color_translucido.setAlpha(ALFA_COLOR_TRANSLUCIDO_ENTIDAD);
        pintor.fillRect(rectangulo_entidad, color_translucido);
    }

    // Indicador punteado de puerta o entrada para estructuras principales
    if (tipo_entidad == TipoEntidad::CABANIA || tipo_entidad == TipoEntidad::CENTRO_COMUNITARIO ||
        tipo_entidad == TipoEntidad::MERCADO || tipo_entidad == TipoEntidad::ENTRADA_CAVERNA) {
        int origen_columna = 0;
        int origen_fila = 0;
        entidad.consultar_origen(origen_columna, origen_fila);
        int ancho_casillas = 0;
        int alto_casillas = 0;
        entidad.consultar_dimensiones(ancho_casillas, alto_casillas);

        int columna_puerta = origen_columna;
        int ancho_puerta = 1;
        if (tipo_entidad == TipoEntidad::CABANIA) {
            columna_puerta += COLUMNA_PUERTA_CABANIA;
        } else if (tipo_entidad == TipoEntidad::CENTRO_COMUNITARIO) {
            columna_puerta += COLUMNA_PUERTA_CENTRO_COMUNITARIO;
            ancho_puerta = ANCHO_PUERTA_CENTRO_COMUNITARIO;
        } else if (tipo_entidad == TipoEntidad::MERCADO) {
            columna_puerta += COLUMNA_PUERTA_MERCADO;
            ancho_puerta = ANCHO_PUERTA_MERCADO;
        } else if (tipo_entidad == TipoEntidad::ENTRADA_CAVERNA) {
            columna_puerta += COLUMNA_PUERTA_CAVERNA;
        }

        int fila_puerta = origen_fila + (tipo_entidad == TipoEntidad::CABANIA ? FILA_PUERTA_CABANIA
                                                                              : alto_casillas - 1);
        QRect rectangulo_puerta(
            columna_puerta * tamano_casilla_px,
            fila_puerta * tamano_casilla_px,
            ancho_puerta * tamano_casilla_px,
            tamano_casilla_px);
        pintor.setPen(QPen(QColor(0, 255, 127), GROSOR_BORDE_PUERTA, Qt::DashLine));
        pintor.drawRect(rectangulo_puerta);
    }

    pintor.setBrush(Qt::NoBrush);
    if (es_seleccionada) {
        pintor.setPen(QPen(QColor(255, 215, 0), GROSOR_BORDE_SELECCIONADO));
    } else {
        pintor.setPen(QPen(color_base, GROSOR_BORDE_NORMAL));
    }
    pintor.drawRect(rectangulo_entidad);

    int alto_etiqueta = tamano_casilla_px > 24 ? 16 : 12;
    QRect rectangulo_etiqueta(
        rectangulo_entidad.x(),
        rectangulo_entidad.bottom() - alto_etiqueta,
        rectangulo_entidad.width(),
        alto_etiqueta);
    pintor.fillRect(rectangulo_etiqueta, QColor(0, 0, 0, ALFA_COLOR_FONDO_ETIQUETA));
    pintor.setPen(Qt::white);
    QFont fuente = pintor.font();
    fuente.setBold(true);
    fuente.setPointSize(tamano_casilla_px > 24 ? 8 : 6);
    pintor.setFont(fuente);
    std::string texto_entidad = ConfiguracionEntidadesEditor::obtener_nombre(tipo_entidad);
    pintor.drawText(rectangulo_etiqueta, Qt::AlignCenter, QString::fromStdString(texto_entidad));
}

void LienzoDeEdicion::paintEvent(QPaintEvent* evento) {
    (void)evento;
    QPainter pintor(this);
    pintor.setRenderHint(QPainter::Antialiasing, false);

    if (mapa_actual == nullptr) {
        pintor.fillRect(rect(), QColor(30, 30, 30));
        pintor.setPen(Qt::white);
        pintor.drawText(rect(), Qt::AlignCenter, "Sin mapa cargado");
        return;
    }

    int cantidad_columnas = mapa_actual->obtener_cantidad_de_columnas();
    int cantidad_filas = mapa_actual->obtener_cantidad_de_filas();
    int margen = obtener_margen_visible();

    // 1. Fondo oscuro
    pintor.fillRect(rect(), QColor(25, 25, 25));

    pintor.save();
    pintor.translate(margen * tamano_casilla_px, margen * tamano_casilla_px);

    // 2. Capa Suelo (Back) con sprites reales
    dibujar_capa_de_tiles(pintor, TipoCapa::SUELO, cantidad_columnas, cantidad_filas);

    // 3. Capa Estructuras (Buildings) con sprites reales
    dibujar_capa_de_tiles(pintor, TipoCapa::ESTRUCTURAS, cantidad_columnas, cantidad_filas);

    // 4. Dibujar Entidades Colocadas con sus sprites reales
    std::vector<EntidadDelMapa> entidades = mapa_actual->listar_todas_las_entidades();
    size_t indice_entidad = 0;
    while (indice_entidad < entidades.size()) {
        uint32_t id_entidad = entidades[indice_entidad].obtener_identificador();
        int origen_columna = 0;
        int origen_fila = 0;
        entidades[indice_entidad].consultar_origen(origen_columna, origen_fila);
        int ancho_casillas = 0;
        int alto_casillas = 0;
        entidades[indice_entidad].consultar_dimensiones(ancho_casillas, alto_casillas);

        QRect rectangulo_entidad(
            origen_columna * tamano_casilla_px,
            origen_fila * tamano_casilla_px,
            ancho_casillas * tamano_casilla_px,
            alto_casillas * tamano_casilla_px);

        bool es_seleccionada =
            (identificador_entidad_seleccionada >= 0 &&
             static_cast<uint32_t>(identificador_entidad_seleccionada) == id_entidad);

        dibujar_entidad(pintor, entidades[indice_entidad], rectangulo_entidad, es_seleccionada);
        indice_entidad++;
    }

    // 5. Dibujar Capa Techos/Frente (Front) con sprites reales
    dibujar_capa_de_tiles(pintor, TipoCapa::FRENTE, cantidad_columnas, cantidad_filas);

    // 5. Dibujar Lineas de Grilla (incluyendo zona de expansion infinita en Caverna)
    pintor.setPen(QColor(0, 0, 0, 45));
    int columna_linea = -margen;
    while (columna_linea <= cantidad_columnas + margen) {
        int x_posicion = columna_linea * tamano_casilla_px;
        pintor.drawLine(
            x_posicion,
            -margen * tamano_casilla_px,
            x_posicion,
            (cantidad_filas + margen) * tamano_casilla_px);
        columna_linea++;
    }
    int fila_linea = -margen;
    while (fila_linea <= cantidad_filas + margen) {
        int y_posicion = fila_linea * tamano_casilla_px;
        pintor.drawLine(
            -margen * tamano_casilla_px,
            y_posicion,
            (cantidad_columnas + margen) * tamano_casilla_px,
            y_posicion);
        fila_linea++;
    }
    if (margen > 0) {
        pintor.setPen(QPen(QColor(100, 140, 180, 110), 1, Qt::DashLine));
        pintor.drawRect(
            0,
            0,
            cantidad_columnas * tamano_casilla_px,
            cantidad_filas * tamano_casilla_px);
    }

    // 4. Previsualización del Hover o Arrastre
    bool hover_en_area_editable =
        (sector_actual == SectorActual::CAVERNA)
            ? (columna_hover >= -margen && columna_hover < cantidad_columnas + margen &&
               fila_hover >= -margen && fila_hover < cantidad_filas + margen)
            : mapa_actual->esta_dentro(columna_hover, fila_hover);
    if (modo_actual == ModoEditor::TERRENO && hover_en_area_editable) {
        if (id_tile_actual > 0) {
            pintor.save();
            pintor.setOpacity(0.65);
            dibujar_tile_en_lienzo(pintor, id_tile_actual, columna_hover, fila_hover);
            pintor.restore();
        }
        QRect rectangulo_hover(
            columna_hover * tamano_casilla_px,
            fila_hover * tamano_casilla_px,
            tamano_casilla_px,
            tamano_casilla_px);
        pintor.setPen(QPen(QColor(255, 255, 255, 220), 2));
        pintor.drawRect(rectangulo_hover);
    } else if (
        modo_actual == ModoEditor::ESTAMPAR && hover_en_area_editable && mapa_actual != nullptr) {
        DefinicionEstampa estampa = CatalogoDeEstampas::obtener_estampa(estampa_a_colocar);
        if (estampa.ancho_casillas > 0 && estampa.alto_casillas > 0) {
            bool puede_colocarse =
                (sector_actual == SectorActual::CAVERNA &&
                 !mapa_actual->esta_dentro(columna_hover, fila_hover))
                    ? false
                    : mapa_actual->estampa_puede_colocarse_en(estampa, columna_hover, fila_hover);
            if (sector_actual == SectorActual::CAVERNA &&
                (columna_hover < 1 || fila_hover < 1 ||
                 columna_hover + estampa.ancho_casillas > cantidad_columnas - 1 ||
                 fila_hover + estampa.alto_casillas > cantidad_filas - 1)) {
                if (estampa.tipo == TipoEstampa::CAVERNA_ANDAMIO_MINERO &&
                    mapa_actual->esta_dentro(columna_hover, fila_hover) &&
                    mapa_actual->esta_dentro(
                        columna_hover + estampa.ancho_casillas - 1,
                        fila_hover + estampa.alto_casillas - 1)) {
                    puede_colocarse =
                        mapa_actual->estampa_puede_colocarse_en(estampa, columna_hover, fila_hover);
                } else {
                    bool tiene_obstaculo_base = false;
                    size_t indice_revision = 0;
                    while (indice_revision < estampa.casillas.size() && !tiene_obstaculo_base) {
                        if (estampa.casillas[indice_revision].capa == TipoCapa::ESTRUCTURAS) {
                            tiene_obstaculo_base = true;
                        } else {
                            indice_revision++;
                        }
                    }
                    puede_colocarse = !tiene_obstaculo_base;
                }
            }
            QRect rectangulo_estampa(
                columna_hover * tamano_casilla_px,
                fila_hover * tamano_casilla_px,
                estampa.ancho_casillas * tamano_casilla_px,
                estampa.alto_casillas * tamano_casilla_px);

            pintor.save();
            pintor.setOpacity(0.70);
            size_t indice_casilla = 0;
            while (indice_casilla < estampa.casillas.size()) {
                CasillaEstampa casilla = estampa.casillas[indice_casilla];
                int columna_destino = columna_hover + casilla.desplazamiento_columna;
                int fila_destino = fila_hover + casilla.desplazamiento_fila;
                dibujar_tile_en_lienzo(pintor, casilla.id_tile, columna_destino, fila_destino);
                indice_casilla++;
            }
            pintor.restore();

            QColor color_estampa = puede_colocarse ? QColor(0, 191, 255, ALFA_COLOR_ESTAMPA)
                                                   : QColor(255, 0, 0, ALFA_COLOR_ESTAMPA);
            QPen lapiz_borde(
                puede_colocarse ? QColor(0, 191, 255) : QColor(255, 0, 0),
                GROSOR_BORDE_NORMAL,
                Qt::DashLine);
            pintor.setPen(lapiz_borde);

            std::set<std::pair<int, int>> casillas_unicas;
            size_t indice_conjunto = 0;
            while (indice_conjunto < estampa.casillas.size()) {
                casillas_unicas.insert(
                    {estampa.casillas[indice_conjunto].desplazamiento_columna,
                     estampa.casillas[indice_conjunto].desplazamiento_fila});
                indice_conjunto++;
            }

            auto iterador_casilla = casillas_unicas.begin();
            while (iterador_casilla != casillas_unicas.end()) {
                int desplazamiento_columna = iterador_casilla->first;
                int desplazamiento_fila = iterador_casilla->second;
                QRect rectangulo_casilla(
                    (columna_hover + desplazamiento_columna) * tamano_casilla_px,
                    (fila_hover + desplazamiento_fila) * tamano_casilla_px,
                    tamano_casilla_px,
                    tamano_casilla_px);
                pintor.fillRect(rectangulo_casilla, color_estampa);
                pintor.drawRect(rectangulo_casilla);
                iterador_casilla++;
            }
        }
    } else if (
        modo_actual == ModoEditor::BORRAR && columna_hover >= 0 && fila_hover >= 0 &&
        mapa_actual != nullptr) {
        auto* estampa_hover = mapa_actual->buscar_estampa_en_casilla(columna_hover, fila_hover);
        if (estampa_hover != nullptr) {
            QRect rectangulo_borrar(
                estampa_hover->columna * tamano_casilla_px,
                estampa_hover->fila * tamano_casilla_px,
                estampa_hover->ancho * tamano_casilla_px,
                estampa_hover->alto * tamano_casilla_px);
            pintor.fillRect(rectangulo_borrar, QColor(255, 69, 0, 100));
            pintor.setPen(QPen(QColor(255, 69, 0), GROSOR_BORDE_NORMAL, Qt::DashLine));
            pintor.drawRect(rectangulo_borrar);
        } else {
            QRect rectangulo_hover(
                columna_hover * tamano_casilla_px,
                fila_hover * tamano_casilla_px,
                tamano_casilla_px,
                tamano_casilla_px);
            pintor.setPen(QPen(QColor(255, 0, 0, 220), 2));
            pintor.drawRect(rectangulo_hover);
        }
    } else if (modo_actual == ModoEditor::COLOCAR_ENTIDAD && hover_en_area_editable) {
        bool es_puesto_comerciante =
            (entidad_a_colocar == TipoEntidad::PUESTO_MARNIE ||
             entidad_a_colocar == TipoEntidad::PUESTO_CLINT ||
             entidad_a_colocar == TipoEntidad::PUESTO_PIERRE ||
             entidad_a_colocar == TipoEntidad::PUESTO_ROBIN);
        int ancho_defecto = 1;
        int alto_defecto = 1;
        ConfiguracionEntidadesEditor::consultar_dimensiones_por_defecto(
            entidad_a_colocar,
            ancho_defecto,
            alto_defecto);
        int columna_fantasma = columna_hover;
        int fila_fantasma = fila_hover;
        if (MapaEditable::es_transicion_de_borde(entidad_a_colocar)) {
            mapa_actual->ajustar_posicion_transicion_borde(
                columna_hover,
                fila_hover,
                columna_fantasma,
                fila_fantasma,
                ancho_defecto,
                alto_defecto);
        }
        bool ubicable = mapa_actual->entidad_puede_ubicarse_en(
            entidad_a_colocar,
            columna_fantasma,
            fila_fantasma,
            ancho_defecto,
            alto_defecto,
            0);
        QRect rectangulo_fantasma(
            columna_fantasma * tamano_casilla_px,
            fila_fantasma * tamano_casilla_px,
            ancho_defecto * tamano_casilla_px,
            alto_defecto * tamano_casilla_px);
        if (es_puesto_comerciante) {
            auto iterador_visual = mapa_visual_entidades.find(entidad_a_colocar);
            if (iterador_visual != mapa_visual_entidades.end() &&
                iterador_visual->second.pixmap != nullptr &&
                !iterador_visual->second.pixmap->isNull()) {
                pintor.save();
                pintor.setOpacity(ubicable ? 0.65 : 0.40);
                pintor.drawPixmap(
                    rectangulo_fantasma,
                    *iterador_visual->second.pixmap,
                    iterador_visual->second.rectangulo_origen);
                pintor.restore();
            }
            pintor.setBrush(Qt::NoBrush);
            pintor.setPen(
                QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
            pintor.drawRect(rectangulo_fantasma);

            QPolygon flecha;
            int centro_x = rectangulo_fantasma.center().x();
            int centro_y = rectangulo_fantasma.center().y();
            int radio_flecha = tamano_casilla_px / 5;
            if (radio_flecha < 4) {
                radio_flecha = 4;
            }
            if (orientacion_entidad_a_colocar == Direccion::ABAJO) {
                int y_borde = rectangulo_fantasma.bottom() - 1;
                flecha << QPoint(centro_x - radio_flecha, y_borde - radio_flecha)
                       << QPoint(centro_x + radio_flecha, y_borde - radio_flecha)
                       << QPoint(centro_x, y_borde);
            } else if (orientacion_entidad_a_colocar == Direccion::ARRIBA) {
                int y_borde = rectangulo_fantasma.top() + 1;
                flecha << QPoint(centro_x - radio_flecha, y_borde + radio_flecha)
                       << QPoint(centro_x + radio_flecha, y_borde + radio_flecha)
                       << QPoint(centro_x, y_borde);
            } else if (orientacion_entidad_a_colocar == Direccion::DERECHA) {
                int x_borde = rectangulo_fantasma.right() - 1;
                flecha << QPoint(x_borde - radio_flecha, centro_y - radio_flecha)
                       << QPoint(x_borde - radio_flecha, centro_y + radio_flecha)
                       << QPoint(x_borde, centro_y);
            } else {
                int x_borde = rectangulo_fantasma.left() + 1;
                flecha << QPoint(x_borde + radio_flecha, centro_y - radio_flecha)
                       << QPoint(x_borde + radio_flecha, centro_y + radio_flecha)
                       << QPoint(x_borde, centro_y);
            }
            pintor.setBrush(QColor(255, 255, 255, 230));
            pintor.setPen(QPen(Qt::black, 1));
            pintor.drawPolygon(flecha);
            pintor.setBrush(Qt::NoBrush);

            int columna_inter = 0;
            int fila_inter = 0;
            int columna_aten = 0;
            int fila_aten = 0;
            mapa_actual->calcular_interaccion_puesto(
                columna_hover,
                fila_hover,
                orientacion_entidad_a_colocar,
                columna_inter,
                fila_inter,
                columna_aten,
                fila_aten);
            QRect rectangulo_venta_fantasma(
                columna_inter * tamano_casilla_px,
                fila_inter * tamano_casilla_px,
                tamano_casilla_px,
                tamano_casilla_px);
            pintor.setBrush(Qt::NoBrush);
            pintor.setPen(QPen(
                ubicable ? QColor(0, 255, 127) : QColor(255, 0, 0),
                GROSOR_BORDE_NORMAL,
                Qt::DashLine));
            pintor.drawRect(rectangulo_venta_fantasma);
        } else {
            bool es_entidad_caverna =
                (entidad_a_colocar == TipoEntidad::ESCALERA_NIVEL ||
                 entidad_a_colocar == TipoEntidad::SALIDA_CAVERNA);
            if (es_entidad_caverna) {
                pintor.save();
                pintor.setOpacity(ubicable ? 0.80 : 0.45);
                dibujar_visual_entidad(
                    pintor,
                    entidad_a_colocar,
                    columna_hover,
                    fila_hover,
                    rectangulo_fantasma);
                pintor.restore();
                pintor.setBrush(Qt::NoBrush);
                pintor.setPen(
                    QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
                pintor.drawRect(rectangulo_fantasma);
            } else {
                QColor color_fantasma = ubicable ? QColor(0, 255, 0, ALFA_COLOR_FANTASMA)
                                                 : QColor(255, 0, 0, ALFA_COLOR_FANTASMA);
                pintor.fillRect(rectangulo_fantasma, color_fantasma);
                pintor.setPen(
                    QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
                pintor.drawRect(rectangulo_fantasma);
            }
        }
    } else if (
        modo_actual == ModoEditor::SELECCIONAR_MOVER && arrastrando_entidad &&
        identificador_entidad_seleccionada >= 0) {
        uint32_t id_seleccionada = static_cast<uint32_t>(identificador_entidad_seleccionada);
        EntidadDelMapa* entidad_ptr =
            mapa_actual->buscar_entidad_por_identificador(id_seleccionada);
        if (entidad_ptr != nullptr) {
            int ancho_ent = 0;
            int alto_ent = 0;
            entidad_ptr->consultar_dimensiones(ancho_ent, alto_ent);
            TipoEntidad tipo_ent = entidad_ptr->obtener_tipo();
            bool es_puesto_comerciante =
                (tipo_ent == TipoEntidad::PUESTO_MARNIE || tipo_ent == TipoEntidad::PUESTO_CLINT ||
                 tipo_ent == TipoEntidad::PUESTO_PIERRE || tipo_ent == TipoEntidad::PUESTO_ROBIN);
            int columna_arrastre_mostrada = columna_arrastre_actual;
            int fila_arrastre_mostrada = fila_arrastre_actual;
            if (MapaEditable::es_transicion_de_borde(tipo_ent)) {
                mapa_actual->ajustar_posicion_transicion_borde(
                    columna_arrastre_actual,
                    fila_arrastre_actual,
                    columna_arrastre_mostrada,
                    fila_arrastre_mostrada,
                    ancho_ent,
                    alto_ent);
            }
            bool ubicable = mapa_actual->entidad_puede_ubicarse_en(
                tipo_ent,
                columna_arrastre_mostrada,
                fila_arrastre_mostrada,
                ancho_ent,
                alto_ent,
                id_seleccionada);
            if (!ubicable && es_puesto_comerciante) {
                EntidadDelMapa* otro = mapa_actual->buscar_entidad_en_casilla(
                    columna_arrastre_actual,
                    fila_arrastre_actual);
                if (otro != nullptr && otro->obtener_identificador() != id_seleccionada) {
                    TipoEntidad tipo_otro = otro->obtener_tipo();
                    if (tipo_otro == TipoEntidad::PUESTO_MARNIE ||
                        tipo_otro == TipoEntidad::PUESTO_CLINT ||
                        tipo_otro == TipoEntidad::PUESTO_PIERRE ||
                        tipo_otro == TipoEntidad::PUESTO_ROBIN) {
                        ubicable = true;
                    }
                }
            }
            QRect rectangulo_arrastre(
                columna_arrastre_mostrada * tamano_casilla_px,
                fila_arrastre_mostrada * tamano_casilla_px,
                ancho_ent * tamano_casilla_px,
                alto_ent * tamano_casilla_px);
            if (es_puesto_comerciante) {
                auto iterador_visual = mapa_visual_entidades.find(tipo_ent);
                if (iterador_visual != mapa_visual_entidades.end() &&
                    iterador_visual->second.pixmap != nullptr &&
                    !iterador_visual->second.pixmap->isNull()) {
                    pintor.save();
                    pintor.setOpacity(ubicable ? 0.65 : 0.40);
                    pintor.drawPixmap(
                        rectangulo_arrastre,
                        *iterador_visual->second.pixmap,
                        iterador_visual->second.rectangulo_origen);
                    pintor.restore();
                }
                pintor.setBrush(Qt::NoBrush);
                pintor.setPen(
                    QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
                pintor.drawRect(rectangulo_arrastre);

                Direccion orientacion = entidad_ptr->obtener_orientacion();
                QPolygon flecha;
                int centro_x = rectangulo_arrastre.center().x();
                int centro_y = rectangulo_arrastre.center().y();
                int radio_flecha = tamano_casilla_px / 5;
                if (radio_flecha < 4) {
                    radio_flecha = 4;
                }
                if (orientacion == Direccion::ABAJO) {
                    int y_borde = rectangulo_arrastre.bottom() - 1;
                    flecha << QPoint(centro_x - radio_flecha, y_borde - radio_flecha)
                           << QPoint(centro_x + radio_flecha, y_borde - radio_flecha)
                           << QPoint(centro_x, y_borde);
                } else if (orientacion == Direccion::ARRIBA) {
                    int y_borde = rectangulo_arrastre.top() + 1;
                    flecha << QPoint(centro_x - radio_flecha, y_borde + radio_flecha)
                           << QPoint(centro_x + radio_flecha, y_borde + radio_flecha)
                           << QPoint(centro_x, y_borde);
                } else if (orientacion == Direccion::DERECHA) {
                    int x_borde = rectangulo_arrastre.right() - 1;
                    flecha << QPoint(x_borde - radio_flecha, centro_y - radio_flecha)
                           << QPoint(x_borde - radio_flecha, centro_y + radio_flecha)
                           << QPoint(x_borde, centro_y);
                } else {
                    int x_borde = rectangulo_arrastre.left() + 1;
                    flecha << QPoint(x_borde + radio_flecha, centro_y - radio_flecha)
                           << QPoint(x_borde + radio_flecha, centro_y + radio_flecha)
                           << QPoint(x_borde, centro_y);
                }
                pintor.setBrush(QColor(255, 255, 255, 230));
                pintor.setPen(QPen(Qt::black, 1));
                pintor.drawPolygon(flecha);
                pintor.setBrush(Qt::NoBrush);

                int columna_inter = 0;
                int fila_inter = 0;
                int columna_aten = 0;
                int fila_aten = 0;
                mapa_actual->calcular_interaccion_puesto(
                    columna_arrastre_actual,
                    fila_arrastre_actual,
                    orientacion,
                    columna_inter,
                    fila_inter,
                    columna_aten,
                    fila_aten);
                QRect rectangulo_venta_arrastre(
                    columna_inter * tamano_casilla_px,
                    fila_inter * tamano_casilla_px,
                    tamano_casilla_px,
                    tamano_casilla_px);
                pintor.setBrush(Qt::NoBrush);
                pintor.setPen(QPen(
                    ubicable ? QColor(0, 255, 127) : QColor(255, 0, 0),
                    GROSOR_BORDE_NORMAL,
                    Qt::DashLine));
                pintor.drawRect(rectangulo_venta_arrastre);
            } else {
                bool es_entidad_caverna =
                    (tipo_ent == TipoEntidad::ESCALERA_NIVEL ||
                     tipo_ent == TipoEntidad::SALIDA_CAVERNA);
                if (es_entidad_caverna) {
                    pintor.save();
                    pintor.setOpacity(ubicable ? 0.80 : 0.45);
                    dibujar_visual_entidad(
                        pintor,
                        tipo_ent,
                        columna_arrastre_actual,
                        fila_arrastre_actual,
                        rectangulo_arrastre);
                    pintor.restore();
                } else {
                    QColor color_arrastre = ubicable ? QColor(0, 255, 0, ALFA_COLOR_ARRASTRE)
                                                     : QColor(255, 0, 0, ALFA_COLOR_ARRASTRE);
                    pintor.fillRect(rectangulo_arrastre, color_arrastre);
                }
                pintor.setPen(
                    QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
                pintor.drawRect(rectangulo_arrastre);
            }
        }
    }
    pintor.restore();
}

void LienzoDeEdicion::mousePressEvent(QMouseEvent* evento) {
    if (mapa_actual == nullptr) {
        return;
    }
    int columna = 0;
    int fila = 0;
    obtener_coordenadas_desde_posicion(evento->pos(), columna, fila);
    int margen = obtener_margen_visible();
    int cantidad_columnas = mapa_actual->obtener_cantidad_de_columnas();
    int cantidad_filas = mapa_actual->obtener_cantidad_de_filas();
    bool dentro_area_editable = (sector_actual == SectorActual::CAVERNA)
                                    ? (columna >= -margen && columna < cantidad_columnas + margen &&
                                       fila >= -margen && fila < cantidad_filas + margen)
                                    : mapa_actual->esta_dentro(columna, fila);
    if (!dentro_area_editable) {
        return;
    }

    if (evento->button() == Qt::LeftButton) {
        if (modo_actual == ModoEditor::TERRENO) {
            mouse_presionado = true;
            pintar_casilla_en_posicion(columna, fila);
        } else if (modo_actual == ModoEditor::ESTAMPAR) {
            DefinicionEstampa definicion_estampa =
                CatalogoDeEstampas::obtener_estampa(estampa_a_colocar);
            if (definicion_estampa.tipo != TipoEstampa::NINGUNA) {
                bool ya_dentro_andamio =
                    (definicion_estampa.tipo == TipoEstampa::CAVERNA_ANDAMIO_MINERO &&
                     mapa_actual->esta_dentro(columna, fila) &&
                     mapa_actual->esta_dentro(
                         columna + definicion_estampa.ancho_casillas - 1,
                         fila + definicion_estampa.alto_casillas - 1));
                if (!ya_dentro_andamio) {
                    asegurar_espacio_en_caverna(
                        columna,
                        fila,
                        definicion_estampa.ancho_casillas,
                        definicion_estampa.alto_casillas);
                }
                if (mapa_actual->estampa_puede_colocarse_en(definicion_estampa, columna, fila)) {
                    mapa_actual->estampar(definicion_estampa, columna, fila);
                    emit mapa_modificado();
                    update();
                }
            }
        } else if (modo_actual == ModoEditor::COLOCAR_ENTIDAD) {
            int ancho_defecto = 1;
            int alto_defecto = 1;
            ConfiguracionEntidadesEditor::consultar_dimensiones_por_defecto(
                entidad_a_colocar,
                ancho_defecto,
                alto_defecto);
            int columna_colocacion = columna;
            int fila_colocacion = fila;
            if (MapaEditable::es_transicion_de_borde(entidad_a_colocar)) {
                mapa_actual->ajustar_posicion_transicion_borde(
                    columna,
                    fila,
                    columna_colocacion,
                    fila_colocacion,
                    ancho_defecto,
                    alto_defecto);
            }
            asegurar_espacio_en_caverna(
                columna_colocacion,
                fila_colocacion,
                ancho_defecto,
                alto_defecto);
            if (mapa_actual->entidad_puede_ubicarse_en(
                    entidad_a_colocar,
                    columna_colocacion,
                    fila_colocacion,
                    ancho_defecto,
                    alto_defecto,
                    0)) {
                uint32_t nuevo_id = mapa_actual->ubicar_entidad(
                    entidad_a_colocar,
                    columna_colocacion,
                    fila_colocacion,
                    ancho_defecto,
                    alto_defecto,
                    orientacion_entidad_a_colocar);
                identificador_entidad_seleccionada = static_cast<int>(nuevo_id);
                emit entidad_seleccionada_cambio(identificador_entidad_seleccionada);
                emit mapa_modificado();
                update();
            }
        } else if (modo_actual == ModoEditor::SELECCIONAR_MOVER) {
            EntidadDelMapa* entidad_tocada = mapa_actual->buscar_entidad_en_casilla(columna, fila);
            if (entidad_tocada != nullptr) {
                identificador_entidad_seleccionada =
                    static_cast<int>(entidad_tocada->obtener_identificador());
                arrastrando_entidad = true;
                columna_arrastre_actual = columna;
                fila_arrastre_actual = fila;
                emit entidad_seleccionada_cambio(identificador_entidad_seleccionada);
            } else {
                identificador_entidad_seleccionada = -1;
                arrastrando_entidad = false;
                emit entidad_seleccionada_cambio(-1);
            }
            update();
        } else if (modo_actual == ModoEditor::BORRAR) {
            borrar_en_casilla(columna, fila);
        }
    } else if (evento->button() == Qt::RightButton) {
        borrar_en_casilla(columna, fila);
    }
}

void LienzoDeEdicion::mouseMoveEvent(QMouseEvent* evento) {
    if (mapa_actual == nullptr) {
        return;
    }
    int columna = 0;
    int fila = 0;
    obtener_coordenadas_desde_posicion(evento->pos(), columna, fila);
    columna_hover = columna;
    fila_hover = fila;

    emit casilla_apuntada(columna, fila);

    if (mouse_presionado && modo_actual == ModoEditor::TERRENO) {
        pintar_casilla_en_posicion(columna, fila);
    } else if (arrastrando_entidad && modo_actual == ModoEditor::SELECCIONAR_MOVER) {
        columna_arrastre_actual = columna;
        fila_arrastre_actual = fila;
        update();
    } else {
        update();
    }
}

void LienzoDeEdicion::mouseReleaseEvent(QMouseEvent* evento) {
    if (evento->button() == Qt::LeftButton) {
        mouse_presionado = false;
        if (trazo_terreno_activo) {
            trazo_terreno_activo = false;
            emit trazo_finalizado();
        }
        if (arrastrando_entidad && identificador_entidad_seleccionada >= 0 &&
            mapa_actual != nullptr) {
            uint32_t id_seleccionada = static_cast<uint32_t>(identificador_entidad_seleccionada);
            bool deslizado = mapa_actual->deslizar_entidad(
                id_seleccionada,
                columna_arrastre_actual,
                fila_arrastre_actual);
            arrastrando_entidad = false;
            if (deslizado) {
                emit mapa_modificado();
            }
            update();
        }
    }
}

void LienzoDeEdicion::leaveEvent(QEvent* evento) {
    (void)evento;
    columna_hover = -1;
    fila_hover = -1;
    update();
}

void LienzoDeEdicion::keyPressEvent(QKeyEvent* evento) {
    if (evento->key() == Qt::Key_R) {
        rotar_elemento_actual();
        evento->accept();
        return;
    }
    QWidget::keyPressEvent(evento);
}

void LienzoDeEdicion::rotar_elemento_actual() {
    if (modo_actual == ModoEditor::ESTAMPAR) {
        TipoEstampa nueva_estampa = CatalogoDeEstampas::rotar_estampa(estampa_a_colocar);
        if (nueva_estampa != estampa_a_colocar) {
            establecer_estampa_a_colocar(nueva_estampa);
            emit estampa_rotada(nueva_estampa);
            update();
        }
    } else if (modo_actual == ModoEditor::TERRENO) {
        uint32_t nuevo_id = CatalogoDeTerrenos::rotar_tile_terreno(id_tile_actual);
        if (nuevo_id != id_tile_actual) {
            establecer_tile(nuevo_id, propiedades_actuales);
            emit tile_rotado(nuevo_id);
            update();
        }
    } else if (modo_actual == ModoEditor::COLOCAR_ENTIDAD) {
        bool es_puesto_comerciante =
            (entidad_a_colocar == TipoEntidad::PUESTO_MARNIE ||
             entidad_a_colocar == TipoEntidad::PUESTO_CLINT ||
             entidad_a_colocar == TipoEntidad::PUESTO_PIERRE ||
             entidad_a_colocar == TipoEntidad::PUESTO_ROBIN);
        if (es_puesto_comerciante) {
            if (orientacion_entidad_a_colocar == Direccion::ABAJO) {
                orientacion_entidad_a_colocar = Direccion::DERECHA;
            } else if (orientacion_entidad_a_colocar == Direccion::DERECHA) {
                orientacion_entidad_a_colocar = Direccion::ARRIBA;
            } else if (orientacion_entidad_a_colocar == Direccion::ARRIBA) {
                orientacion_entidad_a_colocar = Direccion::IZQUIERDA;
            } else {
                orientacion_entidad_a_colocar = Direccion::ABAJO;
            }
            update();
        }
    } else if (
        modo_actual == ModoEditor::SELECCIONAR_MOVER && identificador_entidad_seleccionada >= 0 &&
        mapa_actual != nullptr) {
        uint32_t id_seleccionada = static_cast<uint32_t>(identificador_entidad_seleccionada);
        EntidadDelMapa* seleccionada =
            mapa_actual->buscar_entidad_por_identificador(id_seleccionada);
        if (seleccionada != nullptr) {
            TipoEntidad tipo_entidad = seleccionada->obtener_tipo();
            bool es_puesto_comerciante =
                (tipo_entidad == TipoEntidad::PUESTO_MARNIE ||
                 tipo_entidad == TipoEntidad::PUESTO_CLINT ||
                 tipo_entidad == TipoEntidad::PUESTO_PIERRE ||
                 tipo_entidad == TipoEntidad::PUESTO_ROBIN);
            if (es_puesto_comerciante) {
                seleccionada->rotar_orientacion();
                int origen_columna = 0;
                int origen_fila = 0;
                seleccionada->consultar_origen(origen_columna, origen_fila);
                int columna_interaccion = 0;
                int fila_interaccion = 0;
                int columna_atencion = 0;
                int fila_atencion = 0;
                mapa_actual->calcular_interaccion_puesto(
                    origen_columna,
                    origen_fila,
                    seleccionada->obtener_orientacion(),
                    columna_interaccion,
                    fila_interaccion,
                    columna_atencion,
                    fila_atencion);
                seleccionada->configurar_punto_de_atencion(columna_atencion, fila_atencion);
                emit mapa_modificado();
                update();
            }
        }
    }
}

TipoEstampa LienzoDeEdicion::obtener_estampa_a_colocar() const {
    return estampa_a_colocar;
}

uint32_t LienzoDeEdicion::obtener_id_tile_actual() const {
    return id_tile_actual;
}
