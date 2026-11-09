#include "entrada_del_usuario.h"

#include <algorithm>

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
      },
      pedido_de_tecla{
          {SDL_SCANCODE_ESCAPE, &Pedidos::salir},
          {SDL_SCANCODE_G, &Pedidos::guardar},
          {SDL_SCANCODE_M, &Pedidos::multijugador},
          {SDL_SCANCODE_T, &Pedidos::alternar_chat},
      } {
}

void EntradaDelUsuario::leer(bool escribiendo_texto) {
    modo_escritura = escribiendo_texto;
    if (modo_escritura) {
        teclas_de_movimiento.clear();
    }

    pedidos_actuales = Pedidos{};

    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        procesar(evento);
    }
}

void EntradaDelUsuario::procesar(const SDL_Event& evento) {
    if (evento.type == SDL_QUIT) {
        cierre_pedido = true;
    } else if (evento.type == SDL_TEXTINPUT) {
        if (modo_escritura) {
            pedidos_actuales.texto_ingresado += evento.text.text;
        }
    } else if (evento.type == SDL_KEYDOWN) {
        const SDL_Scancode tecla = evento.key.keysym.scancode;
        if (evento.key.repeat == 0 || (modo_escritura && tecla == SDL_SCANCODE_BACKSPACE)) {
            apretar(tecla);
        }
    } else if (evento.type == SDL_KEYUP) {
        soltar(evento.key.keysym.scancode);
    } else if (evento.type == SDL_MOUSEMOTION) {
        posicion_raton_x = evento.motion.x;
        posicion_raton_y = evento.motion.y;
    } else if (evento.type == SDL_MOUSEWHEEL) {
        pedidos_actuales.giro_de_la_barra -= evento.wheel.y;
        pedidos_actuales.rueda += evento.wheel.y;
    } else if (evento.type == SDL_MOUSEBUTTONDOWN) {
        posicion_raton_x = evento.button.x;
        posicion_raton_y = evento.button.y;
        pedidos_actuales.con_shift = (SDL_GetModState() & KMOD_SHIFT) != 0;
        if (evento.button.button == SDL_BUTTON_LEFT) {
            pedidos_actuales.clic_izquierdo = true;
        } else if (evento.button.button == SDL_BUTTON_RIGHT) {
            pedidos_actuales.clic_derecho = true;
        }
    } else if (
        evento.type == SDL_WINDOWEVENT && evento.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        teclas_de_movimiento.clear();
    }
}

void EntradaDelUsuario::apretar(SDL_Scancode tecla) {
    if (modo_escritura) {
        if (tecla == SDL_SCANCODE_ESCAPE) {
            pedidos_actuales.cancelar_escritura = true;
        } else if (tecla == SDL_SCANCODE_RETURN || tecla == SDL_SCANCODE_KP_ENTER) {
            pedidos_actuales.confirmar_chat = true;
        } else if (tecla == SDL_SCANCODE_BACKSPACE) {
            ++pedidos_actuales.caracteres_a_borrar;
        }
        return;
    }

    if (const auto pedido = pedido_de_tecla.find(tecla); pedido != pedido_de_tecla.end()) {
        pedidos_actuales.*(pedido->second) = true;
    } else if (tecla >= SDL_SCANCODE_1 && tecla <= SDL_SCANCODE_0) {
        pedidos_actuales.ranura_de_la_barra = tecla == SDL_SCANCODE_0 ? 9 : tecla - SDL_SCANCODE_1;
    } else if (movimiento_de_tecla.contains(tecla)) {
        soltar(tecla);
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
int EntradaDelUsuario::raton_x() const {
    return posicion_raton_x;
}

int EntradaDelUsuario::raton_y() const {
    return posicion_raton_y;
}

const EntradaDelUsuario::Pedidos& EntradaDelUsuario::pedidos() const {
    return pedidos_actuales;
}
