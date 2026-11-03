#ifndef DROPS_H
#define DROPS_H

#include <cstdint>
#include <map>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/ranura_inventario.h"
#include "../geometria.h"
#include "../inventario/inventario.h"

// Un ítem tirado en el piso de un sector. Ocupa una casilla y guarda un stack completo de un solo
// tipo de ítem. El id lo asigna Drops y es único dentro de ese mapa.
struct Drop {
    int id = 0;
    int columna = 0;
    int fila = 0;
    TipoItem tipo = TipoItem::NINGUNO;
    uint16_t cantidad = 0;
    Calidad calidad = Calidad::BASE;
};

// Los ítems que están tirados en el piso de UN sector. No sabe quién los recoge ni cómo avisarle
// al cliente, solo guarda los drops y sabe pasarlos a un inventario. Lo usan los subsistemas que
// producen drops (picar, animales, cultivos, tirar un ítem) y el que los recoge.
class Drops {
private:
    std::map<int, Drop> drops;
    int siguiente_id = 1;

public:
    // Deja un ítem en la casilla indicada. Devuelve el id del drop, o 0 si no había nada que
    // dejar. No valida la casilla, quien lo llama ya la validó contra el mapa.
    int dejar(TipoItem tipo, uint16_t cantidad, Calidad calidad, int columna, int fila);

    // El drop con ese id, o nullptr si ya no está.
    const Drop* buscar(int id) const;

    // Los ids de los drops a los que les corresponde el radio (en casillas, diagonal incluida)
    // desde la casilla indicada.
    std::vector<int> ids_cerca_de(const Casilla& centro, int radio_en_casillas) const;

    // Le ofrece el drop a un inventario. Si no entra completo no toca nada y devuelve false, el
    // drop entero queda en el piso para que lo recoja otro jugador con espacio.
    bool recoger(int id, Inventario& destino);

    // Saca un drop sin que lo recoja nadie. Devuelve false si no existía.
    bool quitar(int id);

    // Borra todos los drops del sector. La usa la mazmorra cuando se vacía y se vuelve a sortear.
    void limpiar();

    bool vacio() const;

    // Todos los drops del sector, para armar la instantánea que se manda al cliente.
    void agregar_a(EstadoJuego& estado) const;
};

#endif
