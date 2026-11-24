#include "socket_falso.h"

#include <cstring>
#include <vector>

void SocketFalso::encolar(const std::vector<uint8_t>& bytes) {
    bytes_para_recibir.insert(bytes_para_recibir.end(), bytes.begin(), bytes.end());
}

void SocketFalso::encolar(const void* datos, unsigned int cantidad) {
    const uint8_t* inicio = static_cast<const uint8_t*>(datos);
    bytes_para_recibir.insert(bytes_para_recibir.end(), inicio, inicio + cantidad);
}

void SocketFalso::fallar_en_la_proxima_lectura() {
    fallar_lectura = true;
}

void SocketFalso::fallar_en_el_proximo_envio() {
    fallar_envio = true;
}

const std::vector<uint8_t>& SocketFalso::bytes_enviados() const {
    return enviados;
}

size_t SocketFalso::bytes_pendientes_de_lectura() const {
    return bytes_para_recibir.size() - posicion_de_lectura;
}

int SocketFalso::cantidad_de_apagados() const {
    return apagados;
}

int SocketFalso::ultimo_apagado() const {
    return ultimo_tipo_de_apagado;
}

int SocketFalso::sendsome(const void* datos, unsigned int cantidad) {
    if (fallar_envio) {
        fallar_envio = false;
        return 0;
    }
    guardar_enviados(datos, cantidad);
    return static_cast<int>(cantidad);
}

int SocketFalso::recvsome(void* destino, unsigned int cantidad) {
    if (fallar_lectura) {
        fallar_lectura = false;
        return 0;
    }
    const size_t disponibles = bytes_pendientes_de_lectura();
    const size_t a_copiar = disponibles < cantidad ? disponibles : cantidad;
    if (a_copiar == 0) {
        return 0;
    }
    std::memcpy(destino, bytes_para_recibir.data() + posicion_de_lectura, a_copiar);
    posicion_de_lectura += a_copiar;
    return static_cast<int>(a_copiar);
}

int SocketFalso::sendall(const void* datos, unsigned int cantidad) {
    if (fallar_envio) {
        fallar_envio = false;
        return 0;
    }
    guardar_enviados(datos, cantidad);
    return static_cast<int>(cantidad);
}

int SocketFalso::recvall(void* destino, unsigned int cantidad) {
    if (fallar_lectura) {
        fallar_lectura = false;
        return 0;
    }
    if (bytes_pendientes_de_lectura() < cantidad) {
        return 0;
    }
    std::memcpy(destino, bytes_para_recibir.data() + posicion_de_lectura, cantidad);
    posicion_de_lectura += cantidad;
    return static_cast<int>(cantidad);
}

void SocketFalso::shutdown(int how) {
    ++apagados;
    ultimo_tipo_de_apagado = how;
}

int SocketFalso::close() {
    return 0;
}

void SocketFalso::guardar_enviados(const void* datos, unsigned int cantidad) {
    const uint8_t* inicio = static_cast<const uint8_t*>(datos);
    enviados.insert(enviados.end(), inicio, inicio + cantidad);
}
