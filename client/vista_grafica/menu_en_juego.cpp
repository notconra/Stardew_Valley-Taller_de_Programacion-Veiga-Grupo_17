#include "menu_en_juego.h"

#include <string>

// Rutas constantes a los assets de interfaz
static const char RUTA_ASSET_MARCO[] = "Assets/LooseSprites/menu_box.png";
static const char RUTA_ASSET_BOTON[] = "Assets/hud/regalos_boton.png";
static const char RUTA_ASSET_BOTON_CERRAR[] = "Assets/hud/cerrar_regalos.png";
static const char RUTA_ASSET_CURSORES[] = "Assets/LooseSprites/Cursors.png";
static const char RUTA_ASSET_POLLITO[] = "Assets/Farm/Animals/Baby Chicken Yellow.png";

// Recortes exactos de sprites originales en Cursors.png
static constexpr SDL_Rect ORIGEN_SPRITE_MOCHILA{0, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_PESTANIA_EN_BLANCO{16, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_POLLITO{0, 0, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_JOYSTICK{96, 368, 16, 16};
static constexpr SDL_Rect ORIGEN_SPRITE_PROHIBIDO{112, 368, 16, 16};

MenuEnJuego::MenuEnJuego(
    SDL_Renderer* renderizador, const std::string& ruta_fuente,
    EfectosDeSonido& efectos_de_sonido_referencia, int ancho_pantalla, int alto_pantalla)
    : efectos_de_sonido(efectos_de_sonido_referencia),
      fuente_titulo(renderizador, ruta_fuente, TAMANO_FUENTE_TITULO),
      fuente_opciones(renderizador, ruta_fuente, TAMANO_FUENTE_OPCIONES),
      fuente_botones(renderizador, ruta_fuente, TAMANO_FUENTE_BOTONES),
      fuente_pequena(renderizador, ruta_fuente, TAMANO_FUENTE_PEQUENA),
      textura_marco(RUTA_ASSET_MARCO, renderizador),
      textura_boton(RUTA_ASSET_BOTON, renderizador),
      textura_cursores(RUTA_ASSET_CURSORES, renderizador),
      textura_pollito(RUTA_ASSET_POLLITO, renderizador),
      textura_cerrar(RUTA_ASSET_BOTON_CERRAR, renderizador),
      menu_abierto(false),
      pestania_activa(PestaniaMenu::CONFIGURACION),
      deslizador_activo(DeslizadorArrastrado::NINGUNO),
      es_multijugador(false),
      codigo_partida(""),
      cursor_coordenada_x(0),
      cursor_coordenada_y(0),
      rectangulo_ventana{0, 0, 0, 0},
      rectangulo_boton_cerrar{0, 0, 0, 0},
      rectangulo_pestania_inventario{0, 0, 0, 0},
      rectangulo_pestania_animales{0, 0, 0, 0},
      rectangulo_pestania_configuracion{0, 0, 0, 0},
      rectangulo_pestania_salir{0, 0, 0, 0},
      rectangulo_barra_musica{0, 0, 0, 0},
      rectangulo_barra_efectos{0, 0, 0, 0},
      rectangulo_boton_multijugador{0, 0, 0, 0},
      rectangulo_boton_volver_lobby{0, 0, 0, 0},
      rectangulo_boton_salir_juego{0, 0, 0, 0} {
    recalcular_posiciones(ancho_pantalla, alto_pantalla);
}

bool MenuEnJuego::esta_abierto() const {
    return menu_abierto;
}

void MenuEnJuego::abrir() {
    menu_abierto = true;
    deslizador_activo = DeslizadorArrastrado::NINGUNO;
    pestania_activa = PestaniaMenu::CONFIGURACION;
}

void MenuEnJuego::cerrar() {
    menu_abierto = false;
    deslizador_activo = DeslizadorArrastrado::NINGUNO;
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

    rectangulo_pestania_inventario.w = TAMANO_PESTANIA;
    rectangulo_pestania_inventario.h = TAMANO_PESTANIA;
    rectangulo_pestania_inventario.x = origen_pestanias_x;
    rectangulo_pestania_inventario.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_INACTIVA_Y;

    rectangulo_pestania_animales.w = TAMANO_PESTANIA;
    rectangulo_pestania_animales.h = TAMANO_PESTANIA;
    rectangulo_pestania_animales.x = origen_pestanias_x + paso_pestania;
    rectangulo_pestania_animales.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_INACTIVA_Y;

    rectangulo_pestania_configuracion.w = TAMANO_PESTANIA;
    rectangulo_pestania_configuracion.h = TAMANO_PESTANIA;
    rectangulo_pestania_configuracion.x = origen_pestanias_x + (paso_pestania * 2);
    rectangulo_pestania_configuracion.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_ACTIVA_Y;

    rectangulo_pestania_salir.w = TAMANO_PESTANIA;
    rectangulo_pestania_salir.h = TAMANO_PESTANIA;
    rectangulo_pestania_salir.x = origen_pestanias_x + (paso_pestania * 3);
    rectangulo_pestania_salir.y = rectangulo_ventana.y - DESPLAZAMIENTO_PESTANIA_INACTIVA_Y;

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
}

bool MenuEnJuego::esta_adentro(
    const SDL_Rect& rectangulo, int coordenada_x, int coordenada_y) const {
    bool dentro_en_x =
        (coordenada_x >= rectangulo.x) && (coordenada_x < (rectangulo.x + rectangulo.w));
    bool dentro_en_y =
        (coordenada_y >= rectangulo.y) && (coordenada_y < (rectangulo.y + rectangulo.h));
    return dentro_en_x && dentro_en_y;
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
    if (esta_adentro(rectangulo_pestania_configuracion, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::CONFIGURACION;
        return true;
    }

    if (esta_adentro(rectangulo_pestania_salir, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::SALIR;
        return true;
    }

    if (esta_adentro(rectangulo_pestania_inventario, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::INVENTARIO;
        return true;
    }

    if (esta_adentro(rectangulo_pestania_animales, coordenada_x, coordenada_y)) {
        pestania_activa = PestaniaMenu::ANIMALES;
        return true;
    }

    return false;
}

AccionMenu MenuEnJuego::procesar_clic_pestania_configuracion(int coordenada_x, int coordenada_y) {
    if (esta_adentro(rectangulo_barra_musica, coordenada_x, coordenada_y)) {
        deslizador_activo = DeslizadorArrastrado::MUSICA;
        actualizar_valor_deslizador(coordenada_x, DeslizadorArrastrado::MUSICA);
        return AccionMenu::NINGUNA;
    }

    if (esta_adentro(rectangulo_barra_efectos, coordenada_x, coordenada_y)) {
        deslizador_activo = DeslizadorArrastrado::EFECTOS;
        actualizar_valor_deslizador(coordenada_x, DeslizadorArrastrado::EFECTOS);
        return AccionMenu::NINGUNA;
    }

    if (!es_multijugador &&
        esta_adentro(rectangulo_boton_multijugador, coordenada_x, coordenada_y)) {
        return AccionMenu::ACTIVAR_MULTIJUGADOR;
    }

    return AccionMenu::NINGUNA;
}

AccionMenu MenuEnJuego::procesar_clic_pestania_salir(int coordenada_x, int coordenada_y) {
    if (esta_adentro(rectangulo_boton_volver_lobby, coordenada_x, coordenada_y)) {
        return AccionMenu::VOLVER_AL_LOBBY;
    }

    if (esta_adentro(rectangulo_boton_salir_juego, coordenada_x, coordenada_y)) {
        return AccionMenu::SALIR_DEL_JUEGO;
    }

    return AccionMenu::NINGUNA;
}

AccionMenu MenuEnJuego::clic_presionado(int coordenada_x, int coordenada_y) {
    if (!menu_abierto) {
        return AccionMenu::NINGUNA;
    }

    if (esta_adentro(rectangulo_boton_cerrar, coordenada_x, coordenada_y)) {
        cerrar();
        return AccionMenu::CONTINUAR;
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

    if (deslizador_activo != DeslizadorArrastrado::NINGUNO) {
        actualizar_valor_deslizador(coordenada_x, deslizador_activo);
    }
}

void MenuEnJuego::dibujar_fondo_oscurecido(SDL_Renderer* renderizador) const {
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 140);
    SDL_RenderFillRect(renderizador, nullptr);
}

void MenuEnJuego::dibujar_caja_nueve_rebanadas(
    SDL_Renderer* renderizador, const Textura& textura, const SDL_Rect& destino, int borde_corte,
    int borde_pantalla) const {
    int ancho_imagen = 0;
    int alto_imagen = 0;
    SDL_QueryTexture(textura.ptr(), nullptr, nullptr, &ancho_imagen, &alto_imagen);

    int origen_x[4] = {0, borde_corte, ancho_imagen - borde_corte, ancho_imagen};
    int origen_y[4] = {0, borde_corte, alto_imagen - borde_corte, alto_imagen};

    int destino_x[4] = {
        destino.x,
        destino.x + borde_pantalla,
        destino.x + destino.w - borde_pantalla,
        destino.x + destino.w};

    int destino_y[4] = {
        destino.y,
        destino.y + borde_pantalla,
        destino.y + destino.h - borde_pantalla,
        destino.y + destino.h};

    for (int fila = 0; fila < 3; ++fila) {
        for (int columna = 0; columna < 3; ++columna) {
            SDL_Rect origen{
                origen_x[columna],
                origen_y[fila],
                origen_x[columna + 1] - origen_x[columna],
                origen_y[fila + 1] - origen_y[fila]};
            SDL_Rect parte_destino{
                destino_x[columna],
                destino_y[fila],
                destino_x[columna + 1] - destino_x[columna],
                destino_y[fila + 1] - destino_y[fila]};
            if (parte_destino.w > 0 && parte_destino.h > 0) {
                SDL_RenderCopy(renderizador, textura.ptr(), &origen, &parte_destino);
            }
        }
    }
}

void MenuEnJuego::dibujar_pestania(
    SDL_Renderer* renderizador, PestaniaMenu tipo, const SDL_Rect& rectangulo,
    const Textura& textura_icono, const SDL_Rect& sprite_origen) const {
    bool es_activa = (pestania_activa == tipo);

    SDL_Rect rectangulo_destino = rectangulo;
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
        PestaniaMenu::INVENTARIO,
        rectangulo_pestania_inventario,
        textura_cursores,
        ORIGEN_SPRITE_MOCHILA);

    dibujar_pestania(
        renderizador,
        PestaniaMenu::ANIMALES,
        rectangulo_pestania_animales,
        textura_pollito,
        ORIGEN_SPRITE_POLLITO);

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
    bool sobre_el_boton = esta_adentro(rectangulo, cursor_coordenada_x, cursor_coordenada_y);

    if (sobre_el_boton) {
        SDL_SetTextureColorMod(textura_boton.ptr(), 255, 235, 210);
    } else {
        SDL_SetTextureColorMod(textura_boton.ptr(), 255, 255, 255);
    }

    dibujar_caja_nueve_rebanadas(renderizador, textura_boton, rectangulo, 16, 16);
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

void MenuEnJuego::dibujar_contenido_salir(SDL_Renderer* renderizador) const {
    std::string titulo = "Salir de la Partida";
    int titulo_x = rectangulo_ventana.x + (rectangulo_ventana.w - fuente_titulo.ancho(titulo)) / 2;
    fuente_titulo.dibujar_con_sombra(renderizador, titulo, titulo_x, rectangulo_ventana.y + 40, 2);

    dibujar_boton_con_asset(renderizador, rectangulo_boton_volver_lobby, "Volver al Lobby");

    dibujar_boton_con_asset(renderizador, rectangulo_boton_salir_juego, "Salir del Juego");
}

void MenuEnJuego::dibujar(SDL_Renderer* renderizador) {
    if (!menu_abierto) {
        return;
    }

    dibujar_fondo_oscurecido(renderizador);
    dibujar_pestanias(renderizador);
    dibujar_caja_nueve_rebanadas(
        renderizador,
        textura_marco,
        rectangulo_ventana,
        BORDE_CORTE_ASSET_MARCO,
        BORDE_EN_PANTALLA_MARCO);
    dibujar_boton_cerrar(renderizador);

    if (pestania_activa == PestaniaMenu::CONFIGURACION) {
        dibujar_contenido_configuracion(renderizador);
    } else if (pestania_activa == PestaniaMenu::SALIR) {
        dibujar_contenido_salir(renderizador);
    }
}
