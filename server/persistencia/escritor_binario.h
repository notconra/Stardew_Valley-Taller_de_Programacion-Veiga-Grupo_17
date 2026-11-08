#ifndef ESCRITOR_BINARIO_H
#define ESCRITOR_BINARIO_H

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>

class EscritorBinario {
private:
    std::ostream& stream;

public:
    explicit EscritorBinario(std::ostream& stream_salida);

    void escribir_bytes(const void* datos, size_t cantidad);
    void escribir_bool(bool valor);
    void escribir_u8(uint8_t valor);
    void escribir_u16(uint16_t valor);
    void escribir_u32(uint32_t valor);
    void escribir_double(double valor);
    void escribir_texto(const std::string& texto);

    EscritorBinario(const EscritorBinario&) = delete;
    EscritorBinario& operator=(const EscritorBinario&) = delete;
};

#endif  // ESCRITOR_BINARIO_H
