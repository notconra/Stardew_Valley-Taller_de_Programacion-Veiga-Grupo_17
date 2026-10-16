#include "protocolo.h"

#include <arpa/inet.h>

#include <string>

Protocolo::Protocolo(ISocket& socket_conectado) : socket(socket_conectado) {
}

void Protocolo::enviar_bytes(const void* datos, size_t cantidad) {
    if (socket.sendall(datos, cantidad) == 0) {
        throw ErrorProtocolo("conexión cerrada mientras se enviaba un mensaje");
    }
}

void Protocolo::recibir_bytes(void* buffer, size_t cantidad) {
    if (socket.recvall(buffer, cantidad) == 0) {
        throw ErrorProtocolo("conexión cerrada mientras se recibía un mensaje");
    }
}

void Protocolo::enviar_tipo(TipoMensaje tipo) {
    enviar_u8(static_cast<uint8_t>(tipo));
}

bool Protocolo::intentar_recibir_tipo(TipoMensaje& tipo) {
    uint8_t valor = 0;
    if (socket.recvall(&valor, sizeof(valor)) == 0) {
        return false;
    }
    tipo = static_cast<TipoMensaje>(valor);
    return true;
}

void Protocolo::enviar_u8(uint8_t valor) {
    enviar_bytes(&valor, sizeof(valor));
}

uint8_t Protocolo::recibir_u8() {
    uint8_t valor = 0;
    recibir_bytes(&valor, sizeof(valor));
    return valor;
}

void Protocolo::enviar_u16(uint16_t valor) {
    uint16_t bytes_en_formato_red = htons(valor);
    enviar_bytes(&bytes_en_formato_red, sizeof(bytes_en_formato_red));
}

uint16_t Protocolo::recibir_u16() {
    uint16_t bytes_en_formato_red = 0;
    recibir_bytes(&bytes_en_formato_red, sizeof(bytes_en_formato_red));
    return ntohs(bytes_en_formato_red);
}

void Protocolo::enviar_u32(uint32_t valor) {
    uint32_t bytes_en_formato_red = htonl(valor);
    enviar_bytes(&bytes_en_formato_red, sizeof(bytes_en_formato_red));
}

uint32_t Protocolo::recibir_u32() {
    uint32_t bytes_en_formato_red = 0;
    recibir_bytes(&bytes_en_formato_red, sizeof(bytes_en_formato_red));
    return ntohl(bytes_en_formato_red);
}

void Protocolo::enviar_largo(size_t largo) {
    if (largo > UINT16_MAX) {
        throw ErrorProtocolo("se excedió el largo máximo permitido por el protocolo");
    }
    enviar_u16(static_cast<uint16_t>(largo));
}

void Protocolo::enviar_texto(const std::string& texto) {
    enviar_largo(texto.size());
    if (!texto.empty()) {
        enviar_bytes(texto.data(), texto.size());
    }
}

std::string Protocolo::recibir_texto() {
    uint16_t largo = recibir_u16();
    std::string texto(largo, '\0');
    if (largo > 0) {
        recibir_bytes(texto.data(), largo);
    }
    return texto;
}
