#ifndef ENTRADA_DEL_USUARIO_H
#define ENTRADA_DEL_USUARIO_H

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"

class EntradaDelUsuario {
public:
    struct Pedidos {
        bool salir = false;         // ESC
        bool multijugador = false;  // M (para probar)
        bool clic_izquierdo = false;
        bool clic_derecho = false;
        int ranura_de_la_barra = -1;  // 1-9 y 0 (ranuras 0 a 9), -1 si no se apretó ninguna
        int giro_de_la_barra = 0;     // rueda: positivo avanza a la ranura siguiente
        int rueda = 0;                // rueda sin interpretar: positivo hacia arriba

        bool alternar_chat = false;       // T (solo cuando no se está escribiendo)
        bool confirmar_chat = false;      // Enter mientras se escribe
        int caracteres_a_borrar = 0;      // Retroceso mientras se escribe, con sus repeticiones
        bool cancelar_escritura = false;  // ESC mientras se escribe (no sale de la partida)
        std::string texto_ingresado;      // texto UTF-8 de SDL_TEXTINPUT
    };

    EntradaDelUsuario();

    // "escribiendo_texto" tiene que reflejar si el chat está capturando el teclado. Se pasa antes
    // de leer los eventos para que las teclas de movimiento no se mezclen con el texto.
    void leer(bool escribiendo_texto = false);

    Moverse intencion() const;

    bool pidio_cerrar() const;

    int raton_x() const;
    int raton_y() const;

    const Pedidos& pedidos() const;

private:
    const std::map<SDL_Scancode, Moverse> movimiento_de_tecla;
    const std::map<SDL_Scancode, bool Pedidos::*> pedido_de_tecla;

    std::vector<SDL_Scancode> teclas_de_movimiento;
    bool cierre_pedido = false;
    bool modo_escritura = false;
    int posicion_raton_x = 0;
    int posicion_raton_y = 0;
    Pedidos pedidos_actuales;

    void procesar(const SDL_Event& evento);
    void apretar(SDL_Scancode tecla);
    void soltar(SDL_Scancode tecla);
};

#endif
