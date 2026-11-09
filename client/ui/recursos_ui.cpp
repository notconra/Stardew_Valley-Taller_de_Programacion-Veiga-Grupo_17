#include "recursos_ui.h"

#include <vector>

namespace {

constexpr const char* RUTA_HOJA_CURSORS = "Assets/LooseSprites/Cursors.png";
constexpr const char* RUTA_ICONO_PRADERA = "Assets/LooseSprites/farm_ranching_icon.png";

// Coordenadas originales de los sprites dentro de Cursors.png
constexpr RecursosUI::RegionSprite REGION_DADO = {381, 361, 10, 10};
constexpr RecursosUI::RegionSprite REGION_BOTON_OK = {128, 256, 64, 64};
constexpr RecursosUI::RegionSprite REGION_BOTON_CANCELAR = {192, 256, 64, 64};
constexpr RecursosUI::RegionSprite REGION_GENERO_MASCULINO = {128, 192, 16, 16};
constexpr RecursosUI::RegionSprite REGION_GENERO_FEMENINO = {144, 192, 16, 16};
constexpr RecursosUI::RegionSprite REGION_FLECHA_IZQUIERDA = {352, 495, 12, 11};
constexpr RecursosUI::RegionSprite REGION_FLECHA_DERECHA = {365, 495, 12, 11};

// Coordenadas exactas de los iconos de granja originales dentro de Cursors.png
constexpr RecursosUI::RegionSprite REGIONES_ICONOS_GRANJA[] = {
    {0, 324, 22, 20},   // 0: Granja estándar
    {22, 324, 22, 20},  // 1: Granja fluvial
    {44, 324, 22, 20},  // 2: Granja forestal
    {66, 324, 22, 20},  // 3: Granja de la colina
    {88, 324, 22, 20},  // 4: Granja salvaje
    {0, 345, 22, 20},   // 5: Granja cuatro esquinas
    {22, 345, 22, 20}   // 6: Granja de la playa
};

// Dimensiones de visualización en la interfaz
constexpr int TAMANIO_ICONO_DADO = 30;
constexpr int TAMANIO_BOTON_OK = 64;
constexpr int TAMANIO_BOTON_ELIMINAR = 40;
constexpr int TAMANIO_ICONO_GENERO = 34;
constexpr int ANCHO_BOTON_FLECHA = 36;
constexpr int ALTO_BOTON_FLECHA = 33;

constexpr int ANCHO_ICONO_GRANJA_DESTINO = 44;
constexpr int ALTO_ICONO_GRANJA_DESTINO = 40;

const std::vector<RecursosUI::InfoTipoGranja> TIPOS_DE_GRANJA = {
    {0,
     "Granja estándar",
     "Farm",
     "Una buena porción de tierra cultivable. Ideal para cultivar cosechas."},
    {1,
     "Granja fluvial",
     "Farm_Fishing",
     "Tu granja está repartida en varias islas fluviales. Muy buena para pescar."},
    {2,
     "Granja forestal",
     "Farm_Foraging",
     "El bosque limita el espacio de cultivo, pero ofrece abundantes recursos silvestres."},
    {3,
     "Granja de la colina",
     "Farm_Mining",
     "El terreno rocoso y un río dificultan el cultivo, pero hay un yacimiento minero."},
    {4,
     "Granja salvaje",
     "Farm_Combat",
     "Hay mucha tierra fértil, pero los monstruos salen por la noche."},
    {5,
     "Granja cuatro esquinas",
     "Farm_FourCorners",
     "La tierra está dividida en cuatro áreas distintas, ideal para grupos de granjeros."},
    {6,
     "Granja de la playa",
     "Beach",
     "Una hermosa playa con espacio para cultivos y pesca, pero los aspersores no funcionan en la "
     "arena."},
    {7,
     "Granja de la pradera",
     "Farm_Ranching",
     "Pasto azul masticable que adoran los animales. Viene con un gallinero y dos pollos."}};

}  // namespace

RecursosUI& RecursosUI::instancia() {
    static RecursosUI singleton;
    return singleton;
}

RecursosUI::RecursosUI() {
    hoja_cursors.load(RUTA_HOJA_CURSORS);
    if (!hoja_cursors.isNull()) {
        sprite_dado =
            recortar_y_escalar(hoja_cursors, REGION_DADO, TAMANIO_ICONO_DADO, TAMANIO_ICONO_DADO);
        sprite_ok =
            recortar_y_escalar(hoja_cursors, REGION_BOTON_OK, TAMANIO_BOTON_OK, TAMANIO_BOTON_OK);
        sprite_eliminar = recortar_y_escalar(
            hoja_cursors,
            REGION_BOTON_CANCELAR,
            TAMANIO_BOTON_ELIMINAR,
            TAMANIO_BOTON_ELIMINAR);
        sprite_masculino = recortar_y_escalar(
            hoja_cursors,
            REGION_GENERO_MASCULINO,
            TAMANIO_ICONO_GENERO,
            TAMANIO_ICONO_GENERO);
        sprite_femenino = recortar_y_escalar(
            hoja_cursors,
            REGION_GENERO_FEMENINO,
            TAMANIO_ICONO_GENERO,
            TAMANIO_ICONO_GENERO);
        sprite_flecha_izq = recortar_y_escalar(
            hoja_cursors,
            REGION_FLECHA_IZQUIERDA,
            ANCHO_BOTON_FLECHA,
            ALTO_BOTON_FLECHA);
        sprite_flecha_der = recortar_y_escalar(
            hoja_cursors,
            REGION_FLECHA_DERECHA,
            ANCHO_BOTON_FLECHA,
            ALTO_BOTON_FLECHA);

        sprite_iconos_granja.reserve(TIPOS_DE_GRANJA.size());
        for (const auto& region : REGIONES_ICONOS_GRANJA) {
            sprite_iconos_granja.push_back(recortar_y_escalar(
                hoja_cursors,
                region,
                ANCHO_ICONO_GRANJA_DESTINO,
                ALTO_ICONO_GRANJA_DESTINO));
        }

        QPixmap icono_pradera(RUTA_ICONO_PRADERA);
        if (!icono_pradera.isNull()) {
            sprite_iconos_granja.push_back(icono_pradera.scaled(
                ANCHO_ICONO_GRANJA_DESTINO,
                ALTO_ICONO_GRANJA_DESTINO,
                Qt::IgnoreAspectRatio,
                Qt::FastTransformation));
        }
    }
}

QPixmap RecursosUI::recortar_y_escalar(
    const QPixmap& hoja, const RegionSprite& region, int ancho_destino, int alto_destino) const {
    return hoja.copy(region.x, region.y, region.ancho, region.alto)
        .scaled(ancho_destino, alto_destino, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

QPixmap RecursosUI::icono_granja(int indice) const {
    if (indice >= 0 && indice < static_cast<int>(sprite_iconos_granja.size())) {
        return sprite_iconos_granja[indice];
    }
    return {};
}

void RecursosUI::configurar_boton_icono(QPushButton* boton, const QPixmap& pixmap) {
    if (boton == nullptr || pixmap.isNull()) {
        return;
    }
    boton->setText("");
    boton->setIcon(QIcon(pixmap));
    boton->setIconSize(pixmap.size());
    boton->setCursor(Qt::PointingHandCursor);
}

void RecursosUI::configurar_boton_dado(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_dado);
}

void RecursosUI::configurar_boton_ok(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_ok);
}

void RecursosUI::configurar_boton_eliminar(QPushButton* boton) {
    if (boton == nullptr) {
        return;
    }
    configurar_boton_icono(boton, sprite_eliminar);
    boton->setFixedSize(TAMANIO_BOTON_ELIMINAR, TAMANIO_BOTON_ELIMINAR);
    boton->setProperty("class", "btn_eliminar_partida");
}

void RecursosUI::configurar_boton_masculino(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_masculino);
}

void RecursosUI::configurar_boton_femenino(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_femenino);
}

void RecursosUI::configurar_boton_flecha_izquierda(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_flecha_izq);
}

void RecursosUI::configurar_boton_flecha_derecha(QPushButton* boton) {
    configurar_boton_icono(boton, sprite_flecha_der);
}

void RecursosUI::configurar_boton_granja(QPushButton* boton, int indice) {
    if (boton == nullptr || indice < 0 || indice >= static_cast<int>(sprite_iconos_granja.size())) {
        return;
    }
    configurar_boton_icono(boton, sprite_iconos_granja[indice]);
    if (indice < static_cast<int>(TIPOS_DE_GRANJA.size())) {
        QString tooltip =
            QString("<b>%1</b><br>%2")
                .arg(TIPOS_DE_GRANJA[indice].nombre, TIPOS_DE_GRANJA[indice].descripcion);
        boton->setToolTip(tooltip);
    }
}

const std::vector<RecursosUI::InfoTipoGranja>& RecursosUI::obtener_tipos_granja() {
    return TIPOS_DE_GRANJA;
}
