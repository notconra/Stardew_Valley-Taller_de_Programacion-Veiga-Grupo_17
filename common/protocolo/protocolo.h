#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "../isocket.h"
#include "error_protocolo.h"
#include "tipo_mensaje.h"

// ProtocoloServidor y ProtocoloCliente heredan de acá para reutilizar
// estas primitivas y exponen enviar_algo() y recibir_algo() de mensajes completos.
//
// Formato: los números viajan en big endian (orden de red) y un texto viaja como su largo (u16)
// seguido de sus bytes.
//
// Errores: si la conexión se corta a mitad de un campo, el socket lanza LibError; si se corta
// entre dos campos de un mismo mensaje, o el mensaje es inválido, se lanza ErrorProtocolo. En
// cualquiera de los dos casos la conexión debe darse por perdida, porque puede haber quedado un
// mensaje por la mitad.
class Protocolo {
private:
    // Envían/reciben bytes de un campo que está a mitad de un mensaje, por lo que un socket
    // cerrado (sendall/recvall devuelven 0) es un error. Requieren cantidad > 0.
    void enviar_bytes(const void* datos, size_t cantidad);
    void recibir_bytes(void* destino, size_t cantidad);

protected:
    ISocket& socket;

    explicit Protocolo(ISocket& socket_conectado);

    // Lee el primer byte de un mensaje. Devuelve false si se cerró la conexión justo
    // antes (cierre limpio), y es el único lugar donde eso no es un error.
    bool intentar_recibir_tipo(TipoMensaje& tipo);
    void enviar_tipo(TipoMensaje tipo);

    void enviar_u8(uint8_t valor);
    uint8_t recibir_u8();

    void enviar_u16(uint16_t valor);
    uint16_t recibir_u16();

    void enviar_u32(uint32_t valor);
    uint32_t recibir_u32();

    // Envía una cantidad de elementos como u16. Lanza ErrorProtocolo si no entra en 16 bits.
    void enviar_largo(size_t largo);

    void enviar_texto(const std::string& texto);
    std::string recibir_texto();

public:
    Protocolo(const Protocolo&) = delete;
    Protocolo& operator=(const Protocolo&) = delete;
};

#endif
