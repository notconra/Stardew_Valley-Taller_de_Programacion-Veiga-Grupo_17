#include "cuadro_de_aviso.h"

#include "comunes_de_panel.h"

#include <algorithm>
#include <string>
#include <vector>

static constexpr const char* RUTA_FONDO = "Assets/ui/ventana_aviso.png";
static constexpr int TAMANO_FUENTE = 22;
static constexpr int MARGEN_HORIZONTAL = 48;
static constexpr int MARGEN_VERTICAL = 28;
static constexpr int SEPARACION_DE_LINEAS = 4;
static constexpr int ANCHO_MINIMO = 260;
static constexpr int ANCHO_MAXIMO_EN_LA_VENTANA = 72;
static constexpr SDL_Color COLOR_OSCURECIDO{0, 0, 0, 130};

CuadroDeAviso::CuadroDeAviso(SDL_Renderer* renderer, const std::string& ruta_fuente)
    : fuente(renderer, ruta_fuente, TAMANO_FUENTE),
      fondo(RUTA_FONDO, renderer),
      renderer(renderer) {
}

void CuadroDeAviso::mostrar(const std::string& nuevo_mensaje) {
    mensaje = nuevo_mensaje;
    visible = true;
}

void CuadroDeAviso::cerrar() {
    visible = false;
    mensaje.clear();
}

bool CuadroDeAviso::esta_visible() const {
    return visible;
}

bool CuadroDeAviso::procesar_entrada(const EntradaDelUsuario& entrada) {
    if (!visible) {
        return false;
    }
    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();
    if (pedidos.clic_izquierdo || pedidos.clic_derecho) {
        cerrar();
    }
    return true;
}

void CuadroDeAviso::dibujar(SDL_Renderer* renderer, int ancho_ventana, int alto_ventana) const {
    if (!visible) {
        return;
    }

    const int ancho_maximo = ancho_ventana * ANCHO_MAXIMO_EN_LA_VENTANA / 100;
    const std::vector<std::string> lineas =
        partir_en_lineas(fuente, mensaje, ancho_maximo - 2 * MARGEN_HORIZONTAL);
    int ancho_texto = 0;
    for (const std::string& linea : lineas) {
        ancho_texto = std::max(ancho_texto, fuente.ancho(linea));
    }

    const int alto_renglon = fuente.alto_de_renglon() + SEPARACION_DE_LINEAS;
    const int ancho = std::max(ANCHO_MINIMO, ancho_texto + 2 * MARGEN_HORIZONTAL);
    const int alto = static_cast<int>(lineas.size()) * alto_renglon + 2 * MARGEN_VERTICAL;
    const SDL_Rect panel{(ancho_ventana - ancho) / 2, (alto_ventana - alto) / 2, ancho, alto};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const SDL_Rect pantalla{0, 0, ancho_ventana, alto_ventana};
    SDL_SetRenderDrawColor(
        renderer,
        COLOR_OSCURECIDO.r,
        COLOR_OSCURECIDO.g,
        COLOR_OSCURECIDO.b,
        COLOR_OSCURECIDO.a);
    SDL_RenderFillRect(renderer, &pantalla);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_RenderCopy(renderer, fondo.ptr(), nullptr, &panel);

    int y = panel.y + MARGEN_VERTICAL;
    for (const std::string& linea : lineas) {
        fuente.dibujar_con_sombra(
            renderer,
            linea,
            panel.x + ((panel.w - fuente.ancho(linea)) / 2),
            y,
            1);
        y += alto_renglon;
    }
}
