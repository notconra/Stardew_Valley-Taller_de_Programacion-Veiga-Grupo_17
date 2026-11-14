#ifndef CULTIVOS_H
#define CULTIVOS_H

#include <map>

#include "../../../common/tipo_cultivo.h"
#include "../../../common/tipo_item.h"

// Cómo es cada cultivo según el enunciado: cuántas veces por día se riega, cuántos días tarda en
// estar listo para cosechar y qué semilla y qué producto le corresponden.
struct DefinicionDeCultivo {
    TipoCultivo tipo = TipoCultivo::NINGUNO;
    TipoItem semilla = TipoItem::NINGUNO;
    TipoItem producto = TipoItem::NINGUNO;
    // El día se parte en esta cantidad de turnos y en cada uno hay que regarla una vez.
    int riegos_por_dia = 1;
    // Días (con todos sus riegos) que tarda desde que se planta hasta que se puede cosechar.
    int dias_para_madurar = 1;
    // Si después de cosecharla vuelve a dar, cada cuántos días (la frutilla). 0 si al cosecharla
    // se termina.
    int dias_para_volver_a_dar = 0;
    // Cuántos riegos por día puede saltear sin dejar de crecer ese día (la papa).
    int riegos_que_tolera = 0;
};

// Los cuatro cultivos del juego. Se arma una sola vez (RecursosDeJuego) y después solo se consulta.
class CatalogoDeCultivos {
private:
    std::map<TipoCultivo, DefinicionDeCultivo> definiciones_por_tipo;

    void registrar(const DefinicionDeCultivo& definicion);

public:
    CatalogoDeCultivos();

    // nullptr si no existe.
    const DefinicionDeCultivo* buscar(TipoCultivo tipo) const;

    // El cultivo que crece de esa semilla, o nullptr si el ítem no es una semilla.
    const DefinicionDeCultivo* de_la_semilla(TipoItem semilla) const;

    const std::map<TipoCultivo, DefinicionDeCultivo>& definiciones() const;
};

#endif
