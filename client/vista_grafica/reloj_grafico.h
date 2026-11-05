#ifndef RELOJ_GRAFICO_H
#define RELOJ_GRAFICO_H

#include <SDL2/SDL.h>

#include <array>
#include <cstdint>
#include <string>

#include "../../common/mensajes/mensajes.h"

// Muestra la hora que manda el servidor: un recuadro arriba a la derecha con la estación y su día
// ("PRIMAVERA 3"), el año ("AÑO 1") y la hora ("HH:MM"), y un velo oscuro sobre el mapa que se va
// cerrando a la tarde y de noche. No calcula nada: solo dibuja la última HoraDelJuego que llegó.
class RelojGrafico {
private:
    // Cada letra es de 3x5 "píxeles" (como los números de las cantidades del inventario). Cada
    // fila es un número de 3 bits, el bit más alto es la columna de la izquierda.
    using Letra = std::array<uint8_t, 5>;

    const Letra& letra(char caracter) const;

    // Dibuja el texto con su esquina de arriba a la izquierda en (x, y). "pixel" es el tamaño en
    // pantalla de cada píxel de la letra.
    void dibujar_texto(
        SDL_Renderer* renderer, const std::string& texto, int x, int y, int pixel) const;

    // El número con un cero adelante si tiene una sola cifra ("06", "10").
    std::string dos_cifras(int numero) const;

    // El nombre de la estación como se escribe en el reloj, en mayúsculas.
    std::string nombre_de_la_estacion(Estacion estacion) const;

    // Lo que mide el texto en pantalla, de ancho.
    int ancho_del_texto(const std::string& texto, int pixel) const;

    // Minutos de juego desde las 6 am (de 0 a 1199). Las 0 y la 1 son la madrugada del mismo día.
    int minutos_desde_el_inicio(const HoraDelJuego& hora) const;

public:
    // Qué tan oscuro va el velo, de 0 (de día) a 255. Sube de 18 a 20 y sigue subiendo, más
    // despacio, hasta las 2 am.
    uint8_t opacidad_de_la_noche(const HoraDelJuego& hora) const;

    // El velo de la noche sobre toda la ventana. Se llama después de dibujar el mapa y los
    // personajes, y antes del HUD, para que el HUD no quede oscurecido.
    void oscurecer(
        SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana,
        int alto_ventana) const;

    // El recuadro con el día y la hora, arriba a la derecha.
    void dibujar(
        SDL_Renderer* renderer, const HoraDelJuego& hora, int ancho_ventana,
        int alto_ventana) const;
};

#endif
