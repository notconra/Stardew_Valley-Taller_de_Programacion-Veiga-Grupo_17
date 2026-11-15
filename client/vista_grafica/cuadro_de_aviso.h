#ifndef CUADRO_DE_AVISO_H
#define CUADRO_DE_AVISO_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

#include "entrada_del_usuario.h"
#include "fuente_del_juego.h"
#include "textura.h"

// Cartel modal reutilizable para avisarle algo al jugador. Mientras está visible, el juego queda
// bloqueado, el que lo use tiene que ignorar la entrada del juego y mandar al jugador quieto.
// Se cierra con un clic en cualquier parte de la pantalla (ver procesar_entrada).
// El fondo es una imagen que se estira para que el mensaje entre justo.
class CuadroDeAviso {
private:
    FuenteDelJuego fuente;
    Textura fondo;
    bool visible = false;
    std::string mensaje;

    std::vector<std::string> partir_en_lineas(int ancho_maximo) const;

public:
    // "ruta_fuente" es la .ttf del juego. Lanza std::runtime_error si no se puede abrir la fuente
    // o la imagen de fondo.
    CuadroDeAviso(SDL_Renderer* renderer, const std::string& ruta_fuente);

    void mostrar(const std::string& nuevo_mensaje);
    void cerrar();
    bool esta_visible() const;

    // Si el cartel está visible, un clic (izquierdo o derecho) lo cierra. Devuelve true si estaba
    // visible. En ese caso el que llama tiene que ignorar el resto de la entrada del juego.
    bool procesar_entrada(const EntradaDelUsuario& entrada);

    void dibujar(SDL_Renderer* renderer, int ancho_ventana, int alto_ventana) const;
};

#endif
