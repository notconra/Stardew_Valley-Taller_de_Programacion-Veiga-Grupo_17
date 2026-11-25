#ifndef VISITANTES_H
#define VISITANTES_H

// Arma, a partir de varias lambdas, un único objeto que se puede pasar a std::visit.
// Sirve para recorrer los variants de mensajes sin escribir una clase por cada uno.
template <typename... Lambdas>
struct ManejadoresPorTipo : Lambdas... {
    using Lambdas::operator()...;
};

template <typename... Lambdas>
ManejadoresPorTipo(Lambdas...) -> ManejadoresPorTipo<Lambdas...>;

#endif
