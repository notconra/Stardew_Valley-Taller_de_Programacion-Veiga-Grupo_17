#ifndef CARGADOR_DE_MAZMORRA_H
#define CARGADOR_DE_MAZMORRA_H

#include <random>
#include <string>
#include <vector>

#include "../mapa/mapa.h"
#include "nivel.h"

// Nombres de mapa_destino que usan los TMX de la mazmorra para marcar la escalera y la entrada.
inline constexpr const char* DESTINO_ESCALERA = "siguiente";
inline constexpr const char* DESTINO_ENTRADA = "entrada";

class CargadorMazmorra {
public:
    /*
     * Elige 10 mapas distintos de "disponibles" en orden aleatorio.
     */
    static std::vector<std::string> elegir_niveles(
        const std::vector<std::string>& disponibles, std::mt19937& generador);

    /*
     * Arma la geografía de un nivel a partir del Mapa ya cargado: sus colisiones dicen qué
     * casillas se caminan y sus transiciones, dónde están la entrada, la escalera y la salida.
     */
    static GeografiaNivel crear_geografia(const std::string& id_mapa, const Mapa& mapa);

private:
    /*
     * Pasa un rectángulo en píxeles a casillas, recortado al mapa.
     */
    static Zona a_casillas(const Rectangulo& zona, const Mapa& mapa);

    /*
     * Elige una casilla libre pegada a la salida, para los mapas que no marcan la entrada.
     */
    static Casilla asignar_entrada(const GeografiaNivel& geografia);
};

#endif
