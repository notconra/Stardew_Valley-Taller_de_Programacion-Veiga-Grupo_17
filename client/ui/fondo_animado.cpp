#include "fondo_animado.h"

#include <QLinearGradient>
#include <random>

constexpr float ANCHO_BASE_NUBE_PX = 100.0f;

struct CoordenadasSpriteNube {
    int x;
    int y;
    int ancho;
    int alto;
};

const CoordenadasSpriteNube RECTANGULOS_NUBES[] = {
    {260, 336, 168, 116},
    {115, 349, 123, 82},
    {152, 447, 123, 53},
    {20, 414, 82, 46},
    {22, 471, 104, 29},
    {410, 467, 64, 33},
};

FondoAnimado::FondoAnimado() {
    inicializar_estrellas();
    inicializar_nubes();
}

void FondoAnimado::inicializar_estrellas() {
    estrellas.clear();
    std::mt19937 generador_azar(42);
    std::uniform_int_distribution<int> distribucion_x(10, 990);
    std::uniform_int_distribution<int> distribucion_y(10, 990);
    std::uniform_int_distribution<int> distribucion_tamanio(1, 2);
    std::uniform_int_distribution<int> distribucion_brillo(180, 255);

    for (int i = 0; i < 45; ++i) {
        int brillo = distribucion_brillo(generador_azar);
        estrellas.push_back(Estrella{
            distribucion_x(generador_azar),
            distribucion_y(generador_azar),
            distribucion_tamanio(generador_azar),
            QColor(brillo, brillo, brillo, brillo),
        });
    }
}

void FondoAnimado::inicializar_nubes() {
    QPixmap hoja_de_sprites_nubes("Assets/Minigames/Clouds.png");
    if (hoja_de_sprites_nubes.isNull()) {
        return;
    }

    sprites_nubes.clear();
    for (const auto& rectangulo : RECTANGULOS_NUBES) {
        sprites_nubes.push_back(hoja_de_sprites_nubes.copy(
            rectangulo.x,
            rectangulo.y,
            rectangulo.ancho,
            rectangulo.alto));
    }

    nubes = {
        Nube{-100.0f, 60.0f, 0.35f, 2.8f, 0},
        Nube{450.0f, 130.0f, 0.22f, 3.2f, 1},
        Nube{850.0f, 260.0f, 0.45f, 2.5f, 2},
        Nube{100.0f, 300.0f, 0.18f, 2.5f, 3},
        Nube{750.0f, 70.0f, 0.50f, 2.6f, 4},
        Nube{-60.0f, 380.0f, 0.28f, 2.4f, 5},
    };
}

void FondoAnimado::actualizar(int ancho_pantalla) {
    for (auto& nube : nubes) {
        nube.posicion_x += nube.velocidad;
        float ancho_nube = ANCHO_BASE_NUBE_PX;
        if (nube.indice_sprite >= 0 &&
            nube.indice_sprite < static_cast<int>(sprites_nubes.size())) {
            ancho_nube = sprites_nubes[nube.indice_sprite].width() * nube.escala;
        }
        if (nube.posicion_x > ancho_pantalla) {
            nube.posicion_x = -ancho_nube;
        }
    }
}

void FondoAnimado::dibujar(QPainter& pintor, const QRect& area_pantalla, bool dibujar_nubes) const {
    // Gradiente idéntico al juego original (noche estrellada superior -> día celeste horizonte)
    QLinearGradient gradiente_cielo(0, 0, 0, area_pantalla.height());
    gradiente_cielo.setColorAt(0.00, QColor(8, 30, 80));      // Noche estrellada superior (#081e50)
    gradiente_cielo.setColorAt(0.12, QColor(22, 105, 195));   // Crepúsculo
    gradiente_cielo.setColorAt(0.40, QColor(58, 175, 245));   // Celeste cielo brillante Stardew
    gradiente_cielo.setColorAt(1.00, QColor(135, 215, 255));  // Celeste claro horizonte
    pintor.fillRect(area_pantalla, gradiente_cielo);

    // Estrellas en la franja nocturna superior
    pintor.setPen(Qt::NoPen);
    for (const auto& estrella : estrellas) {
        int posicion_pantalla_x = (estrella.porcentaje_x * area_pantalla.width()) / 1000;
        int posicion_pantalla_y =
            static_cast<int>((estrella.porcentaje_y * area_pantalla.height() * 0.13f) / 1000.0f);
        pintor.fillRect(
            posicion_pantalla_x,
            posicion_pantalla_y,
            estrella.tamanio,
            estrella.tamanio,
            estrella.color);
    }

    if (dibujar_nubes) {
        for (const auto& nube : nubes) {
            if (nube.indice_sprite >= 0 &&
                nube.indice_sprite < static_cast<int>(sprites_nubes.size())) {
                const QPixmap& sprite = sprites_nubes[nube.indice_sprite];
                int ancho_nube = static_cast<int>(sprite.width() * nube.escala);
                int alto_nube = static_cast<int>(sprite.height() * nube.escala);
                pintor.drawPixmap(
                    static_cast<int>(nube.posicion_x),
                    static_cast<int>(nube.posicion_y),
                    ancho_nube,
                    alto_nube,
                    sprite);
            }
        }
    }
}
