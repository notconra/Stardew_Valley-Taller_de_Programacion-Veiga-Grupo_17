#include "lienzo_de_edicion.h"

#include <QColor>
#include <QEvent>
#include <QFont>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <string>
#include <vector>

#include "casilla_del_mapa.h"
#include "entidad_del_mapa.h"
#include "mapa_editable.h"
#include "propiedad_de_casilla.h"
#include "tipo_entidad.h"
#include "tipo_herramienta_editor.h"

LienzoDeEdicion::LienzoDeEdicion(QWidget* padre)
    : QWidget(padre),
      mapa_actual(nullptr),
      sector_actual(SectorActual::GRANJA),
      capa_activa(TipoCapa::SUELO),
      tamano_casilla_px(28),
      modo_actual(ModoEditor::TERRENO),
      terreno_actual(TipoTerrenoPaleta::PASTO),
      entidad_a_colocar(TipoEntidad::CABANIA),
      identificador_entidad_seleccionada(-1),
      arrastrando_entidad(false),
      columna_arrastre_actual(0),
      fila_arrastre_actual(0),
      columna_hover(-1),
      fila_hover(-1),
      mouse_presionado(false) {
    setMouseTracking(true);
    actualizar_tamano_del_lienzo();
}

void LienzoDeEdicion::establecer_mapa(MapaEditable* mapa, SectorActual sector) {
    mapa_actual = mapa;
    sector_actual = sector;
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

void LienzoDeEdicion::establecer_terreno(TipoTerrenoPaleta nuevo_terreno) {
    terreno_actual = nuevo_terreno;
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
    uint32_t id_tile = obtener_id_tile_terreno(terreno_actual, sector_actual);
    uint8_t propiedades = obtener_propiedades_terreno(terreno_actual, sector_actual);
    if (capa_activa == TipoCapa::ESTRUCTURAS) {
        if (terreno_actual == TipoTerrenoPaleta::MURO ||
            terreno_actual == TipoTerrenoPaleta::AGUA) {
            propiedades = propiedades | PropiedadDeCasilla::BLOQUEA;
        }
    }
    mapa_actual->pintar_en_capa(capa_activa, columna, fila, id_tile, propiedades);
    emit mapa_modificado();
    update();
}

static QColor obtener_color_base_terreno(
    uint32_t id_tile, uint8_t propiedades, SectorActual sector) {
    if (sector == SectorActual::CAVERNA) {
        if ((propiedades & PropiedadDeCasilla::BLOQUEA) != 0) {
            if (id_tile == 3) {
                return QColor(31, 67, 102);  // Agua subterranea
            }
            return QColor(32, 31, 36);  // Muro de roca solida
        }
        if (id_tile == 1) {
            return QColor(89, 70, 56);  // Tierra de cueva / fango
        }
        if (id_tile == 2) {
            return QColor(110, 102, 117);  // Sendero minero
        }
        return QColor(61, 59, 64);  // Suelo rocoso de cueva
    }

    if (sector == SectorActual::PUEBLO) {
        if ((propiedades & PropiedadDeCasilla::BLOQUEA) != 0) {
            if (id_tile == 3) {
                return QColor(42, 96, 160);  // Agua / Rio
            }
            return QColor(77, 56, 44);  // Muro de edificio
        }
        if (id_tile == 1) {
            return QColor(186, 140, 83);  // Piso interior
        }
        if (id_tile == 2) {
            return QColor(128, 135, 144);  // Calle / Adoquin
        }
        return QColor(78, 125, 43);  // Pasto urbano
    }

    if ((propiedades & PropiedadDeCasilla::BLOQUEA) != 0) {
        if (id_tile == 3) {
            return QColor(58, 117, 196);  // Agua
        }
        return QColor(74, 74, 74);  // Muro / Roca
    }
    if ((propiedades & PropiedadDeCasilla::CULTIVABLE) != 0 && id_tile == 1) {
        return QColor(139, 90, 43);  // Tierra cultivable arada
    }
    if (id_tile == 2) {
        return QColor(211, 184, 140);  // Camino empedrado
    }
    return QColor(92, 142, 50);  // Pasto natural
}

static QColor obtener_color_entidad(TipoEntidad tipo) {
    if (tipo == TipoEntidad::CABANIA) {
        return QColor(165, 42, 42);  // Madera rojiza
    }
    if (tipo == TipoEntidad::ENTRADA_CAVERNA) {
        return QColor(47, 79, 79);  // Gris caverna
    }
    if (tipo == TipoEntidad::PUESTO_MARNIE) {
        return QColor(46, 139, 87);  // Verde campo
    }
    if (tipo == TipoEntidad::PUESTO_CLINT) {
        return QColor(112, 128, 144);  // Acero forja
    }
    if (tipo == TipoEntidad::PUESTO_PIERRE) {
        return QColor(218, 165, 32);  // Mostrador dorado
    }
    if (tipo == TipoEntidad::PUESTO_ROBIN) {
        return QColor(205, 133, 63);  // Madera carpintería
    }
    if (tipo == TipoEntidad::CENTRO_COMUNITARIO) {
        return QColor(106, 90, 205);  // Púrpura majestuoso
    }
    if (tipo == TipoEntidad::ESCALERA_NIVEL) {
        return QColor(139, 69, 19);  // Peldaños madera/piedra
    }
    if (tipo == TipoEntidad::SALIDA_CAVERNA) {
        return QColor(0, 139, 139);  // Entrada cian
    }
    if (tipo == TipoEntidad::TRANSICION_PUEBLO) {
        return QColor(30, 144, 255);  // Portal azul a pueblo
    }
    if (tipo == TipoEntidad::TRANSICION_GRANJA) {
        return QColor(50, 168, 82);  // Portal verde a granja
    }
    return QColor(100, 100, 100);
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

    // 1. Dibujar Capa Suelo (Back)
    int fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            CasillaDelMapa casilla_suelo =
                mapa_actual->obtener_casilla_en_capa(TipoCapa::SUELO, columna, fila);
            QColor color_suelo = obtener_color_base_terreno(
                casilla_suelo.obtener_id_tile(),
                casilla_suelo.obtener_propiedades(),
                sector_actual);
            QRect rectangulo_casilla(
                columna * tamano_casilla_px,
                fila * tamano_casilla_px,
                tamano_casilla_px,
                tamano_casilla_px);
            pintor.fillRect(rectangulo_casilla, color_suelo);

            // Textura sutil para tierra cultivable en granja
            if (sector_actual == SectorActual::GRANJA && casilla_suelo.es_cultivable() &&
                casilla_suelo.obtener_id_tile() == 1) {
                pintor.setPen(QColor(110, 67, 29));
                int y_linea = rectangulo_casilla.top() + tamano_casilla_px / 2;
                pintor.drawLine(
                    rectangulo_casilla.left(),
                    y_linea,
                    rectangulo_casilla.right(),
                    y_linea);
            }
            columna++;
        }
        fila++;
    }

    // 2. Dibujar Capa Estructuras (Buildings)
    fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            CasillaDelMapa casilla_estructura =
                mapa_actual->obtener_casilla_en_capa(TipoCapa::ESTRUCTURAS, columna, fila);
            if (casilla_estructura.obtener_id_tile() > 0) {
                QColor color_estructura = obtener_color_base_terreno(
                    casilla_estructura.obtener_id_tile(),
                    casilla_estructura.obtener_propiedades(),
                    sector_actual);
                QRect rectangulo_estructura(
                    columna * tamano_casilla_px,
                    fila * tamano_casilla_px,
                    tamano_casilla_px,
                    tamano_casilla_px);
                pintor.fillRect(rectangulo_estructura, color_estructura);
            }
            columna++;
        }
        fila++;
    }

    // 3. Dibujar Entidades Colocadas
    std::vector<EntidadDelMapa> entidades = mapa_actual->listar_todas_las_entidades();
    size_t indice_entidad = 0;
    while (indice_entidad < entidades.size()) {
        uint32_t id_entidad = entidades[indice_entidad].obtener_identificador();
        TipoEntidad tipo_entidad = entidades[indice_entidad].obtener_tipo();
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

        QColor color_entidad = obtener_color_entidad(tipo_entidad);
        pintor.fillRect(rectangulo_entidad, color_entidad);

        // Borde de la entidad
        bool es_seleccionada =
            (identificador_entidad_seleccionada >= 0 &&
             static_cast<uint32_t>(identificador_entidad_seleccionada) == id_entidad);
        if (es_seleccionada) {
            pintor.setPen(QPen(QColor(255, 215, 0), 3));
        } else {
            pintor.setPen(QPen(QColor(255, 255, 255, 180), 2));
        }
        pintor.drawRect(rectangulo_entidad);

        // Texto de la entidad
        pintor.setPen(Qt::white);
        QFont fuente = pintor.font();
        fuente.setBold(true);
        fuente.setPointSize(tamano_casilla_px > 24 ? 9 : 7);
        pintor.setFont(fuente);
        std::string texto_entidad = obtener_nombre_entidad(tipo_entidad);
        pintor.drawText(
            rectangulo_entidad,
            Qt::AlignCenter | Qt::TextWordWrap,
            QString::fromStdString(texto_entidad));

        indice_entidad++;
    }

    // 4. Dibujar Capa Techos (Front)
    fila = 0;
    while (fila < cantidad_filas) {
        int columna = 0;
        while (columna < cantidad_columnas) {
            CasillaDelMapa casilla_frente =
                mapa_actual->obtener_casilla_en_capa(TipoCapa::FRENTE, columna, fila);
            if (casilla_frente.obtener_id_tile() > 0) {
                QColor color_frente = obtener_color_base_terreno(
                    casilla_frente.obtener_id_tile(),
                    casilla_frente.obtener_propiedades(),
                    sector_actual);
                if (capa_activa != TipoCapa::FRENTE) {
                    color_frente.setAlpha(180);
                }
                QRect rectangulo_frente(
                    columna * tamano_casilla_px,
                    fila * tamano_casilla_px,
                    tamano_casilla_px,
                    tamano_casilla_px);
                pintor.fillRect(rectangulo_frente, color_frente);
            }
            columna++;
        }
        fila++;
    }

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
        modo_actual == ModoEditor::COLOCAR_ENTIDAD && columna_hover >= 0 && fila_hover >= 0) {
        int ancho_defecto = 1;
        int alto_defecto = 1;
        consultar_dimensiones_por_defecto_entidad(entidad_a_colocar, ancho_defecto, alto_defecto);
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
        QColor color_fantasma = ubicable ? QColor(0, 255, 0, 110) : QColor(255, 0, 0, 110);
        pintor.fillRect(rectangulo_fantasma, color_fantasma);
        pintor.setPen(QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), 2));
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
            QColor color_arrastre = ubicable ? QColor(0, 255, 0, 130) : QColor(255, 0, 0, 130);
            pintor.fillRect(rectangulo_arrastre, color_arrastre);
            pintor.setPen(QPen(ubicable ? QColor(0, 255, 0) : QColor(255, 0, 0), 2));
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
        } else if (modo_actual == ModoEditor::COLOCAR_ENTIDAD) {
            int ancho_defecto = 1;
            int alto_defecto = 1;
            consultar_dimensiones_por_defecto_entidad(
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
