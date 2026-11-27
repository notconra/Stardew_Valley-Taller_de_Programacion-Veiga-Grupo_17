#include "hud_mercado.h"

#include "comunes_de_panel.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

constexpr const char* RUTA_FONDO_CATALOGO = "Assets/mercado/fondo_catalogo.png";
constexpr const char* RUTA_TARJETA = "Assets/mercado/tarjeta_catalogo.png";
constexpr const char* RUTA_MONEDA = "Assets/mercado/moneda.png";
constexpr const char* RUTA_VACA = "Assets/mercado/vaca.png";
constexpr const char* RUTA_GALLINA = "Assets/mercado/gallinas.png";
constexpr const char* RUTA_BOTON_VENDER = "Assets/mercado/boton_vender_animal.png";
constexpr const char* RUTA_BOTON_VENDER_HOVER = "Assets/mercado/boton_vender_animal_hover.png";
constexpr const char* RUTA_BOTON_VENDER_NO_PERMITIDO =
    "Assets/mercado/boton_vender_animal_no_permitido.png";
constexpr const char* RUTA_BOTON_CERRAR = "Assets/ui/boton_cerrar.png";
constexpr const char* RUTA_VENTANA_DESCRIPCION = "Assets/ui/ventana_aviso.png";
constexpr const char* PREFIJO_RUTA_MARCO_NPC = "Assets/mercado/marco_";
constexpr const char* SUFIJO_RUTA_MARCO_NPC = "_mercado.png";

constexpr int TAMANO_FUENTE = 16;
constexpr int ALTO_RESERVADO_INFERIOR = 110;
constexpr int MARGEN_PANTALLA = 24;
constexpr int ANCHO_MARCO = 150;
constexpr int ANCHO_CATALOGO = 510;
constexpr int ALTO_MINIMO_CATALOGO = 350;
constexpr int SEPARACION_COLUMNAS = 12;
constexpr int SEPARACION_MARCO_Y_DESCRIPCION = 8;

constexpr int BORDE_VENTANA_ORIGEN = 50;
constexpr int BORDE_VENTANA_DESTINO = 14;
constexpr int PADDING_DESCRIPCION = 8;
constexpr int MARGEN_DESCRIPCION = BORDE_VENTANA_DESTINO + PADDING_DESCRIPCION;
constexpr int ALTO_MINIMO_DESCRIPCION = 70;

constexpr int BORDE_FONDO_ORIGEN = 64;
constexpr int MARGEN_TARJETA = 10;
constexpr int SEPARACION_ELEMENTOS = 10;
constexpr int SEPARACION_MONEDA_Y_PRECIO = 6;
constexpr int PORCENTAJE_ALTO_BOTON_VENDER = 30;

constexpr int LADO_BOTON_CERRAR = 16;
constexpr int SOBRESALE_BOTON_CERRAR = 5;
constexpr int ANCHO_PISTA = 6;
constexpr int SEPARACION_PISTA = 8;
constexpr int ALTO_MINIMO_PULGAR = 20;
constexpr int PORCENTAJE_DESPLAZAMIENTO_RUEDA = 50;

constexpr int BRILLO_HOVER = 60;
constexpr int BRILLO_APAGADO = 120;
constexpr int OPACIDAD_APAGADA = 120;

constexpr SDL_Color COLOR_TEXTO{86, 45, 20, 255};
constexpr SDL_Color COLOR_PISTA{150, 100, 50, 255};
constexpr SDL_Color COLOR_PULGAR{240, 205, 130, 255};
constexpr SDL_Color COLOR_VELO_APAGADO{0, 0, 0, 70};

constexpr const char* DESCRIPCION_SIN_SELECCION =
    "Pasa el mouse sobre un producto para ver su descripcion.";
constexpr const char* TEXTO_CATALOGO_VACIO =
    "No tiene nada a la venta. Clic en tu inventario para vender.";

HudMercado::HudMercado(
    SDL_Renderer* renderer_de_dibujo, const IconosDeItems& iconos_de_items,
    const EfectosDeSonido& efectos_de_sonido, const std::string& ruta_fuente)
    : renderer(renderer_de_dibujo),
      iconos(iconos_de_items),
      efectos(efectos_de_sonido),
      fuente(renderer_de_dibujo, ruta_fuente, TAMANO_FUENTE),
      fondo(RUTA_FONDO_CATALOGO, renderer_de_dibujo),
      tarjeta(RUTA_TARJETA, renderer_de_dibujo),
      moneda(RUTA_MONEDA, renderer_de_dibujo),
      vaca(RUTA_VACA, renderer_de_dibujo),
      gallina(RUTA_GALLINA, renderer_de_dibujo),
      boton_vender(RUTA_BOTON_VENDER, renderer_de_dibujo),
      boton_vender_hover(RUTA_BOTON_VENDER_HOVER, renderer_de_dibujo),
      boton_vender_no_permitido(RUTA_BOTON_VENDER_NO_PERMITIDO, renderer_de_dibujo),
      boton_cerrar(RUTA_BOTON_CERRAR, renderer_de_dibujo),
      ventana(RUTA_VENTANA_DESCRIPCION, renderer_de_dibujo) {
}

SDL_Point HudMercado::tamano_de(const Textura& imagen) const {
    SDL_Point tamano{0, 0};
    if (imagen.ptr() != nullptr) {
        SDL_QueryTexture(imagen.ptr(), nullptr, nullptr, &tamano.x, &tamano.y);
    }
    return tamano;
}

int HudMercado::alto_proporcional(const Textura& imagen, int ancho) const {
    const SDL_Point tamano = tamano_de(imagen);
    return (tamano.x > 0) ? ancho * tamano.y / tamano.x : 0;
}

SDL_Rect HudMercado::encajar(const Textura& imagen, const SDL_Rect& caja) const {
    const SDL_Point tamano = tamano_de(imagen);
    if (tamano.x <= 0 || tamano.y <= 0) {
        return caja;
    }
    int ancho = caja.w;
    int alto = ancho * tamano.y / tamano.x;
    if (alto > caja.h) {
        alto = caja.h;
        ancho = alto * tamano.x / tamano.y;
    }
    return SDL_Rect{caja.x + ((caja.w - ancho) / 2), caja.y + ((caja.h - alto) / 2), ancho, alto};
}

void HudMercado::configurar(int ancho, int alto) {
    ancho_ventana = ancho;
    alto_ventana = alto;
    desplazamiento = std::min(desplazamiento, disponer().max_desplazamiento);
}

std::string HudMercado::ruta_del_marco(const std::string& id_npc) const {
    std::string minusculas = id_npc;
    std::transform(minusculas.begin(), minusculas.end(), minusculas.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return std::string(PREFIJO_RUTA_MARCO_NPC) + minusculas + SUFIJO_RUTA_MARCO_NPC;
}

void HudMercado::abrir(const ActualizacionMercado& actualizacion) {
    const std::string ruta = ruta_del_marco(actualizacion.id_npc);
    if (ruta != ruta_marco_cargado) {
        marco_npc = Textura(ruta, renderer);
        ruta_marco_cargado = ruta;
    }

    if (!abierto || actual.id_npc != actualizacion.id_npc) {
        desplazamiento = 0;
    }
    abierto = true;
    actual = actualizacion;
    calcular_alto_descripcion();
    desplazamiento = std::min(desplazamiento, disponer().max_desplazamiento);
}

void HudMercado::cerrar() {
    abierto = false;
    desplazamiento = 0;
    actual = ActualizacionMercado{};
}

bool HudMercado::esta_abierto() const {
    return abierto;
}

std::vector<std::string> HudMercado::envolver_texto(
    const std::string& texto, int ancho_maximo) const {
    std::vector<std::string> lineas;
    std::istringstream parrafos(texto);
    std::string parrafo;
    while (std::getline(parrafos, parrafo)) {
        const auto renglones = partir_en_lineas(fuente, parrafo, ancho_maximo);
        lineas.insert(lineas.end(), renglones.begin(), renglones.end());
    }
    if (lineas.empty()) {
        lineas.push_back("");
    }
    return lineas;
}

void HudMercado::calcular_alto_descripcion() {
    const int ancho_texto = ANCHO_MARCO - (2 * MARGEN_DESCRIPCION);
    std::size_t max_lineas = envolver_texto(DESCRIPCION_SIN_SELECCION, ancho_texto).size();
    for (const ItemCatalogoInfo& info : actual.catalogo) {
        max_lineas = std::max(max_lineas, envolver_texto(info.descripcion, ancho_texto).size());
    }
    alto_descripcion = std::max(
        ALTO_MINIMO_DESCRIPCION,
        static_cast<int>(max_lineas) * fuente.alto_de_renglon() + (2 * MARGEN_DESCRIPCION));
}

HudMercado::Disposicion HudMercado::disponer() const {
    Disposicion d;

    const bool con_descripcion = !actual.catalogo.empty();
    const int alto_desc = con_descripcion ? alto_descripcion : 0;
    const int separacion = con_descripcion ? SEPARACION_MARCO_Y_DESCRIPCION : 0;

    const int alto_marco = alto_proporcional(marco_npc, ANCHO_MARCO);
    const int alto_columna = alto_marco + separacion + alto_desc;
    const int ancho_total =
        ANCHO_MARCO + SEPARACION_COLUMNAS + ANCHO_CATALOGO + SEPARACION_PISTA + ANCHO_PISTA;
    const int alto_total = std::max(alto_columna, ALTO_MINIMO_CATALOGO);

    const int x = (ancho_ventana - ancho_total) / 2;
    const int y =
        std::max(MARGEN_PANTALLA, (alto_ventana - ALTO_RESERVADO_INFERIOR - alto_total) / 2);
    d.total = SDL_Rect{x, y, ancho_total, alto_total};
    d.marco = SDL_Rect{x, y, ANCHO_MARCO, alto_marco};
    d.descripcion = SDL_Rect{x, y + alto_marco + separacion, ANCHO_MARCO, alto_desc};
    d.catalogo = SDL_Rect{x + ANCHO_MARCO + SEPARACION_COLUMNAS, y, ANCHO_CATALOGO, alto_total};
    const SDL_Point nativo = tamano_de(fondo);
    const int borde_x =
        (nativo.x > 0) ? (BORDE_FONDO_ORIGEN * d.catalogo.w + (nativo.x / 2)) / nativo.x : 0;
    const int borde_y =
        (nativo.y > 0) ? (BORDE_FONDO_ORIGEN * d.catalogo.h + (nativo.y / 2)) / nativo.y : 0;
    d.lista = SDL_Rect{
        d.catalogo.x + borde_x,
        d.catalogo.y + borde_y,
        d.catalogo.w - (2 * borde_x),
        d.catalogo.h - (2 * borde_y)};
    d.pista =
        SDL_Rect{d.catalogo.x + d.catalogo.w + SEPARACION_PISTA, d.lista.y, ANCHO_PISTA, d.lista.h};
    d.cerrar = SDL_Rect{
        d.catalogo.x + d.catalogo.w - LADO_BOTON_CERRAR + SOBRESALE_BOTON_CERRAR,
        d.catalogo.y - SOBRESALE_BOTON_CERRAR,
        LADO_BOTON_CERRAR,
        LADO_BOTON_CERRAR};

    d.alto_tarjeta = std::max(1, alto_proporcional(tarjeta, d.lista.w));
    d.alto_contenido = static_cast<int>(actual.catalogo.size()) * d.alto_tarjeta;
    d.max_desplazamiento = std::max(0, d.alto_contenido - d.lista.h);
    return d;
}

SDL_Rect HudMercado::rectangulo_de_tarjeta(const Disposicion& d, int indice) const {
    return SDL_Rect{
        d.lista.x,
        d.lista.y + (indice * d.alto_tarjeta) - desplazamiento,
        d.lista.w,
        d.alto_tarjeta};
}

HudMercado::DisposicionTarjeta HudMercado::disponer_tarjeta(
    const ItemCatalogoInfo& info, const SDL_Rect& rect) const {
    DisposicionTarjeta t;
    const int renglon = fuente.alto_de_renglon();

    const int lado_icono = rect.h - (2 * MARGEN_TARJETA);
    t.icono = SDL_Rect{rect.x + MARGEN_TARJETA, rect.y + MARGEN_TARJETA, lado_icono, lado_icono};
    t.x_nombre = t.icono.x + t.icono.w + SEPARACION_ELEMENTOS;
    t.y_texto = rect.y + ((rect.h - renglon) / 2);

    const std::string precio = std::to_string(info.precio);
    t.x_precio = rect.x + rect.w - MARGEN_TARJETA - fuente.ancho(precio);
    t.moneda =
        SDL_Rect{t.x_precio - SEPARACION_MONEDA_Y_PRECIO - renglon, t.y_texto, renglon, renglon};

    const SDL_Point nativo = tamano_de(boton_vender);
    const int alto_boton = rect.h * PORCENTAJE_ALTO_BOTON_VENDER / 100;
    const int ancho_boton = (nativo.y > 0) ? alto_boton * nativo.x / nativo.y : alto_boton;
    t.boton_vender = SDL_Rect{
        t.moneda.x - SEPARACION_ELEMENTOS - ancho_boton,
        rect.y + ((rect.h - alto_boton) / 2),
        ancho_boton,
        alto_boton};
    return t;
}

int HudMercado::tarjeta_en(const Disposicion& d, int x, int y) const {
    if (!contiene(d.lista, x, y)) {
        return -1;
    }
    const int indice = (y - d.lista.y + desplazamiento) / d.alto_tarjeta;
    return (indice < static_cast<int>(actual.catalogo.size())) ? indice : -1;
}

void HudMercado::desplazar(int rueda, int x, int y) {
    if (!abierto || rueda == 0) {
        return;
    }
    const Disposicion d = disponer();
    if (!contiene(d.catalogo, x, y)) {
        return;
    }
    const int paso = d.alto_tarjeta * PORCENTAJE_DESPLAZAMIENTO_RUEDA / 100;
    desplazamiento = std::clamp(desplazamiento - (rueda * paso), 0, d.max_desplazamiento);
}

std::vector<ComandoCliente> HudMercado::clic_en_tarjeta(
    const Disposicion& d, int indice, int x, int y) {
    std::vector<ComandoCliente> comandos;
    const ItemCatalogoInfo& info = actual.catalogo[indice];

    if (info.es_animal()) {
        const SDL_Rect rect = rectangulo_de_tarjeta(d, indice);
        if (contiene(disponer_tarjeta(info, rect).boton_vender, x, y)) {
            if (info.cantidad_poseida > 0) {
                comandos.push_back(VenderAnimalMercado{info.tipo_animal, 1});
                efectos.reproducir_venta();
            }
        } else if (info.se_puede_comprar) {
            comandos.push_back(ComprarAnimalMercado{info.tipo_animal, 1});
            efectos.reproducir_compra();
        }
    } else if (info.se_puede_comprar && info.tipo_item != TipoItem::NINGUNO) {
        comandos.push_back(ComprarEnMercado{static_cast<uint16_t>(indice), 1});
        efectos.reproducir_compra();
    }
    return comandos;
}

std::vector<ComandoCliente> HudMercado::procesar_clic(
    const InventarioJugador& inventario, const CofreJugador& cofre, HudInventario& hud, int x,
    int y, bool clic_derecho) {
    std::vector<ComandoCliente> comandos;
    if (!abierto) {
        return comandos;
    }
    const Disposicion d = disponer();

    if (hud.hay_item_levantado()) {
        if (!contiene(d.total, x, y)) {
            const auto comando = hud.procesar_clic(inventario, cofre, x, y, clic_derecho, false);
            if (comando.has_value()) {
                comandos.push_back(*comando);
            }
        }
        return comandos;
    }

    if (contiene(d.cerrar, x, y)) {
        comandos.push_back(CerrarMercado{});
        return comandos;
    }

    const int indice = tarjeta_en(d, x, y);
    if (indice >= 0) {
        return clic_en_tarjeta(d, indice, x, y);
    }

    const RanuraApuntada ranura = hud.ranura_en_pantalla(inventario, cofre, x, y);
    if (ranura.zona == ZonaDelHud::BARRA) {
        const RanuraInventario& contenido = inventario.ranuras[ranura.indice];
        if (contenido.tipo != TipoItem::NINGUNO && contenido.cantidad > 0) {
            const uint16_t cantidad = clic_derecho ? 1 : 0;
            comandos.push_back(VenderEnMercado{static_cast<uint8_t>(ranura.indice), cantidad});
            efectos.reproducir_venta();
        }
    }
    return comandos;
}

void HudMercado::dibujar_imagen(
    const Textura& imagen, const SDL_Rect& destino, bool apagada) const {
    SDL_Texture* textura = imagen.ptr();
    if (apagada) {
        SDL_SetTextureColorMod(textura, BRILLO_APAGADO, BRILLO_APAGADO, BRILLO_APAGADO);
        SDL_SetTextureAlphaMod(textura, OPACIDAD_APAGADA);
    }
    SDL_RenderCopy(renderer, textura, nullptr, &destino);
    if (apagada) {
        SDL_SetTextureColorMod(textura, 255, 255, 255);
        SDL_SetTextureAlphaMod(textura, 255);
    }
}

void HudMercado::dibujar_brillo(const Textura& imagen, const SDL_Rect& destino) const {
    SDL_Texture* textura = imagen.ptr();
    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_ADD);
    SDL_SetTextureAlphaMod(textura, BRILLO_HOVER);
    SDL_RenderCopy(renderer, textura, nullptr, &destino);
    SDL_SetTextureAlphaMod(textura, 255);
    SDL_SetTextureBlendMode(textura, SDL_BLENDMODE_BLEND);
}

void HudMercado::dibujar_ventana(const Textura& imagen, const SDL_Rect& destino) const {
    const SDL_Point nativo = tamano_de(imagen);
    const int borde_origen = std::min({BORDE_VENTANA_ORIGEN, nativo.x / 2, nativo.y / 2});
    const int borde_destino = std::min({BORDE_VENTANA_DESTINO, destino.w / 2, destino.h / 2});

    dibujar_nueve_rebanadas(renderer, imagen, destino, borde_origen, borde_destino);
}

void HudMercado::dibujar_tarjeta(
    const ItemCatalogoInfo& info, const SDL_Rect& rect, bool resaltada, int raton_x,
    int raton_y) const {
    const bool apagada = !info.se_puede_comprar;
    const DisposicionTarjeta t = disponer_tarjeta(info, rect);

    dibujar_imagen(tarjeta, rect, apagada);
    if (resaltada && !apagada) {
        dibujar_brillo(tarjeta, rect);
    }

    if (info.es_animal()) {
        const Textura& foto = (info.tipo_animal == TipoAnimal::VACA) ? vaca : gallina;
        dibujar_imagen(foto, encajar(foto, t.icono), apagada);
    } else {
        iconos.dibujar(renderer, info.tipo_item, Calidad::BASE, t.icono);
        if (apagada) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(
                renderer,
                COLOR_VELO_APAGADO.r,
                COLOR_VELO_APAGADO.g,
                COLOR_VELO_APAGADO.b,
                COLOR_VELO_APAGADO.a);
            SDL_RenderFillRect(renderer, &t.icono);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
    }

    const SDL_Color color =
        apagada ? SDL_Color{
                      static_cast<Uint8>(COLOR_TEXTO.r * BRILLO_APAGADO / 255),
                      static_cast<Uint8>(COLOR_TEXTO.g * BRILLO_APAGADO / 255),
                      static_cast<Uint8>(COLOR_TEXTO.b * BRILLO_APAGADO / 255),
                      static_cast<Uint8>(OPACIDAD_APAGADA)}
                : COLOR_TEXTO;
    std::string nombre = info.nombre;
    if (info.es_animal()) {
        nombre += " (tenes " + std::to_string(info.cantidad_poseida) + ")";
    }
    fuente.dibujar(renderer, nombre, t.x_nombre, t.y_texto, color);

    dibujar_imagen(moneda, t.moneda, apagada);
    fuente.dibujar(renderer, std::to_string(info.precio), t.x_precio, t.y_texto, color);

    if (!info.es_animal()) {
        return;
    }

    const bool puede_vender = info.cantidad_poseida > 0;
    const bool encima = contiene(t.boton_vender, raton_x, raton_y);
    const Textura& boton =
        !puede_vender ? boton_vender_no_permitido : (encima ? boton_vender_hover : boton_vender);
    dibujar_imagen(boton, t.boton_vender, false);
}

void HudMercado::dibujar(int raton_x, int raton_y) const {
    if (!abierto) {
        return;
    }
    const Disposicion d = disponer();
    const int bajo_el_raton = tarjeta_en(d, raton_x, raton_y);

    dibujar_imagen(marco_npc, d.marco, false);

    if (!actual.catalogo.empty()) {
        dibujar_ventana(ventana, d.descripcion);
        const std::string texto = (bajo_el_raton >= 0) ? actual.catalogo[bajo_el_raton].descripcion
                                                       : std::string(DESCRIPCION_SIN_SELECCION);
        const SDL_Rect interior{
            d.descripcion.x + BORDE_VENTANA_DESTINO,
            d.descripcion.y + BORDE_VENTANA_DESTINO,
            d.descripcion.w - (2 * BORDE_VENTANA_DESTINO),
            d.descripcion.h - (2 * BORDE_VENTANA_DESTINO)};
        SDL_RenderSetClipRect(renderer, &interior);
        int y_linea = d.descripcion.y + MARGEN_DESCRIPCION;
        for (const std::string& linea :
             envolver_texto(texto, ANCHO_MARCO - (2 * MARGEN_DESCRIPCION))) {
            if (!linea.empty()) {
                fuente.dibujar(
                    renderer,
                    linea,
                    d.descripcion.x + MARGEN_DESCRIPCION,
                    y_linea,
                    COLOR_TEXTO);
            }
            y_linea += fuente.alto_de_renglon();
        }
        SDL_RenderSetClipRect(renderer, nullptr);
    }

    dibujar_imagen(fondo, d.catalogo, false);
    SDL_RenderSetClipRect(renderer, &d.lista);
    if (actual.catalogo.empty()) {
        const int ancho_texto = d.lista.w - (2 * MARGEN_TARJETA);
        int y_texto = d.lista.y + (d.lista.h / 3);
        for (const std::string& linea : envolver_texto(TEXTO_CATALOGO_VACIO, ancho_texto)) {
            if (!linea.empty()) {
                fuente.dibujar(
                    renderer,
                    linea,
                    d.lista.x + ((d.lista.w - fuente.ancho(linea)) / 2),
                    y_texto,
                    COLOR_TEXTO);
            }
            y_texto += fuente.alto_de_renglon();
        }
    }
    for (int i = 0; i < static_cast<int>(actual.catalogo.size()); ++i) {
        const SDL_Rect rect = rectangulo_de_tarjeta(d, i);
        if (rect.y + rect.h < d.lista.y || rect.y > d.lista.y + d.lista.h) {
            continue;
        }
        dibujar_tarjeta(actual.catalogo[i], rect, i == bajo_el_raton, raton_x, raton_y);
    }
    SDL_RenderSetClipRect(renderer, nullptr);

    if (d.max_desplazamiento > 0) {
        SDL_SetRenderDrawColor(
            renderer,
            COLOR_PISTA.r,
            COLOR_PISTA.g,
            COLOR_PISTA.b,
            COLOR_PISTA.a);
        SDL_RenderFillRect(renderer, &d.pista);

        const int alto_pulgar =
            std::max(ALTO_MINIMO_PULGAR, d.pista.h * d.lista.h / d.alto_contenido);
        const SDL_Rect pulgar{
            d.pista.x,
            d.pista.y + ((d.pista.h - alto_pulgar) * desplazamiento / d.max_desplazamiento),
            d.pista.w,
            alto_pulgar};
        SDL_SetRenderDrawColor(
            renderer,
            COLOR_PULGAR.r,
            COLOR_PULGAR.g,
            COLOR_PULGAR.b,
            COLOR_PULGAR.a);
        SDL_RenderFillRect(renderer, &pulgar);
    }

    dibujar_imagen(boton_cerrar, d.cerrar, false);
    if (contiene(d.cerrar, raton_x, raton_y)) {
        dibujar_brillo(boton_cerrar, d.cerrar);
    }
}
