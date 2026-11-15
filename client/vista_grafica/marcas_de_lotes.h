#ifndef MARCAS_DE_LOTES_H
#define MARCAS_DE_LOTES_H

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "iconos_de_lote.h"

// El ícono que flota sobre cada lote del centro comunitario que todavía no se completó, para que se
// sepa dónde hacer clic. Los lugares salen del TMX del mapa (capa Interacciones, tipo = "lote") y
// lo completo de cada lote, de lo que manda el servidor (LoteDelCentro, al entrar y cuando cambia).
// Mientras no se sabe nada de un lote, se lo marca.
class MarcasDeLotes {
private:
    struct Lugar {
        std::string clave;
        int columna;
        int fila;
    };

    const IconosDeLotes& iconos;
    int tamano_de_tile;
    int escala_de_tiles;
    std::vector<Lugar> lugares;
    std::map<std::string, bool> completos;

public:
    MarcasDeLotes(const IconosDeLotes& iconos, const ConfiguracionGraficos& configuracion_graficos);

    // Lee los lotes del mapa al que se entró (en los mapas sin lotes no queda ninguno).
    void cargar_mapa(const ConfiguracionMapa& mapa);

    void recibir(const LoteDelCentro& lote);

    // cam_x y cam_y son la esquina de la cámara, en píxeles de pantalla.
    void dibujar(SDL_Renderer* renderer, int cam_x, int cam_y) const;
};

#endif
