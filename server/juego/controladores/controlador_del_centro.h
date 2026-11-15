#ifndef CONTROLADOR_DEL_CENTRO_H
#define CONTROLADOR_DEL_CENTRO_H

#include <cstdint>
#include <map>
#include <string>

#include "../../../common/tipo_lote.h"
#include "../../persistencia/datos_partida_guardada.h"
#include "../centro_comunitario/catalogo_lotes.h"
#include "../centro_comunitario/centro_comunitario.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"

// El centro comunitario de la partida y lo que hacen los jugadores con él: donar ítems y monedas.
// Saca del jugador lo que el centro acepta y le avisa cómo le quedó el inventario.
class ControladorDelCentro {
private:
    Notificador& notificador;
    CentroComunitario centro;

public:
    ControladorDelCentro(
        const CatalogoDeLotes& catalogo, Notificador& notificador,
        const DatosCentroGuardado& guardado);

    // Dona lo que el lote acepte de esa ranura del inventario (índice absoluto, como en la ventana
    // del cofre) y se lo saca al jugador. Devuelve cuántas unidades se donaron (0 si ninguna).
    uint16_t donar_item(Personaje& personaje, TipoLote lote, uint8_t indice_ranura, uint32_t dia);

    // Cobra al jugador las monedas que pide el lote y lo completa. Devuelve false, sin cobrar
    // nada, si el lote no pide monedas, ya está completo o al jugador no le alcanza.
    bool donar_monedas(Personaje& personaje, TipoLote lote, uint32_t dia);

    // El mapa del centro que corresponde a ese destino (ver CentroComunitario::mapa_para).
    std::string mapa_para(const std::string& destino) const;

    const CentroComunitario& estado() const;
    CentroComunitario& estado();

    DatosCentroGuardado obtener_guardado() const;
};

#endif
