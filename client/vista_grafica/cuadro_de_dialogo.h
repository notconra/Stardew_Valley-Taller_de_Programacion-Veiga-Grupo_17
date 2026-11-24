#ifndef CUADRO_DE_DIALOGO_H
#define CUADRO_DE_DIALOGO_H

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>

#include "entrada_del_usuario.h"
#include "fuente_del_juego.h"
#include "textura.h"

// Cartel modal con lo que dice un NPC. Usa de fondo la imagen "Assets/npcs/dialogo_<id>.png" del
// NPC (retrato a la derecha, texto a la izquierda).
// Igual que CuadroDeAviso: mientras está visible el juego queda bloqueado (el que lo use tiene que
// ignorar la entrada del juego y mandar al jugador quieto) y se cierra con un clic en cualquier
// parte de la pantalla.
class CuadroDeDialogo {
private:
    FuenteDelJuego fuente;
    SDL_Renderer* renderer;
    bool visible = false;
    std::string id_npc;
    std::string mensaje;
    std::map<std::string, Textura> fondos;  // se carga una imagen por NPC, la primera vez

    std::vector<std::string> partir_en_lineas(const std::string& texto, int ancho_maximo) const;

public:
    // "ruta_fuente" es la .ttf del juego. Lanza std::runtime_error si no se puede abrir la fuente.
    CuadroDeDialogo(SDL_Renderer* renderer, const std::string& ruta_fuente);

    // Lanza std::runtime_error si no se puede abrir la imagen de fondo de ese NPC.
    void mostrar(const std::string& nuevo_id_npc, const std::string& texto);
    void cerrar();
    bool esta_visible() const;

    // Si el cartel está visible, un clic (izquierdo o derecho) lo cierra. Devuelve true si estaba
    // visible. En ese caso el que llama tiene que ignorar el resto de la entrada del juego.
    bool procesar_entrada(const EntradaDelUsuario& entrada);

    void dibujar(SDL_Renderer* renderer, int ancho_ventana, int alto_ventana) const;
};

#endif
