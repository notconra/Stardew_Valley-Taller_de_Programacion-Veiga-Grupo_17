#ifndef ENTRADA_DEL_USUARIO_H
#define ENTRADA_DEL_USUARIO_H

#include <SDL2/SDL.h>

#include <map>
#include <vector>

#include "../../common/mensajes/mensajes.h"

// Traduce los eventos de SDL (teclado y ventana) a lo que el usuario quiere hacer. No habla con
// el servidor directamente, solo informa, y la vista decide qué comandos mandar.
//
// Se usan los eventos de tecla (no el estado del teclado) para saber en qué ORDEN se apretaron las
// teclas de movimiento, o sea, si hay dos apretadas manda la última. Así, soltar una tecla no deja
// al personaje parado si la otra sigue apretada.
class EntradaDelUsuario {
private:
    // Qué movimiento pide cada tecla: {horizontal, vertical}.
    const std::map<SDL_Scancode, Moverse> movimiento_de_tecla;

    // Teclas de movimiento apretadas, en orden de pulsación (la última es la que manda).
    std::vector<SDL_Scancode> teclas_de_movimiento;
    bool cierre_pedido = false;
    bool salida_pedida = false;
    bool multijugador_pedido = false;

    void procesar(const SDL_Event& evento);
    void apretar(SDL_Scancode tecla);
    void soltar(SDL_Scancode tecla);

public:
    explicit EntradaDelUsuario();
    // Atiende todos los eventos de SDL pendientes. Se llama una vez por frame. Los pedidos
    // (cerrar, salir, multijugador) valen solo para el frame en que se leyeron.
    void leer();

    // Hacia dónde quiere moverse ahora (NINGUNA si no hay tecla de movimiento apretada).
    Moverse intencion() const;

    // El usuario cerró la ventana.
    bool pidio_cerrar() const;

    // El usuario apretó ESC.
    bool pidio_salir_de_partida() const;

    // Esto lo pongo para probar, despues va con un botón y en otro lado.
    bool pidio_multijugador() const;
};

#endif
