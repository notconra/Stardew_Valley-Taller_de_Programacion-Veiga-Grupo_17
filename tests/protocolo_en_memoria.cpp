#include "protocolo_en_memoria.h"

#include <vector>

AparienciaPersonaje apariencia_de_prueba() {
    return AparienciaPersonaje{true, 3, {11, 22, 33}, 4, {44, 55, 66}, 5, 6, {77, 88, 99}, 321};
}

void ProtocoloEnMemoriaTest::pasar_comandos_al_servidor() {
    pasar_bytes_nuevos(socket_cliente, socket_servidor, bytes_pasados_al_servidor);
}

void ProtocoloEnMemoriaTest::pasar_eventos_al_cliente() {
    pasar_bytes_nuevos(socket_servidor, socket_cliente, bytes_pasados_al_cliente);
}

void ProtocoloEnMemoriaTest::verificar_comando(const ComandoCliente& comando) {
    cliente.enviar_comando(comando);
    pasar_comandos_al_servidor();

    ComandoCliente recibido;
    ASSERT_TRUE(servidor.recibir_comando(recibido));
    EXPECT_EQ(recibido, comando);
    EXPECT_EQ(socket_servidor.bytes_pendientes_de_lectura(), 0u);
}

void ProtocoloEnMemoriaTest::verificar_evento(const EventoServidor& evento) {
    servidor.enviar_evento(evento);
    pasar_eventos_al_cliente();

    EventoServidor recibido;
    ASSERT_TRUE(cliente.recibir_evento(recibido));
    EXPECT_EQ(recibido, evento);
    EXPECT_EQ(socket_cliente.bytes_pendientes_de_lectura(), 0u);
}

void ProtocoloEnMemoriaTest::pasar_bytes_nuevos(
    const SocketFalso& origen, SocketFalso& destino, size_t& bytes_ya_pasados) {
    const std::vector<uint8_t>& enviados = origen.bytes_enviados();
    const std::vector<uint8_t> nuevos(
        enviados.begin() + static_cast<std::ptrdiff_t>(bytes_ya_pasados),
        enviados.end());
    destino.encolar(nuevos);
    bytes_ya_pasados = enviados.size();
}
