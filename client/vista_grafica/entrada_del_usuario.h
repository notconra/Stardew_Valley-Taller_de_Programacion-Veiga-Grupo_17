#ifndef ENTRADA_DEL_USUARIO_H
#define ENTRADA_DEL_USUARIO_H

#include <SDL2/SDL.h>

#include <map>
#include <vector>

#include "../../common/mensajes/mensajes.h"

// Traduce los eventos de SDL (teclado, mouse y ventana) a lo que el usuario quiere hacer. No habla
// con el servidor directamente, solo informa, y la vista decide qué comandos mandar.
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
    // El usuario quiere usar el ítem de la ranura activa (colocar un cofre, por ejemplo).
    bool uso_de_item_pedido = false;
    // El usuario quiere abrir o cerrar el cofre que tiene enfrente.
    bool cofre_pedido = false;

    // Para el inventario: posición actual del mouse y clics que ocurrieron en este frame.
    int posicion_raton_x = 0;
    int posicion_raton_y = 0;
    bool clic_izquierdo_pedido = false;
    bool clic_derecho_pedido = false;
    // Ranura de la barra pedida con 1-9 o 0; -1 si no se apretó ninguna.
    int ranura_de_la_barra_pedida = -1;
    // Giro de la rueda en este frame: positivo avanza a la ranura siguiente, negativo retrocede.
    int desplazamiento_de_la_barra = 0;

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

    // El usuario apretó la tecla para usar el ítem de la ranura activa.
    bool pidio_usar_item() const;

    // Posición del mouse dentro de la ventana, en píxeles.
    int raton_x() const;
    int raton_y() const;

    // Hubo un clic en este frame. Con el izquierdo se agarra/suelta el stack completo y con el
    // derecho se agarra/suelta un solo ítem.
    bool pidio_clic_izquierdo() const;
    bool pidio_clic_derecho() const;

    // Ranura de la barra que se pidió con las teclas 1-9 o 0, o -1 si no se apretó ninguna.
    int ranura_de_la_barra_pedida_por_tecla() const;

    // Giro de la rueda en este frame: positivo avanza a la ranura siguiente, negativo retrocede.
    // Vale 0 si no se movió la rueda.
    int desplazamiento_de_la_barra_pedido() const;
};

#endif
