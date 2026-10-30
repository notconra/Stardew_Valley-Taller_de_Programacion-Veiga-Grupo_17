#include "entrada_del_usuario.h"

#include <algorithm>
#include <map>

EntradaDelUsuario::EntradaDelUsuario()
    : movimiento_de_tecla{
          {SDL_SCANCODE_UP, Moverse{0, -1}},
          {SDL_SCANCODE_W, Moverse{0, -1}},
          {SDL_SCANCODE_DOWN, Moverse{0, 1}},
          {SDL_SCANCODE_S, Moverse{0, 1}},
          {SDL_SCANCODE_LEFT, Moverse{-1, 0}},
          {SDL_SCANCODE_A, Moverse{-1, 0}},
          {SDL_SCANCODE_RIGHT, Moverse{1, 0}},
          {SDL_SCANCODE_D, Moverse{1, 0}},
      } {
}

void EntradaDelUsuario::leer() {
    salida_pedida = false;
    multijugador_pedido = false;
    uso_de_item_pedido = false;
    cofre_pedido = false;
    clic_izquierdo_pedido = false;
    clic_derecho_pedido = false;
    ranura_de_la_barra_pedida = -1;
    desplazamiento_de_la_barra = 0;

    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        procesar(evento);
    }
}

void EntradaDelUsuario::procesar(const SDL_Event& evento) {
    if (evento.type == SDL_QUIT) {
        cierre_pedido = true;
    } else if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0) {
        apretar(evento.key.keysym.scancode);
    } else if (evento.type == SDL_KEYUP) {
        soltar(evento.key.keysym.scancode);
    } else if (evento.type == SDL_MOUSEMOTION) {
        posicion_raton_x = evento.motion.x;
        posicion_raton_y = evento.motion.y;
    } else if (evento.type == SDL_MOUSEWHEEL) {
        // SDL usa y > 0 para rueda hacia arriba, acá usamos positivo = ranura siguiente.
        desplazamiento_de_la_barra -= evento.wheel.y;
    } else if (evento.type == SDL_MOUSEBUTTONDOWN) {
        posicion_raton_x = evento.button.x;
        posicion_raton_y = evento.button.y;
        if (evento.button.button == SDL_BUTTON_LEFT) {
            clic_izquierdo_pedido = true;
        } else if (evento.button.button == SDL_BUTTON_RIGHT) {
            clic_derecho_pedido = true;
        }
    } else if (
        evento.type == SDL_WINDOWEVENT && evento.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        // si la ventana pierde el foco con una tecla apretada, SDL nunca avisa que se soltó y el
        // personaje seguiría caminando solo
        teclas_de_movimiento.clear();
    }
}

void EntradaDelUsuario::apretar(SDL_Scancode tecla) {
    if (tecla == SDL_SCANCODE_ESCAPE) {
        salida_pedida = true;
    } else if (tecla == SDL_SCANCODE_M) {
        multijugador_pedido = true;
    } else if (tecla == SDL_SCANCODE_E) {
        uso_de_item_pedido = true;
    } else if (tecla == SDL_SCANCODE_F) {
        cofre_pedido = true;
    } else if (tecla >= SDL_SCANCODE_1 && tecla <= SDL_SCANCODE_0) {
        // 1-9 eligen las primeras nueve ranuras y 0 la décima.
        ranura_de_la_barra_pedida = tecla == SDL_SCANCODE_0 ? 9 : tecla - SDL_SCANCODE_1;
    } else if (movimiento_de_tecla.contains(tecla)) {
        soltar(tecla);  // por si ya estaba, que quede una sola vez y al final
        teclas_de_movimiento.push_back(tecla);
    }
}

void EntradaDelUsuario::soltar(SDL_Scancode tecla) {
    teclas_de_movimiento.erase(
        std::remove(teclas_de_movimiento.begin(), teclas_de_movimiento.end(), tecla),
        teclas_de_movimiento.end());
}

Moverse EntradaDelUsuario::intencion() const {
    Moverse intencion{};
    // De la tecla más nueva a la más vieja: cada eje queda con la última tecla que lo tocó.
    for (auto tecla = teclas_de_movimiento.rbegin(); tecla != teclas_de_movimiento.rend();
         ++tecla) {
        const Moverse& movimiento = movimiento_de_tecla.at(*tecla);
        if (intencion.horizontal == 0) {
            intencion.horizontal = movimiento.horizontal;
        }
        if (intencion.vertical == 0) {
            intencion.vertical = movimiento.vertical;
        }
    }
    return intencion;
}

bool EntradaDelUsuario::pidio_cerrar() const {
    return cierre_pedido;
}

bool EntradaDelUsuario::pidio_salir_de_partida() const {
    return salida_pedida;
}

bool EntradaDelUsuario::pidio_multijugador() const {
    return multijugador_pedido;
}

bool EntradaDelUsuario::pidio_usar_item() const {
    return uso_de_item_pedido;
}

int EntradaDelUsuario::raton_x() const {
    return posicion_raton_x;
}

int EntradaDelUsuario::raton_y() const {
    return posicion_raton_y;
}

bool EntradaDelUsuario::pidio_clic_izquierdo() const {
    return clic_izquierdo_pedido;
}

bool EntradaDelUsuario::pidio_clic_derecho() const {
    return clic_derecho_pedido;
}

int EntradaDelUsuario::ranura_de_la_barra_pedida_por_tecla() const {
    return ranura_de_la_barra_pedida;
}

int EntradaDelUsuario::desplazamiento_de_la_barra_pedido() const {
    return desplazamiento_de_la_barra;
}
