#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "../client/red/protocolo_cliente.h"
#include "socket_falso.h"

// Entradas inválidas que el cliente debe rechazar: truncamiento, tipos desconocidos, enums fuera
// de rango y cierre limpio de la conexión.
class ProtocoloClienteErroresTest : public ::testing::Test {
protected:
    SocketFalso socket;
    ProtocoloCliente protocolo{socket};
};

TEST_F(ProtocoloClienteErroresTest, RecibirEventoConConexionCerradaDevuelveFalso) {
    EventoServidor evento;
    EXPECT_FALSE(protocolo.recibir_evento(evento));
}

TEST_F(ProtocoloClienteErroresTest, RecibirEventoConTipoDeComandoLanza) {
    socket.encolar({0x01});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirEventoConTipoDesconocidoLanza) {
    socket.encolar({0xFF});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirEventoTruncadoLanza) {
    socket.encolar({0x83, 0x00, 0x05, 'a', 'b'});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirEventoConCamposVaciosLanza) {
    socket.encolar({0x81});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirErrorConCodigoCeroLanza) {
    socket.encolar({0x81, 0x00, 0x00, 0x00});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirErrorConCodigoSieteLanza) {
    socket.encolar({0x81, 0x07, 0x00, 0x00});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirEntradaConBooleanoInvalidoLanza) {
    const std::vector<uint8_t> bytes =
        {0x83, 0x00, 0x06, 'A', 'B', 'C', '1', '2', '3', 0x00, 0x00, 0x00, 0x01, 0x02};
    socket.encolar(bytes);

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirJugadoresConDireccionInvalidaLanza) {
    const std::vector<uint8_t> bytes = {
        0x8A, 0x00, 0x06, 'g', 'r', 'a', 'n', 'j',  'a',  0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x06, 'g',  'r',  'a', 'n', 'j', 'a', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    socket.encolar(bytes);

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirEnemigoConDireccionInvalidaLanza) {
    const std::vector<uint8_t> bytes = {0x8F, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
                                        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03,
                                        0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 0x04};
    socket.encolar(bytes);

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirNpcsConDireccionInvalidaLanza) {
    const std::vector<uint8_t> bytes = {0xA4, 0x00, 0x06, 'p',  'u',  'e',  'b',  'l',  'o',
                                        0x00, 0x01, 0x00, 0x06, 'm',  'a',  'r',  'n',  'i',
                                        'e',  0x00, 0x06, 'p',  'u',  'e',  'b',  'l',  'o',
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    socket.encolar(bytes);

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirChatConCanalInvalidoLanza) {
    socket.encolar({0x91, 0x03});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirChatConColorInvalidoLanza) {
    socket.encolar({0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirTierraConCultivoInvalidoLanza) {
    socket.encolar({0x90, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x05});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirTierraConEtapaInvalidaLanza) {
    socket.encolar({0x90, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirHoraConEstacionInvalidaLanza) {
    socket.encolar({0x92, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x01, 0x04});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirHoraConDiaDeLaSemanaInvalidoLanza) {
    socket.encolar({0x92, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x01, 0x00, 0x00, 0x01, 0x07});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirDescansoConEstadoInvalidoLanza) {
    socket.encolar({0x94, 0x03});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirCasillaConsultadaConMarcaInvalidaLanza) {
    socket.encolar({0x93, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x03});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirLoteDelCentroConLoteInvalidoLanza) {
    socket.encolar({0xA6, 0x05});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}

TEST_F(ProtocoloClienteErroresTest, RecibirAvisoConTextoExcesivoLanza) {
    socket.encolar({0x98, 0x04, 0x01});

    EventoServidor evento;
    EXPECT_THROW(protocolo.recibir_evento(evento), ErrorProtocolo);
}
