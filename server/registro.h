#ifndef REGISTRO_H
#define REGISTRO_H

#include <iostream>
#include <mutex>
#include <string>

// Logger temporal hasta q tengamos uno mas lindo o confirmemos q anda todo y lo volemos.
// El mutex evita que los hilos mezclen sus líneas.
inline void registrar(const std::string& texto) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "[servidor] " << texto << std::endl;
}

#endif
