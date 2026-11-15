#ifndef CONTROLADOR_DEL_CENTRO_H
#define CONTROLADOR_DEL_CENTRO_H

#include <cstdint>
#include <map>
#include <string>

#include "../../../common/mensajes/mensajes.h"
#include "../../../common/tipo_lote.h"
#include "../../persistencia/datos_partida_guardada.h"
#include "../centro_comunitario/catalogo_lotes.h"
#include "../centro_comunitario/centro_comunitario.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../recursos/recursos_de_juego.h"
#include "../sector/gestor_de_sectores.h"

// El centro comunitario de la partida y lo que hacen los jugadores con él: abrir un lote con clic
// derecho sobre su lugar en el mapa, donar ítems y donar monedas. Saca del jugador lo que el centro
// acepta y, cada vez que un lote cambia, se lo manda a todos para que lo vea quien lo tenga
// abierto.
class ControladorDelCentro {
private:
    GestorDeSectores& sectores;
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    CentroComunitario centro;

    // El lote que hay en la casilla del clic, si el clic es secundario, cae sobre un lote del mapa
    // del jugador y lo tiene al alcance.
    const DefinicionDeLote* lote_clickeado(const Personaje& personaje, const UsarItem& uso) const;

    void avisar_a_todos(TipoLote lote);

public:
    ControladorDelCentro(
        GestorDeSectores& sectores, const RecursosDeJuego& recursos, Notificador& notificador,
        const DatosCentroGuardado& guardado);

    // El clic sobre un lote (le manda el lote al jugador para que abra la ventana), DonarAlLote y
    // DonarMonedasAlLote. Devuelve true si consumió el comando; un UsarItem que no cae sobre un
    // lote lo deja pasar para las acciones. "dia" es el día del juego.
    bool atender(Personaje& personaje, const ComandoCliente& comando, uint32_t dia);

    // Dona lo que el lote acepte de esa ranura del inventario (índice absoluto, como en la ventana
    // del cofre) y se lo saca al jugador. Devuelve cuántas unidades se donaron (0 si ninguna).
    uint16_t donar_item(Personaje& personaje, TipoLote lote, uint8_t indice_ranura, uint32_t dia);

    // Cobra al jugador las monedas que pide el lote y lo completa. Devuelve false, sin cobrar
    // nada, si el lote no pide monedas, ya está completo o al jugador no le alcanza.
    bool donar_monedas(Personaje& personaje, TipoLote lote, uint32_t dia);

    // El lote con lo que pide y lo que se lleva aportado, listo para mandar.
    LoteDelCentro estado_del_lote(TipoLote lote, bool abrir) const;

    // El mapa del centro que corresponde a ese destino (ver CentroComunitario::mapa_para).
    std::string mapa_para(const std::string& destino) const;

    const CentroComunitario& estado() const;
    CentroComunitario& estado();

    DatosCentroGuardado obtener_guardado() const;
};

#endif
