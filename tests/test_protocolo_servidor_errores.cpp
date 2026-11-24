#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "../server/red/protocolo_servidor.h"
#include "socket_falso.h"

// Entradas inválidas que el servidor debe rechazar: truncamiento, tipos desconocidos, enums fuera
// de rango y cierre limpio de la conexión.
class ProtocoloServidorErroresTest : public ::testing::Test {
protected:
    SocketFalso socket;
    ProtocoloServidor protocolo{socket};
};

TEST_F(ProtocoloServidorErroresTest, RecibirComandoConConexionCerradaDevuelveFalso) {
    ComandoCliente comando;
    EXPECT_FALSE(protocolo.recibir_comando(comando));
}

TEST_F(ProtocoloServidorErroresTest, RecibirComandoConTipoDeEventoLanza) {
    socket.encolar({0x81});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirComandoConTipoDesconocidoLanza) {
    socket.encolar({0xFF});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirComandoTruncadoLanza) {
    socket.encolar({0x01, 0x11, 0x22});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirComandoSinCamposLanza) {
    socket.encolar({0x07});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirMoverseConHorizontalMenorLanza) {
    socket.encolar({0x07, 0xFE});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirMoverseConHorizontalMayorLanza) {
    socket.encolar({0x07, 0x02});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirMoverseConVerticalMenorLanza) {
    socket.encolar({0x07, 0x00, 0xFE});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirMoverseConVerticalMayorLanza) {
    socket.encolar({0x07, 0x00, 0x02});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirUsarItemConBooleanoInvalidoLanza) {
    socket.encolar({0x0A, 0x00, 0x00, 0x02});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirDonarAlLoteConLoteInvalidoLanza) {
    socket.encolar({0x1E, CANTIDAD_DE_LOTES});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirDonarMonedasConLoteInvalidoLanza) {
    socket.encolar({0x1F, CANTIDAD_DE_LOTES});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirChatConTextoExcesivoLanza) {
    socket.encolar({0x10, 0x04, 0x01});

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}

TEST_F(ProtocoloServidorErroresTest, RecibirCrearPartidaTruncadaLanza) {
    const std::vector<uint8_t> bytes =
        {0x03, 0x00, 0x09, 'L', 'a', ' ', 'g', 'r', 'a', 'n', 'j', 'a', 0x00, 0x0A, 'a', 'b'};
    socket.encolar(bytes);

    ComandoCliente comando;
    EXPECT_THROW(protocolo.recibir_comando(comando), ErrorProtocolo);
}
