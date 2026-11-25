#include "lector_binario.h"

#include <stdexcept>
#include <string>

LectorBinario::LectorBinario(std::istream& stream_entrada) : stream(stream_entrada) {
}

void LectorBinario::leer_bytes(void* buffer, size_t cantidad) {
    stream.read(reinterpret_cast<char*>(buffer), cantidad);
    if (!stream) {
        throw std::runtime_error(
            "Error al leer datos del stream: fin inesperado o archivo corrupto");
    }
}

bool LectorBinario::leer_bool() {
    return leer_u8() != 0;
}

uint8_t LectorBinario::leer_u8() {
    uint8_t valor = 0;
    leer_bytes(&valor, sizeof(valor));
    return valor;
}

uint16_t LectorBinario::leer_u16() {
    uint16_t valor = 0;
    leer_bytes(&valor, sizeof(valor));
    return valor;
}

uint32_t LectorBinario::leer_u32() {
    uint32_t valor = 0;
    leer_bytes(&valor, sizeof(valor));
    return valor;
}

double LectorBinario::leer_double() {
    double valor = 0.0;
    leer_bytes(&valor, sizeof(valor));
    return valor;
}

std::string LectorBinario::leer_texto() {
    uint16_t largo = leer_u16();
    std::string texto(largo, '\0');
    if (largo > 0) {
        leer_bytes(texto.data(), largo);
    }
    return texto;
}
