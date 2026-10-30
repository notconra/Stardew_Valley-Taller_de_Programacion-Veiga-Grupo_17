#include "identidad_cliente.h"

#include <cstdint>
#include <fstream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>

constexpr const char* RUTA_ARCHIVO = "cliente_id.dat";

uint32_t IdentidadCliente::obtener_o_crear() {
    uint32_t id = leer_id();
    if (id > 0) {
        return id;
    }

    id = generar_id_aleatorio();
    guardar_id(id);
    return id;
}

uint32_t IdentidadCliente::leer_id() {
    std::ifstream archivo(RUTA_ARCHIVO, std::ios::binary);
    if (!archivo.is_open()) {
        return 0;
    }

    uint32_t id = 0;
    if (!archivo.read(reinterpret_cast<char*>(&id), sizeof(id))) {
        return 0;
    }

    return id;
}

void IdentidadCliente::guardar_id(uint32_t id) {
    std::ofstream archivo(RUTA_ARCHIVO, std::ios::binary | std::ios::trunc);
    if (!archivo.is_open()) {
        throw std::runtime_error(
            "No se pudo abrir el archivo de identidad para escritura: " +
            std::string(RUTA_ARCHIVO));
    }

    archivo.write(reinterpret_cast<const char*>(&id), sizeof(id));
    if (!archivo) {
        throw std::runtime_error(
            "Error al escribir el ID de cliente en: " + std::string(RUTA_ARCHIVO));
    }
}

uint32_t IdentidadCliente::generar_id_aleatorio() {
    std::random_device rd;
    std::mt19937 generador(rd());
    std::uniform_int_distribution<uint32_t> distribucion(1, std::numeric_limits<uint32_t>::max());
    return distribucion(generador);
}
