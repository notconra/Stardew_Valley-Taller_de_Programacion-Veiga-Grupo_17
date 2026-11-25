#include "ventana_de_lista.h"

#include <algorithm>

VentanaDeLista::VentanaDeLista(size_t filas_a_mostrar) : filas_visibles(filas_a_mostrar) {
}

size_t VentanaDeLista::ultimo_inicio_posible(size_t total) const {
    return total > filas_visibles ? total - filas_visibles : 0;
}

size_t VentanaDeLista::desde(size_t total) const {
    return std::min(primero, ultimo_inicio_posible(total));
}

size_t VentanaDeLista::visibles(size_t total) const {
    return std::min(total - desde(total), filas_visibles);
}

bool VentanaDeLista::hay_arriba(size_t total) const {
    return desde(total) > 0;
}

bool VentanaDeLista::hay_abajo(size_t total) const {
    return desde(total) + filas_visibles < total;
}

void VentanaDeLista::subir(size_t total) {
    primero = desde(total);
    if (primero > 0) {
        --primero;
    }
}

void VentanaDeLista::bajar(size_t total) {
    primero = std::min(desde(total) + 1, ultimo_inicio_posible(total));
}

void VentanaDeLista::volver_al_principio() {
    primero = 0;
}
