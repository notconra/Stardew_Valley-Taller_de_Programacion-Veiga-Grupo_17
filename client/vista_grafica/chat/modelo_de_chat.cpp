#include "modelo_de_chat.h"

#include <algorithm>
#include <deque>
#include <string>

#include "texto_utf8.h"

ModeloDeChat::ModeloDeChat(size_t maximo_de_mensajes_permitidos)
    : maximo_de_mensajes(maximo_de_mensajes_permitidos) {
}

void ModeloDeChat::recibir(const MensajeDeChat& mensaje) {
    historial.push_back(mensaje);
    if (historial.size() > maximo_de_mensajes) {
        historial.pop_front();
    }

    if (!abierto) {
        ++mensajes_sin_leer;
    } else if (posicion_del_historial > 0) {
        ++posicion_del_historial;
    }
}

void ModeloDeChat::abrir(size_t mensajes_visibles) {
    abierto = true;

    const size_t sin_leer = std::min(mensajes_sin_leer, historial.size());
    const size_t entran = std::max<size_t>(1, mensajes_visibles);
    if (sin_leer > entran) {
        posicion_del_historial = sin_leer - entran;
    } else {
        posicion_del_historial = 0;
    }
}

void ModeloDeChat::cerrar() {
    abierto = false;
    escribiendo = false;
    mensajes_sin_leer = 0;
}

bool ModeloDeChat::esta_abierto() const {
    return abierto;
}

size_t ModeloDeChat::cantidad_sin_leer() const {
    return mensajes_sin_leer;
}

void ModeloDeChat::limpiar() {
    cerrar();
    historial.clear();
    texto.clear();
    posicion_del_historial = 0;
}

const std::deque<MensajeDeChat>& ModeloDeChat::mensajes() const {
    return historial;
}

void ModeloDeChat::empezar_a_escribir() {
    escribiendo = true;
    abierto = true;
}

void ModeloDeChat::terminar_de_escribir() {
    escribiendo = false;
}

bool ModeloDeChat::esta_escribiendo() const {
    return escribiendo;
}

const std::string& ModeloDeChat::texto_en_edicion() const {
    return texto;
}

void ModeloDeChat::agregar_texto(const std::string& texto_nuevo) {
    size_t posicion = 0;
    while (posicion < texto_nuevo.size()) {
        const size_t largo = std::min(
            longitud_de_caracter_utf8(texto_nuevo, posicion),
            texto_nuevo.size() - posicion);
        if (texto.size() + largo > LARGO_MAXIMO_DE_MENSAJE_DE_CHAT) {
            return;
        }
        texto.append(texto_nuevo, posicion, largo);
        posicion += largo;
    }
}

void ModeloDeChat::borrar_caracter() {
    if (texto.empty()) {
        return;
    }

    size_t posicion = texto.size() - 1;
    while (posicion > 0 && es_continuacion_de_utf8(texto[posicion])) {
        --posicion;
    }
    texto.erase(posicion);
}

std::string ModeloDeChat::confirmar_y_limpiar() {
    std::string mensaje;
    const size_t inicio = texto.find_first_not_of(" \t\n\r");
    if (inicio != std::string::npos) {
        const size_t fin = texto.find_last_not_of(" \t\n\r");
        mensaje = texto.substr(inicio, fin - inicio + 1);
    }
    texto.clear();
    return mensaje;
}

void ModeloDeChat::desplazar(int mensajes, size_t desplazamiento_maximo) {
    const long nueva_posicion = static_cast<long>(posicion_del_historial) + mensajes;
    posicion_del_historial = static_cast<size_t>(
        std::clamp<long>(nueva_posicion, 0, static_cast<long>(desplazamiento_maximo)));
}

void ModeloDeChat::ir_al_final() {
    posicion_del_historial = 0;
}

size_t ModeloDeChat::desplazamiento() const {
    return posicion_del_historial;
}
