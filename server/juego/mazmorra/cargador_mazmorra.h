#ifndef CARGADOR_DE_MAZMORRA_H
#define CARGADOR_DE_MAZMORRA_H

#include <random>
#include <string>
#include <vector>

#define DESTINO_ESCALERA "siguiente"
#define DESTINO_ENTRADA "entrada"

#include "../../../common/lector_de_mapas_tmx/lector_de_mapas.h"
#include "nivel.h"

class CargadorMazmorra {
public:
    /*
     * Elige 10 mapas distintos de "disponibles" en orden aleatorio.
     */
    static std::vector<std::string> elegir_niveles(
        const std::vector<std::string>& disponibles, std::mt19937&& generador);

    /*
     * Carga los niveles indicados (nombres de archivo dentro de "carpeta"), en el mismo orden.
     */
    static std::vector<GeografiaNivel> cargar_niveles(
        const std::string& carpeta, const std::vector<std::string>& nombres);

    /*
     * Carga un solo TMX.
     */
    static GeografiaNivel cargar_nivel(const std::string& ruta_tmx);

private:
    /*
     * Pasa un rectángulo en píxeles a casillas, recortado al mapa.
     */
    static Zona a_casillas(const TransicionTMX& transicion, const TamanioMapa& tamanio);

    /*
     * Elige una casilla libre pegada a la salida.
     */
    static PosicionEnNivel asignar_entrada(const GeografiaNivel& geografia);
};

#endif
