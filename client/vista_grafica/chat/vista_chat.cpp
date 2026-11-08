#include "vista_chat.h"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "estilo_del_chat.h"
#include "texto_utf8.h"

namespace estilo = estilo_del_chat;

constexpr const char* APERTURA_DE_EMOTE = "[emote:";

static SDL_Color color_de_jugador(ColorDeJugador color) {
    switch (color) {
        case ColorDeJugador::ROJO:
            return estilo::JUGADOR_ROJO;
        case ColorDeJugador::VERDE:
            return estilo::JUGADOR_VERDE;
        case ColorDeJugador::AMARILLO:
            return estilo::JUGADOR_AMARILLO;
        case ColorDeJugador::AZUL:
            return estilo::JUGADOR_AZUL;
    }
    return estilo::TEXTO;
}

static SDL_Rect achicar(const SDL_Rect& r, int margen) {
    return SDL_Rect{
        r.x + margen,
        r.y + margen,
        std::max(1, r.w - (2 * margen)),
        std::max(1, r.h - (2 * margen))};
}

static SDL_Rect en_la_ventana(const SDL_Rect& ventana, const estilo::RectanguloEnLaImagen& r) {
    const int x0 = ventana.x + (r.x * ventana.w / estilo::ANCHO_DE_LA_IMAGEN);
    const int y0 = ventana.y + (r.y * ventana.h / estilo::ALTO_DE_LA_IMAGEN);
    const int x1 = ventana.x + ((r.x + r.ancho) * ventana.w / estilo::ANCHO_DE_LA_IMAGEN);
    const int y1 = ventana.y + ((r.y + r.alto) * ventana.h / estilo::ALTO_DE_LA_IMAGEN);
    return SDL_Rect{x0, y0, x1 - x0, y1 - y0};
}

static void rellenar(SDL_Renderer* renderer, const SDL_Rect& r, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &r);
}

static void dibujar_circulo(SDL_Renderer* renderer, int cx, int cy, int radio, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int y = -radio; y <= radio; ++y) {
        const int x = static_cast<int>(std::sqrt(radio * radio - y * y));

        SDL_RenderDrawLine(renderer, cx - x, cy + y, cx + x, cy + y);
    }
}

static void dibujar_recuadro(SDL_Renderer* renderer, const SDL_Rect& r, SDL_Color fondo) {
    rellenar(renderer, r, fondo);
    const SDL_Color borde = estilo::BORDE_DE_LOS_RECUADROS;
    SDL_SetRenderDrawColor(renderer, borde.r, borde.g, borde.b, borde.a);
    SDL_RenderDrawRect(renderer, &r);
}

static SDL_Texture* cargar_textura(SDL_Renderer* renderer, const std::string& ruta) {
    SDL_Texture* textura = IMG_LoadTexture(renderer, ruta.c_str());
    if (textura == nullptr) {
        throw std::runtime_error("no se pudo cargar '" + ruta + "': " + IMG_GetError());
    }
    return textura;
}

static PiezaChat pieza_de_imagen(SDL_Texture* textura, int lado, int separacion) {
    PiezaChat pieza;
    pieza.imagen = textura;
    pieza.ancho = lado;
    pieza.alto = lado + separacion;
    return pieza;
}

static const ImagenChat* leer_imagen(
    const std::string& texto, size_t& posicion, const std::string& apertura, char cierre,
    const CatalogoDeImagenes& catalogo) {
    if (texto.compare(posicion, apertura.size(), apertura) != 0) {
        return nullptr;
    }
    const size_t inicio = posicion + apertura.size();
    const size_t fin = texto.find(cierre, inicio);
    if (fin == std::string::npos) {
        return nullptr;
    }
    const ImagenChat* imagen = catalogo.buscar(texto.substr(inicio, fin - inicio));
    if (imagen != nullptr) {
        posicion = fin + 1;
    }
    return imagen;
}

VistaChat::VistaChat(const ConfiguracionChat& configuracion, SDL_Renderer* renderer)
    : fuente(
          TTF_OpenFont(configuracion.ruta_fuente.c_str(), configuracion.tamano_de_fuente),
          TTF_CloseFont),
      emojis(renderer, configuracion.ruta_carpeta_de_emojis),
      emotes(renderer, configuracion.ruta_carpeta_de_emotes),
      cache_de_texto(renderer, fuente.get()),
      icono(cargar_textura(renderer, configuracion.ruta_icono), SDL_DestroyTexture),
      fondo(cargar_textura(renderer, estilo::RUTA_DE_LA_IMAGEN), SDL_DestroyTexture),
      modelo(static_cast<size_t>(configuracion.maximo_de_mensajes)) {
    if (fuente == nullptr) {
        throw std::runtime_error(
            "no se pudo cargar la fuente del chat: " + std::string(TTF_GetError()));
    }
    SDL_SetTextureScaleMode(fondo.get(), estilo::FILTRO_DE_LA_IMAGEN);
    alto_de_linea = TTF_FontLineSkip(fuente.get());
    lado_de_emoji = TTF_FontHeight(fuente.get());
}

void VistaChat::limpiar() {
    modelo.limpiar();
    lineas_de_mensajes.clear();
    mensaje_confirmado.reset();
    desenfocar_entrada();
    fila_inicial_de_elementos = 0;
    recalcular_lineas_de_entrada();
}

void VistaChat::recibir(const MensajeDeChat& mensaje) {
    modelo.recibir(mensaje);
    lineas_de_mensajes.push_back(envolver(piezas_del_mensaje(mensaje), ancho_de_mensajes));
    if (lineas_de_mensajes.size() > modelo.mensajes().size()) {
        lineas_de_mensajes.pop_front();
    }
}

std::optional<EnviarMensajeDeChat> VistaChat::tomar_mensaje_a_enviar() {
    return std::exchange(mensaje_confirmado, std::nullopt);
}

bool VistaChat::esta_escribiendo() const {
    return modelo.esta_escribiendo();
}

ConsumoDeEntradaChat VistaChat::procesar_entrada(
    const EntradaDelUsuario& entrada, int ancho_pantalla, int alto_pantalla) {
    ajustar_envoltura(disponer(ancho_pantalla, alto_pantalla));
    cache_de_texto.liberar_si_excede_el_limite();
    raton = SDL_Point{entrada.raton_x(), entrada.raton_y()};

    const EntradaDelUsuario::Pedidos& pedidos = entrada.pedidos();
    const DisposicionChat d = disponer(ancho_pantalla, alto_pantalla);

    if (pedidos.alternar_chat) {
        if (modelo.esta_abierto()) {
            cerrar_chat();
        } else {
            abrir_chat(d);
        }
    }

    if (modelo.esta_escribiendo()) {
        editar_texto(pedidos);
        if (pedidos.confirmar_chat) {
            enviar_texto_escrito();
        } else if (pedidos.cancelar_escritura) {
            desenfocar_entrada();
        }
    }

    ConsumoDeEntradaChat consumo;
    manejar_clic(pedidos, d, consumo);
    manejar_rueda(pedidos, d, consumo);
    return consumo;
}

void VistaChat::manejar_clic(
    const EntradaDelUsuario::Pedidos& pedidos, const DisposicionChat& d,
    ConsumoDeEntradaChat& consumo) {
    if (!pedidos.clic_izquierdo && !pedidos.clic_derecho) {
        return;
    }
    const bool izquierdo = pedidos.clic_izquierdo;
    const auto sobre = [this](const SDL_Rect& r) {
        return SDL_PointInRect(&raton, &r) == SDL_TRUE;
    };

    if (!modelo.esta_abierto()) {
        if (sobre(d.boton_abrir)) {
            abrir_chat(d);
            consumo.clic = true;
        }
        return;
    }

    const bool estaba_escribiendo = modelo.esta_escribiendo();
    const bool boton_de_bajar_visible = desplazamiento_efectivo(d.area_de_mensajes.h) > 0;

    if (izquierdo && sobre(d.boton_cerrar)) {
        cerrar_chat();
    } else if (izquierdo && boton_de_bajar_visible && sobre(d.boton_ir_al_final)) {
        modelo.ir_al_final();
    } else if (izquierdo && sobre(d.boton_emojis)) {
        alternar_menu_de_emojis();
    } else if (izquierdo && menu_de_emojis_abierto && sobre(d.menu_de_emojis)) {
        if (sobre(d.solapa_emojis) || sobre(d.solapa_emotes)) {
            pestana_actual = sobre(d.solapa_emojis) ? PestanaMenu::EMOJIS : PestanaMenu::EMOTES;
            fila_inicial_de_elementos = 0;
        } else {
            elegir_del_menu(d);
        }
    } else if (izquierdo && sobre(d.entrada)) {
        enfocar_entrada();
    } else if (estaba_escribiendo) {
        desenfocar_entrada();
    }

    consumo.clic = estaba_escribiendo || sobre(d.ventana);
}

void VistaChat::manejar_rueda(
    const EntradaDelUsuario::Pedidos& pedidos, const DisposicionChat& d,
    ConsumoDeEntradaChat& consumo) {
    if (pedidos.rueda == 0) {
        return;
    }

    const bool sobre_la_ventana =
        modelo.esta_abierto() && SDL_PointInRect(&raton, &d.ventana) == SDL_TRUE;

    if (sobre_la_ventana && menu_de_emojis_abierto &&
        SDL_PointInRect(&raton, &d.menu_de_emojis) == SDL_TRUE) {
        const int ultima_fila_inicial =
            std::max(0, d.filas_totales_del_menu - d.filas_visibles_del_menu);
        fila_inicial_de_elementos =
            std::clamp(fila_inicial_de_elementos - pedidos.rueda, 0, ultima_fila_inicial);
        consumo.rueda = true;
    } else if (sobre_la_ventana) {
        modelo.desplazar(
            pedidos.rueda * estilo::MENSAJES_POR_MUESCA,
            desplazamiento_maximo(d.area_de_mensajes.h));
        consumo.rueda = true;
    } else if (modelo.esta_escribiendo()) {
        consumo.rueda = true;
    }
}

void VistaChat::abrir_chat(const DisposicionChat& d) {
    const size_t total = lineas_de_mensajes.size();
    const size_t sin_leer = std::min(modelo.cantidad_sin_leer(), total);

    int usado = 0;
    size_t entran = 0;
    for (size_t i = total - sin_leer; i < total; ++i) {
        const int alto = lineas_de_mensajes[i].alto;
        if (entran > 0 && usado + alto > d.area_de_mensajes.h) {
            break;
        }
        usado += alto;
        ++entran;
    }
    modelo.abrir(entran);
}

void VistaChat::cerrar_chat() {
    desenfocar_entrada();
    modelo.cerrar();
}

void VistaChat::enfocar_entrada() {
    modelo.empezar_a_escribir();
    SDL_StartTextInput();
}

void VistaChat::desenfocar_entrada() {
    modelo.terminar_de_escribir();
    menu_de_emojis_abierto = false;
    SDL_StopTextInput();
}

void VistaChat::editar_texto(const EntradaDelUsuario::Pedidos& pedidos) {
    for (int i = 0; i < pedidos.caracteres_a_borrar; ++i) {
        modelo.borrar_caracter();
    }
    if (!pedidos.texto_ingresado.empty()) {
        modelo.agregar_texto(pedidos.texto_ingresado);
    }
    if (pedidos.caracteres_a_borrar > 0 || !pedidos.texto_ingresado.empty()) {
        recalcular_lineas_de_entrada();
    }
}

void VistaChat::enviar_texto_escrito() {
    std::string texto = modelo.confirmar_y_limpiar();
    if (!texto.empty()) {
        mensaje_confirmado = EnviarMensajeDeChat{std::move(texto)};
    }
    recalcular_lineas_de_entrada();
    desenfocar_entrada();
}

void VistaChat::alternar_menu_de_emojis() {
    if (menu_de_emojis_abierto) {
        menu_de_emojis_abierto = false;
        return;
    }
    enfocar_entrada();
    menu_de_emojis_abierto = true;
    pestana_actual = PestanaMenu::EMOJIS;
    fila_inicial_de_elementos = 0;
}

void VistaChat::elegir_del_menu(const DisposicionChat& d) {
    const int x = raton.x - d.origen_de_celdas.x;
    const int y = raton.y - d.origen_de_celdas.y;
    if (x < 0 || y < 0) {
        return;
    }
    const int columna = x / d.celda_del_menu;
    const int fila = y / d.celda_del_menu;
    if (columna >= d.columnas_del_menu || fila >= d.filas_visibles_del_menu) {
        return;
    }

    const std::vector<ImagenChat>& imagenes = catalogo_actual().todas();
    const size_t indice =
        static_cast<size_t>(((fila_inicial_de_elementos + fila) * d.columnas_del_menu) + columna);
    if (indice >= imagenes.size()) {
        return;
    }

    const std::string& nombre = imagenes[indice].nombre;
    if (pestana_actual == PestanaMenu::EMOJIS) {
        modelo.agregar_texto(":" + nombre + ":");
        recalcular_lineas_de_entrada();
    } else {
        mensaje_confirmado = EnviarMensajeDeChat{APERTURA_DE_EMOTE + nombre + "]"};
        desenfocar_entrada();
    }
}

int VistaChat::ancho_de_texto(const std::string& texto) const {
    int ancho = 0;
    int alto = 0;
    if (texto.empty() || TTF_SizeUTF8(fuente.get(), texto.c_str(), &ancho, &alto) != 0) {
        return 0;
    }
    return ancho;
}

const CatalogoDeImagenes& VistaChat::catalogo_actual() const {
    return pestana_actual == PestanaMenu::EMOJIS ? emojis : emotes;
}

DisposicionChat VistaChat::disponer(int ancho_pantalla, int alto_pantalla) const {
    DisposicionChat d;
    d.boton_abrir = {
        estilo::MARGEN_DE_PANTALLA,
        estilo::MARGEN_DE_PANTALLA,
        estilo::LADO_DEL_BOTON_DE_APERTURA,
        estilo::LADO_DEL_BOTON_DE_APERTURA};

    d.ventana = {
        estilo::MARGEN_DE_PANTALLA,
        estilo::MARGEN_DE_PANTALLA,
        std::max(
            1,
            std::min(
                estilo::ANCHO_DE_LA_VENTANA,
                ancho_pantalla - (2 * estilo::MARGEN_DE_PANTALLA))),
        std::max(
            1,
            std::min(
                estilo::ALTO_DE_LA_VENTANA,
                alto_pantalla - (2 * estilo::MARGEN_DE_PANTALLA)))};
    d.header = en_la_ventana(d.ventana, estilo::TITULO_EN_LA_IMAGEN);
    d.boton_cerrar = en_la_ventana(d.ventana, estilo::BOTON_CERRAR_EN_LA_IMAGEN);
    const SDL_Rect cuerpo =
        achicar(en_la_ventana(d.ventana, estilo::CUERPO_EN_LA_IMAGEN), estilo::RELLENO_DEL_CUERPO);

    d.lineas_de_entrada_visibles = std::clamp(
        static_cast<int>(lineas_de_entrada.lineas.size()),
        1,
        estilo::MAXIMO_DE_LINEAS_DE_ENTRADA);
    const int alto_de_la_entrada =
        (d.lineas_de_entrada_visibles * alto_de_linea) + (2 * estilo::RELLENO_DE_ENTRADA);
    d.entrada = {cuerpo.x, cuerpo.y + cuerpo.h - alto_de_la_entrada, cuerpo.w, alto_de_la_entrada};
    d.boton_emojis = {
        d.entrada.x + d.entrada.w - estilo::RELLENO_DE_ENTRADA - estilo::LADO_DEL_BOTON_DE_EMOJIS,
        d.entrada.y + ((d.entrada.h - estilo::LADO_DEL_BOTON_DE_EMOJIS) / 2),
        estilo::LADO_DEL_BOTON_DE_EMOJIS,
        estilo::LADO_DEL_BOTON_DE_EMOJIS};

    d.area_de_mensajes = {
        cuerpo.x,
        cuerpo.y,
        cuerpo.w,
        std::max(1, d.entrada.y - estilo::SEPARACION - cuerpo.y)};
    d.boton_ir_al_final = {
        d.area_de_mensajes.x + d.area_de_mensajes.w - estilo::LADO_DEL_BOTON_DE_BAJAR,
        d.area_de_mensajes.y + d.area_de_mensajes.h - estilo::LADO_DEL_BOTON_DE_BAJAR,
        estilo::LADO_DEL_BOTON_DE_BAJAR,
        estilo::LADO_DEL_BOTON_DE_BAJAR};

    d.celda_del_menu = lado_de_emoji + (2 * estilo::RELLENO_DE_CELDA);
    d.columnas_del_menu =
        std::max(1, (cuerpo.w - (2 * estilo::RELLENO_DEL_MENU)) / d.celda_del_menu);
    const int cantidad = static_cast<int>(catalogo_actual().todas().size());
    d.filas_totales_del_menu = (cantidad + d.columnas_del_menu - 1) / d.columnas_del_menu;
    d.filas_visibles_del_menu =
        std::clamp(d.filas_totales_del_menu, 1, estilo::MAXIMO_DE_FILAS_DEL_MENU);

    const int alto_del_menu = estilo::ALTO_DE_SOLAPAS +
                              (d.filas_visibles_del_menu * d.celda_del_menu) +
                              (2 * estilo::RELLENO_DEL_MENU);
    d.menu_de_emojis = {
        cuerpo.x,
        d.entrada.y - estilo::SEPARACION - alto_del_menu,
        std::min(
            cuerpo.w,
            (d.columnas_del_menu * d.celda_del_menu) + (2 * estilo::RELLENO_DEL_MENU)),
        alto_del_menu};
    d.origen_de_celdas = {
        d.menu_de_emojis.x + estilo::RELLENO_DEL_MENU,
        d.menu_de_emojis.y + estilo::ALTO_DE_SOLAPAS + estilo::RELLENO_DEL_MENU};
    d.solapa_emojis = {
        d.menu_de_emojis.x + 4,
        d.menu_de_emojis.y + 4,
        estilo::ANCHO_DE_SOLAPA,
        estilo::ALTO_DE_SOLAPAS - 8};
    d.solapa_emotes = {
        d.solapa_emojis.x + estilo::ANCHO_DE_SOLAPA + 4,
        d.solapa_emojis.y,
        estilo::ANCHO_DE_SOLAPA,
        estilo::ALTO_DE_SOLAPAS - 8};
    return d;
}

PiezaChat VistaChat::pieza_de_texto(std::string texto, SDL_Color color) const {
    PiezaChat pieza;
    pieza.ancho = ancho_de_texto(texto);
    pieza.alto = alto_de_linea;
    pieza.texto = std::move(texto);
    pieza.color = color;
    return pieza;
}

std::vector<PiezaChat> VistaChat::dividir_en_piezas(
    const std::string& texto, SDL_Color color, bool con_emotes) const {
    std::vector<PiezaChat> piezas;
    std::string palabra;
    const auto cerrar_palabra = [&]() {
        if (!palabra.empty()) {
            piezas.push_back(pieza_de_texto(palabra, color));
            palabra.clear();
        }
    };

    for (size_t i = 0; i < texto.size();) {
        size_t fin = i;
        const ImagenChat* emote =
            con_emotes ? leer_imagen(texto, fin, APERTURA_DE_EMOTE, ']', emotes) : nullptr;
        const ImagenChat* emoji =
            emote == nullptr ? leer_imagen(texto, fin, ":", ':', emojis) : nullptr;
        if (emote != nullptr || emoji != nullptr) {
            cerrar_palabra();
            piezas.push_back(
                emote != nullptr ? pieza_de_imagen(
                                       emote->textura,
                                       estilo::ALTO_DE_EMOTE,
                                       estilo::SEPARACION_ENTRE_EMOTES)
                                 : pieza_de_imagen(emoji->textura, lado_de_emoji, 0));
            i = fin;
            continue;
        }

        palabra += texto[i];
        if (texto[i] == ' ') {
            cerrar_palabra();
        }
        ++i;
    }
    cerrar_palabra();
    return piezas;
}

std::vector<PiezaChat> VistaChat::piezas_del_mensaje(const MensajeDeChat& mensaje) const {
    std::string prefijo = mensaje.remitente + ": ";
    SDL_Color color_del_prefijo = color_de_jugador(mensaje.color_del_remitente);
    SDL_Color color_del_cuerpo = estilo::TEXTO;

    switch (mensaje.canal) {
        case CanalDeChat::GLOBAL:
            break;
        case CanalDeChat::PRIVADO:
            prefijo = "(Privado) " + mensaje.remitente + ": ";
            break;
        case CanalDeChat::SISTEMA:
            prefijo = "Sistema: ";
            color_del_prefijo = estilo::SISTEMA;
            color_del_cuerpo = estilo::SISTEMA;
            break;
        case CanalDeChat::COMERCIANTE:
            color_del_prefijo = estilo::COMERCIANTE;
            break;
    }

    std::vector<PiezaChat> piezas = dividir_en_piezas(prefijo, color_del_prefijo, false);
    std::vector<PiezaChat> cuerpo = dividir_en_piezas(mensaje.texto, color_del_cuerpo, true);
    piezas.insert(
        piezas.end(),
        std::make_move_iterator(cuerpo.begin()),
        std::make_move_iterator(cuerpo.end()));
    return piezas;
}

TextoEnvueltoChat VistaChat::envolver(
    const std::vector<PiezaChat>& piezas, int ancho_maximo) const {
    TextoEnvueltoChat resultado;
    std::vector<LineaChat>& lineas = resultado.lineas;

    const auto nueva_linea = [&]() { lineas.push_back(LineaChat{{}, 0, alto_de_linea}); };
    const auto agregar = [&](PiezaChat pieza) {
        if (!lineas.back().piezas.empty() && lineas.back().ancho + pieza.ancho > ancho_maximo) {
            nueva_linea();
        }
        LineaChat& linea = lineas.back();
        linea.ancho += pieza.ancho;
        linea.alto = std::max(linea.alto, pieza.alto);
        linea.piezas.push_back(std::move(pieza));
    };

    nueva_linea();
    for (const PiezaChat& pieza : piezas) {
        if (pieza.imagen != nullptr || pieza.ancho <= ancho_maximo) {
            agregar(pieza);
            continue;
        }

        std::string trozo;
        for (size_t posicion = 0; posicion < pieza.texto.size();) {
            const size_t largo = std::min(
                longitud_de_caracter_utf8(pieza.texto, posicion),
                pieza.texto.size() - posicion);
            const std::string candidato = trozo + pieza.texto.substr(posicion, largo);
            if (!trozo.empty() && ancho_de_texto(candidato) > ancho_maximo) {
                agregar(pieza_de_texto(std::move(trozo), pieza.color));
                trozo.clear();
                nueva_linea();
                continue;
            }
            trozo = candidato;
            posicion += largo;
        }
        if (!trozo.empty()) {
            agregar(pieza_de_texto(std::move(trozo), pieza.color));
        }
    }

    for (const LineaChat& linea : lineas) {
        resultado.alto += linea.alto;
    }
    return resultado;
}

void VistaChat::ajustar_envoltura(const DisposicionChat& d) {
    const int ancho_del_texto_escrito = std::max(
        1,
        d.area_de_mensajes.w - (3 * estilo::RELLENO_DE_ENTRADA) - estilo::LADO_DEL_BOTON_DE_EMOJIS);
    if (d.area_de_mensajes.w == ancho_de_mensajes && ancho_del_texto_escrito == ancho_de_entrada) {
        return;
    }

    ancho_de_mensajes = d.area_de_mensajes.w;
    ancho_de_entrada = ancho_del_texto_escrito;
    lineas_de_mensajes.clear();
    for (const MensajeDeChat& mensaje : modelo.mensajes()) {
        lineas_de_mensajes.push_back(envolver(piezas_del_mensaje(mensaje), ancho_de_mensajes));
    }
    recalcular_lineas_de_entrada();
}

void VistaChat::recalcular_lineas_de_entrada() {
    lineas_de_entrada = envolver(
        dividir_en_piezas(modelo.texto_en_edicion(), estilo::TEXTO, false),
        ancho_de_entrada);
}

size_t VistaChat::desplazamiento_maximo(int alto_disponible) const {
    int acumulado = 0;
    for (size_t i = 0; i < lineas_de_mensajes.size(); ++i) {
        acumulado += lineas_de_mensajes[i].alto;
        if (acumulado >= alto_disponible) {
            return lineas_de_mensajes.size() - (i + 1);
        }
    }
    return 0;
}

size_t VistaChat::desplazamiento_efectivo(int alto_disponible) const {
    return std::min(modelo.desplazamiento(), desplazamiento_maximo(alto_disponible));
}

std::pair<size_t, size_t> VistaChat::ventana_de_mensajes(int alto_disponible) const {
    const size_t fin = lineas_de_mensajes.size() - desplazamiento_efectivo(alto_disponible);

    size_t inicio = fin;
    int usado = 0;
    while (inicio > 0) {
        const int alto = lineas_de_mensajes[inicio - 1].alto;
        if (inicio < fin && usado + alto > alto_disponible) {
            break;
        }
        usado += alto;
        --inicio;
    }
    return {inicio, fin};
}

void VistaChat::dibujar(SDL_Renderer* renderer, int ancho_pantalla, int alto_pantalla) const {
    const DisposicionChat d = disponer(ancho_pantalla, alto_pantalla);
    if (!modelo.esta_abierto()) {
        dibujar_boton_de_apertura(renderer, d);
        return;
    }
    SDL_RenderCopy(renderer, fondo.get(), nullptr, &d.ventana);
    dibujar_encabezado(renderer, d);
    dibujar_mensajes(renderer, d);
    dibujar_boton_ir_al_final(renderer, d);
    dibujar_entrada(renderer, d);
    if (menu_de_emojis_abierto) {
        dibujar_menu_de_emojis(renderer, d);
    }
}

void VistaChat::dibujar_boton_de_apertura(SDL_Renderer* renderer, const DisposicionChat& d) const {
    const SDL_Rect& boton = d.boton_abrir;
    SDL_RenderCopy(renderer, icono.get(), nullptr, &boton);

    const size_t sin_leer = modelo.cantidad_sin_leer();
    if (sin_leer == 0) {
        return;
    }
    const std::string numero = sin_leer > estilo::MAXIMO_DE_NO_LEIDOS_VISIBLE
                                   ? "+" + std::to_string(estilo::MAXIMO_DE_NO_LEIDOS_VISIBLE)
                                   : std::to_string(sin_leer);
    const int ancho_del_numero = ancho_de_texto(numero);
    const int alto_del_aviso = alto_de_linea + 2;
    const int ancho_del_aviso = std::max(alto_del_aviso, ancho_del_numero + 10);
    const SDL_Rect aviso{
        boton.x + boton.w - ancho_del_aviso + 4,
        boton.y - 4,
        ancho_del_aviso,
        alto_del_aviso};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const int radio = aviso.h / 2;
    dibujar_circulo(renderer, aviso.x + radio, aviso.y + radio, radio + 1, estilo::BORDE_DEL_AVISO);
    dibujar_circulo(renderer, aviso.x + radio, aviso.y + radio, radio, estilo::AVISO_DE_NO_LEIDOS);

    dibujar_texto(
        renderer,
        numero,
        estilo::TEXTO,
        aviso.x + ((aviso.w - ancho_del_numero) / 2),
        aviso.y + ((aviso.h - alto_de_linea) / 2));
}

void VistaChat::dibujar_encabezado(SDL_Renderer* renderer, const DisposicionChat& d) const {
    const TexturaDeTexto* texto = cache_de_texto.obtener("Chat", estilo::TITULO);

    if (texto == nullptr) {
        return;
    }

    constexpr float ESCALA_TITULO = 1.3f;
    const int ancho = static_cast<int>(texto->ancho * ESCALA_TITULO);
    const int alto = static_cast<int>(texto->alto * ESCALA_TITULO);
    const SDL_Rect destino{
        d.header.x + estilo::RELLENO_DEL_TITULO,
        d.header.y + (d.header.h - alto) / 2 - 10,
        ancho,
        alto};
    SDL_RenderCopy(renderer, texto->textura, nullptr, &destino);
}

void VistaChat::dibujar_mensajes(SDL_Renderer* renderer, const DisposicionChat& d) const {
    const SDL_Rect& area = d.area_de_mensajes;
    SDL_RenderSetClipRect(renderer, &area);

    if (lineas_de_mensajes.empty()) {
        dibujar_texto(renderer, "No hay mensajes.", estilo::TEXTO_APAGADO, area.x, area.y);
    } else {
        const auto [inicio, fin] = ventana_de_mensajes(area.h);
        int y = area.y;
        for (size_t i = inicio; i < fin; ++i) {
            for (const LineaChat& linea : lineas_de_mensajes[i].lineas) {
                dibujar_linea(renderer, linea, area.x, y);
                y += linea.alto;
            }
        }
    }
    SDL_RenderSetClipRect(renderer, nullptr);
}

void VistaChat::dibujar_boton_ir_al_final(SDL_Renderer* renderer, const DisposicionChat& d) const {
    if (desplazamiento_efectivo(d.area_de_mensajes.h) == 0) {
        return;
    }
    const SDL_Rect& boton = d.boton_ir_al_final;
    dibujar_recuadro(renderer, boton, estilo::FONDO_DEL_SELECTOR);

    const int x = boton.x + (boton.w / 2);
    const int y = boton.y + (boton.h / 2);
    SDL_SetRenderDrawColor(
        renderer,
        estilo::FLECHA.r,
        estilo::FLECHA.g,
        estilo::FLECHA.b,
        estilo::FLECHA.a);
    for (int grosor = 0; grosor < 2; ++grosor) {
        SDL_RenderDrawLine(renderer, x - 5, y - 2 + grosor, x, y + 3 + grosor);
        SDL_RenderDrawLine(renderer, x + 5, y - 2 + grosor, x, y + 3 + grosor);
    }
}

void VistaChat::dibujar_entrada(SDL_Renderer* renderer, const DisposicionChat& d) const {
    const bool escribiendo = modelo.esta_escribiendo();
    dibujar_recuadro(renderer, d.entrada, estilo::FONDO_DE_LA_ENTRADA);

    const SDL_Rect area{
        d.entrada.x + estilo::RELLENO_DE_ENTRADA,
        d.entrada.y + estilo::RELLENO_DE_ENTRADA,
        ancho_de_entrada,
        d.lineas_de_entrada_visibles * alto_de_linea};
    SDL_RenderSetClipRect(renderer, &area);

    if (!escribiendo && modelo.texto_en_edicion().empty()) {
        dibujar_texto(renderer, "Escribir...", estilo::TEXTO_APAGADO, area.x, area.y);
    } else {
        const std::vector<LineaChat>& lineas = lineas_de_entrada.lineas;
        const size_t visibles = static_cast<size_t>(d.lineas_de_entrada_visibles);
        int y = area.y;
        for (size_t i = lineas.size() > visibles ? lineas.size() - visibles : 0; i < lineas.size();
             ++i) {
            dibujar_linea(renderer, lineas[i], area.x, y);
            y += alto_de_linea;
        }

        if (escribiendo && ((SDL_GetTicks() / estilo::PERIODO_DEL_CURSOR_MS) % 2) == 0) {
            const int ancho_de_la_ultima = lineas.empty() ? 0 : lineas.back().ancho;
            rellenar(
                renderer,
                SDL_Rect{area.x + ancho_de_la_ultima, y - alto_de_linea + 2, 2, alto_de_linea - 4},
                estilo::TEXTO);
        }
    }
    SDL_RenderSetClipRect(renderer, nullptr);

    if (menu_de_emojis_abierto) {
        rellenar(renderer, d.boton_emojis, estilo::CELDA_BAJO_EL_RATON);
    }
    if (!emojis.todas().empty()) {
        const SDL_Rect destino = achicar(d.boton_emojis, 2);
        SDL_RenderCopy(renderer, emojis.todas()[0].textura, nullptr, &destino);
    }
}

void VistaChat::dibujar_menu_de_emojis(SDL_Renderer* renderer, const DisposicionChat& d) const {
    dibujar_recuadro(renderer, d.menu_de_emojis, estilo::FONDO_DEL_SELECTOR);

    const bool en_emojis = pestana_actual == PestanaMenu::EMOJIS;
    const auto dibujar_solapa = [&](const SDL_Rect& solapa, const char* nombre, bool activa) {
        rellenar(renderer, solapa, activa ? estilo::PESTANA_ACTIVA : estilo::PESTANA_INACTIVA);
        dibujar_texto(renderer, nombre, estilo::TITULO, solapa.x + 6, solapa.y + 2);
    };
    dibujar_solapa(d.solapa_emojis, "Emojis", en_emojis);
    dibujar_solapa(d.solapa_emotes, "Emotes", !en_emojis);

    const std::vector<ImagenChat>& imagenes = catalogo_actual().todas();
    if (imagenes.empty()) {
        dibujar_texto(
            renderer,
            en_emojis ? "Sin emojis." : "Sin emotes.",
            estilo::TEXTO_APAGADO,
            d.origen_de_celdas.x,
            d.origen_de_celdas.y);
    }

    const int primera_celda = fila_inicial_de_elementos * d.columnas_del_menu;
    const int celdas_visibles = d.filas_visibles_del_menu * d.columnas_del_menu;
    for (int celda = 0; celda < celdas_visibles; ++celda) {
        const size_t indice = static_cast<size_t>(primera_celda + celda);
        if (indice >= imagenes.size()) {
            break;
        }
        const SDL_Rect rect{
            d.origen_de_celdas.x + ((celda % d.columnas_del_menu) * d.celda_del_menu),
            d.origen_de_celdas.y + ((celda / d.columnas_del_menu) * d.celda_del_menu),
            d.celda_del_menu,
            d.celda_del_menu};
        if (SDL_PointInRect(&raton, &rect) == SDL_TRUE) {
            rellenar(renderer, rect, estilo::CELDA_BAJO_EL_RATON);
        }
        const SDL_Rect destino = achicar(rect, estilo::RELLENO_DE_CELDA);
        SDL_RenderCopy(renderer, imagenes[indice].textura, nullptr, &destino);
    }
}

void VistaChat::dibujar_linea(SDL_Renderer* renderer, const LineaChat& linea, int x, int y) const {
    for (const PiezaChat& pieza : linea.piezas) {
        if (pieza.imagen != nullptr) {
            const SDL_Rect destino{
                x,
                y + std::max(0, (alto_de_linea - pieza.ancho) / 2),
                pieza.ancho,
                pieza.ancho};
            SDL_RenderCopy(renderer, pieza.imagen, nullptr, &destino);
        } else {
            dibujar_texto(renderer, pieza.texto, pieza.color, x, y);
        }
        x += pieza.ancho;
    }
}

void VistaChat::dibujar_texto(
    SDL_Renderer* renderer, const std::string& texto, SDL_Color color, int x, int y) const {
    const TexturaDeTexto* rasterizado = cache_de_texto.obtener(texto, color);
    if (rasterizado == nullptr) {
        return;
    }
    const SDL_Rect destino{x, y, rasterizado->ancho, rasterizado->alto};
    SDL_RenderCopy(renderer, rasterizado->textura, nullptr, &destino);
}
