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
#include "../centro_comunitario/seguimiento_pelea.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../recursos/recursos_de_juego.h"
#include "../sector/gestor_de_sectores.h"
#include "controlador_de_dia.h"

// El centro comunitario de la partida y lo que hacen los jugadores con él: abrir un lote (con
// AccionAbrirLote, clic derecho sobre su lugar en el mapa), donar ítems y donar monedas. Saca del
// jugador lo que el centro acepta y, cada vez que un lote cambia, se lo manda a todos para que lo
// vea quien lo tenga abierto.
//
// Cuando un lote se completa reparte el premio a todos los jugadores que ya estaban en la partida,
// conectados o no (lo que se le da a un desconectado queda en su Personaje y se guarda con él). El
// que entra por primera vez después no recibe nada.
//  - Cultivos: 5 semillas de cada cultivo, al buzón de regalos (así no se pierden si el inventario
//    está lleno).
//  - Metales: 500 monedas.
//  - Productos animales: una vaca y dos gallinas para la granja (una sola vez, la granja es de
//    todos). Lo que no entra por el tope se paga a cada jugador con el precio de compra del
//    animal en el mercado.
//  - Pelea: la vida y la energía máximas suben un 25%.
//  - Dinero: el descuento del mercado del día siguiente (lo consulta el mercado, acá no se da
//  nada).
//
// También sigue lo que hace cada jugador en la mazmorra (con SeguimientoDePelea) y cumple los
// logros del set de pelea: AccionAtacar le avisa cada muerte y ControladorDeEnemigos cada golpe que
// recibe un jugador; en qué piso está cada uno lo mira solo, en cada tick.
class ControladorDelCentro {
private:
    std::map<uint32_t, Personaje>& personajes;
    GestorDeSectores& sectores;
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    const ControladorDelDia& dia;
    CentroComunitario centro;
    SeguimientoDePelea seguimiento;

    // Dona lo que el lote acepte de esa ranura del inventario (índice absoluto, como en la ventana
    // del cofre) y se lo saca al jugador.
    void donar_item(Personaje& personaje, TipoLote lote, uint8_t indice_ranura);

    // Cobra al jugador las monedas que pide el lote y lo completa. No cobra nada si el lote no
    // pide monedas, ya está completo o al jugador no le alcanza.
    void donar_monedas(Personaje& personaje, TipoLote lote);

    // Le avisa al seguimiento en qué piso de la mazmorra está cada jugador y, al que bajó de un
    // piso con una calavera sin que lo lastimaran, le cumple el logro.
    void seguir_a_los_jugadores();
    // El piso de la mazmorra donde está el jugador, o 0 si está afuera, desmayado o desconectado.
    int piso_de(const Personaje& personaje) const;
    bool hay_una_calavera_en_el_piso(int piso) const;
    void cumplir_logro(LogroDePelea logro);
    uint32_t dia_de_hoy() const;

    void avisar_a_todos(TipoLote lote);

    // Dónde está el lote en el mapa del centro, para que el cliente lo marque. Si el mapa no lo
    // tiene, deja el mapa vacío.
    void completar_lugar(LoteDelCentro& estado, const std::string& clave) const;

    void entregar_premio(TipoLote lote);
    void dar_semillas_a_todos();
    void dar_monedas_a_todos(uint32_t cantidad);
    void dar_animales_a_la_granja();
    // Agrega el animal a la granja. Devuelve lo que hay que pagarle a cada jugador si no entró.
    uint32_t agregar_animal_o_cotizarlo(TipoAnimal tipo);
    void dar_mejora_de_pelea_a_todos();
    void avisar_en_el_chat(const std::string& texto);

public:
    ControladorDelCentro(
        std::map<uint32_t, Personaje>& personajes, GestorDeSectores& sectores,
        const RecursosDeJuego& recursos, Notificador& notificador, const ControladorDelDia& dia,
        const DatosCentroGuardado& guardado);

    // DonarAlLote y DonarMonedasAlLote. Devuelve true si consumió el comando.
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // El jugador le dio el golpe final a ese enemigo de su sector.
    void al_matar_enemigo(const Personaje& personaje, const Sector& sector, const Enemigo& muerto);

    // Un enemigo lastimó al jugador.
    void al_lastimar_un_enemigo_a(uint32_t id_jugador);

    // Cheats: da por completo el lote con esa clave (la del TMX) o todos los que falten, con sus
    // premios. El primero devuelve false si no hay un lote con esa clave o ya estaba completo.
    bool completar_lote(const std::string& clave);
    void completar_todos_los_lotes();

    // Le manda al jugador el lote con esa clave (la del TMX) para que abra su ventana. Devuelve
    // false si no hay un lote con esa clave.
    bool abrir_lote(const Personaje& personaje, const std::string& clave);

    // Reparte el premio de los lotes que se completaron desde la última vez. Se llama cada tick.
    void actualizar();

    // Al entrar: el estado de todos los lotes, para que el cliente sepa cuáles marcar en el mapa.
    void al_conectarse(const Personaje& personaje);

    // El lote con lo que pide y lo que se lleva aportado, listo para mandar.
    LoteDelCentro estado_del_lote(TipoLote lote, bool abrir) const;

    // El mapa del centro que corresponde a ese destino (ver CentroComunitario::mapa_para).
    std::string mapa_para(const std::string& destino) const;

    const CentroComunitario& estado() const;
    CentroComunitario& estado();

    DatosCentroGuardado obtener_guardado() const;
};

#endif
