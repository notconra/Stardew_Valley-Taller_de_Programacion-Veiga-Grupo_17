#include "pantalla_de_descanso.h"

#include <string>

#define OPCION_SI 0
#define OPCION_LISTO 0

PantallaDeDescanso::PantallaDeDescanso(
    SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana)
    : caja(renderer, ruta_fuente, alto_ventana) {
}

void PantallaDeDescanso::preguntar(const PreguntaDeDormir& nueva_pregunta) {
    pregunta = nueva_pregunta;
}

void PantallaDeDescanso::cancelar_la_pregunta() {
    pregunta.reset();
}

bool PantallaDeDescanso::esta_preguntando() const {
    return pregunta.has_value();
}

bool PantallaDeDescanso::visible(const EstadoDelDescanso& descanso) const {
    return esta_preguntando() || descanso.estado != EstadoDeDescanso::DESPIERTO;
}

CajaConOpciones::Contenido PantallaDeDescanso::contenido(const EstadoDelDescanso& descanso) const {
    CajaConOpciones::Contenido resultado;
    if (descanso.estado == EstadoDeDescanso::DESPIERTO) {
        resultado.lineas.push_back("Queres dormir?");
        resultado.opciones = {"Si", "No"};
        return resultado;
    }
    const std::string cuenta =
        std::to_string(descanso.listos) + "/" + std::to_string(descanso.jugadores);
    resultado.lineas.push_back(
        descanso.estado == EstadoDeDescanso::DESMAYADO ? "Te desmayaste." : "Te fuiste a dormir.");
    if (descanso.dio_el_ok) {
        resultado.lineas.push_back("Esperando a los demas jugadores (" + cuenta + ")");
        return resultado;
    }
    resultado.lineas.push_back("Listos: " + cuenta);
    resultado.opciones.push_back("Listo");
    if (descanso.puede_levantarse) {
        resultado.opciones.push_back("Levantarse");
    }
    return resultado;
}

std::optional<ComandoCliente> PantallaDeDescanso::procesar_entrada(
    const EntradaDelUsuario& entrada, const EstadoDelDescanso& descanso, int ancho_ventana,
    int alto_ventana) {
    if (descanso.estado != EstadoDeDescanso::DESPIERTO) {
        // Si se desmayó o pasó el día mientras preguntaba, la pregunta ya no tiene sentido.
        pregunta.reset();
    }
    if (!visible(descanso)) {
        return std::nullopt;
    }
    if (esta_preguntando()) {
        const std::optional<size_t> elegida =
            caja.procesar_entrada(entrada, contenido(descanso), ancho_ventana, alto_ventana, true);
        if (!elegida.has_value()) {
            return std::nullopt;
        }
        const PreguntaDeDormir cama = *pregunta;
        pregunta.reset();
        if (*elegida != OPCION_SI) {
            return std::nullopt;
        }
        return Dormir{cama.columna, cama.fila};
    }
    // Sin flechas: moverse ya es levantarse.
    const std::optional<size_t> elegida =
        caja.procesar_entrada(entrada, contenido(descanso), ancho_ventana, alto_ventana, false);
    if (!elegida.has_value()) {
        return std::nullopt;
    }
    if (*elegida == OPCION_LISTO) {
        return DarElOk{};
    }
    return Levantarse{};
}

void PantallaDeDescanso::dibujar(
    SDL_Renderer* renderer, const EstadoDelDescanso& descanso, int ancho_ventana,
    int alto_ventana) const {
    if (visible(descanso)) {
        caja.dibujar(renderer, contenido(descanso), ancho_ventana, alto_ventana);
    }
}
