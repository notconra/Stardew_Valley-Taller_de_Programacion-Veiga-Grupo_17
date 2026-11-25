#include <gtest/gtest.h>

#include "../common/muebles/catalogo_muebles.h"
#include "protocolo_en_memoria.h"

// Eventos que viajan del servidor al cliente.
// Cada test envía un evento con el ProtocoloServidor y verifica que el ProtocoloCliente reciba
// exactamente el mismo evento.
class ProtocoloEventosTest : public ProtocoloEnMemoriaTest {};

namespace {

// Piezas que se usan en más de un evento.
EstadoJugador jugador_uno() {
    return EstadoJugador{
        1,
        "granja",
        0x05060708,
        0x090A0B0C,
        Direccion::DERECHA,
        true,
        7,
        TipoAccion::ATACAR,
        TipoItem::PAPA,
        Calidad::EXCELENTE,
        9,
        EstadoDeDescanso::ACOSTADO,
        80,
        70,
        110,
        120,
        true,
        0x0D0E0F10,
        apariencia_de_prueba()};
}

EstadoJugador jugador_dos() {
    return EstadoJugador{
        2,
        "granja",
        0x05060708,
        0x090A0B0C,
        Direccion::DERECHA,
        false,
        7,
        TipoAccion::ATACAR,
        TipoItem::PIEDRA,
        Calidad::PERFECTA,
        10,
        EstadoDeDescanso::DESPIERTO,
        80,
        70,
        110,
        120,
        false,
        0x0D0E0F10,
        apariencia_de_prueba()};
}

EstadoNpc npc_uno() {
    return EstadoNpc{"marnie", "pueblo", 0x71727374, 0x75767778, Direccion::DERECHA, true, 0x797A};
}

EstadoNpc npc_dos() {
    return EstadoNpc{"lewis", "pueblo", 0x71727374, 0x75767778, Direccion::DERECHA, false, 0x797A};
}

// Valores de muebles, no entran en 8 bits, así que detectan si algún campo sigue viajando como
// un solo byte.
constexpr TipoItem ITEM_MUEBLE = static_cast<TipoItem>(PRIMER_ITEM_DE_MUEBLE + 1);
constexpr TipoObjeto OBJETO_MUEBLE = static_cast<TipoObjeto>(PRIMER_OBJETO_DE_MUEBLE + 1);

}  // namespace

// Errores y partidas

TEST_F(ProtocoloEventosTest, ErrorServidor) {
    verificar_evento(ErrorServidor{CodigoError::NOMBRE_JUGADOR_REPETIDO, "nombre repetido"});
}

TEST_F(ProtocoloEventosTest, ListaPartidas) {
    const InfoPartida info1{
        "ABC123",
        "La granja",
        "Jugador de La granja",
        apariencia_de_prueba(),
        true,
        3,
        4,
        5,
        0xA1A2A3A4,
        0xA5A6A7A8,
        0x01020304};
    const InfoPartida info2{
        "XYZ789",
        "El pueblo",
        "Jugador de El pueblo",
        apariencia_de_prueba(),
        false,
        3,
        4,
        5,
        0xA1A2A3A4,
        0xA5A6A7A8,
        0x05060708};

    verificar_evento(ListaPartidas{{info1, info2}});
}

TEST_F(ProtocoloEventosTest, ListaPartidasVacia) {
    verificar_evento(ListaPartidas{});
}

TEST_F(ProtocoloEventosTest, ListaMundos) {
    verificar_evento(ListaMundos{{"Isla", "Valle del norte"}});
}

TEST_F(ProtocoloEventosTest, ListaMundosVacia) {
    verificar_evento(ListaMundos{});
}

TEST_F(ProtocoloEventosTest, CodigoPartidaDisponible) {
    verificar_evento(CodigoPartidaDisponible{});
}

TEST_F(ProtocoloEventosTest, EntradaExitosaAPartida) {
    verificar_evento(EntradaExitosaAPartida{"ABC123", 0x11223344, true});
}

TEST_F(ProtocoloEventosTest, MultijugadorActivado) {
    verificar_evento(MultijugadorActivado{"XYZ789"});
}

TEST_F(ProtocoloEventosTest, PartidaAbandonada) {
    verificar_evento(PartidaAbandonada{});
}

TEST_F(ProtocoloEventosTest, PartidaCerradaPorError) {
    verificar_evento(PartidaCerradaPorError{});
}

TEST_F(ProtocoloEventosTest, JugadoresDeLaPartida) {
    const JugadorDeLaPartida jugador1{1, "Ana", true};
    const JugadorDeLaPartida jugador2{2, "Beto", false};

    verificar_evento(JugadoresDeLaPartida{{jugador1, jugador2}});
}

TEST_F(ProtocoloEventosTest, JugadoresDeLaPartidaVacio) {
    verificar_evento(JugadoresDeLaPartida{});
}

// Estado del mundo

TEST_F(ProtocoloEventosTest, EstadoJuego) {
    const EstadoObjetoDelMundo objeto1{11, "granja", 0x15161718, 0x191A1B1C, TipoObjeto::COFRE};
    const EstadoObjetoDelMundo objeto2{12, "granja", 0x15161718, 0x191A1B1C, TipoObjeto::COFRE};
    const EstadoDrop
        drop1{21, 0x25262728, 0x292A2B2C, TipoItem::PIEDRA, 0x2D2E, Calidad::EXCELENTE};
    const EstadoDrop
        drop2{22, 0x25262728, 0x292A2B2C, TipoItem::PIEDRA, 0x2D2E, Calidad::EXCELENTE};
    const EstadoEnemigo enemigo1{
        31,
        TipoEnemigo::VAMPIRO,
        0x35363738,
        0x393A3B3C,
        0x3D3E,
        0x3F40,
        true,
        false,
        Direccion::IZQUIERDA,
        0x4142,
        true,
        false};
    const EstadoEnemigo enemigo2{
        32,
        TipoEnemigo::VAMPIRO,
        0x35363738,
        0x393A3B3C,
        0x3D3E,
        0x3F40,
        false,
        true,
        Direccion::IZQUIERDA,
        0x4142,
        false,
        true};
    const CasillaTratada casilla1{
        41,
        0x85868788,
        TipoCultivo::CALABAZA,
        ETAPA_DE_CULTIVO_MAXIMA,
        true,
        false,
        true,
        false};
    const CasillaTratada casilla2{
        42,
        0x85868788,
        TipoCultivo::CALABAZA,
        ETAPA_DE_CULTIVO_MAXIMA,
        false,
        true,
        false,
        true};
    const EstadoTrampa trampa1{51, TipoTrampa::PINCHES, 0x55565758, 0x595A5B5C, true};
    const EstadoTrampa trampa2{52, TipoTrampa::PINCHES, 0x55565758, 0x595A5B5C, false};
    const EstadoAnimal
        animal1{61, TipoAnimal::GALLINA, 2, 0x65666768, 0x696A6B6C, Direccion::ARRIBA, true, false};
    const EstadoAnimal
        animal2{62, TipoAnimal::GALLINA, 2, 0x65666768, 0x696A6B6C, Direccion::ARRIBA, false, true};

    verificar_evento(EstadoJuego{
        "granja",
        {jugador_uno(), jugador_dos()},
        {objeto1, objeto2},
        {drop1, drop2},
        {enemigo1, enemigo2},
        {casilla1, casilla2},
        {trampa1, trampa2},
        {animal1, animal2},
        {71, 72},
        {npc_uno(), npc_dos()},
        {},
        4});
}

TEST_F(ProtocoloEventosTest, EstadoJuegoVacio) {
    verificar_evento(EstadoJuego{"granja", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, 0});
}

TEST_F(ProtocoloEventosTest, JugadoresEnSector) {
    verificar_evento(JugadoresEnSector{"granja", {jugador_uno(), jugador_dos()}});
}

TEST_F(ProtocoloEventosTest, JugadoresEnSectorVacio) {
    verificar_evento(JugadoresEnSector{"granja", {}});
}

TEST_F(ProtocoloEventosTest, NpcsEnSector) {
    verificar_evento(NpcsEnSector{"pueblo", {npc_uno(), npc_dos()}});
}

TEST_F(ProtocoloEventosTest, NpcsEnSectorVacio) {
    verificar_evento(NpcsEnSector{"pueblo", {}});
}

TEST_F(ProtocoloEventosTest, LlamasEnSector) {
    const EstadoLlama azul{0x01020304, false, 0x05060708, 0x090A0B0C};
    const EstadoLlama verde{0x11121314, true, 0x15161718, 0x191A1B1C};

    verificar_evento(LlamasEnSector{"mazmorra", {azul, verde}});
}

TEST_F(ProtocoloEventosTest, LlamasEnSectorVacio) {
    verificar_evento(LlamasEnSector{"mazmorra", {}});
}

TEST_F(ProtocoloEventosTest, PosicionesDeLosJugadores) {
    const PosicionDeJugador en_la_granja{0x01020304, "granja", 0x05060708, 0x090A0B0C, 0};
    const PosicionDeJugador en_la_mazmorra{0x11121314, "mina_3", 0x15161718, 0x191A1B1C, 7};

    verificar_evento(PosicionesDeLosJugadores{{en_la_granja, en_la_mazmorra}});
}

TEST_F(ProtocoloEventosTest, PosicionesDeLosJugadoresVacio) {
    verificar_evento(PosicionesDeLosJugadores{});
}

TEST_F(ProtocoloEventosTest, HoraDelJuego) {
    verificar_evento(HoraDelJuego{0xF1F2F3F4, 18, 45, 12, Estacion::OTONO, 0xF5F6, 3, true});
}

// Objetos, drops, enemigos, trampas, tierra y animales

TEST_F(ProtocoloEventosTest, ObjetoColocado) {
    verificar_evento(ObjetoColocado{0x11223344, 0x55667788, 0x99AABBCC, TipoObjeto::CAMA});
}

TEST_F(ProtocoloEventosTest, ObjetoRetirado) {
    verificar_evento(ObjetoRetirado{0x11223344});
}

TEST_F(ProtocoloEventosTest, DropAparecido) {
    verificar_evento(DropAparecido{EstadoDrop{
        0x21222324,
        0x25262728,
        0x292A2B2C,
        TipoItem::PIEDRA,
        0x2D2E,
        Calidad::EXCELENTE}});
}

TEST_F(ProtocoloEventosTest, DropRetirado) {
    verificar_evento(DropRetirado{0x11223344, 0x55667788});
}

TEST_F(ProtocoloEventosTest, EnemigoActualizado) {
    verificar_evento(EnemigoActualizado{EstadoEnemigo{
        0x31323334,
        TipoEnemigo::VAMPIRO,
        0x35363738,
        0x393A3B3C,
        0x3D3E,
        0x3F40,
        true,
        false,
        Direccion::IZQUIERDA,
        0x4142,
        true,
        false}});
}

TEST_F(ProtocoloEventosTest, TrampaActualizada) {
    verificar_evento(TrampaActualizada{
        EstadoTrampa{0x51525354, TipoTrampa::PINCHES, 0x55565758, 0x595A5B5C, true}});
}

TEST_F(ProtocoloEventosTest, TierraCambiada) {
    verificar_evento(TierraCambiada{
        CasillaTratada{
            0x81828384,
            0x85868788,
            TipoCultivo::CALABAZA,
            ETAPA_DE_CULTIVO_MAXIMA,
            true,
            false,
            true,
            false},
        true});
}

TEST_F(ProtocoloEventosTest, AnimalActualizado) {
    verificar_evento(AnimalActualizado{EstadoAnimal{
        0x61626364,
        TipoAnimal::GALLINA,
        2,
        0x65666768,
        0x696A6B6C,
        Direccion::ARRIBA,
        true,
        false}});
}

TEST_F(ProtocoloEventosTest, AnimalRetirado) {
    verificar_evento(AnimalRetirado{0x11223344});
}

TEST_F(ProtocoloEventosTest, VacaOrdeniable) {
    verificar_evento(VacaOrdeniable{0x11223344, true});
}

TEST_F(ProtocoloEventosTest, ObjetoColocadoConMueble) {
    verificar_evento(ObjetoColocado{0x11223344, 0x55667788, 0x99AABBCC, OBJETO_MUEBLE});
}

TEST_F(ProtocoloEventosTest, DropAparecidoConMueble) {
    verificar_evento(DropAparecido{
        EstadoDrop{0x21222324, 0x25262728, 0x292A2B2C, ITEM_MUEBLE, 0x2D2E, Calidad::EXCELENTE}});
}

TEST_F(ProtocoloEventosTest, JugadoresEnSectorConMuebleEnLaMano) {
    EstadoJugador jugador = jugador_uno();
    jugador.item_en_mano = ITEM_MUEBLE;

    verificar_evento(JugadoresEnSector{"granja", {jugador}});
}

// Casillas y descanso

TEST_F(ProtocoloEventosTest, CasillaConsultada) {
    verificar_evento(CasillaConsultada{0x01020304, 0x05060708, 3, 2, MarcaDeCasilla::VALIDA});
}

TEST_F(ProtocoloEventosTest, EstadoDelDescanso) {
    verificar_evento(EstadoDelDescanso{EstadoDeDescanso::ACOSTADO, false, 2, 3, true});
}

TEST_F(ProtocoloEventosTest, PreguntaDeDormir) {
    verificar_evento(PreguntaDeDormir{0x21222324, 0x25262728});
}

// Inventario, cofre y chat

TEST_F(ProtocoloEventosTest, ActualizacionInventario) {
    const RanuraInventario papa{TipoItem::PAPA, 1, Calidad::PERFECTA};
    const RanuraInventario frutilla{TipoItem::FRUTILLA, 2, Calidad::PERFECTA};
    const InventarioJugador inventario{0xD1D2D3D4, 2, {papa, frutilla}};

    verificar_evento(ActualizacionInventario{inventario});
}

TEST_F(ProtocoloEventosTest, ActualizacionCofre) {
    const RanuraInventario piedra{TipoItem::PIEDRA, 3, Calidad::PERFECTA};
    const CofreJugador cofre{0xD5D6D7D8, true, {piedra}};

    verificar_evento(ActualizacionCofre{cofre});
}

TEST_F(ProtocoloEventosTest, MensajeDeChat) {
    verificar_evento(
        MensajeDeChat{CanalDeChat::PRIVADO, "Ana", "Beto", "Hola", ColorDeJugador::VERDE});
}

TEST_F(ProtocoloEventosTest, ActualizacionInventarioConMueble) {
    const RanuraInventario papa{TipoItem::PAPA, 1, Calidad::PERFECTA};
    const RanuraInventario silla{ITEM_MUEBLE, 2, Calidad::PERFECTA};
    const InventarioJugador inventario{0xD1D2D3D4, 2, {papa, silla}};

    verificar_evento(ActualizacionInventario{inventario});
}

// Regalos, mercado y avisos

TEST_F(ProtocoloEventosTest, RegalosPendientes) {
    const RanuraInventario item{TipoItem::PAPA, 0x9192, Calidad::PERFECTA};
    const RegaloRecibido regalo1{"Ana", item};
    const RegaloRecibido regalo2{"Beto", item};

    verificar_evento(RegalosPendientes{{regalo1, regalo2}});
}

TEST_F(ProtocoloEventosTest, RegalosPendientesVacio) {
    verificar_evento(RegalosPendientes{});
}

TEST_F(ProtocoloEventosTest, ActualizacionMercado) {
    const ItemCatalogoInfo item1{
        TipoItem::PAPA,
        TipoAnimal::NINGUNO,
        "Papa",
        0xB1B2B3B4,
        true,
        0xB5B6,
        "Descripción de Papa"};
    const ItemCatalogoInfo item2{
        TipoItem::PAPA,
        TipoAnimal::NINGUNO,
        "Gallina",
        0xB1B2B3B4,
        false,
        0xB5B6,
        "Descripción de Gallina"};

    verificar_evento(ActualizacionMercado{"marnie", {item1, item2}});
}

TEST_F(ProtocoloEventosTest, ActualizacionMercadoVacio) {
    verificar_evento(ActualizacionMercado{"marnie", {}});
}

TEST_F(ProtocoloEventosTest, CerrarVentanaMercado) {
    verificar_evento(CerrarVentanaMercado{});
}

TEST_F(ProtocoloEventosTest, AvisoEnPantalla) {
    verificar_evento(AvisoEnPantalla{"El mercado está cerrado"});
}

TEST_F(ProtocoloEventosTest, ActualizacionMercadoConMueble) {
    const ItemCatalogoInfo silla{
        ITEM_MUEBLE,
        TipoAnimal::NINGUNO,
        "Silla de nogal",
        0xB1B2B3B4,
        true,
        0xB5B6,
        "Descripción de Silla de nogal"};

    verificar_evento(ActualizacionMercado{"robin", {silla}});
}

// NPCs y centro comunitario

TEST_F(ProtocoloEventosTest, DialogoNpc) {
    verificar_evento(DialogoNpc{"marnie", "Hola, granjero"});
}

TEST_F(ProtocoloEventosTest, LoteDelCentro) {
    const RequisitoDeLote requisito1{
        TipoDeRequisito::ITEM,
        TipoItem::PAPA,
        Calidad::EXCELENTE,
        LogroDePelea::ORCOS,
        0xC1C2C3C4,
        0xC5C6C7C8};
    const RequisitoDeLote requisito2{
        TipoDeRequisito::DINERO,
        TipoItem::NINGUNO,
        Calidad::EXCELENTE,
        LogroDePelea::ORCOS,
        0xC1C2C3C4,
        0xC5C6C7C8};

    verificar_evento(LoteDelCentro{
        TipoLote::METALES,
        true,
        "Metales",
        false,
        {requisito1, requisito2},
        "centro",
        0xE1E2E3E4,
        0xE5E6E7E8});
}

TEST_F(ProtocoloEventosTest, CentroTerminado) {
    verificar_evento(CentroTerminado{});
}

// Varios eventos seguidos

// Los eventos deben llegar completos y en el mismo orden en que se enviaron.
TEST_F(ProtocoloEventosTest, VariosEventosEnSecuencia) {
    servidor.enviar_evento(ErrorServidor{CodigoError::NOMBRE_JUGADOR_REPETIDO, "nombre repetido"});
    servidor.enviar_evento(AvisoEnPantalla{"El mercado está cerrado"});
    pasar_eventos_al_cliente();

    EventoServidor recibido;

    ASSERT_TRUE(cliente.recibir_evento(recibido));
    const EventoServidor esperado_primero =
        ErrorServidor{CodigoError::NOMBRE_JUGADOR_REPETIDO, "nombre repetido"};
    EXPECT_EQ(recibido, esperado_primero);

    ASSERT_TRUE(cliente.recibir_evento(recibido));
    const EventoServidor esperado_segundo = AvisoEnPantalla{"El mercado está cerrado"};
    EXPECT_EQ(recibido, esperado_segundo);

    EXPECT_EQ(socket_cliente.bytes_pendientes_de_lectura(), 0u);
}
