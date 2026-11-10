#include "comando_animal.h"

#include <string>

#include "../mundo.h"

ComandoAnimal::ComandoAnimal(Mundo& mundo_de_juego) : mundo(mundo_de_juego) {
}

bool ComandoAnimal::reconoce(const std::string& palabra_clave) const {
    return palabra_clave == "animal" || palabra_clave == "avanzar_dia";
}

void ComandoAnimal::ejecutar(const ContextoDeComandoDeChat& contexto) {
    if (contexto.palabra_clave == "avanzar_dia") {
        ejecutar_avanzar_dia(contexto);
    } else {
        ejecutar_animal(contexto);
    }
}

void ComandoAnimal::ejecutar_avanzar_dia(const ContextoDeComandoDeChat& contexto) {
    mundo.avanzar_dia_cheat();
    contexto.respuestas.responder_a_jugador(contexto.id_jugador, "Dia avanzado con exito.");
}

void ComandoAnimal::ejecutar_animal(const ContextoDeComandoDeChat& contexto) {
    if (contexto.argumentos.empty()) {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Uso: /animal vaca | /animal gallina | /animal estado | /avanzar_dia");
        return;
    }
    const std::string& subcomando = contexto.argumentos[0];
    if (subcomando == "vaca") {
        if (mundo.agregar_vaca_en_granja_cerca(contexto.id_jugador)) {
            contexto.respuestas.responder_a_jugador(
                contexto.id_jugador,
                "Vaca agregada a la granja.");
        } else {
            contexto.respuestas.responder_a_jugador(
                contexto.id_jugador,
                "No se pudo agregar vaca (limite de 5 alcanzado o sin espacio libre).");
        }
    } else if (subcomando == "gallina") {
        if (mundo.agregar_gallina_en_granja_cerca(contexto.id_jugador)) {
            contexto.respuestas.responder_a_jugador(
                contexto.id_jugador,
                "Gallina agregada a la granja.");
        } else {
            contexto.respuestas.responder_a_jugador(
                contexto.id_jugador,
                "No se pudo agregar gallina (limite de 10 alcanzado o sin espacio libre).");
        }
    } else if (subcomando == "estado") {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            mundo.estado_animales_granja());
    } else {
        contexto.respuestas.responder_a_jugador(
            contexto.id_jugador,
            "Subcomando desconocido. Usa vaca, gallina o estado.");
    }
}
