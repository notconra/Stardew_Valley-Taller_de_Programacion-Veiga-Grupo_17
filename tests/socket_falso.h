#ifndef TESTS_SOCKET_FALSO_H
#define TESTS_SOCKET_FALSO_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "../common/isocket.h"

// Socket en memoria para probar el protocolo sin abrir conexiones reales.
// Implementa ISocket igual que el socket que usamos en el protocolo.
// Permite encolar los bytes que va a leer el protocolo y guarda todo lo que se envía.
class SocketFalso : public ISocket {
public:
    void encolar(const std::vector<uint8_t>& bytes);

    void encolar(const void* datos, unsigned int cantidad);

    void fallar_en_la_proxima_lectura();

    void fallar_en_el_proximo_envio();

    const std::vector<uint8_t>& bytes_enviados() const;

    size_t bytes_pendientes_de_lectura() const;

    int cantidad_de_apagados() const;

    int ultimo_apagado() const;

    int sendsome(const void* datos, unsigned int cantidad) override;

    int recvsome(void* destino, unsigned int cantidad) override;

    int sendall(const void* datos, unsigned int cantidad) override;

    int recvall(void* destino, unsigned int cantidad) override;

    void shutdown(int how) override;

    int close() override;

private:
    void guardar_enviados(const void* datos, unsigned int cantidad);

    std::vector<uint8_t> bytes_para_recibir;
    size_t posicion_de_lectura = 0;
    std::vector<uint8_t> enviados;
    bool fallar_lectura = false;
    bool fallar_envio = false;
    int apagados = 0;
    int ultimo_tipo_de_apagado = 0;
};

#endif
