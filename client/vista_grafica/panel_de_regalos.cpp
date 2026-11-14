#include "panel_de_regalos.h"

#include <algorithm>
#include <string>
#include <vector>

#include "chat/estilo_del_chat.h"

namespace {
constexpr PanelDeRegalos::Caja PANEL{0, 0, 640, 467};
constexpr PanelDeRegalos::Caja PESTANA_ENVIAR{16, 16, 288, 72};
constexpr PanelDeRegalos::Caja PESTANA_RECIBIDOS{328, 16, 288, 72};
constexpr PanelDeRegalos::Caja TITULO{24, 112, 440, 44};
constexpr PanelDeRegalos::Caja BOTON{472, 112, 144, 44};
constexpr PanelDeRegalos::Caja CRUZ{648, -56, 48, 48};
constexpr PanelDeRegalos::Caja PRIMERA_FILA{32, 168, 576, 72};
constexpr int PASO_DE_FILA = 80;
constexpr int FILAS_VISIBLES = 3;
constexpr PanelDeRegalos::Caja PRIMERA_RANURA{50, 172, 60, 60};
constexpr int PASO_DE_RANURA = 68;
constexpr int RANURAS_POR_FILA = 8;
constexpr PanelDeRegalos::Caja FLECHA_ARRIBA{650, 180, 44, 48};
constexpr PanelDeRegalos::Caja FLECHA_ABAJO{650, 340, 44, 48};
constexpr SDL_Rect FLECHA_ARRIBA_EN_CURSORS{421, 459, 11, 12};
constexpr SDL_Rect FLECHA_ABAJO_EN_CURSORS{421, 472, 11, 12};
constexpr int PIXELES_DE_IMAGEN_POR_PIXEL = 4;
constexpr int ALTO_DE_VENTANA_POR_PIXEL = 170;
constexpr int TAMANO_DE_LETRA = 5;
constexpr int TAMANO_DE_LETRA_DE_PESTANA = 8;
constexpr int MARGEN_DEL_TEXTO = 3;
constexpr int SOMBRA_DEL_TEXTO = 1;
constexpr int MARGEN_IZQUIERDO_DE_LA_FILA = 5;
constexpr int LADO_DEL_ESTADO = 5;
constexpr int LADO_DEL_ICONO_DE_REGALO = 14;
constexpr int MARGEN_SUPERIOR_DEL_ICONO_DE_REGALO = 3;
constexpr int SEPARACION_ENTRE_ICONO_Y_TEXTO = 1;
constexpr int MARGEN_DEL_ITEM_EN_LA_RANURA = 3;

bool contiene(const SDL_Rect& rectangulo, int x, int y) {
    const SDL_Point punto{x, y};
    return SDL_PointInRect(&punto, &rectangulo) == SDL_TRUE;
}

int a_pantalla(int medida_en_la_imagen, int pixel) {
    return medida_en_la_imagen * pixel / PIXELES_DE_IMAGEN_POR_PIXEL;
}

int pixel_para(int ancho_de_ventana, int alto_de_ventana) {
    const int pixel_que_entra_a_lo_ancho =
        ancho_de_ventana * PIXELES_DE_IMAGEN_POR_PIXEL / PANEL.ancho;
    const int pixel_que_entra_a_lo_alto = alto_de_ventana / ALTO_DE_VENTANA_POR_PIXEL;
    return std::max(1, std::min(pixel_que_entra_a_lo_alto, pixel_que_entra_a_lo_ancho));
}

SDL_Rect boton_junto_al_chat() {
    const int margen = estilo_del_chat::MARGEN_DE_PANTALLA;
    const int lado = estilo_del_chat::LADO_DEL_BOTON_DE_APERTURA;
    return SDL_Rect{2 * margen + lado, margen, lado, lado};
}

}  // namespace

PanelDeRegalos::PanelDeRegalos(
    SDL_Renderer* renderer, const std::string& ruta_fuente, const IconosDeItems& iconos_de_items,
    int ancho_de_ventana, int alto_de_ventana)
    : iconos(iconos_de_items),
      pixel(pixel_para(ancho_de_ventana, alto_de_ventana)),
      panel_x((ancho_de_ventana - a_pantalla(PANEL.ancho, pixel)) / 2),
      panel_y((alto_de_ventana - a_pantalla(PANEL.alto, pixel)) / 2),
      boton_del_hud(boton_junto_al_chat()),
      fuente(renderer, ruta_fuente, TAMANO_DE_LETRA * pixel),
      fuente_de_pestana(renderer, ruta_fuente, TAMANO_DE_LETRA_DE_PESTANA * pixel),
      icono_del_boton("Assets/hud/hud_regalo.png", renderer),
      icono_del_boton_con_aviso("Assets/hud/hud_regalo_notificacion.png", renderer),
      fondo_del_panel("Assets/hud/regalos_panel.png", renderer),
      fondo_de_fila("Assets/hud/regalos_fila.png", renderer),
      fondo_de_ranura("Assets/hud/regalos_ranura.png", renderer),
      fondo_del_boton("Assets/hud/regalos_boton.png", renderer),
      icono_en_linea("Assets/hud/regalos_en_linea.png", renderer),
      icono_desconectado("Assets/hud/regalos_desconectado.png", renderer),
      icono_de_cerrar("Assets/hud/cerrar_regalos.png", renderer),
      cursores("Assets/LooseSprites/Cursors.png", renderer),
      ventana_de_jugadores(FILAS_VISIBLES),
      ventana_de_regalos(FILAS_VISIBLES) {
}

bool PanelDeRegalos::esta_abierto() const {
    return abierto;
}

void PanelDeRegalos::cerrar() {
    abierto = false;
    destinatario.reset();
}

bool PanelDeRegalos::captura_el_clic(int x, int y) const {
    return abierto || contiene(boton_del_hud, x, y);
}

SDL_Rect PanelDeRegalos::rectangulo_en_pantalla(const Caja& caja, int dx, int dy) const {
    return SDL_Rect{
        panel_x + a_pantalla(caja.x + dx, pixel),
        panel_y + a_pantalla(caja.y + dy, pixel),
        a_pantalla(caja.ancho, pixel),
        a_pantalla(caja.alto, pixel)};
}

SDL_Rect PanelDeRegalos::rectangulo_de_la_fila(int indice) const {
    return rectangulo_en_pantalla(PRIMERA_FILA, 0, indice * PASO_DE_FILA);
}

SDL_Rect PanelDeRegalos::rectangulo_de_la_ranura(int indice) const {
    const int columna = indice % RANURAS_POR_FILA;
    const int fila = indice / RANURAS_POR_FILA;
    return rectangulo_en_pantalla(PRIMERA_RANURA, columna * PASO_DE_RANURA, fila * PASO_DE_RANURA);
}

std::vector<JugadorDeLaPartida> PanelDeRegalos::otros_jugadores(const Datos& datos) {
    std::vector<JugadorDeLaPartida> otros;
    for (const JugadorDeLaPartida& jugador : datos.jugadores.jugadores) {
        if (jugador.id_jugador != datos.id_propio) {
            otros.push_back(jugador);
        }
    }
    return otros;
}

std::string PanelDeRegalos::nombre_de(const Datos& datos, uint32_t id_jugador) {
    for (const JugadorDeLaPartida& jugador : datos.jugadores.jugadores) {
        if (jugador.id_jugador == id_jugador) {
            return jugador.nombre;
        }
    }
    return "?";
}

std::optional<ComandoCliente> PanelDeRegalos::procesar_clic(
    const Datos& datos, int x, int y, bool clic_derecho) {
    if (!abierto) { 
        abrir(datos);
        return std::nullopt;
    }
    if (clic_cierra_el_panel(x, y)) {
        cerrar();
        return std::nullopt;
    }
    if (contiene(rectangulo_en_pantalla(PESTANA_ENVIAR), x, y)) {
        pestana_actual = Pestana::ENVIAR;
        return std::nullopt;
    }
    if (contiene(rectangulo_en_pantalla(PESTANA_RECIBIDOS), x, y)) {
        pestana_actual = Pestana::RECIBIDOS;
        destinatario.reset();
        return std::nullopt;
    }
    if (pestana_actual == Pestana::RECIBIDOS) {
        return clic_en_recibidos(datos, x, y);
    }
    return clic_en_enviar(datos, x, y, clic_derecho);
}

void PanelDeRegalos::abrir(const Datos& datos) {
    abierto = true;
    pestana_actual = datos.pendientes.regalos.empty() ? Pestana::ENVIAR : Pestana::RECIBIDOS;
}

bool PanelDeRegalos::clic_cierra_el_panel(int x, int y) const {
    const bool sobre_la_cruz = contiene(rectangulo_en_pantalla(CRUZ), x, y);
    const bool sobre_el_panel = contiene(rectangulo_en_pantalla(PANEL), x, y);
    const bool sobre_una_flechita = contiene(rectangulo_en_pantalla(FLECHA_ARRIBA), x, y) ||
                                    contiene(rectangulo_en_pantalla(FLECHA_ABAJO), x, y);
    return sobre_la_cruz || (!sobre_el_panel && !sobre_una_flechita);
}

bool PanelDeRegalos::clic_en_flechas(VentanaDeLista& lista, size_t total, int x, int y) const {
    if (contiene(rectangulo_en_pantalla(FLECHA_ARRIBA), x, y)) {
        lista.subir(total);
        return true;
    }
    if (contiene(rectangulo_en_pantalla(FLECHA_ABAJO), x, y)) {
        lista.bajar(total);
        return true;
    }
    return false;
}

std::optional<ComandoCliente> PanelDeRegalos::clic_en_recibidos(const Datos& datos, int x, int y) {
    const size_t cantidad_de_regalos = datos.pendientes.regalos.size();
    if (contiene(rectangulo_en_pantalla(BOTON), x, y) && cantidad_de_regalos > 0) {
        ventana_de_regalos.volver_al_principio();
        return AbrirRegalos{};
    }
    clic_en_flechas(ventana_de_regalos, cantidad_de_regalos, x, y);
    return std::nullopt;
}

std::optional<ComandoCliente> PanelDeRegalos::clic_en_enviar(
    const Datos& datos, int x, int y, bool clic_derecho) {
    if (!destinatario.has_value()) {
        elegir_destinatario(datos, x, y);
        return std::nullopt;
    }
    if (contiene(rectangulo_en_pantalla(BOTON), x, y)) {
        destinatario.reset();
        return std::nullopt;
    }
    return regalar_desde_la_barra(datos, x, y, clic_derecho);
}

void PanelDeRegalos::elegir_destinatario(const Datos& datos, int x, int y) {
    const std::vector<JugadorDeLaPartida> otros = otros_jugadores(datos);
    if (clic_en_flechas(ventana_de_jugadores, otros.size(), x, y)) {
        return;
    }

    const size_t primero_visible = ventana_de_jugadores.desde(otros.size());
    for (size_t fila = 0; fila < ventana_de_jugadores.visibles(otros.size()); ++fila) {
        if (contiene(rectangulo_de_la_fila(static_cast<int>(fila)), x, y)) {
            destinatario = otros[primero_visible + fila].id_jugador;
            return;
        }
    }
}

std::optional<ComandoCliente> PanelDeRegalos::regalar_desde_la_barra(
    const Datos& datos, int x, int y, bool clic_derecho) const {
    const std::vector<RanuraInventario>& ranuras = datos.inventario.ranuras;
    for (size_t i = 0; i < ranuras.size(); ++i) {
        const bool clic_en_esta_ranura =
            contiene(rectangulo_de_la_ranura(static_cast<int>(i)), x, y);
        if (clic_en_esta_ranura && ranuras[i].cantidad > 0) {
            const uint16_t cantidad = clic_derecho ? 1 : 0;
            return EnviarRegalo{*destinatario, static_cast<uint8_t>(i), cantidad};
        }
    }
    return std::nullopt;
}

void PanelDeRegalos::dibujar(SDL_Renderer* renderer, const Datos& datos) const {
    dibujar_boton_del_hud(renderer, datos);
    if (!abierto) {
        return;
    }

    dibujar_marco_y_pestanas(renderer, datos);
    if (pestana_actual == Pestana::RECIBIDOS) {
        dibujar_recibidos(renderer, datos);
    } else if (destinatario.has_value()) {
        dibujar_barra(renderer, datos);
    } else {
        dibujar_lista_de_jugadores(renderer, datos);
    }
}

void PanelDeRegalos::dibujar_boton_del_hud(SDL_Renderer* renderer, const Datos& datos) const {
    const bool hay_regalos = !datos.pendientes.regalos.empty();
    const Textura& icono = hay_regalos ? icono_del_boton_con_aviso : icono_del_boton;
    SDL_RenderCopy(renderer, icono.ptr(), nullptr, &boton_del_hud);
}

void PanelDeRegalos::dibujar_marco_y_pestanas(SDL_Renderer* renderer, const Datos& datos) const {
    const SDL_Rect panel = rectangulo_en_pantalla(PANEL);
    const SDL_Rect cruz = rectangulo_en_pantalla(CRUZ);
    SDL_RenderCopy(renderer, fondo_del_panel.ptr(), nullptr, &panel);
    SDL_RenderCopy(renderer, icono_de_cerrar.ptr(), nullptr, &cruz);

    const size_t cantidad_de_regalos = datos.pendientes.regalos.size();
    const std::string recibidos = cantidad_de_regalos > 0
                                      ? "Recibidos (" + std::to_string(cantidad_de_regalos) + ")"
                                      : "Recibidos";
    escribir_centrado(
        renderer,
        "Enviar",
        rectangulo_en_pantalla(PESTANA_ENVIAR),
        fuente_de_pestana);
    escribir_centrado(
        renderer,
        recibidos,
        rectangulo_en_pantalla(PESTANA_RECIBIDOS),
        fuente_de_pestana);
}

void PanelDeRegalos::dibujar_lista_de_jugadores(SDL_Renderer* renderer, const Datos& datos) const {
    escribir(renderer, "Elegi a quien regalarle", rectangulo_en_pantalla(TITULO));

    const std::vector<JugadorDeLaPartida> otros = otros_jugadores(datos);
    dibujar_filas(
        renderer,
        ventana_de_jugadores,
        otros.size(),
        [&](const SDL_Rect& fila, size_t i) {
            const JugadorDeLaPartida& jugador = otros[i];
            const int lado = LADO_DEL_ESTADO * pixel;
            const SDL_Rect estado{
                fila.x + MARGEN_IZQUIERDO_DE_LA_FILA * pixel,
                fila.y + (fila.h - lado) / 2,
                lado,
                lado};
            const Textura& punto = jugador.conectado ? icono_en_linea : icono_desconectado;
            SDL_RenderCopy(renderer, punto.ptr(), nullptr, &estado);

            const std::string conexion = jugador.conectado ? "en linea" : "desconectado";
            const SDL_Rect renglon{estado.x + lado, fila.y, 0, fila.h};
            escribir(renderer, jugador.nombre + " - " + conexion, renglon);
        });
}

void PanelDeRegalos::dibujar_barra(SDL_Renderer* renderer, const Datos& datos) const {
    escribir(
        renderer,
        "Para " + nombre_de(datos, *destinatario) + " (clic der.: de a uno)",
        rectangulo_en_pantalla(TITULO));

    const std::vector<RanuraInventario>& ranuras = datos.inventario.ranuras;
    for (size_t i = 0; i < ranuras.size(); ++i) {
        const SDL_Rect ranura = rectangulo_de_la_ranura(static_cast<int>(i));
        SDL_RenderCopy(renderer, fondo_de_ranura.ptr(), nullptr, &ranura);
        if (ranuras[i].cantidad == 0) {
            continue;
        }
        const int margen = MARGEN_DEL_ITEM_EN_LA_RANURA * pixel;
        const SDL_Rect item{
            ranura.x + margen,
            ranura.y + margen,
            ranura.w - 2 * margen,
            ranura.h - 2 * margen};
        iconos.dibujar(renderer, ranuras[i].tipo, ranuras[i].calidad, item);
    }

    const SDL_Rect volver = rectangulo_en_pantalla(BOTON);
    SDL_RenderCopy(renderer, fondo_del_boton.ptr(), nullptr, &volver);
    escribir_centrado(renderer, "Volver", volver, fuente);
}

void PanelDeRegalos::dibujar_recibidos(SDL_Renderer* renderer, const Datos& datos) const {
    const std::vector<RegaloRecibido>& regalos = datos.pendientes.regalos;
    escribir(
        renderer,
        regalos.empty() ? "No tenes regalos sin abrir" : "Regalos sin abrir",
        rectangulo_en_pantalla(TITULO));

    dibujar_filas(
        renderer,
        ventana_de_regalos,
        regalos.size(),
        [&](const SDL_Rect& fila, size_t i) {
            const RegaloRecibido& regalo = regalos[i];
            const int lado = LADO_DEL_ICONO_DE_REGALO * pixel;
            const SDL_Rect icono{
                fila.x + MARGEN_IZQUIERDO_DE_LA_FILA * pixel,
                fila.y + MARGEN_SUPERIOR_DEL_ICONO_DE_REGALO * pixel,
                lado,
                lado};
            iconos.dibujar(renderer, regalo.item.tipo, regalo.item.calidad, icono);

            const std::string detalle =
                "x" + std::to_string(regalo.item.cantidad) + " de " + regalo.remitente;
            const SDL_Rect renglon{
                icono.x + lado + SEPARACION_ENTRE_ICONO_Y_TEXTO * pixel,
                fila.y,
                0,
                fila.h};
            escribir(renderer, detalle, renglon);
        });

    if (!regalos.empty()) {
        const SDL_Rect abrir_todo = rectangulo_en_pantalla(BOTON);
        SDL_RenderCopy(renderer, fondo_del_boton.ptr(), nullptr, &abrir_todo);
        escribir_centrado(renderer, "Abrir todo", abrir_todo, fuente);
    }
}

void PanelDeRegalos::dibujar_filas(
    SDL_Renderer* renderer, const VentanaDeLista& lista, size_t total,
    const std::function<void(const SDL_Rect&, size_t)>& dibujar_contenido) const {
    const size_t primero_visible = lista.desde(total);
    for (size_t fila = 0; fila < lista.visibles(total); ++fila) {
        const SDL_Rect rectangulo = rectangulo_de_la_fila(static_cast<int>(fila));
        SDL_RenderCopy(renderer, fondo_de_fila.ptr(), nullptr, &rectangulo);
        dibujar_contenido(rectangulo, primero_visible + fila);
    }
    dibujar_flechas(renderer, lista, total);
}

void PanelDeRegalos::dibujar_flechas(
    SDL_Renderer* renderer, const VentanaDeLista& lista, size_t total) const {
    if (lista.hay_arriba(total)) {
        const SDL_Rect arriba = rectangulo_en_pantalla(FLECHA_ARRIBA);
        SDL_RenderCopy(renderer, cursores.ptr(), &FLECHA_ARRIBA_EN_CURSORS, &arriba);
    }
    if (lista.hay_abajo(total)) {
        const SDL_Rect abajo = rectangulo_en_pantalla(FLECHA_ABAJO);
        SDL_RenderCopy(renderer, cursores.ptr(), &FLECHA_ABAJO_EN_CURSORS, &abajo);
    }
}

void PanelDeRegalos::escribir(
    SDL_Renderer* renderer, const std::string& contenido, const SDL_Rect& renglon) const {
    const int x = renglon.x + MARGEN_DEL_TEXTO * pixel;
    const int y = renglon.y + renglon.h / 2 - fuente.centro_de_las_letras();
    fuente.dibujar_con_sombra(renderer, contenido, x, y, SOMBRA_DEL_TEXTO * pixel / 2);
}

void PanelDeRegalos::escribir_centrado(
    SDL_Renderer* renderer, const std::string& contenido, const SDL_Rect& rectangulo,
    const FuenteDelJuego& letra) const {
    const int x = rectangulo.x + (rectangulo.w - letra.ancho(contenido)) / 2;
    const int y = rectangulo.y + rectangulo.h / 2 - letra.centro_de_las_letras();
    letra.dibujar_con_sombra(renderer, contenido, x, y, SOMBRA_DEL_TEXTO * pixel / 2);
}