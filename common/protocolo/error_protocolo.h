#ifndef ERROR_PROTOCOLO_H
#define ERROR_PROTOCOLO_H

#include <stdexcept>
#include <string>

// Se lanza cuando la conexión se corta a mitad de un mensaje o llega un byte inválido.
class ErrorProtocolo : public std::runtime_error {
public:
    explicit ErrorProtocolo(const std::string& mensaje) : std::runtime_error(mensaje) {
    }
};

#endif
