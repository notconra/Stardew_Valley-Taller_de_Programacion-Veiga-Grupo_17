#include "escritor_binario.h"

#include <limits>
#include <stdexcept>
#include <string>

EscritorBinario::EscritorBinario(std::ostream& stream_salida) : stream(stream_salida) {
}

void EscritorBinario::escribir_bytes(const void* datos, size_t cantidad) {
    stream.write(reinterpret_cast<const char*>(datos), cantidad);
    if (!stream) {
        throw std::runtime_error("Error al escribir datos en el stream");
    }
}

void EscritorBinario::escribir_bool(bool valor) {
    escribir_u8(valor ? 1 : 0);
}

void EscritorBinario::escribir_u8(uint8_t valor) {
    escribir_bytes(&valor, sizeof(valor));
}

void EscritorBinario::escribir_u16(uint16_t valor) {
    escribir_bytes(&valor, sizeof(valor));
}

void EscritorBinario::escribir_u32(uint32_t valor) {
    escribir_bytes(&valor, sizeof(valor));
}

void EscritorBinario::escribir_double(double valor) {
    escribir_bytes(&valor, sizeof(valor));
}

void EscritorBinario::escribir_texto(const std::string& texto) {
    if (texto.size() > std::numeric_limits<uint16_t>::max()) {
        throw std::runtime_error("Texto demasiado largo para serializar");
    }
    escribir_u16(static_cast<uint16_t>(texto.size()));
    if (!texto.empty()) {
        escribir_bytes(texto.data(), texto.size());
    }
}
