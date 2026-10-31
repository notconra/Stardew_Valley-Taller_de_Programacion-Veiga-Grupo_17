#ifndef OBJETOS_DEL_MUNDO_H
#define OBJETOS_DEL_MUNDO_H

#include <map>
#include <random>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_objeto.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../colisiones/colisiones.h"

// Un objeto ya colocado en un mapa. Ocupa una sola casilla. El id lo asigna GestorDeObjetos y es
// único dentro de ese mapa.
struct ObjetoDelMundo {
    int id = 0;
    int columna = 0;
    int fila = 0;
    TipoObjeto tipo = TipoObjeto::NINGUNO;
};

// Lo que pasó al romper un objeto. Si cantidad es 0, no dejó nada. El sistema de acciones decide
// después si el producto va al inventario o queda tirado en el piso.
struct ResultadoDeRotura {
    bool se_rompio = false;
    TipoItem producto = TipoItem::NINGUNO;
    int cantidad = 0;
};

// Cómo es un tipo de objeto, cuánto hay que picarlo, con qué herramienta y qué deja.
// El objeto que se ve en el mapa (TipoObjeto) y el ítem que deja (TipoItem) son cosas distintas.
struct DefinicionDeObjeto {
    TipoObjeto tipo = TipoObjeto::NINGUNO;
    std::string clave;          // la que se escribe en el TOML
    int golpes_para_picar = 0;  // cuántos golpes de la herramienta hacen falta para romperlo
    TipoItem herramienta = TipoItem::NINGUNO;
    double probabilidad_de_no_dejar = 0.0;
    TipoItem producto = TipoItem::NINGUNO;
    int cantidad = 1;
};

// Todos los tipos de objeto que existen. Se arma una sola vez al arrancar (RecursosDeJuego) y
// después solo se consulta.
class CatalogoDeObjetos {
private:
    std::map<TipoObjeto, DefinicionDeObjeto> definiciones_por_tipo;

public:
    // Lanza std::runtime_error si una definición no tiene tipo o clave, o si hay tipos o claves
    // repetidas.
    explicit CatalogoDeObjetos(const std::vector<DefinicionDeObjeto>& definiciones);

    // Devuelve nullptr si el tipo no existe.
    const DefinicionDeObjeto* buscar(TipoObjeto tipo) const;

    // Busca por la clave que se escribe en el TOML. Devuelve nullptr si no existe.
    const DefinicionDeObjeto* buscar_por_clave(const std::string& clave) const;
};

// Los objetos colocados en un mapa durante una partida. Marca su casilla en Colisiones, asigna
// ids y sabe qué deja cada uno al romperse. La acción de picar la maneja el sistema de acciones,
// acá solo se coloca y se rompe.
class GestorDeObjetos {
private:
    std::string id_mapa;
    const CatalogoDeObjetos& catalogo;
    Colisiones& colisiones;
    std::mt19937 motor{std::random_device{}()};
    std::map<int, ObjetoDelMundo> objetos_por_id;
    int siguiente_id = 1;

public:
    // Coloca los objetos fijos que trae la configuración del mapa. Lanza std::runtime_error si
    // alguno tiene un tipo desconocido o no se puede colocar.
    GestorDeObjetos(
        const std::string& id_mapa, const CatalogoDeObjetos& catalogo, Colisiones& colisiones,
        const ConfiguracionObjetos& configuracion);

    // Coloca un objeto y devuelve su id, o 0 si no se pudo (tipo inexistente, casilla fuera del
    // mapa u ocupada). Si falla no cambia nada. Es lo que usa la mazmorra para poblar cada nivel.
    int colocar(TipoObjeto tipo, int columna, int fila);

    // Agrega todos los objetos al estado que se manda al cliente.
    void agregar_a(EstadoJuego& estado) const;

    // Rompe el objeto, libera su casilla y devuelve lo que dejó. Si el id no existe, se_rompio
    // queda en false.
    ResultadoDeRotura romper(int id);

    // El objeto que está en esa casilla, o nullptr si no hay ninguno. El puntero deja de ser
    // válido si el objeto se rompe.
    const ObjetoDelMundo* objeto_en(int columna, int fila) const;

    int cantidad_de_objetos() const;
};

#endif
