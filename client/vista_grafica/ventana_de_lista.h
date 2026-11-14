#ifndef VENTANA_DE_LISTA_H
#define VENTANA_DE_LISTA_H

#include <cstddef>

// Una lista de la que se ven solo unas pocas filas a la vez. Sabe desde cuál mostrar y se mueve de
// a una fila con las flechitas. Como la lista puede achicarse (se abrieron regalos, un jugador se
// fue), cada consulta recibe el largo de ahora y acota el punto de partida a él.
class VentanaDeLista {
private:
    size_t filas_visibles;
    size_t primero = 0;

    size_t ultimo_inicio_posible(size_t total) const;

public:
    explicit VentanaDeLista(size_t filas_a_mostrar);

    // Índice de la primera fila que se ve.
    size_t desde(size_t total) const;

    // Cuántas filas se ven (menos que filas_visibles si la lista es corta).
    size_t visibles(size_t total) const;

    bool hay_arriba(size_t total) const;
    bool hay_abajo(size_t total) const;

    void subir(size_t total);
    void bajar(size_t total);
    void volver_al_principio();
};

#endif
