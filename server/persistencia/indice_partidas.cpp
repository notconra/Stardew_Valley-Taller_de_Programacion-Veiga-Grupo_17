#include "indice_partidas.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "escritor_binario.h"
#include "lector_binario.h"

constexpr std::array<uint8_t, 4> FIRMA_INDICE = {'S', 'V', 'I', 'X'};
constexpr uint16_t VERSION_INDICE = 1;
constexpr const char* RUTA_INDICE_DEFECTO = "partidas/partidas.idx";

IndicePartidas::IndicePartidas() : ruta_archivo(RUTA_INDICE_DEFECTO) {
}

void IndicePartidas::registrar_o_actualizar(const EntradaIndicePartida& entrada) {
    partidas[entrada.codigo] = entrada;
}

std::vector<EntradaIndicePartida> IndicePartidas::listar_todas() const {
    std::vector<EntradaIndicePartida> resultado;
    for (const auto& par : partidas) {
        resultado.push_back(par.second);
    }
    return resultado;
}

void IndicePartidas::guardar() const {
    std::filesystem::path ruta_fs(ruta_archivo);
    if (ruta_fs.has_parent_path()) {
        std::filesystem::create_directories(ruta_fs.parent_path());
    }

    std::ofstream archivo(ruta_archivo, std::ios::binary | std::ios::trunc);
    if (!archivo.is_open()) {
        throw std::runtime_error(
            "No se pudo abrir el archivo de índice para escritura: " + ruta_archivo);
    }

    EscritorBinario escritor(archivo);
    escritor.escribir_bytes(FIRMA_INDICE.data(), FIRMA_INDICE.size());
    escritor.escribir_u16(VERSION_INDICE);

    escritor.escribir_u32(static_cast<uint32_t>(partidas.size()));
    for (const auto& par : partidas) {
        const EntradaIndicePartida& entrada = par.second;
        escritor.escribir_texto(entrada.codigo);
        escritor.escribir_texto(entrada.nombre);
        escritor.escribir_texto(entrada.ruta_archivo);
    }
}

void IndicePartidas::cargar() {
    if (!std::filesystem::exists(ruta_archivo)) {
        partidas.clear();
        return;
    }

    std::ifstream archivo(ruta_archivo, std::ios::binary);
    if (!archivo.is_open()) {
        throw std::runtime_error(
            "No se pudo abrir el archivo de índice para lectura: " + ruta_archivo);
    }

    LectorBinario lector(archivo);

    std::array<uint8_t, 4> firma{};
    lector.leer_bytes(firma.data(), firma.size());
    if (firma != FIRMA_INDICE) {
        throw std::runtime_error(
            "Firma de archivo inválida: no es un archivo de índice de partidas válido");
    }

    uint16_t version = lector.leer_u16();
    if (version != VERSION_INDICE) {
        throw std::runtime_error("Versión de índice no soportada: " + std::to_string(version));
    }

    partidas.clear();
    uint32_t cant_entradas = lector.leer_u32();
    for (uint32_t indice = 0; indice < cant_entradas; ++indice) {
        EntradaIndicePartida entrada;
        entrada.codigo = lector.leer_texto();
        entrada.nombre = lector.leer_texto();
        entrada.ruta_archivo = lector.leer_texto();
        partidas[entrada.codigo] = entrada;
    }
}
