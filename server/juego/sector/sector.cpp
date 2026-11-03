#include "sector.h"

#include <optional>
#include <vector>

const ObjetoDelMundo* Sector::objeto_al_frente(uint32_t id_jugador) const {
    const std::optional<Casilla> casilla = jugadores.casilla_de_enfrente(id_jugador);
    if (!casilla.has_value()) {
        return nullptr;
    }
    const ObjetoDelMundo* objeto = objetos.objeto_en(casilla->columna, casilla->fila);
    if (objeto == nullptr || objeto->tipo == TipoObjeto::NINGUNO) {
        return nullptr;
    }
    return objeto;
}

const ObjetoDelMundo* Sector::cofre_al_frente(uint32_t id_jugador) const {
    const ObjetoDelMundo* objeto = objeto_al_frente(id_jugador);
    if (objeto == nullptr || objeto->tipo != TipoObjeto::COFRE) {
        return nullptr;
    }
    return objeto;
}

void Sector::avisar(Notificador& notificador, const EventoServidor& evento) const {
    const std::vector<uint32_t> ids = jugadores.ids();
    for (uint32_t id_jugador : ids) {
        notificador.a_jugador(id_jugador, evento);
    }
}

int Sector::colocar_objeto(Notificador& notificador, TipoObjeto tipo, int columna, int fila) {
    const int id = objetos.colocar(tipo, columna, fila);
    if (id == 0) {
        return 0;
    }

    ObjetoColocado evento;
    evento.id = static_cast<uint32_t>(id);
    evento.columna = static_cast<uint32_t>(columna);
    evento.fila = static_cast<uint32_t>(fila);
    evento.tipo = tipo;
    avisar(notificador, evento);
    return id;
}

void Sector::retirar_objeto(Notificador& notificador, int id_objeto) {
    objetos.retirar(id_objeto);

    ObjetoRetirado evento;
    evento.id = static_cast<uint32_t>(id_objeto);
    avisar(notificador, evento);
}

ResultadoDeGolpe Sector::golpear_objeto(
    Notificador& notificador, int id_objeto, TipoItem herramienta) {
    const ResultadoDeGolpe golpe = objetos.golpear(id_objeto, herramienta);
    if (golpe.golpeo && golpe.rotura.se_rompio) {
        ObjetoRetirado evento;
        evento.id = static_cast<uint32_t>(id_objeto);
        avisar(notificador, evento);
    }
    return golpe;
}

int Sector::soltar(
    Notificador& notificador, TipoItem tipo, uint16_t cantidad, Calidad calidad, int columna,
    int fila) {
    if (!mapa.esta_dentro(columna, fila)) {
        return 0;
    }

    const int id = drops.dejar(tipo, cantidad, calidad, columna, fila);
    if (id == 0) {
        return 0;
    }

    EstadoDrop estado;
    estado.id = static_cast<uint32_t>(id);
    estado.columna = static_cast<uint32_t>(columna);
    estado.fila = static_cast<uint32_t>(fila);
    estado.tipo = tipo;
    estado.cantidad = cantidad;
    estado.calidad = calidad;

    DropAparecido evento;
    evento.drop = estado;
    avisar(notificador, evento);
    return id;
}
