#ifndef CAJA_CON_OPCIONES_H
#define CAJA_CON_OPCIONES_H

#include <SDL2/SDL.h>

#include <optional>
#include <string>
#include <vector>

#include "entrada_del_usuario.h"
#include "fuente_del_juego.h"
#include "textura.h"

// La caja de texto de Stardew: abajo en el centro de la pantalla (LooseSprites/textBox.png), unas
// líneas de texto y, si hace falta, una lista de opciones para elegir. La opción elegida se
// marca con un recuadro rojo; se cambia pasando el mouse por encima o con las flechas (si quien la
// usa lo permite) y se confirma con un clic o con Enter.
//
// No sabe qué significa cada opción: devuelve cuál se eligió y quien la usa decide qué hacer. El
// texto se le pasa en cada llamada; lo único que recuerda es qué opción está marcada.
class CajaConOpciones {
public:
    struct Contenido {
        std::vector<std::string> lineas;
        // Vacío si la caja solo muestra texto.
        std::vector<std::string> opciones;
    };

private:
    Textura caja;
    const int escala;
    const FuenteDelJuego fuente;

    size_t elegida = 0;
    // Las opciones de la última vez: si cambian, se vuelve a marcar la primera.
    std::vector<std::string> opciones_anteriores;
    // Dónde estaba el mouse la última vez: solo elige una opción si se movió, así no le gana a las
    // flechas un mouse que quedó quieto encima de otra opción.
    SDL_Point raton_anterior{-1, -1};

    struct Disposicion {
        SDL_Rect caja{};
        int primera_linea_y = 0;
        std::vector<SDL_Rect> opciones;
    };

    Disposicion disponer(const Contenido& contenido, int ancho_ventana, int alto_ventana) const;

    // La caja estirada a ese tamaño sin deformar los bordes: las 4 esquinas quedan iguales, los
    // bordes se estiran a lo largo y el centro en las dos direcciones.
    void dibujar_caja(SDL_Renderer* renderer, const SDL_Rect& destino) const;

    static bool contiene(const SDL_Rect& rectangulo, int x, int y);

public:
    // El tamaño de la caja y de su letra salen del alto de la ventana. "ruta_fuente" es la .ttf.
    // Lanza std::runtime_error si no encuentra la imagen de la caja o la fuente.
    CajaConOpciones(SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana);

    // Mueve la opción marcada (mouse, y flechas si "con_flechas") y devuelve la que se eligió con
    // un clic o con Enter, o nada.
    std::optional<size_t> procesar_entrada(
        const EntradaDelUsuario& entrada, const Contenido& contenido, int ancho_ventana,
        int alto_ventana, bool con_flechas);

    void dibujar(
        SDL_Renderer* renderer, const Contenido& contenido, int ancho_ventana,
        int alto_ventana) const;

    CajaConOpciones(const CajaConOpciones&) = delete;
    CajaConOpciones& operator=(const CajaConOpciones&) = delete;
};

#endif
