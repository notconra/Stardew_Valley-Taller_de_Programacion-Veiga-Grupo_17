#ifndef ENTRADA_DEL_USUARIO_H
#define ENTRADA_DEL_USUARIO_H

#include <SDL2/SDL.h>

#include <map>
#include <vector>

#include "../../common/mensajes/mensajes.h"

class EntradaDelUsuario {
public:
    struct Pedidos {
        bool salir = false;    // ESC
        bool guardar = false;  // F5 (cheat: guardar la partida ya, sin esperar al fin del día)
        bool multijugador = false;  // M (para probar)
        bool confirmar = false;     // Enter (el OK para pasar al día siguiente)
        bool clic_izquierdo = false;
        bool clic_derecho = false;
        int ranura_de_la_barra = -1;  // 1-9 y 0 (ranuras 0 a 9), -1 si no se apretó ninguna
        int giro_de_la_barra = 0;     // rueda: positivo avanza a la ranura siguiente
    };

    EntradaDelUsuario();

    void leer();

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
    int posicion_raton_x = 0;
    int posicion_raton_y = 0;
    Pedidos pedidos_actuales;

    void procesar(const SDL_Event& evento);
    void apretar(SDL_Scancode tecla);
    void soltar(SDL_Scancode tecla);
};

#endif
