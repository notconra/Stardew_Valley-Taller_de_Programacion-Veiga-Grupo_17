#ifndef OBJETOS_DEL_MUNDO_H
#define OBJETOS_DEL_MUNDO_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/tipo_item.h"
#include "../../../common/tipo_objeto.h"
#include "../colisiones/colisiones.h"
#include "../mapa/mapa.h"
#include "../mapa/propiedad_de_casilla.h"

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
    std::string clave;          // identificador de texto, único entre todas las definiciones
    int golpes_para_picar = 0;  // cuántos golpes de la herramienta hacen falta para romperlo
    TipoItem herramienta = TipoItem::NINGUNO;
    double probabilidad_de_no_dejar = 0.0;
    TipoItem producto = TipoItem::NINGUNO;
    int cantidad = 1;
    // El ítem del inventario que el jugador gasta para colocar este objeto (el cofre, un mueble).
    // NINGUNO si el jugador no puede colocarlo, como pasa con minerales y árboles, que solo
    // genera el mundo. Dos objetos no pueden compartir el mismo ítem.
    TipoItem item_colocable = TipoItem::NINGUNO;
};

// Todos los tipos de objeto que existen. Se arma una sola vez al arrancar (RecursosDeJuego) y
// después solo se consulta.
class CatalogoDeObjetos {
private:
    std::map<TipoObjeto, DefinicionDeObjeto> definiciones_por_tipo;

public:
    // Lanza std::runtime_error si una definición no tiene tipo o clave, si hay tipos, claves o
    // item_colocable repetidos, si la probabilidad de no dejar producto no está entre 0 y 1, o si
    // deja un producto en cantidad menor a uno.
    explicit CatalogoDeObjetos(const std::vector<DefinicionDeObjeto>& definiciones);

    // Devuelve nullptr si el tipo no existe.
    const DefinicionDeObjeto* buscar(TipoObjeto tipo) const;

    // Busca por clave de texto. Devuelve nullptr si no existe.
    const DefinicionDeObjeto* buscar_por_clave(const std::string& clave) const;

    // El objeto que se coloca al gastar ese ítem. Devuelve nullptr si el ítem no coloca nada.
    const DefinicionDeObjeto* buscar_por_item(TipoItem item) const;

    // Todas las definiciones, ordenadas por tipo.
    const std::map<TipoObjeto, DefinicionDeObjeto>& definiciones() const;
};

// Peso relativo de un tipo de objeto al elegir cuál colocar al azar. No hace falta que los pesos
// sumen 100, hacemos que la probabilidad de cada tipo sea su peso sobre la suma de todos.
struct PesoDeObjeto {
    TipoObjeto tipo = TipoObjeto::NINGUNO;
    double peso = 0.0;
};

// Los objetos colocados en un mapa durante una partida. Marca su casilla en Colisiones, asigna ids
// y sabe qué deja cada uno al romperse. Lo usan tanto los jugadores (colocar un cofre) como la
// mazmorra (poblar un nivel con cosas).
class GestorDeObjetos {
private:
    std::string id_mapa;
    const Mapa& mapa;
    const CatalogoDeObjetos& catalogo;
    Colisiones& colisiones;
    std::mt19937 motor{std::random_device{}()};
    std::map<int, ObjetoDelMundo> objetos_por_id;
    std::vector<int> id_por_casilla;  // id del objeto en cada casilla (0 = ninguno), fila por fila
    int siguiente_id = 1;             // no se reinicia, así un id viejo nunca apunta a otro objeto

    // Posición de la casilla en id_por_casilla. La casilla tiene que estar dentro del mapa.
    std::size_t indice(int columna, int fila) const;

public:
    // Arranca sin objetos: quien arma el mapa los coloca con colocar() o poblar_al_azar().
    GestorDeObjetos(
        const std::string& id_del_mapa, const Mapa& mapa_de_partida,
        const CatalogoDeObjetos& catalogo_de_objetos, Colisiones& colisiones_del_mapa);

    // Coloca un objeto y devuelve su id, o 0 si no se pudo (tipo inexistente, casilla fuera del
    // mapa, bloqueada por el terreno u ocupada). Si falla no cambia nada.
    int colocar(TipoObjeto tipo, int columna, int fila);

    // Coloca hasta "cantidad" objetos en casillas libres elegidas al azar, sin repetir casilla.
    // El tipo de cada uno se sortea según "pesos". Si propiedad_requerida no es NINGUNA, solo
    // sirven las casillas del terreno que la tengan. Devuelve cuántos colocó, que es menos que
    // "cantidad" si no alcanzan las casillas libres. Es la API pensada para que la mazmorra
    // pueble cada nivel (y para árboles u otros objetos que se generan solos).
    // Lanza std::invalid_argument si cantidad es negativa, si pesos está vacío, tiene un tipo
    // que no está en el catálogo, un peso negativo o todos en cero.
    int poblar_al_azar(
        const std::vector<PesoDeObjeto>& pesos, int cantidad,
        uint8_t propiedad_requerida = PropiedadDeCasilla::NINGUNA);

    // Agrega todos los objetos al estado que se manda al cliente.
    void agregar_a(EstadoJuego& estado) const;

    // Rompe el objeto, lo retira y devuelve lo que dejó, con el sorteo de
    // probabilidad_de_no_dejar. Si el id no existe, se_rompio queda en false.
    ResultadoDeRotura romper(int id);

    // Saca el objeto del mapa y libera su casilla, sin dejar nada. Devuelve false si el id no
    // existe. Es para deshacer una colocación o reemplazar objetos, no para picar.
    bool retirar(int id);

    // Retira los objetos que genera el mundo y deja los que colocó un
    // jugador (los que tienen item_colocable). Es para regenerar un nivel
    // de la mazmorra antes de volver a poblarlo, sin que el jugador pierda lo que construyó.
    // Devuelve cuántos retiró.
    int retirar_objetos_naturales();

    // Fija la semilla de los sorteos de poblar_al_azar y romper, para generar siempre el mismo
    // nivel o para probar. Si no se llama, la semilla es aleatoria.
    void fijar_semilla(std::mt19937::result_type semilla);

    // El objeto que está en esa casilla, o nullptr si no hay ninguno. El puntero deja de ser
    // válido si el objeto se retira o se rompe.
    const ObjetoDelMundo* objeto_en(int columna, int fila) const;

    int cantidad_de_objetos() const;
};

#endif
