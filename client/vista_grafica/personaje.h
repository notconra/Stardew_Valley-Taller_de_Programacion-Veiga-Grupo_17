#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <SDL2/SDL.h>

#include "../../common/direccion.h"
#include "../../common/mensajes/mensajes.h"
#include "../../common/ranura_inventario.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "iconos_de_items.h"
#include "textura.h"

// Aspecto de un personaje armado por capas: cuerpo (con brazos y la superposición de la acción
// en la misma hoja), pantalón, pelo, sombrero y remera.
//
// haciendo_accion/ms_de_accion son estado LOCAL (del cliente que dibuja), no vienen del
// servidor: solo se ve la animación de acción en la pantalla de quien la hace.
class Personaje {
private:
    Textura cuerpo;
    Textura pantalon;
    Textura pelo;
    Textura sombrero;
    Textura remera;

    int ancho;
    int alto;
    int escala_de_tiles;
    int ms_por_cuadro_caminando;

    int indice_pelo;
    int indice_sombrero;
    int indice_remera;
    int indice_pantalon;
    int offset_y_remera;
    int offset_y_pelo;

    int ancho_hoja_cuerpo;  // ancho total del PNG del cuerpo, lo usa el pantalón para alinearse
    int columnas_pantalon;
    int columnas_pelo;
    int columnas_sombrero;
    int columnas_remera;

public:
    Personaje(
        const ConfiguracionJugador& configuracion_jugador,
        const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer);

    int ancho_del_sprite() const;
    int alto_del_sprite() const;

    // en_mano es el ítem de la ranura activa (TipoItem::NINGUNO si no se conoce o está vacía).
    // Pico, pala y espada se ven en la mano; cualquier otro ítem se sostiene sobre la cabeza.
    void dibujar(
        SDL_Renderer* renderer, const EstadoJugador& jugador, bool haciendo_accion,
        Uint32 ms_de_accion, int cam_x, int cam_y, const RanuraInventario& en_mano,
        const IconosDeItems& iconos) const;
};

#endif