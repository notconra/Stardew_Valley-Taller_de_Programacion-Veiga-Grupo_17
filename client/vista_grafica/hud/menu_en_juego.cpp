#include "menu_en_juego.h"

#include "comunes_de_panel.h"

#include <algorithm>
#include <map>
#include <string>

// Rutas constantes a los assets de interfaz
static const char RUTA_ASSET_MARCO[] = "Assets/LooseSprites/menu_box.png";
static const char RUTA_ASSET_BOTON[] = "Assets/hud/regalos_boton.png";
static const char RUTA_ASSET_BOTON_CERRAR[] = "Assets/hud/cerrar_regalos.png";
static const char RUTA_ASSET_CURSORES[] = "Assets/LooseSprites/Cursors.png";
static const char RUTA_ASSET_POLLITO[] = "Assets/Farm/Animals/Baby Chicken Yellow.png";
static const char RUTA_ASSET_LECHE[] = "Assets/items/leche.png";
static const char RUTA_ASSET_HUEVO[] = "Assets/items/huevo.png";

// Recortes exactos de sprites originales en Cursors.png
static constexpr SDL_Rect ORIGEN_SPRITE_PESTANIA_EN_BLANCO{16, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_POLLITO{0, 0, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_BOLSA{80, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_JOYSTICK{96, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_PROHIBIDO{112, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_BOTON_OK{128, 256, 64, 64};
static constexpr SDL_Rect ORIGEN_SPRITE_BOTON_CANCELAR{192, 256, 64, 64};
static constexpr SDL_Rect ORIGEN_SPRITE_GALLINAS{3, 448, 26, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_VACA{40, 449, 17, 14};

// Pestaña del centro comunitario: una fila por lote.
#define PRIMERA_FILA_DE_LOTES_Y 88
#define ALTO_DE_FILA_DE_LOTE 60
#define ALTO_DE_TARJETA_DE_LOTE 54
#define MARGEN_DE_TARJETA_DE_LOTE 35
#define LADO_DEL_ICONO_DE_LOTE 40
#define NOMBRE_DE_LOTE_X 62
#define BARRA_DE_AVANCE_X 300
#define ANCHO_DE_BARRA_DE_AVANCE 130
#define ALTO_DE_BARRA_DE_AVANCE 14
#define SEPARACION_TEXTO_DE_AVANCE 10
#define ESCALA_DE_LA_TILDE 2
#define TEXTO_DE_LOTE_COMPLETO "Completo"
#define TEXTO_SIN_DATOS_DEL_CENTRO "Todavia no hay datos del centro."

// Cartel de felicitación por completar el centro (la fuente no tiene acentos).
#define ANCHO_CARTEL_FELICITACION 560
#define ALTO_CARTEL_FELICITACION 230
#define TITULO_FELICITACION_Y 36
#define TEXTO_FELICITACION_Y 84
#define BOTONES_FELICITACION_Y 140
#define ANCHO_BOTON_FELICITACION 230
#define ALTO_BOTON_FELICITACION 56
#define SEPARACION_BOTONES_FELICITACION 24
#define TITULO_FELICITACION "Felicitaciones!"
#define TEXTO_FELICITACION "Completaron el centro comunitario!"
#define BOTON_SEGUIR_JUGANDO "Seguir jugando"
#define BOTON_VOLVER_AL_LOBBY "Volver al lobby"

MenuEnJuego::MenuEnJuego(
    SDL_Renderer* renderizador, const std::string& ruta_fuente,
    EfectosDeSonido& efectos_de_sonido_referencia, const IconosDeLotes& iconos_de_lotes_referencia,
    int ancho_pantalla, int alto_pantalla)
    : efectos_de_sonido(efectos_de_sonido_referencia),
      iconos_de_lotes(iconos_de_lotes_referencia),
      fuente_titulo(renderizador, ruta_fuente, TAMANO_FUENTE_TITULO),
      fuente_opciones(renderizador, ruta_fuente, TAMANO_FUENTE_OPCIONES),
      fuente_botones(renderizador, ruta_fuente, TAMANO_FUENTE_BOTONES),
      fuente_pequena(renderizador, ruta_fuente, TAMANO_FUENTE_PEQUENA),
      textura_marco(RUTA_ASSET_MARCO, renderizador),
      textura_boton(RUTA_ASSET_BOTON, renderizador),
      textura_cursores(RUTA_ASSET_CURSORES, renderizador),
      textura_pollito(RUTA_ASSET_POLLITO, renderizador),
      textura_cerrar(RUTA_ASSET_BOTON_CERRAR, renderizador),
      textura_leche(RUTA_ASSET_LECHE, renderizador),
      textura_huevo(RUTA_ASSET_HUEVO, renderizador),
      menu_abierto(false),
      felicitacion_visible(false),
      despierto(true),
      cartel_confirmacion_activo(false),
      accion_pendiente_confirmacion(AccionMenu::NINGUNA),
      pestania_activa(PestaniaMenu::ANIMALES),
      deslizador_activo(DeslizadorArrastrado::NINGUNO),
      es_multijugador(false),
      codigo_partida(""),
      cursor_coordenada_x(0),
      cursor_coordenada_y(0),
      rectangulo_ventana{0, 0, 0, 0},
      rectangulo_boton_cerrar{0, 0, 0, 0},
      rectangulo_pestania_animales{0, 0, 0, 0},
      rectangulo_pestania_centro_comunitario{0, 0, 0, 0},
      rectangulo_pestania_configuracion{0, 0, 0, 0},
      rectangulo_pestania_salir{0, 0, 0, 0},
      rectangulo_cartel_confirmacion{0, 0, 0, 0},
      rectangulo_boton_ok_salida{0, 0, 0, 0},
      rectangulo_boton_cancelar_salida{0, 0, 0, 0},
      rectangulo_barra_musica{0, 0, 0, 0},
      rectangulo_barra_efectos{0, 0, 0, 0},
      rectangulo_boton_multijugador{0, 0, 0, 0},
      rectangulo_boton_volver_lobby{0, 0, 0, 0},
      rectangulo_boton_salir_juego{0, 0, 0, 0},
      rectangulo_cartel_felicitacion{0, 0, 0, 0},
      rectangulo_boton_seguir_jugando{0, 0, 0, 0},
      rectangulo_boton_lobby_felicitacion{0, 0, 0, 0} {
    recalcular_posiciones(ancho_pantalla, alto_pantalla);
}

bool MenuEnJuego::esta_abierto() const {
    return menu_abierto || felicitacion_visible;
}

void MenuEnJuego::mostrar_felicitacion() {
    menu_abierto = false;
    deslizador_activo = DeslizadorArrastrado::NINGUNO;
    cartel_confirmacion_activo = false;
    accion_pendiente_confirmacion = AccionMenu::NINGUNA;
    felicitacion_visible = true;
}

void MenuEnJuego::abrir() {
    menu_abierto = true;
    deslizador_activo = DeslizadorArrastrado::NINGUNO;
    pestania_activa = PestaniaMenu::ANIMALES;
    cartel_confirmacion_activo = false;
    accion_pendiente_confirmacion = AccionMenu::NINGUNA;
}

void MenuEnJuego::cerrar() {
    menu_abierto = false;
    felicitacion_visible = false;
    deslizador_activo = DeslizadorArrastrado::NINGUNO;
    cartel_confirmacion_activo = false;
    accion_pendiente_confirmacion = AccionMenu::NINGUNA;
}

void MenuEnJuego::configurar_despierto(bool estado_despierto) {
    despierto = estado_despierto;
}

bool MenuEnJuego::cancelar_confirmacion_si_esta_activa() {
    if (cartel_confirmacion_activo) {
        cartel_confirmacion_activo = false;
        accion_pendiente_confirmacion = AccionMenu::NINGUNA;
        return true;
    }
    return false;
}

void MenuEnJuego::alternar() {
    menu_abierto ? cerrar() : abrir();
}

void MenuEnJuego::configurar_dimensiones_pantalla(int ancho_pantalla, int alto_pantalla) {
    recalcular_posiciones(ancho_pantalla, alto_pantalla);
}

void MenuEnJuego::marcar_multijugador(const std::string& codigo) {
    es_multijugador = true;
    codigo_partida = codigo;
}

void MenuEnJuego::recalcular_posiciones(int ancho_pantalla, int alto_pantalla) {
    rectangulo_ventana.w = ANCHO_VENTANA;
    rectangulo_ventana.h = ALTO_VENTANA;
    rectangulo_ventana.x = (ancho_pantalla - rectangulo_ventana.w) / 2;
    rectangulo_ventana.y = (alto_pantalla - rectangulo_ventana.h) / 2 + 25;

    rectangulo_boton_cerrar.w = LADO_BOTON_CERRAR;
    rectangulo_boton_cerrar.h = LADO_BOTON_CERRAR;
    rectangulo_boton_cerrar.x =
        rectangulo_ventana.x + rectangulo_ventana.w + DESPLAZAMIENTO_EXTERIOR_BOTON_CERRAR_X;
    rectangulo_boton_cerrar.y = rectangulo_ventana.y - DESPLAZAMIENTO_EXTERIOR_BOTON_CERRAR_Y;

    int origen_pestanias_x = rectangulo_ventana.x + MARGEN_IZQUIERDO_PESTANIAS;
    int paso_pestania = TAMANO_PESTANIA + SEPARACION_PESTANIAS;

    rectangulo_pestania_animales.w = TAMANO_PESTANIA;
    rectangulo_pestania_animales.h = TAMANO_PESTANIA + 12;
    rectangulo_pestania_animales.x = origen_pestanias_x;
    rectangulo_pestania_animales.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_ACTIVA_Y;

    rectangulo_pestania_centro_comunitario.w = TAMANO_PESTANIA;
    rectangulo_pestania_centro_comunitario.h = TAMANO_PESTANIA + 12;
    rectangulo_pestania_centro_comunitario.x = origen_pestanias_x + paso_pestania;
    rectangulo_pestania_centro_comunitario.y =
        rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_ACTIVA_Y;

    rectangulo_pestania_configuracion.w = TAMANO_PESTANIA;
    rectangulo_pestania_configuracion.h = TAMANO_PESTANIA + 12;
    rectangulo_pestania_configuracion.x = origen_pestanias_x + (paso_pestania * 2);
    rectangulo_pestania_configuracion.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_ACTIVA_Y;

    rectangulo_pestania_salir.w = TAMANO_PESTANIA;
    rectangulo_pestania_salir.h = TAMANO_PESTANIA + 12;
    rectangulo_pestania_salir.x = origen_pestanias_x + (paso_pestania * 3);
    rectangulo_pestania_salir.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_ACTIVA_Y;

    rectangulo_cartel_confirmacion.w = ANCHO_CARTEL_CONFIRMACION;
    rectangulo_cartel_confirmacion.h = ALTO_CARTEL_CONFIRMACION;
    rectangulo_cartel_confirmacion.x =
        rectangulo_ventana.x + (rectangulo_ventana.w - ANCHO_CARTEL_CONFIRMACION) / 2;
    rectangulo_cartel_confirmacion.y =
        rectangulo_ventana.y + (rectangulo_ventana.h - ALTO_CARTEL_CONFIRMACION) / 2;

    int centro_cartel_x = rectangulo_cartel_confirmacion.x + (rectangulo_cartel_confirmacion.w / 2);
    int botones_posicion_y = rectangulo_cartel_confirmacion.y + 120;

    rectangulo_boton_ok_salida.w = LADO_BOTON_DIALOGO;
    rectangulo_boton_ok_salida.h = LADO_BOTON_DIALOGO;
    rectangulo_boton_ok_salida.x = centro_cartel_x - LADO_BOTON_DIALOGO - 30;
    rectangulo_boton_ok_salida.y = botones_posicion_y;

    rectangulo_boton_cancelar_salida.w = LADO_BOTON_DIALOGO;
    rectangulo_boton_cancelar_salida.h = LADO_BOTON_DIALOGO;
    rectangulo_boton_cancelar_salida.x = centro_cartel_x + 30;
    rectangulo_boton_cancelar_salida.y = botones_posicion_y;

    rectangulo_barra_musica.w = ANCHO_BARRA_VOLUMEN;
    rectangulo_barra_musica.h = ALTO_BARRA_VOLUMEN;
    rectangulo_barra_musica.x = rectangulo_ventana.x + 180;
    rectangulo_barra_musica.y = rectangulo_ventana.y + 118;

    rectangulo_barra_efectos.w = ANCHO_BARRA_VOLUMEN;
    rectangulo_barra_efectos.h = ALTO_BARRA_VOLUMEN;
    rectangulo_barra_efectos.x = rectangulo_ventana.x + 180;
    rectangulo_barra_efectos.y = rectangulo_ventana.y + 183;

    rectangulo_boton_multijugador.w = ANCHO_BOTON_MULTIJUGADOR;
    rectangulo_boton_multijugador.h = ALTO_BOTON_MULTIJUGADOR;
    rectangulo_boton_multijugador.x =
        rectangulo_ventana.x + (rectangulo_ventana.w - ANCHO_BOTON_MULTIJUGADOR) / 2;
    rectangulo_boton_multijugador.y = rectangulo_ventana.y + 262;

    rectangulo_boton_volver_lobby.w = ANCHO_BOTON_SALIDA;
    rectangulo_boton_volver_lobby.h = ALTO_BOTON_SALIDA;
    rectangulo_boton_volver_lobby.x =
        rectangulo_ventana.x + (rectangulo_ventana.w - ANCHO_BOTON_SALIDA) / 2;
    rectangulo_boton_volver_lobby.y = rectangulo_ventana.y + 140;

    rectangulo_boton_salir_juego.w = ANCHO_BOTON_SALIDA;
    rectangulo_boton_salir_juego.h = ALTO_BOTON_SALIDA;
    rectangulo_boton_salir_juego.x = rectangulo_boton_volver_lobby.x;
    rectangulo_boton_salir_juego.y =
        rectangulo_boton_volver_lobby.y + ALTO_BOTON_SALIDA + SEPARACION_VERTICAL_BOTONES_SALIDA;

    rectangulo_cartel_felicitacion = SDL_Rect{
        (ancho_pantalla - ANCHO_CARTEL_FELICITACION) / 2,
        (alto_pantalla - ALTO_CARTEL_FELICITACION) / 2,
        ANCHO_CARTEL_FELICITACION,
        ALTO_CARTEL_FELICITACION};
    const int ancho_de_los_botones = 2 * ANCHO_BOTON_FELICITACION + SEPARACION_BOTONES_FELICITACION;
    const int botones_x =
        rectangulo_cartel_felicitacion.x + (ANCHO_CARTEL_FELICITACION - ancho_de_los_botones) / 2;
    const int botones_y = rectangulo_cartel_felicitacion.y + BOTONES_FELICITACION_Y;
    rectangulo_boton_seguir_jugando =
        SDL_Rect{botones_x, botones_y, ANCHO_BOTON_FELICITACION, ALTO_BOTON_FELICITACION};
    rectangulo_boton_lobby_felicitacion = SDL_Rect{
        botones_x + ANCHO_BOTON_FELICITACION + SEPARACION_BOTONES_FELICITACION,
        botones_y,
        ANCHO_BOTON_FELICITACION,
        ALTO_BOTON_FELICITACION};
}

void MenuEnJuego::actualizar_valor_deslizador(int coordenada_x, DeslizadorArrastrado deslizador) {
    SDL_Rect rectangulo_barra{};
    if (deslizador == DeslizadorArrastrado::MUSICA) {
        rectangulo_barra = rectangulo_barra_musica;
    } else if (deslizador == DeslizadorArrastrado::EFECTOS) {
        rectangulo_barra = rectangulo_barra_efectos;
    } else {
        return;
    }

    int porcentaje = ((coordenada_x - (rectangulo_barra.x + 2)) * 100) /
                     (rectangulo_barra.w - 4 - ANCHO_BOTON_DESLIZADOR);
    if (porcentaje < 0) {
        porcentaje = 0;
    } else if (porcentaje > 100) {
        porcentaje = 100;
    }

    if (deslizador == DeslizadorArrastrado::MUSICA) {
        efectos_de_sonido.fijar_volumen_musica(porcentaje);
    } else if (deslizador == DeslizadorArrastrado::EFECTOS) {
        efectos_de_sonido.fijar_volumen_efectos(porcentaje);
    }
}

bool MenuEnJuego::procesar_clic_en_pestanias(int coordenada_x, int coordenada_y) {
    if (contiene(rectangulo_pestania_animales, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::ANIMALES;
        return true;
    }

    if (contiene(rectangulo_pestania_centro_comunitario, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::CENTRO_COMUNITARIO;
        return true;
    }

    if (contiene(rectangulo_pestania_configuracion, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::CONFIGURACION;
        return true;
    }

    if (contiene(rectangulo_pestania_salir, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::SALIR;
        return true;
    }

    return false;
}

AccionMenu MenuEnJuego::procesar_clic_pestania_configuracion(int coordenada_x, int coordenada_y) {
    if (!es_multijugador &&
        contiene(rectangulo_boton_multijugador, coordenada_x, coordenada_y)) {
        return AccionMenu::ACTIVAR_MULTIJUGADOR;
    }

    if (contiene(rectangulo_barra_musica, coordenada_x, coordenada_y)) {
        deslizador_activo = DeslizadorArrastrado::MUSICA;
        actualizar_valor_deslizador(coordenada_x, DeslizadorArrastrado::MUSICA);
    } else if (contiene(rectangulo_barra_efectos, coordenada_x, coordenada_y)) {
        deslizador_activo = DeslizadorArrastrado::EFECTOS;
        actualizar_valor_deslizador(coordenada_x, DeslizadorArrastrado::EFECTOS);
    }

    return AccionMenu::NINGUNA;
}

AccionMenu MenuEnJuego::procesar_clic_pestania_salir(int coordenada_x, int coordenada_y) {
    if (contiene(rectangulo_boton_volver_lobby, coordenada_x, coordenada_y)) {
        if (despierto) {
            return AccionMenu::VOLVER_AL_LOBBY;
        }
        cartel_confirmacion_activo = true;
        accion_pendiente_confirmacion = AccionMenu::VOLVER_AL_LOBBY;
        return AccionMenu::NINGUNA;
    }

    if (contiene(rectangulo_boton_salir_juego, coordenada_x, coordenada_y)) {
        if (despierto) {
            return AccionMenu::SALIR_DEL_JUEGO;
        }
        cartel_confirmacion_activo = true;
        accion_pendiente_confirmacion = AccionMenu::SALIR_DEL_JUEGO;
        return AccionMenu::NINGUNA;
    }

    return AccionMenu::NINGUNA;
}

AccionMenu MenuEnJuego::procesar_clic_felicitacion(int coordenada_x, int coordenada_y) {
    if (contiene(rectangulo_boton_seguir_jugando, coordenada_x, coordenada_y)) {
        felicitacion_visible = false;
        return AccionMenu::NINGUNA;
    }
    if (contiene(rectangulo_boton_lobby_felicitacion, coordenada_x, coordenada_y)) {
        // La partida se guardó al completarse el centro: no hace falta confirmar.
        felicitacion_visible = false;
        return AccionMenu::VOLVER_AL_LOBBY;
    }
    return AccionMenu::NINGUNA;
}

AccionMenu MenuEnJuego::clic_presionado(int coordenada_x, int coordenada_y) {
    if (felicitacion_visible) {
        return procesar_clic_felicitacion(coordenada_x, coordenada_y);
    }
    if (!menu_abierto) {
        return AccionMenu::NINGUNA;
    }

    if (cartel_confirmacion_activo) {
        if (contiene(rectangulo_boton_ok_salida, coordenada_x, coordenada_y)) {
            cartel_confirmacion_activo = false;
            AccionMenu accion_a_ejecutar = accion_pendiente_confirmacion;
            accion_pendiente_confirmacion = AccionMenu::NINGUNA;
            return accion_a_ejecutar;
        }
        if (contiene(rectangulo_boton_cancelar_salida, coordenada_x, coordenada_y)) {
            cartel_confirmacion_activo = false;
            accion_pendiente_confirmacion = AccionMenu::NINGUNA;
            return AccionMenu::NINGUNA;
        }
        if (!contiene(rectangulo_cartel_confirmacion, coordenada_x, coordenada_y)) {
            cartel_confirmacion_activo = false;
            accion_pendiente_confirmacion = AccionMenu::NINGUNA;
        }
        return AccionMenu::NINGUNA;
    }

    if (contiene(rectangulo_boton_cerrar, coordenada_x, coordenada_y)) {
        cerrar();
        return AccionMenu::NINGUNA;
    }

    if (procesar_clic_en_pestanias(coordenada_x, coordenada_y)) {
        return AccionMenu::NINGUNA;
    }

    if (pestania_activa == PestaniaMenu::CONFIGURACION) {
        return procesar_clic_pestania_configuracion(coordenada_x, coordenada_y);
    }

    if (pestania_activa == PestaniaMenu::SALIR) {
        return procesar_clic_pestania_salir(coordenada_x, coordenada_y);
    }

    return AccionMenu::NINGUNA;
}

void MenuEnJuego::clic_liberado() {
    deslizador_activo = DeslizadorArrastrado::NINGUNO;
}

void MenuEnJuego::raton_movido(int coordenada_x, int coordenada_y) {
    cursor_coordenada_x = coordenada_x;
    cursor_coordenada_y = coordenada_y;

    if (!cartel_confirmacion_activo && deslizador_activo != DeslizadorArrastrado::NINGUNO) {
        actualizar_valor_deslizador(coordenada_x, deslizador_activo);
    }
}

void MenuEnJuego::dibujar_fondo_oscurecido(SDL_Renderer* renderizador) const {
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 140);
    SDL_RenderFillRect(renderizador, nullptr);
}

void MenuEnJuego::dibujar_pestania(
    SDL_Renderer* renderizador, PestaniaMenu tipo, const SDL_Rect& rectangulo,
    const Textura& textura_icono, const SDL_Rect& sprite_origen) const {
    bool es_activa = (pestania_activa == tipo);

    SDL_Rect rectangulo_destino{rectangulo.x, 0, TAMANO_PESTANIA, TAMANO_PESTANIA};
    if (es_activa) {
        rectangulo_destino.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_ACTIVA_Y;
    } else {
        rectangulo_destino.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_INACTIVA_Y;
        SDL_SetTextureColorMod(textura_icono.ptr(), 160, 140, 120);
    }

    if (tipo == PestaniaMenu::ANIMALES) {
        if (!es_activa) {
            SDL_SetTextureColorMod(textura_cursores.ptr(), 160, 140, 120);
        }
        SDL_RenderCopy(
            renderizador,
            textura_cursores.ptr(),
            &ORIGEN_SPRITE_PESTANIA_EN_BLANCO,
            &rectangulo_destino);
        SDL_SetTextureColorMod(textura_cursores.ptr(), 255, 255, 255);
    }

    SDL_RenderCopy(renderizador, textura_icono.ptr(), &sprite_origen, &rectangulo_destino);

    SDL_SetTextureColorMod(textura_icono.ptr(), 255, 255, 255);
}

void MenuEnJuego::dibujar_pestanias(SDL_Renderer* renderizador) const {
    dibujar_pestania(
        renderizador,
        PestaniaMenu::ANIMALES,
        rectangulo_pestania_animales,
        textura_pollito,
        ORIGEN_SPRITE_POLLITO);

    dibujar_pestania(
        renderizador,
        PestaniaMenu::CENTRO_COMUNITARIO,
        rectangulo_pestania_centro_comunitario,
        textura_cursores,
        ORIGEN_SPRITE_BOLSA);

    dibujar_pestania(
        renderizador,
        PestaniaMenu::CONFIGURACION,
        rectangulo_pestania_configuracion,
        textura_cursores,
        ORIGEN_SPRITE_JOYSTICK);

    dibujar_pestania(
        renderizador,
        PestaniaMenu::SALIR,
        rectangulo_pestania_salir,
        textura_cursores,
        ORIGEN_SPRITE_PROHIBIDO);
}

void MenuEnJuego::dibujar_boton_cerrar(SDL_Renderer* renderizador) const {
    SDL_RenderCopy(renderizador, textura_cerrar.ptr(), nullptr, &rectangulo_boton_cerrar);
}

void MenuEnJuego::dibujar_barra_volumen(
    SDL_Renderer* renderizador, const std::string& etiqueta, const SDL_Rect& rectangulo_barra,
    int porcentaje_volumen) const {
    fuente_opciones.dibujar_con_sombra(
        renderizador,
        etiqueta,
        rectangulo_ventana.x + 60,
        rectangulo_barra.y - 2,
        2);

    // Fondo oscuro de la barra
    SDL_SetRenderDrawColor(renderizador, 45, 30, 20, 255);
    SDL_RenderFillRect(renderizador, &rectangulo_barra);

    // Borde exterior dorado/madera de la barra
    SDL_SetRenderDrawColor(renderizador, 139, 90, 43, 255);
    SDL_RenderDrawRect(renderizador, &rectangulo_barra);

    // Relleno de volumen (color caramelo neutral constante)
    int ancho_relleno = ((rectangulo_barra.w - 4) * porcentaje_volumen) / 100;
    if (ancho_relleno > 0) {
        SDL_Rect rectangulo_progreso{
            rectangulo_barra.x + 2,
            rectangulo_barra.y + 2,
            ancho_relleno,
            rectangulo_barra.h - 4};
        SDL_SetRenderDrawColor(renderizador, 218, 154, 62, 255);
        SDL_RenderFillRect(renderizador, &rectangulo_progreso);
    }

    // Perilla deslizadora marfil con contorno marrón
    int perilla_x =
        rectangulo_barra.x + 2 +
        (((rectangulo_barra.w - 4 - ANCHO_BOTON_DESLIZADOR) * porcentaje_volumen) / 100);
    int perilla_y = rectangulo_barra.y - 4;
    SDL_Rect rectangulo_perilla{
        perilla_x,
        perilla_y,
        ANCHO_BOTON_DESLIZADOR,
        rectangulo_barra.h + 8};

    SDL_SetRenderDrawColor(renderizador, 235, 175, 100, 255);
    SDL_RenderFillRect(renderizador, &rectangulo_perilla);

    SDL_SetRenderDrawColor(renderizador, 90, 45, 20, 255);
    SDL_RenderDrawRect(renderizador, &rectangulo_perilla);

    SDL_Rect rectangulo_brillo_perilla{
        perilla_x + 2,
        perilla_y + 2,
        ANCHO_BOTON_DESLIZADOR - 4,
        rectangulo_barra.h + 4};
    SDL_SetRenderDrawColor(renderizador, 255, 210, 145, 255);
    SDL_RenderDrawRect(renderizador, &rectangulo_brillo_perilla);

    std::string porcentaje_texto = std::to_string(porcentaje_volumen) + "%";
    fuente_opciones.dibujar_con_sombra(
        renderizador,
        porcentaje_texto,
        rectangulo_ventana.x + 500,
        rectangulo_barra.y - 2,
        2);
}

void MenuEnJuego::dibujar_boton_con_asset(
    SDL_Renderer* renderizador, const SDL_Rect& rectangulo, const std::string& etiqueta) const {
    bool sobre_el_boton = contiene(rectangulo, cursor_coordenada_x, cursor_coordenada_y);

    if (sobre_el_boton) {
        SDL_SetTextureColorMod(textura_boton.ptr(), 255, 235, 210);
    } else {
        SDL_SetTextureColorMod(textura_boton.ptr(), 255, 255, 255);
    }

    dibujar_nueve_rebanadas(renderizador, textura_boton, rectangulo, 16, 16);
    SDL_SetTextureColorMod(textura_boton.ptr(), 255, 255, 255);

    int texto_coordenada_x = rectangulo.x + (rectangulo.w - fuente_botones.ancho(etiqueta)) / 2;
    int texto_coordenada_y = rectangulo.y + (rectangulo.h - fuente_botones.alto_de_renglon()) / 2;
    fuente_botones
        .dibujar_con_sombra(renderizador, etiqueta, texto_coordenada_x, texto_coordenada_y, 2);
}

void MenuEnJuego::dibujar_contenido_configuracion(SDL_Renderer* renderizador) const {
    std::string titulo = "Opciones y Sonido";
    int titulo_x = rectangulo_ventana.x + (rectangulo_ventana.w - fuente_titulo.ancho(titulo)) / 2;
    fuente_titulo.dibujar_con_sombra(renderizador, titulo, titulo_x, rectangulo_ventana.y + 40, 2);

    dibujar_barra_volumen(
        renderizador,
        "Musica",
        rectangulo_barra_musica,
        efectos_de_sonido.obtener_volumen_musica());

    dibujar_barra_volumen(
        renderizador,
        "Efectos",
        rectangulo_barra_efectos,
        efectos_de_sonido.obtener_volumen_efectos());

    // Separador decorativo
    SDL_SetRenderDrawColor(renderizador, 139, 90, 43, 255);
    SDL_RenderDrawLine(
        renderizador,
        rectangulo_ventana.x + 50,
        rectangulo_ventana.y + 235,
        rectangulo_ventana.x + rectangulo_ventana.w - 50,
        rectangulo_ventana.y + 235);

    if (!es_multijugador) {
        dibujar_boton_con_asset(renderizador, rectangulo_boton_multijugador, "Multijugador");

        std::string aclaracion = "Convierte la partida actual a multijugador";
        int aclaracion_x =
            rectangulo_ventana.x + (rectangulo_ventana.w - fuente_pequena.ancho(aclaracion)) / 2;
        fuente_pequena.dibujar_con_sombra(
            renderizador,
            aclaracion,
            aclaracion_x,
            rectangulo_ventana.y + 328,
            1);
    } else {
        std::string titulo_multi = "Partida Multijugador Activa";
        int titulo_multi_x =
            rectangulo_ventana.x + (rectangulo_ventana.w - fuente_botones.ancho(titulo_multi)) / 2;
        fuente_botones.dibujar_con_sombra(
            renderizador,
            titulo_multi,
            titulo_multi_x,
            rectangulo_ventana.y + 265,
            2);

        std::string texto_codigo = "Codigo de invitacion: " + codigo_partida;
        int codigo_x =
            rectangulo_ventana.x + (rectangulo_ventana.w - fuente_titulo.ancho(texto_codigo)) / 2;
        fuente_titulo.dibujar_con_sombra(
            renderizador,
            texto_codigo,
            codigo_x,
            rectangulo_ventana.y + 310,
            2);
    }
}

void MenuEnJuego::dibujar_contenido_animales(
    SDL_Renderer* renderizador, const DatosAnimales& datos) const {
    std::string titulo = "Animales de la Granja";
    int titulo_x = rectangulo_ventana.x + (rectangulo_ventana.w - fuente_titulo.ancho(titulo)) / 2;
    fuente_titulo.dibujar_con_sombra(renderizador, titulo, titulo_x, rectangulo_ventana.y + 36, 2);

    // Separador decorativo
    SDL_SetRenderDrawColor(renderizador, 139, 90, 43, 255);
    SDL_RenderDrawLine(
        renderizador,
        rectangulo_ventana.x + 40,
        rectangulo_ventana.y + 72,
        rectangulo_ventana.x + rectangulo_ventana.w - 40,
        rectangulo_ventana.y + 72);

    int ancho_tarjeta = rectangulo_ventana.w - 70;
    int alto_tarjeta = 135;
    int tarjeta_x = rectangulo_ventana.x + 35;
    int tarjeta_vacas_y = rectangulo_ventana.y + 85;

    // Tarjeta Vacas
    SDL_Rect rectangulo_tarjeta_vacas{tarjeta_x, tarjeta_vacas_y, ancho_tarjeta, alto_tarjeta};
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, 242, 215, 168, 180);
    SDL_RenderFillRect(renderizador, &rectangulo_tarjeta_vacas);
    SDL_SetRenderDrawColor(renderizador, 160, 110, 60, 255);
    SDL_RenderDrawRect(renderizador, &rectangulo_tarjeta_vacas);

    // Sprite Vaca (17x14 escalado a 51x42)
    SDL_Rect sprite_vaca_destino{tarjeta_x + 22, tarjeta_vacas_y + (alto_tarjeta - 42) / 2, 51, 42};
    SDL_RenderCopy(renderizador, textura_cursores.ptr(), &ORIGEN_SPRITE_VACA, &sprite_vaca_destino);

    // Información de Vacas
    fuente_opciones
        .dibujar_con_sombra(renderizador, "Vacas", tarjeta_x + 95, tarjeta_vacas_y + 16, 2);

    std::string texto_total_vacas =
        "Total en la granja: " + std::to_string(datos.cantidad_de_vacas) + " (limite: 5)";
    fuente_botones.dibujar_con_sombra(
        renderizador,
        texto_total_vacas,
        tarjeta_x + 95,
        tarjeta_vacas_y + 48,
        1);

    SDL_Rect rectangulo_icono_leche{tarjeta_x + 95, tarjeta_vacas_y + 80, 32, 32};
    SDL_RenderCopy(renderizador, textura_leche.ptr(), nullptr, &rectangulo_icono_leche);

    std::string texto_ordenie =
        "Listas para ordeniar: " + std::to_string(datos.vacas_para_ordeniar) + " de " +
        std::to_string(datos.cantidad_de_vacas);
    fuente_botones
        .dibujar_con_sombra(renderizador, texto_ordenie, tarjeta_x + 138, tarjeta_vacas_y + 86, 1);

    // Tarjeta Gallinas
    int tarjeta_gallinas_y = tarjeta_vacas_y + alto_tarjeta + 15;
    SDL_Rect rectangulo_tarjeta_gallinas{
        tarjeta_x,
        tarjeta_gallinas_y,
        ancho_tarjeta,
        alto_tarjeta};
    SDL_SetRenderDrawColor(renderizador, 242, 215, 168, 180);
    SDL_RenderFillRect(renderizador, &rectangulo_tarjeta_gallinas);
    SDL_SetRenderDrawColor(renderizador, 160, 110, 60, 255);
    SDL_RenderDrawRect(renderizador, &rectangulo_tarjeta_gallinas);

    // Sprite Gallinas (26x16 escalado a 78x48)
    SDL_Rect sprite_gallinas_destino{
        tarjeta_x + 10,
        tarjeta_gallinas_y + (alto_tarjeta - 48) / 2,
        78,
        48};
    SDL_RenderCopy(
        renderizador,
        textura_cursores.ptr(),
        &ORIGEN_SPRITE_GALLINAS,
        &sprite_gallinas_destino);

    // Información de Gallinas
    fuente_opciones
        .dibujar_con_sombra(renderizador, "Gallinas", tarjeta_x + 100, tarjeta_gallinas_y + 16, 2);

    std::string texto_total_gallinas =
        "Total en la granja: " + std::to_string(datos.cantidad_de_gallinas) + " (limite: 10)";
    fuente_botones.dibujar_con_sombra(
        renderizador,
        texto_total_gallinas,
        tarjeta_x + 100,
        tarjeta_gallinas_y + 48,
        1);

    SDL_Rect rectangulo_icono_huevo{tarjeta_x + 100, tarjeta_gallinas_y + 80, 32, 32};
    SDL_RenderCopy(renderizador, textura_huevo.ptr(), nullptr, &rectangulo_icono_huevo);

    std::string texto_huevos =
        "Huevos para recolectar: " + std::to_string(datos.huevos_para_recolectar);
    fuente_botones.dibujar_con_sombra(
        renderizador,
        texto_huevos,
        tarjeta_x + 142,
        tarjeta_gallinas_y + 86,
        1);
}

void MenuEnJuego::dibujar_contenido_centro_comunitario(
    SDL_Renderer* renderizador, const std::map<TipoLote, LoteDelCentro>& lotes) const {
    std::string titulo = "Centro Comunitario";
    int titulo_x = rectangulo_ventana.x + (rectangulo_ventana.w - fuente_titulo.ancho(titulo)) / 2;
    fuente_titulo.dibujar_con_sombra(renderizador, titulo, titulo_x, rectangulo_ventana.y + 40, 2);

    // Separador decorativo
    SDL_SetRenderDrawColor(renderizador, 139, 90, 43, 255);
    SDL_RenderDrawLine(
        renderizador,
        rectangulo_ventana.x + 40,
        rectangulo_ventana.y + 75,
        rectangulo_ventana.x + rectangulo_ventana.w - 40,
        rectangulo_ventana.y + 75);

    if (lotes.empty()) {
        const std::string aviso = TEXTO_SIN_DATOS_DEL_CENTRO;
        fuente_botones.dibujar_con_sombra(
            renderizador,
            aviso,
            rectangulo_ventana.x + (rectangulo_ventana.w - fuente_botones.ancho(aviso)) / 2,
            rectangulo_ventana.y + PRIMERA_FILA_DE_LOTES_Y,
            1);
        return;
    }
    int fila_y = rectangulo_ventana.y + PRIMERA_FILA_DE_LOTES_Y;
    for (const auto& [tipo, lote] : lotes) {
        dibujar_lote(renderizador, lote, fila_y);
        fila_y += ALTO_DE_FILA_DE_LOTE;
    }
}

void MenuEnJuego::dibujar_lote(
    SDL_Renderer* renderizador, const LoteDelCentro& lote, int fila_y) const {
    const SDL_Rect tarjeta{
        rectangulo_ventana.x + MARGEN_DE_TARJETA_DE_LOTE,
        fila_y,
        rectangulo_ventana.w - 2 * MARGEN_DE_TARJETA_DE_LOTE,
        ALTO_DE_TARJETA_DE_LOTE};
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, 242, 215, 168, 180);
    SDL_RenderFillRect(renderizador, &tarjeta);
    SDL_SetRenderDrawColor(renderizador, 160, 110, 60, 255);
    SDL_RenderDrawRect(renderizador, &tarjeta);

    const int margen_del_icono = (ALTO_DE_TARJETA_DE_LOTE - LADO_DEL_ICONO_DE_LOTE) / 2;
    const SDL_Rect icono{
        tarjeta.x + margen_del_icono,
        tarjeta.y + margen_del_icono,
        LADO_DEL_ICONO_DE_LOTE,
        LADO_DEL_ICONO_DE_LOTE};
    iconos_de_lotes.dibujar(renderizador, lote.lote, icono);

    const int texto_y = tarjeta.y + (tarjeta.h - fuente_botones.alto_de_renglon()) / 2;
    fuente_botones
        .dibujar_con_sombra(renderizador, lote.nombre, tarjeta.x + NOMBRE_DE_LOTE_X, texto_y, 1);

    const int estado_x = tarjeta.x + BARRA_DE_AVANCE_X;
    if (lote.completo) {
        const SDL_Rect tilde{
            estado_x,
            tarjeta.y + (tarjeta.h - ALTO_DE_LA_TILDE * ESCALA_DE_LA_TILDE) / 2,
            ANCHO_DE_LA_TILDE * ESCALA_DE_LA_TILDE,
            ALTO_DE_LA_TILDE * ESCALA_DE_LA_TILDE};
        iconos_de_lotes.dibujar_tilde(renderizador, tilde);
        fuente_botones.dibujar_con_sombra(
            renderizador,
            TEXTO_DE_LOTE_COMPLETO,
            tilde.x + tilde.w + SEPARACION_TEXTO_DE_AVANCE,
            texto_y,
            1);
        return;
    }

    uint32_t aportado = 0;
    uint32_t pedido = 0;
    for (const RequisitoDeLote& requisito : lote.requisitos) {
        aportado += std::min(requisito.aportado, requisito.cantidad);
        pedido += requisito.cantidad;
    }
    const SDL_Rect barra{
        estado_x,
        tarjeta.y + (tarjeta.h - ALTO_DE_BARRA_DE_AVANCE) / 2,
        ANCHO_DE_BARRA_DE_AVANCE,
        ALTO_DE_BARRA_DE_AVANCE};
    dibujar_barra_de_avance(renderizador, barra, aportado, pedido);
    fuente_pequena.dibujar_con_sombra(
        renderizador,
        std::to_string(aportado) + " de " + std::to_string(pedido),
        barra.x + barra.w + SEPARACION_TEXTO_DE_AVANCE,
        tarjeta.y + (tarjeta.h - fuente_pequena.alto_de_renglon()) / 2,
        1);
}

void MenuEnJuego::dibujar_barra_de_avance(
    SDL_Renderer* renderizador, const SDL_Rect& rectangulo_barra, uint32_t aportado,
    uint32_t pedido) const {
    // Mismos colores que la barra de volumen.
    SDL_SetRenderDrawColor(renderizador, 45, 30, 20, 255);
    SDL_RenderFillRect(renderizador, &rectangulo_barra);
    SDL_SetRenderDrawColor(renderizador, 139, 90, 43, 255);
    SDL_RenderDrawRect(renderizador, &rectangulo_barra);
    if (pedido == 0 || aportado == 0) {
        return;
    }
    const int ancho_relleno =
        static_cast<int>(static_cast<uint64_t>(rectangulo_barra.w - 4) * aportado / pedido);
    const SDL_Rect relleno{
        rectangulo_barra.x + 2,
        rectangulo_barra.y + 2,
        ancho_relleno,
        rectangulo_barra.h - 4};
    SDL_SetRenderDrawColor(renderizador, 218, 154, 62, 255);
    SDL_RenderFillRect(renderizador, &relleno);
}

void MenuEnJuego::dibujar_felicitacion(SDL_Renderer* renderizador) const {
    dibujar_nueve_rebanadas(
        renderizador,
        textura_marco,
        rectangulo_cartel_felicitacion,
        BORDE_CORTE_ASSET_MARCO,
        BORDE_EN_PANTALLA_MARCO);

    const std::string titulo = TITULO_FELICITACION;
    fuente_titulo.dibujar_con_sombra(
        renderizador,
        titulo,
        rectangulo_cartel_felicitacion.x +
            (rectangulo_cartel_felicitacion.w - fuente_titulo.ancho(titulo)) / 2,
        rectangulo_cartel_felicitacion.y + TITULO_FELICITACION_Y,
        2);

    const std::string texto = TEXTO_FELICITACION;
    fuente_opciones.dibujar_con_sombra(
        renderizador,
        texto,
        rectangulo_cartel_felicitacion.x +
            (rectangulo_cartel_felicitacion.w - fuente_opciones.ancho(texto)) / 2,
        rectangulo_cartel_felicitacion.y + TEXTO_FELICITACION_Y,
        2);

    dibujar_boton_con_asset(renderizador, rectangulo_boton_seguir_jugando, BOTON_SEGUIR_JUGANDO);
    dibujar_boton_con_asset(
        renderizador,
        rectangulo_boton_lobby_felicitacion,
        BOTON_VOLVER_AL_LOBBY);
}

void MenuEnJuego::dibujar_contenido_salir(SDL_Renderer* renderizador) const {
    std::string titulo = "Salir de la Partida";
    int titulo_x = rectangulo_ventana.x + (rectangulo_ventana.w - fuente_titulo.ancho(titulo)) / 2;
    fuente_titulo.dibujar_con_sombra(renderizador, titulo, titulo_x, rectangulo_ventana.y + 40, 2);

    dibujar_boton_con_asset(renderizador, rectangulo_boton_volver_lobby, "Volver al Lobby");

    dibujar_boton_con_asset(renderizador, rectangulo_boton_salir_juego, "Salir del Juego");
}

void MenuEnJuego::dibujar_cartel_confirmacion(SDL_Renderer* renderizador) const {
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 110);
    SDL_RenderFillRect(renderizador, &rectangulo_ventana);

    dibujar_nueve_rebanadas(
        renderizador,
        textura_marco,
        rectangulo_cartel_confirmacion,
        BORDE_CORTE_ASSET_MARCO,
        BORDE_EN_PANTALLA_MARCO);

    std::string titulo = "Estas seguro de salir?";
    int titulo_coordenada_x =
        rectangulo_cartel_confirmacion.x +
        (rectangulo_cartel_confirmacion.w - fuente_opciones.ancho(titulo)) / 2;
    int titulo_coordenada_y = rectangulo_cartel_confirmacion.y + 35;
    fuente_opciones
        .dibujar_con_sombra(renderizador, titulo, titulo_coordenada_x, titulo_coordenada_y, 2);

    std::string subtitulo = "El progreso no guardado de hoy se perdera.";
    int subtitulo_coordenada_x =
        rectangulo_cartel_confirmacion.x +
        (rectangulo_cartel_confirmacion.w - fuente_pequena.ancho(subtitulo)) / 2;
    int subtitulo_coordenada_y = rectangulo_cartel_confirmacion.y + 75;
    fuente_pequena.dibujar_con_sombra(
        renderizador,
        subtitulo,
        subtitulo_coordenada_x,
        subtitulo_coordenada_y,
        1);

    bool sobre_boton_ok =
        contiene(rectangulo_boton_ok_salida, cursor_coordenada_x, cursor_coordenada_y);
    if (sobre_boton_ok) {
        SDL_SetTextureColorMod(textura_cursores.ptr(), 255, 235, 200);
    } else {
        SDL_SetTextureColorMod(textura_cursores.ptr(), 255, 255, 255);
    }
    SDL_RenderCopy(
        renderizador,
        textura_cursores.ptr(),
        &ORIGEN_SPRITE_BOTON_OK,
        &rectangulo_boton_ok_salida);

    bool sobre_boton_cancelar =
        contiene(rectangulo_boton_cancelar_salida, cursor_coordenada_x, cursor_coordenada_y);
    if (sobre_boton_cancelar) {
        SDL_SetTextureColorMod(textura_cursores.ptr(), 255, 235, 200);
    } else {
        SDL_SetTextureColorMod(textura_cursores.ptr(), 255, 255, 255);
    }
    SDL_RenderCopy(
        renderizador,
        textura_cursores.ptr(),
        &ORIGEN_SPRITE_BOTON_CANCELAR,
        &rectangulo_boton_cancelar_salida);

    SDL_SetTextureColorMod(textura_cursores.ptr(), 255, 255, 255);
}

void MenuEnJuego::dibujar(
    SDL_Renderer* renderizador, const DatosAnimales& datos_animales,
    const std::map<TipoLote, LoteDelCentro>& lotes) const {
    if (felicitacion_visible) {
        dibujar_fondo_oscurecido(renderizador);
        dibujar_felicitacion(renderizador);
        return;
    }
    if (!menu_abierto) {
        return;
    }

    dibujar_fondo_oscurecido(renderizador);
    dibujar_pestanias(renderizador);
    dibujar_nueve_rebanadas(
        renderizador,
        textura_marco,
        rectangulo_ventana,
        BORDE_CORTE_ASSET_MARCO,
        BORDE_EN_PANTALLA_MARCO);
    dibujar_boton_cerrar(renderizador);

    if (pestania_activa == PestaniaMenu::ANIMALES) {
        dibujar_contenido_animales(renderizador, datos_animales);
    } else if (pestania_activa == PestaniaMenu::CENTRO_COMUNITARIO) {
        dibujar_contenido_centro_comunitario(renderizador, lotes);
    } else if (pestania_activa == PestaniaMenu::CONFIGURACION) {
        dibujar_contenido_configuracion(renderizador);
    } else if (pestania_activa == PestaniaMenu::SALIR) {
        dibujar_contenido_salir(renderizador);
    }

    if (cartel_confirmacion_activo) {
        dibujar_cartel_confirmacion(renderizador);
    }
}
