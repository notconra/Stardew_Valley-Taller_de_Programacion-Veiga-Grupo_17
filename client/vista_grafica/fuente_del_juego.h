#ifndef FUENTE_DEL_JUEGO_H
#define FUENTE_DEL_JUEGO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <memory>
#include <string>

#include "chat/cache_de_texto.h"

// La letra de Stardew (la misma .ttf del chat, la ruta sale de [chat] ruta_fuente en
// cliente.toml) en un tamaño fijo, con el marrón y la sombra de los menús del juego. Esa fuente
// solo trae los caracteres básicos: sin tildes, ñ, ¿ ni ¡ (esos salen como un cuadrado), así que
// los textos se escriben sin ellos.
//
// Las imágenes de los textos las guarda CacheDeTexto, la misma caché que usa el chat, así dibujar
// el mismo texto en cada cuadro no cuesta nada. Lanza std::runtime_error si no puede abrir la
// fuente.
class FuenteDelJuego {
private:
    std::unique_ptr<TTF_Font, void (*)(TTF_Font*)> fuente;
    mutable CacheDeTexto textos;

public:
    FuenteDelJuego(SDL_Renderer* renderer, const std::string& ruta, int tamano_en_puntos);

    // El texto (en UTF-8) con su esquina de arriba a la izquierda en (x, y).
    void dibujar(
        SDL_Renderer* renderer, const std::string& texto, int x, int y, SDL_Color color) const;

    // Como en el juego: el texto marrón oscuro con una sombra clara abajo a la izquierda.
    // "sombra" es cuántos píxeles se corre la sombra.
    void dibujar_con_sombra(
        SDL_Renderer* renderer, const std::string& texto, int x, int y, int sombra) const;

    int ancho(const std::string& texto) const;
    int alto_de_renglon() const;

    // Dónde cae, desde arriba del renglón, el medio de los números: para centrar un texto en un
    // recuadro.
    int centro_de_las_letras() const;

    FuenteDelJuego(const FuenteDelJuego&) = delete;
    FuenteDelJuego& operator=(const FuenteDelJuego&) = delete;
};

#endif
