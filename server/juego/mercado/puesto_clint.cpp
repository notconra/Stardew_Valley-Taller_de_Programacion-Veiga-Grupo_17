#include "puesto_clint.h"

PuestoClint::PuestoClint(const ConfiguracionMercadoServidor& config)
    : PuestoDeMercado(
          "clint", config,
          {TipoItem::PIEDRA,
           TipoItem::CARBON,
           TipoItem::LAPISLAZULI,
           TipoItem::PLATA,
           TipoItem::ORO}) {
}
