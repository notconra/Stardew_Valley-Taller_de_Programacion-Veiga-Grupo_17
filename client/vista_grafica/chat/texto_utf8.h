#ifndef TEXTO_UTF8_H
#define TEXTO_UTF8_H

#include <cstddef>
#include <string>

inline size_t longitud_de_caracter_utf8(const std::string& texto, size_t posicion) {
    const unsigned char c = static_cast<unsigned char>(texto[posicion]);
    return c < 0xC0 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : c < 0xF8 ? 4 : 1;
}

inline bool es_continuacion_de_utf8(char caracter) {
    return (static_cast<unsigned char>(caracter) & 0xC0) == 0x80;
}

#endif
