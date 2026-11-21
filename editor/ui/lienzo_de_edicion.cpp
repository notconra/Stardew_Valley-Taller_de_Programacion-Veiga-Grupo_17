#include "lienzo_de_edicion.h"

#include <QColor>
#include <QEvent>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <map>
#include <string>
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

static constexpr int COLUMNA_PUERTA_CABANIA = 5;
static constexpr int FILA_PUERTA_CABANIA = 7;
static constexpr int COLUMNA_PUERTA_CAVERNA = 1;
static constexpr int FILA_PUERTA_CAVERNA = 3;
static constexpr int COLUMNA_PUERTA_CENTRO_COMUNITARIO = 5;
static constexpr int ANCHO_PUERTA_CENTRO_COMUNITARIO = 2;

LienzoDeEdicion::LienzoDeEdicion(QWidget* padre)
    : QWidget(padre),
      mapa_actual(nullptr),
      sector_actual(SectorActual::GRANJA),
      capa_activa(TipoCapa::SUELO),
      tamano_casilla_px(28),
      modo_actual(ModoEditor::TERRENO),
      id_tile_actual(CatalogoDeTerrenos::obtener_tile_base(SectorActual::GRANJA)),
      propiedades_actuales(
          PropiedadDeCasilla::CULTIVABLE | PropiedadDeCasilla::DECORABLE |
          PropiedadDeCasilla::ADMITE_HIERBA),
      entidad_a_colocar(TipoEntidad::CABANIA),
      estampa_a_colocar(TipoEstampa::ESTANQUE_NATURAL),
      pixmap_tileset(QString::fromStdString(
          CatalogoDeTerrenos::obtener_tileset_sector(SectorActual::GRANJA).ruta_imagen)),
      datos_tileset(CatalogoDeTerrenos::obtener_tileset_sector(SectorActual::GRANJA)),
      pixmap_cabania(QString(RUTA_SPRITE_CABANIA)),
      pixmap_exteriores_entidades(QString(RUTA_SPRITE_EXTERIORES)),
      pixmap_caverna_entidades(QString(RUTA_SPRITE_CAVERNA)),
      pixmap_pueblo_entidades(QString(RUTA_SPRITE_PUEBLO)),
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
      mouse_presionado(false) {
    setMouseTracking(true);
    inicializar_catalogo_visual_entidades();
    actualizar_tamano_del_lienzo();
}

void LienzoDeEdicion::establecer_mapa(MapaEditable* mapa, SectorActual sector) {
    mapa_actual = mapa;
    sector_actual = sector;
    datos_tileset = CatalogoDeTerrenos::obtener_tileset_sector(sector);
    pixmap_tileset.load(QString::fromStdString(datos_tileset.ruta_imagen));
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

void LienzoDeEdicion::establecer_capa_activa(TipoCapa nueva_capa) {
    capa_activa = nueva_capa;
    update();
}

TipoCapa LienzoDeEdicion::obtener_capa_activa() const {
    return capa_activa;
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

void LienzoDeEdicion::obtener_coordenadas_desde_posicion(
    const QPoint& posicion, int& columna, int& fila) const {
    if (tamano_casilla_px <= 0) {
        columna = 0;
        fila = 0;
        return;
    }
    columna = posicion.x() / tamano_casilla_px;
    fila = posicion.y() / tamano_casilla_px;
}

void LienzoDeEdicion::actualizar_tamano_del_lienzo() {
    if (mapa_actual == nullptr) {
        setFixedSize(400, 400);
        return;
    }
    int ancho_total = mapa_actual->obtener_cantidad_de_columnas() * tamano_casilla_px;
    int alto_total = mapa_actual->obtener_cantidad_de_filas() * tamano_casilla_px;
    setFixedSize(ancho_total, alto_total);
}

void LienzoDeEdicion::pintar_casilla_en_posicion(int columna, int fila) {
    if (mapa_actual == nullptr || !mapa_actual->esta_dentro(columna, fila)) {
        return;
    }
    mapa_actual->pintar_en_capa(capa_activa, columna, fila, id_tile_actual, propiedades_actuales);
    emit mapa_modificado();
    update();
}

void LienzoDeEdicion::dibujar_capa_de_tiles(
    QPainter& pintor, TipoCapa tipo_capa, int cantidad_columnas, int cantidad_filas) {
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            CasillaDelMapa casilla = mapa_actual->obtener_casilla_en_capa(tipo_capa, columna, fila);
            uint32_t id_tile = casilla.obtener_id_tile();
            if (id_tile > 0 && !pixmap_tileset.isNull() &&
                datos_tileset.primer_identificador_global > 0 &&
                datos_tileset.cantidad_de_columnas > 0) {
                uint32_t indice_local = id_tile - datos_tileset.primer_identificador_global;
                int columna_sprite = static_cast<int>(
                    indice_local % static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));
                int fila_sprite = static_cast<int>(
                    indice_local / static_cast<uint32_t>(datos_tileset.cantidad_de_columnas));

                QRect rectangulo_destino(
                    columna * tamano_casilla_px,
                    fila * tamano_casilla_px,
                    tamano_casilla_px,
                    tamano_casilla_px);
                QRect rectangulo_origen(
                    columna_sprite * datos_tileset.ancho_tile,
                    fila_sprite * datos_tileset.alto_tile,
                    datos_tileset.ancho_tile,
                    datos_tileset.alto_tile);

                pintor.drawPixmap(rectangulo_destino, pixmap_tileset, rectangulo_origen);
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
    {TipoEntidad::ESCALERA_NIVEL, QColor(139, 69, 19)},
    {TipoEntidad::SALIDA_CAVERNA, QColor(0, 139, 139)},
    {TipoEntidad::TRANSICION_PUEBLO, QColor(30, 144, 255)},
    {TipoEntidad::TRANSICION_GRANJA, QColor(50, 168, 82)}};

void LienzoDeEdicion::inicializar_catalogo_visual_entidades() {
    mapa_visual_entidades = {
        {TipoEntidad::CABANIA, {&pixmap_cabania, QRect(16, 0, 144, 144)}},
        {TipoEntidad::ENTRADA_CAVERNA, {&pixmap_exteriores_entidades, QRect(128, 1040, 48, 64)}},
        {TipoEntidad::CENTRO_COMUNITARIO, {&pixmap_pueblo_entidades, QRect(0, 640, 192, 160)}},
        {TipoEntidad::ESCALERA_NIVEL, {&pixmap_caverna_entidades, QRect(208, 160, 16, 16)}},
        {TipoEntidad::SALIDA_CAVERNA, {&pixmap_caverna_entidades, QRect(48, 112, 16, 16)}},
        {TipoEntidad::PUESTO_MARNIE, {&pixmap_retrato_marnie, QRect(0, 0, 64, 64)}},
        {TipoEntidad::PUESTO_CLINT, {&pixmap_retrato_clint, QRect(0, 0, 64, 64)}},
        {TipoEntidad::PUESTO_PIERRE, {&pixmap_retrato_pierre, QRect(0, 0, 64, 64)}},
        {TipoEntidad::PUESTO_ROBIN, {&pixmap_retrato_robin, QRect(0, 0, 64, 64)}}};
}

void LienzoDeEdicion::dibujar_entidad(
    QPainter& pintor, const EntidadDelMapa& entidad, const QRect& rectangulo_entidad,
    bool es_seleccionada) {
    TipoEntidad tipo_entidad = entidad.obtener_tipo();
    auto iterador_color = COLORES_ENTIDADES.find(tipo_entidad);
    QColor color_base =
        iterador_color != COLORES_ENTIDADES.end() ? iterador_color->second : QColor(100, 100, 100);

    QColor color_fondo = color_base;
    color_fondo.setAlpha(ALFA_COLOR_FONDO_ENTIDAD);
    pintor.fillRect(rectangulo_entidad, color_fondo);

    if (tipo_entidad == TipoEntidad::ENTRADA_CAVERNA) {
        int x_hueco = rectangulo_entidad.x() + tamano_casilla_px;
        int y_hueco = rectangulo_entidad.y() + tamano_casilla_px;
        int ancho_hueco = tamano_casilla_px;
        int alto_hueco = rectangulo_entidad.height() - tamano_casilla_px;
        pintor.fillRect(QRect(x_hueco, y_hueco, ancho_hueco, alto_hueco), QColor(0, 0, 0));
    }

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
        tipo_entidad == TipoEntidad::ENTRADA_CAVERNA) {
        int origen_columna = 0;
        int origen_fila = 0;
        entidad.consultar_origen(origen_columna, origen_fila);
        int ancho_casillas = 0;
        int alto_casillas = 0;
        entidad.consultar_dimensiones(ancho_casillas, alto_casillas);

        int columna_puerta = origen_columna + (tipo_entidad == TipoEntidad::ENTRADA_CAVERNA
                                                   ? COLUMNA_PUERTA_CAVERNA
                                                   : COLUMNA_PUERTA_CABANIA);
        int fila_puerta = origen_fila + (tipo_entidad == TipoEntidad::CABANIA ? FILA_PUERTA_CABANIA
                                                                              : alto_casillas - 1);
        int ancho_puerta =
            (tipo_entidad == TipoEntidad::CENTRO_COMUNITARIO) ? ANCHO_PUERTA_CENTRO_COMUNITARIO : 1;
        QRect rectangulo_puerta(
            columna_puerta * tamano_casilla_px,
            fila_puerta * tamano_casilla_px,
            ancho_puerta * tamano_casilla_px,
            tamano_casilla_px);
        pintor.setPen(QPen(QColor(0, 255, 127), GROSOR_BORDE_PUERTA, Qt::DashLine));
        pintor.drawRect(rectangulo_puerta);
    }

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

    // 1. Fondo oscuro
    pintor.fillRect(rect(), QColor(25, 25, 25));

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

    // 5. Dibujar Lineas de Grilla
    pintor.setPen(QColor(0, 0, 0, 45));
    int columna_linea = 0;
    while (columna_linea <= cantidad_columnas) {
        int x_posicion = columna_linea * tamano_casilla_px;
        pintor.drawLine(x_posicion, 0, x_posicion, cantidad_filas * tamano_casilla_px);
        columna_linea++;
    }
    int fila_linea = 0;
    while (fila_linea <= cantidad_filas) {
        int y_posicion = fila_linea * tamano_casilla_px;
        pintor.drawLine(0, y_posicion, cantidad_columnas * tamano_casilla_px, y_posicion);
        fila_linea++;
    }

    // 4. Previsualización del Hover o Arrastre
    if (modo_actual == ModoEditor::TERRENO && columna_hover >= 0 && fila_hover >= 0 &&
        mapa_actual->esta_dentro(columna_hover, fila_hover)) {
        QRect rectangulo_hover(
            columna_hover * tamano_casilla_px,
            fila_hover * tamano_casilla_px,
            tamano_casilla_px,
            tamano_casilla_px);
        pintor.setPen(QPen(QColor(255, 255, 255, 220), 2));
        pintor.drawRect(rectangulo_hover);
    } else if (
        modo_actual == ModoEditor::ESTAMPAR && columna_hover >= 0 && fila_hover >= 0 &&
        mapa_actual != nullptr) {
        DefinicionEstampa estampa = CatalogoDeEstampas::obtener_estampa(estampa_a_colocar);
        if (estampa.ancho_casillas > 0 && estampa.alto_casillas > 0) {
            bool dentro = mapa_actual->esta_dentro(columna_hover, fila_hover) &&
                          mapa_actual->esta_dentro(
                              columna_hover + estampa.ancho_casillas - 1,
                              fila_hover + estampa.alto_casillas - 1);
            QRect rectangulo_estampa(
                columna_hover * tamano_casilla_px,
                fila_hover * tamano_casilla_px,
                estampa.ancho_casillas * tamano_casilla_px,
                estampa.alto_casillas * tamano_casilla_px);
            QColor color_estampa = dentro ? QColor(0, 191, 255, ALFA_COLOR_ESTAMPA)
                                          : QColor(255, 0, 0, ALFA_COLOR_ESTAMPA);
            pintor.fillRect(rectangulo_estampa, color_estampa);
            pintor.setPen(QPen(
                dentro ? QColor(0, 191, 255) : QColor(255, 0, 0),
                GROSOR_BORDE_NORMAL,
                Qt::DashLine));
            pintor.drawRect(rectangulo_estampa);
        }
    } else if (
        modo_actual == ModoEditor::COLOCAR_ENTIDAD && columna_hover >= 0 && fila_hover >= 0) {
        int ancho_defecto = 1;
        int alto_defecto = 1;
        ConfiguracionEntidadesEditor::consultar_dimensiones_por_defecto(
            entidad_a_colocar,
            ancho_defecto,
            alto_defecto);
        bool ubicable = mapa_actual->entidad_puede_ubicarse_en(
            entidad_a_colocar,
            columna_hover,
            fila_hover,
            ancho_defecto,
            alto_defecto,
            0);
        QRect rectangulo_fantasma(
            columna_hover * tamano_casilla_px,
            fila_hover * tamano_casilla_px,
            ancho_defecto * tamano_casilla_px,
            alto_defecto * tamano_casilla_px);
        QColor color_fantasma = ubicable ? QColor(0, 255, 0, ALFA_COLOR_FANTASMA)
                                         : QColor(255, 0, 0, ALFA_COLOR_FANTASMA);
        pintor.fillRect(rectangulo_fantasma, color_fantasma);
        pintor.setPen(QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
        pintor.drawRect(rectangulo_fantasma);
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
            bool ubicable = mapa_actual->entidad_puede_ubicarse_en(
                tipo_ent,
                columna_arrastre_actual,
                fila_arrastre_actual,
                ancho_ent,
                alto_ent,
                id_seleccionada);
            QRect rectangulo_arrastre(
                columna_arrastre_actual * tamano_casilla_px,
                fila_arrastre_actual * tamano_casilla_px,
                ancho_ent * tamano_casilla_px,
                alto_ent * tamano_casilla_px);
            QColor color_arrastre = ubicable ? QColor(0, 255, 0, ALFA_COLOR_ARRASTRE)
                                             : QColor(255, 0, 0, ALFA_COLOR_ARRASTRE);
            pintor.fillRect(rectangulo_arrastre, color_arrastre);
            pintor.setPen(
                QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), GROSOR_BORDE_NORMAL));
            pintor.drawRect(rectangulo_arrastre);
        }
    }
}

void LienzoDeEdicion::mousePressEvent(QMouseEvent* evento) {
    if (mapa_actual == nullptr) {
        return;
    }
    int columna = 0;
    int fila = 0;
    obtener_coordenadas_desde_posicion(evento->pos(), columna, fila);
    if (!mapa_actual->esta_dentro(columna, fila)) {
        return;
    }

    if (evento->button() == Qt::LeftButton) {
        mouse_presionado = true;
        if (modo_actual == ModoEditor::TERRENO) {
            pintar_casilla_en_posicion(columna, fila);
        } else if (modo_actual == ModoEditor::ESTAMPAR) {
            DefinicionEstampa definicion_estampa =
                CatalogoDeEstampas::obtener_estampa(estampa_a_colocar);
            if (definicion_estampa.tipo != TipoEstampa::NINGUNA) {
                mapa_actual->estampar(definicion_estampa, columna, fila);
                emit mapa_modificado();
                update();
            }
        } else if (modo_actual == ModoEditor::COLOCAR_ENTIDAD) {
            int ancho_defecto = 1;
            int alto_defecto = 1;
            ConfiguracionEntidadesEditor::consultar_dimensiones_por_defecto(
                entidad_a_colocar,
                ancho_defecto,
                alto_defecto);
            if (mapa_actual->entidad_puede_ubicarse_en(
                    entidad_a_colocar,
                    columna,
                    fila,
                    ancho_defecto,
                    alto_defecto,
                    0)) {
                uint32_t nuevo_id = mapa_actual->ubicar_entidad(
                    entidad_a_colocar,
                    columna,
                    fila,
                    ancho_defecto,
                    alto_defecto);
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
            EntidadDelMapa* entidad_tocada = mapa_actual->buscar_entidad_en_casilla(columna, fila);
            if (entidad_tocada != nullptr) {
                mapa_actual->retirar_entidad(entidad_tocada->obtener_identificador());
                identificador_entidad_seleccionada = -1;
                emit entidad_seleccionada_cambio(-1);
            } else {
                mapa_actual->borrar_en_capa(capa_activa, columna, fila);
            }
            emit mapa_modificado();
            update();
        }
    } else if (evento->button() == Qt::RightButton) {
        EntidadDelMapa* entidad_tocada = mapa_actual->buscar_entidad_en_casilla(columna, fila);
        if (entidad_tocada != nullptr) {
            mapa_actual->retirar_entidad(entidad_tocada->obtener_identificador());
            identificador_entidad_seleccionada = -1;
            emit entidad_seleccionada_cambio(-1);
        } else {
            mapa_actual->borrar_en_capa(capa_activa, columna, fila);
        }
        emit mapa_modificado();
        update();
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
