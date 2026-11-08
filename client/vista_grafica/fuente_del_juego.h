#ifndef FUENTE_DEL_JUEGO_H
#define FUENTE_DEL_JUEGO_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstdint>
#include <map>
#include <string>
#include <utility>

// La letra de Stardew (Assets/Fonts/Stardew_Valley.ttf, la misma que usa el chat) en un tamaño
// fijo. Esa fuente solo trae los caracteres básicos: sin tildes, ñ, ¿ ni ¡ (esos salen como un
// cuadrado), así que los textos se escriben sin ellos.
//
// Cada texto distinto se convierte una vez en una imagen y se guarda, así dibujar el mismo texto
// en cada cuadro no cuesta nada. Si se acumulan muchos, se descartan todos y se vuelven a crear.
// Lanza std::runtime_error si no puede abrir la fuente.
class FuenteDelJuego {
private:
    TTF_Font* fuente;
    mutable std::map<std::pair<std::string, uint32_t>, SDL_Texture*> textos;

    SDL_Texture* textura_de(
        SDL_Renderer* renderer, const std::string& texto, SDL_Color color) const;
    void olvidar_textos() const;

public:
    explicit FuenteDelJuego(int tamano_en_puntos);
    ~FuenteDelJuego();

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
