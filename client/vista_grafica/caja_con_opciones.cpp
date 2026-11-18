#include "caja_con_opciones.h"

#include <algorithm>
#include <string>
#include <vector>

#define RUTA_IMAGEN_DE_LA_CAJA "Assets/LooseSprites/textBox.png"

#define BORDE_DE_LA_CAJA 6

#define ALTO_DE_VENTANA_POR_ESCALA 240
#define ESCALA_MINIMA 2

#define PUNTOS_DE_LA_LETRA_POR_ESCALA 12
#define RELLENO_ARRIBA 11
#define RELLENO_ABAJO 13
#define RELLENO_A_LOS_COSTADOS 24
#define DISTANCIA_AL_BORDE_DE_ABAJO 12
#define ANCHO_MINIMO_EN_LA_VENTANA 0.55

// Entre el texto y la primera opción, y alrededor del texto de cada opción.
#define SEPARACION_DE_LAS_OPCIONES 4
#define RELLENO_DE_LA_OPCION 3
#define SANGRIA_DE_LA_OPCION 4

#define COLOR_DE_LA_ELEGIDA_R 220
#define COLOR_DE_LA_ELEGIDA_G 40
#define COLOR_DE_LA_ELEGIDA_B 30

CajaConOpciones::CajaConOpciones(
    SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana)
    : caja(RUTA_IMAGEN_DE_LA_CAJA, renderer),
      escala(std::max(ESCALA_MINIMA, alto_ventana / ALTO_DE_VENTANA_POR_ESCALA)),
      fuente(renderer, ruta_fuente, PUNTOS_DE_LA_LETRA_POR_ESCALA * escala) {
}

bool CajaConOpciones::contiene(const SDL_Rect& rectangulo, int x, int y) {
    return x >= rectangulo.x && x < rectangulo.x + rectangulo.w && y >= rectangulo.y &&
           y < rectangulo.y + rectangulo.h;
}

CajaConOpciones::Disposicion CajaConOpciones::disponer(
    const Contenido& contenido, int ancho_ventana, int alto_ventana) const {
    const int alto_renglon = fuente.alto_de_renglon();
    const int relleno_opcion = RELLENO_DE_LA_OPCION * escala;
    const int sangria = SANGRIA_DE_LA_OPCION * escala;
    const int alto_opcion = alto_renglon + 2 * relleno_opcion;

    int ancho_texto = 0;
    for (const std::string& linea : contenido.lineas) {
        ancho_texto = std::max(ancho_texto, fuente.ancho(linea));
    }
    for (const std::string& opcion : contenido.opciones) {
        ancho_texto = std::max(ancho_texto, fuente.ancho(opcion) + 2 * (sangria + relleno_opcion));
    }

    int alto_contenido = static_cast<int>(contenido.lineas.size()) * alto_renglon;
    if (!contenido.opciones.empty()) {
        alto_contenido += SEPARACION_DE_LAS_OPCIONES * escala +
                          static_cast<int>(contenido.opciones.size()) * alto_opcion;
    }

    Disposicion disposicion;
    const int ancho = std::min(
        ancho_ventana,
        std::max(
            static_cast<int>(ancho_ventana * ANCHO_MINIMO_EN_LA_VENTANA),
            ancho_texto + 2 * RELLENO_A_LOS_COSTADOS * escala));
    const int alto = alto_contenido + (RELLENO_ARRIBA + RELLENO_ABAJO) * escala;
    disposicion.caja = {
        (ancho_ventana - ancho) / 2,
        alto_ventana - alto - DISTANCIA_AL_BORDE_DE_ABAJO * escala,
        ancho,
        alto};
    disposicion.primera_linea_y = disposicion.caja.y + RELLENO_ARRIBA * escala;

    const int izquierda = disposicion.caja.x + RELLENO_A_LOS_COSTADOS * escala;
    const int ancho_opcion = disposicion.caja.w - 2 * RELLENO_A_LOS_COSTADOS * escala;
    int y = disposicion.primera_linea_y + static_cast<int>(contenido.lineas.size()) * alto_renglon +
            SEPARACION_DE_LAS_OPCIONES * escala;
    for (size_t i = 0; i < contenido.opciones.size(); ++i) {
        disposicion.opciones.push_back({izquierda, y, ancho_opcion, alto_opcion});
        y += alto_opcion;
    }
    return disposicion;
}

std::optional<size_t> CajaConOpciones::procesar_entrada(
    const EntradaDelUsuario& entrada, const Contenido& contenido, int ancho_ventana,
    int alto_ventana, bool con_flechas) {
    if (contenido.opciones != opciones_anteriores) {
        opciones_anteriores = contenido.opciones;
        elegida = 0;
    }
    if (contenido.opciones.empty()) {
        return std::nullopt;
    }
    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();
    const Disposicion disposicion = disponer(contenido, ancho_ventana, alto_ventana);
    const int x = entrada.raton_x();
    const int y = entrada.raton_y();

    std::optional<size_t> bajo_el_raton;
    for (size_t i = 0; i < disposicion.opciones.size(); ++i) {
        if (contiene(disposicion.opciones[i], x, y)) {
            bajo_el_raton = i;
        }
    }
    const bool se_movio_el_raton = x != raton_anterior.x || y != raton_anterior.y;
    raton_anterior = {x, y};
    if (bajo_el_raton.has_value() && se_movio_el_raton) {
        elegida = *bajo_el_raton;
    }

    const size_t cantidad = contenido.opciones.size();
    if (con_flechas && pedidos.opcion_anterior) {
        elegida = (elegida + cantidad - 1) % cantidad;
    }
    if (con_flechas && pedidos.opcion_siguiente) {
        elegida = (elegida + 1) % cantidad;
    }

    if (pedidos.clic_izquierdo && bajo_el_raton.has_value()) {
        elegida = *bajo_el_raton;
        return elegida;
    }
    if (pedidos.confirmar) {
        return elegida;
    }
    return std::nullopt;
}

void CajaConOpciones::dibujar_caja(SDL_Renderer* renderer, const SDL_Rect& destino) const {
    int ancho_imagen = 0;
    int alto_imagen = 0;
    SDL_QueryTexture(caja.ptr(), nullptr, nullptr, &ancho_imagen, &alto_imagen);
    const int borde = BORDE_DE_LA_CAJA;
    const int borde_en_pantalla = borde * escala;

    const int origen_x[4] = {0, borde, ancho_imagen - borde, ancho_imagen};
    const int origen_y[4] = {0, borde, alto_imagen - borde, alto_imagen};
    const int destino_x[4] = {
        destino.x,
        destino.x + borde_en_pantalla,
        destino.x + destino.w - borde_en_pantalla,
        destino.x + destino.w};
    const int destino_y[4] = {
        destino.y,
        destino.y + borde_en_pantalla,
        destino.y + destino.h - borde_en_pantalla,
        destino.y + destino.h};

    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            const SDL_Rect origen{
                origen_x[columna],
                origen_y[fila],
                origen_x[columna + 1] - origen_x[columna],
                origen_y[fila + 1] - origen_y[fila]};
            const SDL_Rect parte{
                destino_x[columna],
                destino_y[fila],
                destino_x[columna + 1] - destino_x[columna],
                destino_y[fila + 1] - destino_y[fila]};
            if (parte.w > 0 && parte.h > 0) {
                SDL_RenderCopy(renderer, caja.ptr(), &origen, &parte);
            }
        }
    }
}

void CajaConOpciones::dibujar(
    SDL_Renderer* renderer, const Contenido& contenido, int ancho_ventana, int alto_ventana) const {
    const Disposicion disposicion = disponer(contenido, ancho_ventana, alto_ventana);
    dibujar_caja(renderer, disposicion.caja);

    const int alto_renglon = fuente.alto_de_renglon();
    const int sombra = std::max(1, escala / 2);
    const int izquierda = disposicion.caja.x + RELLENO_A_LOS_COSTADOS * escala;
    int y = disposicion.primera_linea_y;
    for (const std::string& linea : contenido.lineas) {
        fuente.dibujar_con_sombra(
            renderer,
            linea,
            izquierda,
            y + alto_renglon / 2 - fuente.centro_de_las_letras(),
            sombra);
        y += alto_renglon;
    }

    const int relleno_opcion = RELLENO_DE_LA_OPCION * escala;
    const int sangria = SANGRIA_DE_LA_OPCION * escala;
    for (size_t i = 0; i < contenido.opciones.size(); ++i) {
        const SDL_Rect& lugar = disposicion.opciones[i];
        if (i == elegida) {
            SDL_SetRenderDrawColor(
                renderer,
                COLOR_DE_LA_ELEGIDA_R,
                COLOR_DE_LA_ELEGIDA_G,
                COLOR_DE_LA_ELEGIDA_B,
                SDL_ALPHA_OPAQUE);
            for (int grosor = 0; grosor < escala / 2 + 1; ++grosor) {
                const SDL_Rect recuadro{
                    lugar.x + grosor,
                    lugar.y + grosor,
                    lugar.w - 2 * grosor,
                    lugar.h - 2 * grosor};
                SDL_RenderDrawRect(renderer, &recuadro);
            }
        }
        fuente.dibujar_con_sombra(
            renderer,
            contenido.opciones[i],
            lugar.x + sangria + relleno_opcion,
            lugar.y + relleno_opcion + alto_renglon / 2 - fuente.centro_de_las_letras(),
            sombra);
    }
}
