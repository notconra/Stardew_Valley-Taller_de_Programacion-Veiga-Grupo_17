#ifndef BUZON_H
#define BUZON_H

#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/ranura_inventario.h"
#include "../inventario/inventario.h"

// Los regalos que recibió un jugador y todavía no abrió. Un regalo queda acá, esté el dueño
// conectado o no, hasta que el dueño lo abre: recién ahí pasa a su inventario. El buzón se guarda
// con la partida, y para eso se arma desde (y se lee como) una lista de RegaloRecibido.
class Buzon {
private:
    std::vector<RegaloRecibido> regalos;

public:
    Buzon() = default;
    explicit Buzon(std::vector<RegaloRecibido> guardados);

    void depositar(std::string remitente, const RanuraInventario& item);

    // Pasa al inventario todo lo que entre, en el orden en que llegó. Lo que no entra sigue acá.
    void vaciar_en(Inventario& inventario);

    bool vacio() const;
    const std::vector<RegaloRecibido>& pendientes() const;
};

#endif
