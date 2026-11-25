#include <gtest/gtest.h>
#include <sys/socket.h>

#include <cstdint>

#include "protocolo_en_memoria.h"

// Comandos que viajan del cliente al servidor.
// Cada test envía un comando con el ProtocoloCliente y verifica que el ProtocoloServidor reciba
// exactamente el mismo comando.
class ProtocoloComandosTest : public ProtocoloEnMemoriaTest {};

// Partidas

TEST_F(ProtocoloComandosTest, Identificarse) {
    verificar_comando(Identificarse{0x11223344});
}

TEST_F(ProtocoloComandosTest, ListarPartidas) {
    verificar_comando(ListarPartidas{});
}

TEST_F(ProtocoloComandosTest, CrearPartida) {
    verificar_comando(CrearPartida{"La granja", "Ana", apariencia_de_prueba()});
}

TEST_F(ProtocoloComandosTest, UnirsePartida) {
    verificar_comando(UnirsePartida{"ABC123", "Beto", apariencia_de_prueba()});
}

TEST_F(ProtocoloComandosTest, RetomarPartida) {
    verificar_comando(RetomarPartida{"ABC123"});
}

TEST_F(ProtocoloComandosTest, EliminarPartida) {
    verificar_comando(EliminarPartida{"XYZ789"});
}

TEST_F(ProtocoloComandosTest, ListarMundos) {
    verificar_comando(ListarMundos{});
}

TEST_F(ProtocoloComandosTest, VerificarCodigoPartida) {
    verificar_comando(VerificarCodigoPartida{"P0"});
}

TEST_F(ProtocoloComandosTest, ActivarMultijugador) {
    verificar_comando(ActivarMultijugador{});
}

TEST_F(ProtocoloComandosTest, SalirDePartida) {
    verificar_comando(SalirDePartida{});
}

TEST_F(ProtocoloComandosTest, GuardarPartida) {
    verificar_comando(GuardarPartida{});
}

// Movimiento

TEST_F(ProtocoloComandosTest, Moverse) {
    verificar_comando(Moverse{-1, 1});
}

TEST_F(ProtocoloComandosTest, MoverseConTodosLosEjesValidos) {
    for (int8_t horizontal = -1; horizontal <= 1; ++horizontal) {
        for (int8_t vertical = -1; vertical <= 1; ++vertical) {
            verificar_comando(Moverse{horizontal, vertical});
        }
    }
}

// Inventario, barra y cofre

TEST_F(ProtocoloComandosTest, SeleccionarRanuraDeLaBarra) {
    verificar_comando(SeleccionarRanuraDeLaBarra{7});
}

TEST_F(ProtocoloComandosTest, UsarItem) {
    verificar_comando(UsarItem{ModoUso::SECUNDARIO, 0xFE, true, 0x01020304, 0x05060708});
}

TEST_F(ProtocoloComandosTest, UsarItemEnModoPrincipal) {
    verificar_comando(UsarItem{ModoUso::PRINCIPAL, 1, false, 0, 0});
}

TEST_F(ProtocoloComandosTest, MoverItemEntreRanuras) {
    verificar_comando(MoverItemEntreRanuras{2, 5, 0x0A0B});
}

TEST_F(ProtocoloComandosTest, MoverItemConCofre) {
    verificar_comando(MoverItemConCofre{3, 0xFE, 0x0C0D, true});
}

TEST_F(ProtocoloComandosTest, CerrarCofre) {
    verificar_comando(CerrarCofre{});
}

TEST_F(ProtocoloComandosTest, SolicitarInventario) {
    verificar_comando(SolicitarInventario{});
}

TEST_F(ProtocoloComandosTest, SoltarItem) {
    verificar_comando(SoltarItem{4, 0x0E0F});
}

// Casillas, chat y confirmaciones

TEST_F(ProtocoloComandosTest, ConsultarCasilla) {
    verificar_comando(ConsultarCasilla{0x10111213, 0x14151617});
}

TEST_F(ProtocoloComandosTest, DarElOk) {
    verificar_comando(DarElOk{});
}

TEST_F(ProtocoloComandosTest, EnviarMensajeDeChat) {
    verificar_comando(EnviarMensajeDeChat{"Hola a todos"});
}

// Mercado

TEST_F(ProtocoloComandosTest, ComprarEnMercado) {
    verificar_comando(ComprarEnMercado{0x1819, 0x1A1B});
}

TEST_F(ProtocoloComandosTest, VenderEnMercado) {
    verificar_comando(VenderEnMercado{6, 0x1C1D});
}

TEST_F(ProtocoloComandosTest, ComprarAnimalMercado) {
    verificar_comando(ComprarAnimalMercado{TipoAnimal::GALLINA, 0x1E1F});
}

TEST_F(ProtocoloComandosTest, VenderAnimalMercado) {
    verificar_comando(VenderAnimalMercado{TipoAnimal::VACA, 0x2021});
}

TEST_F(ProtocoloComandosTest, CerrarMercado) {
    verificar_comando(CerrarMercado{});
}

// Regalos, NPCs y centro comunitario

TEST_F(ProtocoloComandosTest, EnviarRegalo) {
    verificar_comando(EnviarRegalo{0x22232425, 8, 0x2627});
}

TEST_F(ProtocoloComandosTest, AbrirRegalos) {
    verificar_comando(AbrirRegalos{});
}

TEST_F(ProtocoloComandosTest, RechazarRegalo) {
    verificar_comando(RechazarRegalo{5});
}

TEST_F(ProtocoloComandosTest, HablarConNpc) {
    verificar_comando(HablarConNpc{"marnie"});
}

TEST_F(ProtocoloComandosTest, DonarAlLote) {
    verificar_comando(DonarAlLote{TipoLote::METALES, 9});
}

TEST_F(ProtocoloComandosTest, DonarAlLoteConTodosLosLotesValidos) {
    for (uint8_t lote = 0; lote < CANTIDAD_DE_LOTES; ++lote) {
        verificar_comando(DonarAlLote{static_cast<TipoLote>(lote), 3});
    }
}

TEST_F(ProtocoloComandosTest, DonarMonedasAlLote) {
    verificar_comando(DonarMonedasAlLote{TipoLote::DINERO});
}

TEST_F(ProtocoloComandosTest, DonarMonedasAlLoteConTodosLosLotesValidos) {
    for (uint8_t lote = 0; lote < CANTIDAD_DE_LOTES; ++lote) {
        verificar_comando(DonarMonedasAlLote{static_cast<TipoLote>(lote)});
    }
}

// Dormir

TEST_F(ProtocoloComandosTest, Dormir) {
    verificar_comando(Dormir{0x2A2B2C2D, 0x2E2F3031});
}

TEST_F(ProtocoloComandosTest, Levantarse) {
    verificar_comando(Levantarse{});
}

// Varios comandos seguidos y cierre

// Los comandos deben llegar completos y en el mismo orden en que se enviaron.
TEST_F(ProtocoloComandosTest, VariosComandosEnSecuencia) {
    cliente.enviar_comando(Identificarse{0x11223344});
    cliente.enviar_comando(Moverse{-1, 1});
    pasar_comandos_al_servidor();

    ComandoCliente recibido;

    ASSERT_TRUE(servidor.recibir_comando(recibido));
    const ComandoCliente esperado_primero = Identificarse{0x11223344};
    EXPECT_EQ(recibido, esperado_primero);

    ASSERT_TRUE(servidor.recibir_comando(recibido));
    const ComandoCliente esperado_segundo = Moverse{-1, 1};
    EXPECT_EQ(recibido, esperado_segundo);

    EXPECT_EQ(socket_servidor.bytes_pendientes_de_lectura(), 0u);
}

TEST_F(ProtocoloComandosTest, CerrarApagaElSocket) {
    cliente.cerrar();

    EXPECT_EQ(socket_cliente.cantidad_de_apagados(), 1);
    EXPECT_EQ(socket_cliente.ultimo_apagado(), SHUT_RDWR);
}
