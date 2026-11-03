#ifndef MANEJADORES_POR_TIPO_H
#define MANEJADORES_POR_TIPO_H

// Arma, a partir de varias lambdas, un único objeto que se puede pasar a std::visit.
template <typename... Lambdas>
struct ManejadoresPorTipo : Lambdas... {
    using Lambdas::operator()...;
};

template <typename... Lambdas>
ManejadoresPorTipo(Lambdas...) -> ManejadoresPorTipo<Lambdas...>;

#endif
