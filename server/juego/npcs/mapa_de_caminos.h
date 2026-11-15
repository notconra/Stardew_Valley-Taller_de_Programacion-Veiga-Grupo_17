#ifndef MAPA_DE_CAMINOS_H
#define MAPA_DE_CAMINOS_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "../geometria.h"
#include "../sector/gestor_de_sectores.h"

// El "mapa de calles" de los NPC para saber por dónde se camina.
// Numera todas las casillas de todos los mapas con un unico entero, el "nodo":
//
//     nodo = primer nodo del mapa + fila * ancho del mapa + columna
//
// Esto es para que una casilla de cualquier mapa es un simple int y los caminos son listas de int.
// Une cada casilla con sus cuatro vecinas y, en las casillas de puerta, con la casilla donde se
// aparece del otro lado (por eso un camino puede cruzar de un mapa a otro). Tambien recuerda que
// casillas tienen un NPC encima.
//
// El terreno se lee una sola vez, al construirlo. Como no cambia, el camino entre dos nodos se
// calcula una unica vez y queda guardado.
class MapaDeCaminos {
public:
    static constexpr int SIN_NODO = -1;

private:
    struct MapaMedido {
        std::string id;
        int primer_nodo;
        int ancho;  // en casillas
        int alto;   // en casillas
    };

    std::vector<MapaMedido> mapas;
    std::unordered_map<std::string, int> indice_de_mapa_por_id;

    // Un dato por nodo.
    std::vector<int> mapa_del_nodo;    // indice en "mapas"
    std::vector<char> caminable;       // 1 si el terreno deja pisarla
    std::vector<char> ocupado;         // 1 si hay un NPC parado ahi
    std::vector<int> salto_de_puerta;  // adonde lleva la puerta de este nodo, o SIN_NODO

    // Caminos ya calculados, por (origen, destino). Un camino que no existe se guarda vacio.
    std::unordered_map<uint64_t, std::vector<int>> caminos_guardados;

    // Memoria que reutilizan todas las busquedas para no pedir memoria en cada una.
    // "visitado" guarda el numero de la ultima busqueda que paso por el nodo, asi no hace falta
    // borrarlo entre busquedas.
    std::vector<int> anterior;
    std::vector<unsigned> visitado;
    unsigned numero_de_busqueda = 0;
    std::vector<int> cola;

    // Mide cada mapa, numera sus casillas y anota cuales son caminables segun las colisiones.
    // El ancho y el alto se miden preguntando "esta_dentro" casilla por casilla; si Mapa ya los
    // sabe, se pueden usar directamente.
    void medir_mapas(GestorDeSectores& sectores);

    // Convierte cada transicion del mapa (puerta, escalera) en un salto de la casilla de la
    // puerta a la casilla donde se aparece del otro lado.
    void unir_mapas_por_las_puertas(GestorDeSectores& sectores);

    // Elige la casilla caminable mas cercana al centro de la zona, prefiriendo las que estan
    // dentro.
    int buscar_puerta(const Sector& sector, int indice_de_mapa, const Rectangulo& zona) const;

    int nodo_en(int indice_de_mapa, int columna, int fila) const;

    // Las casillas de arriba, abajo, izquierda y derecha que existen en el mismo mapa (sin mirar
    // si son caminables). Devuelve cuantas son.
    int vecinos_en_el_mapa(int nodo, int vecinos[4]) const;

    // Busqueda bfs, camino más corto
    std::vector<int> buscar_camino(int origen, int destino, bool esquivar_ocupados);

public:
    explicit MapaDeCaminos(GestorDeSectores& sectores);

    // De (mapa, columna, fila) a nodo y viceversa. Devuelven SIN_NODO si la casilla no existe.
    int nodo_de(const std::string& mapa, int columna, int fila) const;
    int nodo_desplazado(int nodo, int columnas, int filas) const;
    const std::string& mapa_de(int nodo) const;
    int columna_de(int nodo) const;
    int fila_de(int nodo) const;

    // El terreno deja pisarla (no es pared).
    bool es_caminable(int nodo) const;

    // Hay un NPC parado en la casilla.
    bool esta_ocupado(int nodo) const;
    void ocupar(int nodo);
    void liberar(int nodo);

    // El camino mas corto de origen a destino, incluyendo ambos extremos. Atraviesa las puertas
    // entre mapas. Devuelve una lista vacia si no hay forma de llegar.
    // Con "esquivar_ocupados" no pasa por casillas con un NPC (el destino si puede estar ocupado).
    // Ese camino depende de donde estan los NPC en este momento, asi que no se guarda.
    std::vector<int> camino(int origen, int destino, bool esquivar_ocupados);

    // La casilla caminable mas cercana al nodo (el mismo si ya lo es). Con "respetar_ocupadas"
    // tampoco elige una que tenga un NPC encima. SIN_NODO si no hay ninguna en ese mapa.
    int casilla_libre_cercana(int nodo, bool respetar_ocupadas);
};

#endif
