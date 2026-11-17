#ifndef MENSAJES_H
#define MENSAJES_H

#include <variant>

#include "mensajes_error.h"
#include "mensajes_lobby.h"
#include "mensajes_partida.h"
// Incluir acá los archivos donde se agreguen nuevos mensajes (!!!)

// Único lugar donde se listan todos los mensajes que viajan por la red.
// Para agregar un mensaje nuevo hay que definir su struct en mensajes/, agregarlo al variant que
// corresponda acá, agregar su valor en tipo_mensaje, escribir su recibir_algo() y su enviar_algo()
// en ProtocoloServidor y ProtocoloCliente.

using ComandoCliente = std::variant<
    // Lobby
    Identificarse, ListarPartidas, CrearPartida, UnirsePartida, RetomarPartida, EliminarPartida,

    // Partida
    ActivarMultijugador, SalirDePartida, Moverse, GuardarPartida,

    // Inventario y acciones
    SeleccionarRanuraDeLaBarra, UsarItem, MoverItemEntreRanuras, MoverItemConCofre, CerrarCofre,
    SolicitarInventario, SoltarItem, ConsultarCasilla,

    // Fin del día
    DarElOk,

    // Chat
    EnviarMensajeDeChat,

    // Regalos
    EnviarRegalo, AbrirRegalos,

    // Mercado
    ComprarEnMercado, VenderEnMercado, ComprarAnimalMercado, VenderAnimalMercado, CerrarMercado,

    // NPCs
    HablarConNpc>;

using EventoServidor = std::variant<
    // Error
    ErrorServidor,

    // Lobby
    ListaPartidas, EntradaExitosaAPartida,

    // Partida
    MultijugadorActivado, PartidaAbandonada, PartidaCerradaPorError,

    // - Sector
    EstadoJuego, JugadoresEnSector, ObjetoColocado, ObjetoRetirado, DropAparecido, DropRetirado,
    EnemigoActualizado, TrampaActualizada, TierraCambiada, AnimalActualizado, AnimalRetirado,
    VacaOrdeniable,

    // - Tiempo
    HoraDelJuego,

    // - Lo que se marca en el piso
    CasillaConsultada,

    // - Fin del día
    EstadoDelDescanso,

    // - Inventario y cofres
    ActualizacionInventario, ActualizacionCofre,

    // - Chat
    MensajeDeChat,

    // - Regalos
    JugadoresDeLaPartida, RegalosPendientes,

    // - Mercado
    ActualizacionMercado, CerrarVentanaMercado,

    // - Avisos
    AvisoEnPantalla,

    // - NPCs
    NpcsEnSector, DialogoNpc,

    // - Llamas de las Skulls
    LlamasEnSector>;

#endif
