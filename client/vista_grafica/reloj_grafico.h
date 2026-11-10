#ifndef RELOJ_GRAFICO_H
#define RELOJ_GRAFICO_H

#include <SDL2/SDL.h>

#include <cstdint>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "fuente_del_juego.h"
#include "textura.h"

// Muestra la hora que manda el servidor con el reloj de Stardew (de LooseSprites/Cursors): arriba
// a la derecha, el día de la semana y el número ("Lun. 3"), el ícono de la estación, y la hora
// ("6:00 am"), con la aguja que va del sol a la luna a lo largo del día. También el velo oscuro
// sobre el mapa que se va cerrando a la tarde y de noche. No calcula nada: solo dibuja la última
// HoraDelJuego que llegó. Los textos van sin tildes porque la fuente no las tiene ("Mie.",
// "Sab."). Lanza std::runtime_error si no encuentra la imagen del reloj o la fuente.
class RelojGrafico {
private:
    Textura cursores;
    // Cuántas veces se agranda la imagen del reloj. Depende del alto de la ventana.
    const int escala;
    const FuenteDelJuego fuente;

    // "Lun.", "Mar.", ... para el día de la semana (0 es lunes).
    std::string abreviatura_del_dia(uint8_t dia_de_la_semana) const;

    // "6:00 am", "12:30 pm".
    std::string hora_en_texto(const HoraDelJuego& hora) const;

    // Minutos de juego desde las 6 am (de 0 a 1200). Las 0 y la 1 son la madrugada del mismo día.
    int minutos_desde_el_inicio(const HoraDelJuego& hora) const;

    // Copia un pedazo de la imagen del reloj, agrandado "escala" veces, con su esquina en (x, y).
    void copiar(SDL_Renderer* renderer, const SDL_Rect& origen, int x, int y) const;

    // El texto centrado en un recuadro del reloj (en píxeles de la imagen, desde su esquina).
    void escribir_centrado(
        SDL_Renderer* renderer, const std::string& texto, const SDL_Rect& recuadro, int x_reloj,
        int y_reloj) const;

    void dibujar_aguja(
        SDL_Renderer* renderer, const HoraDelJuego& hora, int x_reloj, int y_reloj) const;

public:
    // El tamaño del reloj y de su letra salen del alto de la ventana. "ruta_fuente" es la .ttf.
    RelojGrafico(SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana);

    // Qué tan oscuro va el velo, de 0 (de día) a 255. Sube de 18 a 20 y sigue subiendo, más
    // despacio, hasta las 2 am.
    uint8_t opacidad_de_la_noche(const HoraDelJuego& hora) const;

    // El velo de la noche sobre toda la ventana. Se llama después de dibujar el mapa y los
    // personajes, y antes del HUD, para que el HUD no quede oscurecido.
    void oscurecer(
        SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana,
        int alto_ventana) const;

    // El reloj, arriba a la derecha.
    void dibujar(SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana) const;
};

#endif
