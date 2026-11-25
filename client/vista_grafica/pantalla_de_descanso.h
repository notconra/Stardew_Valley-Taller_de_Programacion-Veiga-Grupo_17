#ifndef PANTALLA_DE_DESCANSO_H
#define PANTALLA_DE_DESCANSO_H

#include <SDL2/SDL.h>

#include <optional>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "caja_con_opciones.h"
#include "entrada_del_usuario.h"

// Todo lo de irse a dormir, en la caja de texto de abajo como en Stardew (CajaConOpciones):
// - la pregunta "Queres dormir?" (Si / No) que manda el servidor al hacer clic en una cama libre.
//   Se elige con el mouse, con las flechas (o W y S) y Enter; ESC es "No".
// - mientras espera el día siguiente, acostado o desmayado: qué le pasó, cuántos jugadores ya
//   dieron el OK y las opciones "Listo" y, si todavía puede, "Levantarse". Se eligen con el mouse
//   o con Enter (la marcada empieza siendo "Listo"); moverse también lo levanta, eso lo decide el
//   servidor.
// Lanza std::runtime_error si no encuentra la imagen de la caja o la fuente.
class PantallaDeDescanso {
private:
    CajaConOpciones caja;
    // La cama por la que preguntó el servidor, mientras se espera la respuesta.
    std::optional<PreguntaDeDormir> pregunta;

    CajaConOpciones::Contenido contenido(const EstadoDelDescanso& descanso) const;

public:
    // El tamaño de la caja y de su letra salen del alto de la ventana. "ruta_fuente" es la .ttf.
    PantallaDeDescanso(SDL_Renderer* renderer, const std::string& ruta_fuente, int alto_ventana);

    void preguntar(const PreguntaDeDormir& nueva_pregunta);
    // Como contestar "No".
    void cancelar_la_pregunta();
    bool esta_preguntando() const;

    // Si hay que mostrarla: está preguntando o el jugador no está despierto.
    bool visible(const EstadoDelDescanso& descanso) const;

    // Lo que hay que mandarle al servidor por lo que eligió el jugador en este cuadro, o nada.
    std::optional<ComandoCliente> procesar_entrada(
        const EntradaDelUsuario& entrada, const EstadoDelDescanso& descanso, int ancho_ventana,
        int alto_ventana);

    // Se dibuja al final, encima de todo (también del HUD).
    void dibujar(
        SDL_Renderer* renderer, const EstadoDelDescanso& descanso, int ancho_ventana,
        int alto_ventana) const;
};

#endif
