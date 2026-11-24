#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../common/protocolo/protocolo.h"
#include "socket_falso.h"

// Primitivas base del protocolo, u8, i8, u16, u32, booleanos, textos, apariencias, big endian y
// largos máximos.
// Expone las primitivas protegidas de Protocolo para probarlas de forma aislada.
class ProtocoloDePrueba : public Protocolo {
public:
    explicit ProtocoloDePrueba(ISocket& socket_conectado) : Protocolo(socket_conectado) {
    }

    using Protocolo::enviar_apariencia;
    using Protocolo::enviar_booleano;
    using Protocolo::enviar_i8;
    using Protocolo::enviar_largo;
    using Protocolo::enviar_texto;
    using Protocolo::enviar_tipo;
    using Protocolo::enviar_u16;
    using Protocolo::enviar_u32;
    using Protocolo::enviar_u8;
    using Protocolo::intentar_recibir_tipo;
    using Protocolo::recibir_apariencia;
    using Protocolo::recibir_booleano;
    using Protocolo::recibir_i8;
    using Protocolo::recibir_texto;
    using Protocolo::recibir_u16;
    using Protocolo::recibir_u32;
    using Protocolo::recibir_u8;
};

class ProtocoloTest : public ::testing::Test {
protected:
    SocketFalso socket;
    ProtocoloDePrueba protocolo{socket};
};

TEST_F(ProtocoloTest, EnviaU8) {
    protocolo.enviar_u8(0xAB);

    const std::vector<uint8_t> esperado = {0xAB};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeU8) {
    socket.encolar({0xCD});

    EXPECT_EQ(protocolo.recibir_u8(), 0xCD);
}

TEST_F(ProtocoloTest, EnviaI8Negativo) {
    protocolo.enviar_i8(-2);

    const std::vector<uint8_t> esperado = {0xFE};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeI8Negativo) {
    socket.encolar({0xFE});

    EXPECT_EQ(protocolo.recibir_i8(), -2);
}

TEST_F(ProtocoloTest, EnviaI8Minimo) {
    protocolo.enviar_i8(-128);

    const std::vector<uint8_t> esperado = {0x80};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeI8Minimo) {
    socket.encolar({0x80});

    EXPECT_EQ(protocolo.recibir_i8(), -128);
}

TEST_F(ProtocoloTest, EnviaBooleanoVerdadero) {
    protocolo.enviar_booleano(true);

    const std::vector<uint8_t> esperado = {0x01};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, EnviaBooleanoFalso) {
    protocolo.enviar_booleano(false);

    const std::vector<uint8_t> esperado = {0x00};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeBooleanoVerdadero) {
    socket.encolar({0x01});

    EXPECT_TRUE(protocolo.recibir_booleano());
}

TEST_F(ProtocoloTest, RecibeBooleanoFalso) {
    socket.encolar({0x00});

    EXPECT_FALSE(protocolo.recibir_booleano());
}

TEST_F(ProtocoloTest, RecibeBooleanoInvalidoLanza) {
    socket.encolar({0x02});

    EXPECT_THROW(protocolo.recibir_booleano(), ErrorProtocolo);
}

TEST_F(ProtocoloTest, EnviaU16EnOrdenDeRed) {
    protocolo.enviar_u16(0x1234);

    const std::vector<uint8_t> esperado = {0x12, 0x34};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeU16EnOrdenDeRed) {
    socket.encolar({0x12, 0x34});

    EXPECT_EQ(protocolo.recibir_u16(), 0x1234);
}

TEST_F(ProtocoloTest, EnviaU32EnOrdenDeRed) {
    protocolo.enviar_u32(0x12345678);

    const std::vector<uint8_t> esperado = {0x12, 0x34, 0x56, 0x78};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeU32EnOrdenDeRed) {
    socket.encolar({0x12, 0x34, 0x56, 0x78});

    EXPECT_EQ(protocolo.recibir_u32(), 0x12345678u);
}

TEST_F(ProtocoloTest, EnviaLargoCero) {
    protocolo.enviar_largo(0);

    const std::vector<uint8_t> esperado = {0x00, 0x00};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, EnviaLargoMaximo) {
    protocolo.enviar_largo(UINT16_MAX);

    const std::vector<uint8_t> esperado = {0xFF, 0xFF};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, EnviaLargoExcedidoLanza) {
    EXPECT_THROW(protocolo.enviar_largo(static_cast<size_t>(UINT16_MAX) + 1), ErrorProtocolo);
    EXPECT_TRUE(socket.bytes_enviados().empty());
}

TEST_F(ProtocoloTest, EnviaTextoVacio) {
    protocolo.enviar_texto("");

    const std::vector<uint8_t> esperado = {0x00, 0x00};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, EnviaTexto) {
    protocolo.enviar_texto("granja");

    const std::vector<uint8_t> esperado = {0x00, 0x06, 'g', 'r', 'a', 'n', 'j', 'a'};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, EnviaTextoConBytesNulos) {
    const std::string texto("a\0b", 3);
    protocolo.enviar_texto(texto);

    const std::vector<uint8_t> esperado = {0x00, 0x03, 'a', 0x00, 'b'};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, EnviaTextoMaximo) {
    const std::string texto(1024, 'x');
    protocolo.enviar_texto(texto);

    ASSERT_EQ(socket.bytes_enviados().size(), 1026u);
    EXPECT_EQ(socket.bytes_enviados()[0], 0x04);
    EXPECT_EQ(socket.bytes_enviados()[1], 0x00);
}

TEST_F(ProtocoloTest, EnviaTextoExcedidoLanza) {
    EXPECT_THROW(protocolo.enviar_texto(std::string(1025, 'x')), ErrorProtocolo);
    EXPECT_TRUE(socket.bytes_enviados().empty());
}

TEST_F(ProtocoloTest, RecibeTextoVacio) {
    socket.encolar({0x00, 0x00});

    EXPECT_EQ(protocolo.recibir_texto(), "");
}

TEST_F(ProtocoloTest, RecibeTexto) {
    socket.encolar({0x00, 0x06, 'g', 'r', 'a', 'n', 'j', 'a'});

    EXPECT_EQ(protocolo.recibir_texto(), "granja");
}

TEST_F(ProtocoloTest, RecibeTextoMaximo) {
    const std::string texto(1024, 'x');
    std::vector<uint8_t> bytes = {0x04, 0x00};
    bytes.insert(bytes.end(), texto.begin(), texto.end());
    socket.encolar(bytes);

    EXPECT_EQ(protocolo.recibir_texto(), texto);
}

TEST_F(ProtocoloTest, RecibeTextoExcedidoLanza) {
    socket.encolar({0x04, 0x01});

    EXPECT_THROW(protocolo.recibir_texto(), ErrorProtocolo);
}

TEST_F(ProtocoloTest, EnviaApariencia) {
    const AparienciaPersonaje
        apariencia{true, 3, {11, 22, 33}, 4, {44, 55, 66}, 5, 6, {77, 88, 99}, 321};
    protocolo.enviar_apariencia(apariencia);

    const std::vector<uint8_t> esperado = {
        0x01,
        0x03,
        0x0B,
        0x16,
        0x21,
        0x04,
        0x2C,
        0x37,
        0x42,
        0x05,
        0x06,
        0x4D,
        0x58,
        0x63,
        0x01,
        0x41};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, RecibeApariencia) {
    const AparienciaPersonaje
        apariencia{true, 3, {11, 22, 33}, 4, {44, 55, 66}, 5, 6, {77, 88, 99}, 321};
    const std::vector<uint8_t> bytes = {
        0x01,
        0x03,
        0x0B,
        0x16,
        0x21,
        0x04,
        0x2C,
        0x37,
        0x42,
        0x05,
        0x06,
        0x4D,
        0x58,
        0x63,
        0x01,
        0x41};
    socket.encolar(bytes);

    EXPECT_EQ(protocolo.recibir_apariencia(), apariencia);
}

TEST_F(ProtocoloTest, RecibeAparienciaConSombreroNegativo) {
    const AparienciaPersonaje
        apariencia{true, 3, {11, 22, 33}, 4, {44, 55, 66}, 5, 6, {77, 88, 99}, -1};
    const std::vector<uint8_t> bytes = {
        0x01,
        0x03,
        0x0B,
        0x16,
        0x21,
        0x04,
        0x2C,
        0x37,
        0x42,
        0x05,
        0x06,
        0x4D,
        0x58,
        0x63,
        0xFF,
        0xFF};
    socket.encolar(bytes);

    EXPECT_EQ(protocolo.recibir_apariencia(), apariencia);
}

TEST_F(ProtocoloTest, EnviaTipo) {
    protocolo.enviar_tipo(TipoMensaje::MOVERSE);

    const std::vector<uint8_t> esperado = {0x07};
    EXPECT_EQ(socket.bytes_enviados(), esperado);
}

TEST_F(ProtocoloTest, IntentarRecibirTipo) {
    socket.encolar({0x07});

    TipoMensaje tipo{};
    ASSERT_TRUE(protocolo.intentar_recibir_tipo(tipo));
    EXPECT_EQ(tipo, TipoMensaje::MOVERSE);
}

TEST_F(ProtocoloTest, IntentarRecibirTipoSinBytesDevuelveFalso) {
    TipoMensaje tipo{};
    EXPECT_FALSE(protocolo.intentar_recibir_tipo(tipo));
}

TEST_F(ProtocoloTest, EnviarConSocketCerradoLanza) {
    socket.fallar_en_el_proximo_envio();

    EXPECT_THROW(protocolo.enviar_u8(1), ErrorProtocolo);
}

TEST_F(ProtocoloTest, RecibirConSocketCerradoLanza) {
    EXPECT_THROW(protocolo.recibir_u8(), ErrorProtocolo);
}

TEST_F(ProtocoloTest, RecibirCampoParcialLanza) {
    socket.encolar({0x12});

    EXPECT_THROW(protocolo.recibir_u16(), ErrorProtocolo);
}
