#ifndef TIERRA_TRATADA_H
#define TIERRA_TRATADA_H

#include <set>
#include <string>
#include <utility>

#include "../../../common/mensajes/mensajes.h"
#include "../base/geometria/casilla.h"
#include "../base/mapas/mapa.h"

class TierraTratada {
private:
    const Mapa& mapa;
    std::set<std::pair<int, int>> tratadas;

public:
    explicit TierraTratada(const Mapa& mapa_de_la_tierra);

    // Trata la tierra de la casilla. Solo se puede en una casilla cultivable que no esté bloqueada
    // ni ya tratada. Devuelve true si la casilla cambió.
    bool tratar(const Casilla& casilla);

    // La vuelve a tierra común. Devuelve true si estaba tratada.
    bool volver_a_comun(const Casilla& casilla);

    bool esta_tratada(const Casilla& casilla) const;

    // Si tratar() cambiaría la casilla: cultivable, sin bloquear y todavía sin tratar.
    bool se_puede_tratar(const Casilla& casilla) const;

    // Agrega las casillas tratadas de este mapa al estado, si tiene alguna.
    void agregar_a(EstadoTierra& estado_tierra, const std::string& id_mapa) const;
};

#endif
