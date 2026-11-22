#include "cuadro_de_dialogo.h"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

static constexpr const char* CARPETA_DE_DIALOGOS = "Assets/npcs/dialogo_";
static constexpr int TAMANO_FUENTE = 26;
static constexpr int SEPARACION_DE_LINEAS = 4;
static constexpr SDL_Color COLOR_OSCURECIDO{0, 0, 0, 130};

static constexpr int ANCHO_DEL_FONDO = 1023;
static constexpr int ALTO_DEL_FONDO = 324;

// Tamaño del cartel, en porcentaje de la ventana.
static constexpr int ANCHO_EN_LA_VENTANA = 64;
static constexpr int ALTO_MAXIMO_EN_LA_VENTANA = 35;

// Donde va el texto dentro del cartel, en porcentaje del cartel.
static constexpr int TEXTO_DESDE = 6;
static constexpr int TEXTO_ANCHO = 50;

CuadroDeDialogo::CuadroDeDialogo(SDL_Renderer* renderer, const std::string& ruta_fuente)
    : fuente(renderer, ruta_fuente, TAMANO_FUENTE), renderer(renderer) {
}

void CuadroDeDialogo::mostrar(const std::string& nuevo_id_npc, const std::string& texto) {
    if (fondos.count(nuevo_id_npc) == 0) {
        std::string ruta = std::string(CARPETA_DE_DIALOGOS) + nuevo_id_npc + ".png";
        if (!std::filesystem::exists(ruta)) {
            ruta = std::string(CARPETA_DE_DIALOGOS) + "npc.png";
        }
        fondos.emplace(nuevo_id_npc, Textura(ruta, renderer));
    }
    id_npc = nuevo_id_npc;
    mensaje = texto;
    visible = true;
}

void CuadroDeDialogo::cerrar() {
    visible = false;
    id_npc.clear();
    mensaje.clear();
}

bool CuadroDeDialogo::esta_visible() const {
    return visible;
}

bool CuadroDeDialogo::procesar_entrada(const EntradaDelUsuario& entrada) {
    if (!visible) {
        return false;
    }
    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();
    if (pedidos.clic_izquierdo || pedidos.clic_derecho) {
        cerrar();
    }
    return true;
}

std::vector<std::string> CuadroDeDialogo::partir_en_lineas(
    const std::string& texto, int ancho_maximo) const {
    std::vector<std::string> lineas;
    std::istringstream palabras(texto);
    std::string linea;
    std::string palabra;
    while (palabras >> palabra) {
        const std::string tentativa = linea.empty() ? palabra : linea + " " + palabra;
        if (fuente.ancho(tentativa) > ancho_maximo && !linea.empty()) {
            lineas.push_back(linea);
            linea = palabra;
        } else {
            linea = tentativa;
        }
    }
    if (!linea.empty() || lineas.empty()) {
        lineas.push_back(linea);
    }
    return lineas;
}

void CuadroDeDialogo::dibujar(SDL_Renderer* renderer, int ancho_ventana, int alto_ventana) const {
    if (!visible) {
        return;
    }

    int ancho = ancho_ventana * ANCHO_EN_LA_VENTANA / 100;
    int alto = ancho * ALTO_DEL_FONDO / ANCHO_DEL_FONDO;
    const int alto_maximo = alto_ventana * ALTO_MAXIMO_EN_LA_VENTANA / 100;
    if (alto > alto_maximo) {
        alto = alto_maximo;
        ancho = alto * ANCHO_DEL_FONDO / ALTO_DEL_FONDO;
    }
    const SDL_Rect panel{(ancho_ventana - ancho) / 2, (alto_ventana - alto) * 3 / 4, ancho, alto};

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

    SDL_RenderCopy(renderer, fondos.at(id_npc).ptr(), nullptr, &panel);

    const int ancho_del_texto = panel.w * TEXTO_ANCHO / 100;
    const std::vector<std::string> lineas = partir_en_lineas(mensaje, ancho_del_texto);
    const int alto_renglon = fuente.alto_de_renglon() + SEPARACION_DE_LINEAS;
    const int alto_del_texto = static_cast<int>(lineas.size()) * alto_renglon;
    const int x = panel.x + panel.w * TEXTO_DESDE / 100;
    int y = panel.y + (panel.h - alto_del_texto) / 2;
    for (const std::string& linea : lineas) {
        fuente.dibujar_con_sombra(renderer, linea, x, y, 1);
        y += alto_renglon;
    }
}
