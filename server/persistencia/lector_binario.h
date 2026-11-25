#ifndef LECTOR_BINARIO_H
#define LECTOR_BINARIO_H

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>

class LectorBinario {
private:
    std::istream& stream;

public:
    explicit LectorBinario(std::istream& stream_entrada);

    void leer_bytes(void* buffer, size_t cantidad);
    bool leer_bool();
    uint8_t leer_u8();
    uint16_t leer_u16();
    uint32_t leer_u32();
    double leer_double();
    std::string leer_texto();

    LectorBinario(const LectorBinario&) = delete;
    LectorBinario& operator=(const LectorBinario&) = delete;
};

#endif  // LECTOR_BINARIO_H
