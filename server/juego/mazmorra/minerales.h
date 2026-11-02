#ifndef MINERALES_H
#define MINERALES_H

#include <vector>

#include "../objetos_del_mundo/objetos_del_mundo.h"

// Los minerales del juego, expresados como objetos del mundo. No hay una clase por mineral: cada
// uno es una fila de datos en definiciones(). RecursosDeJuego las suma al catálogo genérico junto
// con las definiciones de otros módulos (árboles, muebles, ...).
//
// Cuántos aparecen por nivel lo decide [mazmorra] en la configuración; con qué frecuencia relativa
// aparece cada uno lo decide pesos(), que depende del número de nivel.
class Minerales {
public:
    static std::vector<DefinicionDeObjeto> definiciones();

    // Frecuencia relativa de cada mineral en un nivel. Los pesos no tienen que sumar 100: la
    // probabilidad de cada tipo es su peso sobre el total. El oro recién aparece a partir de
    // "oro_desde_nivel", así que en los niveles anteriores su peso es cero y nunca se sortea.
    static std::vector<PesoDeObjeto> pesos(int numero_de_nivel, int oro_desde_nivel);
};

#endif
