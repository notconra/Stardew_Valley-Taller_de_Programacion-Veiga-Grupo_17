#include "ventana_de_lote.h"

#include <algorithm>
#include <string>

#define RUTA_DE_LA_HOJA "Assets/LooseSprites/JunimoNote.png"
#define RUTA_DE_LOS_DIBUJOS "Assets/LooseSprites/BundleSprites.png"
#define RUTA_DEL_BOTON "Assets/hud/regalos_boton.png"
#define RUTA_DE_LA_CRUZ "Assets/hud/cerrar_regalos.png"

#define PANEL_X_EN_LA_HOJA 320
#define PANEL_Y_EN_LA_HOJA 0
#define ANCHO_DEL_PANEL 320
#define ALTO_DEL_PANEL 180
#define MARGEN_DE_PANTALLA 20

#define COLUMNAS_DEL_INVENTARIO 6
#define LADO_DE_RANURA 16
#define PRIMERA_COLUMNA 31
#define PASO_DE_COLUMNA 18
#define FILAS_DEL_INVENTARIO 6
const int FILAS_EN_LA_HOJA[FILAS_DEL_INVENTARIO] = {35, 53, 72, 91, 110, 128};

#define MARCO_X 218
#define MARCO_Y 23
#define LADO_DEL_DIBUJO 32
#define CAJA_X 187
#define CAJA_Y 114
#define CAJA_ANCHO 92
#define CAJA_ALTO 43
#define CENTRO_DEL_LADO_DERECHO 233

#define CASILLERO_X_EN_LA_HOJA 512
#define CASILLERO_Y_EN_LA_HOJA 244
#define LADO_DE_CASILLERO 18
#define SEPARACION_DE_CASILLEROS 4
#define CASILLERO_Y 117

#define TITULO_Y 60
#define RENGLON_Y 74
#define ALTO_DE_RENGLON 10
#define PRIMER_LOGRO_Y 72
#define LOGROS_X 165
#define BOTON_ALTO 14
#define LADO_DE_LA_CRUZ 12
#define TAMANO_DEL_TITULO 7
#define TAMANO_DEL_TEXTO 6
#define LADO_DE_LA_TILDE 9

#define BRILLO_DE_LO_QUE_FALTA 170
#define OPACIDAD_DE_LO_QUE_FALTA 150

#define COLUMNAS_DE_DIBUJOS 8
const int DIBUJO_DEL_LOTE[CANTIDAD_DE_LOTES] = {3, 11, 6, 14, 10};

const char* const TEXTO_DEL_LOGRO[] = {
    "3 esqueletos en pisos seguidos",
    "La pareja de vampiros",
    "4 orcos del mismo grupo",
    "Calavera sin recibir dano",
};

VentanaDeLote::VentanaDeLote(
    SDL_Renderer* renderer, const std::string& ruta_fuente, const IconosDeItems& iconos_de_items,
    int ancho_de_ventana, int alto_de_ventana)
    : iconos(iconos_de_items),
      hoja(RUTA_DE_LA_HOJA, renderer),
      dibujos(RUTA_DE_LOS_DIBUJOS, renderer),
      boton(RUTA_DEL_BOTON, renderer),
      cruz(RUTA_DE_LA_CRUZ, renderer) {
    configurar(renderer, ruta_fuente, ancho_de_ventana, alto_de_ventana);
}

void VentanaDeLote::configurar(
    SDL_Renderer* renderer, const std::string& ruta_fuente, int ancho_de_ventana,
    int alto_de_ventana) {
    escala = std::max(
        1,
        std::min(
            (ancho_de_ventana - 2 * MARGEN_DE_PANTALLA) / ANCHO_DEL_PANEL,
            (alto_de_ventana - 2 * MARGEN_DE_PANTALLA) / ALTO_DEL_PANEL));
    panel_x = (ancho_de_ventana - ANCHO_DEL_PANEL * escala) / 2;
    panel_y = (alto_de_ventana - ALTO_DEL_PANEL * escala) / 2;
    fuente_del_titulo.reset();
    fuente_del_texto.reset();
    fuente_del_titulo.emplace(renderer, ruta_fuente, TAMANO_DEL_TITULO * escala);
    fuente_del_texto.emplace(renderer, ruta_fuente, TAMANO_DEL_TEXTO * escala);
}

void VentanaDeLote::recibir(const LoteDelCentro& estado) {
    if (estado.abrir) {
        lote = estado;
        ranura_agarrada.reset();
        confirmando = false;
        return;
    }
    if (lote.has_value() && lote->lote == estado.lote) {
        lote = estado;
        lote->abrir = true;
        if (estado.completo) {
            confirmando = false;
        }
    }
}

bool VentanaDeLote::esta_abierta() const {
    return lote.has_value();
}

void VentanaDeLote::cerrar() {
    lote.reset();
    ranura_agarrada.reset();
    confirmando = false;
}

bool VentanaDeLote::captura_el_clic() const {
    return esta_abierta();
}

// Medidas

SDL_Rect VentanaDeLote::en_pantalla(int x, int y, int ancho, int alto) const {
    return SDL_Rect{panel_x + x * escala, panel_y + y * escala, ancho * escala, alto * escala};
}

bool VentanaDeLote::contiene(const SDL_Rect& rectangulo, int x, int y) {
    const SDL_Point punto{x, y};
    return SDL_PointInRect(&punto, &rectangulo) == SDL_TRUE;
}

SDL_Rect VentanaDeLote::ranura_del_inventario(int indice) const {
    const int columna = indice % COLUMNAS_DEL_INVENTARIO;
    const int fila = std::min(indice / COLUMNAS_DEL_INVENTARIO, FILAS_DEL_INVENTARIO - 1);
    return en_pantalla(
        PRIMERA_COLUMNA + columna * PASO_DE_COLUMNA,
        FILAS_EN_LA_HOJA[fila],
        LADO_DE_RANURA,
        LADO_DE_RANURA);
}

SDL_Rect VentanaDeLote::caja() const {
    return en_pantalla(CAJA_X, CAJA_Y, CAJA_ANCHO, CAJA_ALTO);
}

SDL_Rect VentanaDeLote::casillero(int indice, int cantidad) const {
    const int ancho_total =
        cantidad * LADO_DE_CASILLERO + (cantidad - 1) * SEPARACION_DE_CASILLEROS;
    const int x = CAJA_X + (CAJA_ANCHO - ancho_total) / 2 +
                  indice * (LADO_DE_CASILLERO + SEPARACION_DE_CASILLEROS);
    return en_pantalla(x, CASILLERO_Y, LADO_DE_CASILLERO, LADO_DE_CASILLERO);
}

SDL_Rect VentanaDeLote::boton_de_donar() const {
    return en_pantalla(CAJA_X + 16, CAJA_Y + 24, CAJA_ANCHO - 32, BOTON_ALTO);
}

SDL_Rect VentanaDeLote::boton_si() const {
    return en_pantalla(CAJA_X + 6, CAJA_Y + 24, CAJA_ANCHO / 2 - 9, BOTON_ALTO);
}

SDL_Rect VentanaDeLote::boton_no() const {
    return en_pantalla(CAJA_X + CAJA_ANCHO / 2 + 3, CAJA_Y + 24, CAJA_ANCHO / 2 - 9, BOTON_ALTO);
}

SDL_Rect VentanaDeLote::boton_de_cerrar() const {
    return en_pantalla(ANCHO_DEL_PANEL - LADO_DE_LA_CRUZ - 4, 4, LADO_DE_LA_CRUZ, LADO_DE_LA_CRUZ);
}

bool VentanaDeLote::es_de_items() const {
    return lote.has_value() && !lote->requisitos.empty() &&
           lote->requisitos.front().tipo == TipoDeRequisito::ITEM;
}

const RequisitoDeLote* VentanaDeLote::requisito_de_monedas() const {
    if (!lote.has_value()) {
        return nullptr;
    }
    for (const RequisitoDeLote& requisito : lote->requisitos) {
        if (requisito.tipo == TipoDeRequisito::DINERO) {
            return &requisito;
        }
    }
    return nullptr;
}

// Clics

std::optional<ComandoCliente> VentanaDeLote::procesar_clic(
    const InventarioJugador& inventario, uint32_t monedas, int x, int y) {
    if (!lote.has_value()) {
        return std::nullopt;
    }
    if (contiene(boton_de_cerrar(), x, y)) {
        cerrar();
        return std::nullopt;
    }
    if (es_de_items()) {
        return clic_en_lote_de_items(inventario, x, y);
    }
    if (requisito_de_monedas() != nullptr) {
        return clic_en_lote_de_dinero(monedas, x, y);
    }
    return std::nullopt;
}

std::optional<ComandoCliente> VentanaDeLote::clic_en_lote_de_items(
    const InventarioJugador& inventario, int x, int y) {
    if (ranura_agarrada.has_value()) {
        const uint8_t ranura = *ranura_agarrada;
        ranura_agarrada.reset();
        if (contiene(caja(), x, y) && !lote->completo) {
            return ComandoCliente{DonarAlLote{lote->lote, ranura}};
        }
        return std::nullopt;
    }
    const int cantidad = std::min(
        static_cast<int>(inventario.ranuras.size()),
        COLUMNAS_DEL_INVENTARIO * FILAS_DEL_INVENTARIO);
    for (int i = 0; i < cantidad; ++i) {
        if (contiene(ranura_del_inventario(i), x, y) &&
            inventario.ranuras[i].tipo != TipoItem::NINGUNO) {
            ranura_agarrada = static_cast<uint8_t>(i);
            break;
        }
    }
    return std::nullopt;
}

std::optional<ComandoCliente> VentanaDeLote::clic_en_lote_de_dinero(
    uint32_t monedas, int x, int y) {
    const RequisitoDeLote* requisito = requisito_de_monedas();
    if (lote->completo || requisito == nullptr) {
        return std::nullopt;
    }
    if (confirmando) {
        if (contiene(boton_si(), x, y)) {
            confirmando = false;
            return ComandoCliente{DonarMonedasAlLote{lote->lote}};
        }
        if (contiene(boton_no(), x, y)) {
            confirmando = false;
        }
        return std::nullopt;
    }
    if (contiene(boton_de_donar(), x, y) && monedas >= requisito->cantidad) {
        confirmando = true;
    }
    return std::nullopt;
}

// Dibujo

void VentanaDeLote::dibujar(
    SDL_Renderer* renderer, const InventarioJugador& inventario, uint32_t monedas, int raton_x,
    int raton_y) const {
    if (!lote.has_value()) {
        return;
    }
    const SDL_Rect origen{PANEL_X_EN_LA_HOJA, PANEL_Y_EN_LA_HOJA, ANCHO_DEL_PANEL, ALTO_DEL_PANEL};
    const SDL_Rect destino = en_pantalla(0, 0, ANCHO_DEL_PANEL, ALTO_DEL_PANEL);
    SDL_RenderCopy(renderer, hoja.ptr(), &origen, &destino);
    const SDL_Rect cerrar_en_pantalla = boton_de_cerrar();
    SDL_RenderCopy(renderer, cruz.ptr(), nullptr, &cerrar_en_pantalla);

    dibujar_inventario(renderer, inventario);
    dibujar_encabezado(renderer);
    if (es_de_items()) {
        dibujar_casilleros(renderer);
    } else if (requisito_de_monedas() != nullptr) {
        dibujar_dinero(renderer, monedas);
    } else {
        dibujar_logros(renderer);
    }
    dibujar_item_agarrado(renderer, inventario, raton_x, raton_y);
}

void VentanaDeLote::dibujar_inventario(
    SDL_Renderer* renderer, const InventarioJugador& inventario) const {
    const int cantidad = std::min(
        static_cast<int>(inventario.ranuras.size()),
        COLUMNAS_DEL_INVENTARIO * FILAS_DEL_INVENTARIO);
    for (int i = 0; i < cantidad; ++i) {
        const RanuraInventario& ranura = inventario.ranuras[i];
        if (ranura.tipo == TipoItem::NINGUNO ||
            (ranura_agarrada.has_value() && *ranura_agarrada == i)) {
            continue;
        }
        const SDL_Rect rectangulo = ranura_del_inventario(i);
        iconos.dibujar(renderer, ranura.tipo, ranura.calidad, rectangulo);
        if (ranura.cantidad > 1) {
            const std::string texto = std::to_string(ranura.cantidad);
            fuente_del_texto->dibujar_con_sombra(
                renderer,
                texto,
                rectangulo.x + rectangulo.w - fuente_del_texto->ancho(texto),
                rectangulo.y + rectangulo.h - fuente_del_texto->alto_de_renglon() + 2 * escala,
                escala);
        }
    }
}

void VentanaDeLote::dibujar_encabezado(SDL_Renderer* renderer) const {
    const int indice = static_cast<int>(lote->lote);
    const int dibujo = DIBUJO_DEL_LOTE[std::clamp(indice, 0, CANTIDAD_DE_LOTES - 1)];
    const SDL_Rect origen{
        (dibujo % COLUMNAS_DE_DIBUJOS) * LADO_DEL_DIBUJO,
        (dibujo / COLUMNAS_DE_DIBUJOS) * LADO_DEL_DIBUJO,
        LADO_DEL_DIBUJO,
        LADO_DEL_DIBUJO};
    const SDL_Rect destino = en_pantalla(MARCO_X, MARCO_Y, LADO_DEL_DIBUJO, LADO_DEL_DIBUJO);
    SDL_RenderCopy(renderer, dibujos.ptr(), &origen, &destino);

    const int centro = panel_x + CENTRO_DEL_LADO_DERECHO * escala;
    escribir_centrado(
        renderer,
        *fuente_del_titulo,
        lote->nombre,
        centro,
        panel_y + TITULO_Y * escala);
    if (lote->completo) {
        escribir_centrado(
            renderer,
            *fuente_del_texto,
            "COMPLETO",
            centro,
            panel_y + (RENGLON_Y + ALTO_DE_RENGLON * 2) * escala);
    }
}

void VentanaDeLote::dibujar_casilleros(SDL_Renderer* renderer) const {
    const bool pide_calidad = std::any_of(
        lote->requisitos.begin(),
        lote->requisitos.end(),
        [](const RequisitoDeLote& requisito) { return requisito.calidad_minima > Calidad::BASE; });
    if (pide_calidad) {
        escribir_centrado(
            renderer,
            *fuente_del_texto,
            "Calidad increible o perfecta",
            panel_x + CENTRO_DEL_LADO_DERECHO * escala,
            panel_y + RENGLON_Y * escala);
    }

    const int cantidad = static_cast<int>(lote->requisitos.size());
    const SDL_Rect origen{
        CASILLERO_X_EN_LA_HOJA,
        CASILLERO_Y_EN_LA_HOJA,
        LADO_DE_CASILLERO,
        LADO_DE_CASILLERO};
    for (int i = 0; i < cantidad; ++i) {
        const RequisitoDeLote& requisito = lote->requisitos[i];
        const SDL_Rect rectangulo = casillero(i, cantidad);
        SDL_RenderCopy(renderer, hoja.ptr(), &origen, &rectangulo);

        const bool cumplido = requisito.aportado >= requisito.cantidad;
        const Icono icono = iconos.obtener_icono(requisito.item, requisito.calidad_minima);
        const SDL_Rect adentro{
            rectangulo.x + escala,
            rectangulo.y + escala,
            rectangulo.w - 2 * escala,
            rectangulo.h - 2 * escala};
        if (icono.textura != nullptr) {
            const Uint8 brillo = cumplido ? 255 : BRILLO_DE_LO_QUE_FALTA;
            SDL_SetTextureColorMod(icono.textura, brillo, brillo, brillo);
            SDL_SetTextureAlphaMod(icono.textura, cumplido ? 255 : OPACIDAD_DE_LO_QUE_FALTA);
            const bool usa_toda_la_textura = icono.origen.w == 0 || icono.origen.h == 0;
            SDL_RenderCopy(
                renderer,
                icono.textura,
                usa_toda_la_textura ? nullptr : &icono.origen,
                &adentro);
            SDL_SetTextureColorMod(icono.textura, 255, 255, 255);
            SDL_SetTextureAlphaMod(icono.textura, 255);
        }
        escribir_centrado(
            renderer,
            *fuente_del_texto,
            std::to_string(requisito.aportado) + "/" + std::to_string(requisito.cantidad),
            rectangulo.x + rectangulo.w / 2,
            rectangulo.y + rectangulo.h + escala);
    }
}

void VentanaDeLote::dibujar_dinero(SDL_Renderer* renderer, uint32_t monedas) const {
    const RequisitoDeLote* requisito = requisito_de_monedas();
    const int centro = panel_x + CENTRO_DEL_LADO_DERECHO * escala;
    const std::string pedidas = std::to_string(requisito->cantidad) + " monedas";
    escribir_centrado(renderer, *fuente_del_texto, pedidas, centro, caja().y + 4 * escala);
    if (lote->completo) {
        return;
    }
    if (confirmando) {
        escribir_centrado(
            renderer,
            *fuente_del_texto,
            "Seguro que queres donar " + pedidas + "?",
            centro,
            panel_y + RENGLON_Y * escala);
        dibujar_boton(renderer, boton_si(), "SI", true);
        dibujar_boton(renderer, boton_no(), "NO", true);
        return;
    }
    escribir_centrado(
        renderer,
        *fuente_del_texto,
        "Tenes " + std::to_string(monedas) + " monedas",
        centro,
        panel_y + RENGLON_Y * escala);
    dibujar_boton(renderer, boton_de_donar(), "DONAR", monedas >= requisito->cantidad);
}

void VentanaDeLote::dibujar_logros(SDL_Renderer* renderer) const {
    int cumplidos = 0;
    for (std::size_t i = 0; i < lote->requisitos.size(); ++i) {
        const RequisitoDeLote& requisito = lote->requisitos[i];
        const bool cumplido = requisito.aportado >= requisito.cantidad;
        cumplidos += cumplido ? 1 : 0;
        const int indice = static_cast<int>(requisito.logro);
        const int y = PRIMER_LOGRO_Y + static_cast<int>(i) * ALTO_DE_RENGLON;

        const SDL_Rect origen{
            CASILLERO_X_EN_LA_HOJA,
            CASILLERO_Y_EN_LA_HOJA,
            LADO_DE_CASILLERO,
            LADO_DE_CASILLERO};
        const SDL_Rect tilde = en_pantalla(LOGROS_X, y, LADO_DE_LA_TILDE, LADO_DE_LA_TILDE);
        SDL_RenderCopy(renderer, hoja.ptr(), &origen, &tilde);
        if (cumplido) {
            escribir_centrado(
                renderer,
                *fuente_del_texto,
                "X",
                tilde.x + tilde.w / 2,
                tilde.y + (tilde.h - fuente_del_texto->alto_de_renglon()) / 2 + escala);
        }
        fuente_del_texto->dibujar_con_sombra(
            renderer,
            TEXTO_DEL_LOGRO[std::clamp(indice, 0, 3)],
            panel_x + (LOGROS_X + LADO_DE_LA_TILDE + 3) * escala,
            panel_y + y * escala,
            escala);
    }
    escribir_centrado(
        renderer,
        *fuente_del_texto,
        std::to_string(cumplidos) + " de " + std::to_string(lote->requisitos.size()),
        panel_x + CENTRO_DEL_LADO_DERECHO * escala,
        caja().y + (CAJA_ALTO / 2 - 4) * escala);
}

void VentanaDeLote::dibujar_boton(
    SDL_Renderer* renderer, const SDL_Rect& rectangulo, const std::string& texto,
    bool habilitado) const {
    const Uint8 brillo = habilitado ? 255 : 120;
    SDL_SetTextureColorMod(boton.ptr(), brillo, brillo, brillo);
    SDL_RenderCopy(renderer, boton.ptr(), nullptr, &rectangulo);
    SDL_SetTextureColorMod(boton.ptr(), 255, 255, 255);
    escribir_centrado(
        renderer,
        *fuente_del_texto,
        texto,
        rectangulo.x + rectangulo.w / 2,
        rectangulo.y + (rectangulo.h - fuente_del_texto->alto_de_renglon()) / 2);
}

void VentanaDeLote::dibujar_item_agarrado(
    SDL_Renderer* renderer, const InventarioJugador& inventario, int raton_x, int raton_y) const {
    if (!ranura_agarrada.has_value() || *ranura_agarrada >= inventario.ranuras.size()) {
        return;
    }
    const RanuraInventario& ranura = inventario.ranuras[*ranura_agarrada];
    const int lado = LADO_DE_RANURA * escala;
    const SDL_Rect destino{raton_x - lado / 2, raton_y - lado / 2, lado, lado};
    iconos.dibujar(renderer, ranura.tipo, ranura.calidad, destino);
}

void VentanaDeLote::escribir_centrado(
    SDL_Renderer* renderer, const FuenteDelJuego& fuente, const std::string& texto, int centro_x,
    int y) const {
    fuente.dibujar_con_sombra(renderer, texto, centro_x - fuente.ancho(texto) / 2, y, escala);
}
