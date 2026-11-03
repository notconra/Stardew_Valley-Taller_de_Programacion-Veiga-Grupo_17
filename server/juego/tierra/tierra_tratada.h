#ifndef TIERRA_TRATADA_H
#define TIERRA_TRATADA_H

#include <set>
#include <utility>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../geometria.h"
#include "../mapa/mapa.h"

// Estado dinámico de la tierra de UN sector (mapa). El terreno fijo (Mapa) dice qué casillas son
// cultivables; acá se recuerda cuáles ya fueron tratadas con la pala. Es de cada partida, igual que
// los objetos colocados y los drops, por eso vive en Sector.
//
// Cuando se agregue el cultivo se podría agregar acá el estado de cada
// casilla (cultivo, días de crecimiento, regado...), sin tocar el Mapa ni el protocolo de terreno.
// A chequearr
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

    // Agrega las casillas tratadas de este mapa a la lista que se manda en el snapshot del sector.
    void agregar_a(std::vector<CasillaTratada>& casillas) const;
};

#endif
