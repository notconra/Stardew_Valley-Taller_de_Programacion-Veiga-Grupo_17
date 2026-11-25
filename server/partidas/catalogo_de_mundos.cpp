#include "catalogo_de_mundos.h"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "../../common/editor/cargador_de_mundo_tmx.h"
#include "../../common/editor/validador_de_mundo.h"

std::vector<std::string> CatalogoDeMundos::listar() const {
    std::vector<std::string> mundos;
    std::error_code error;
    if (!std::filesystem::is_directory(CARPETA_DE_MUNDOS, error)) {
        return mundos;
    }
    for (const auto& entrada : std::filesystem::directory_iterator(CARPETA_DE_MUNDOS, error)) {
        if (!entrada.is_directory()) {
            continue;
        }
        const std::string nombre = entrada.path().filename().string();
        try {
            const ResultadoValidacion resultado = ValidadorDeMundo::validar(
                CargadorDeMundoTMX::cargar_mundo(entrada.path().string()));
            if (resultado.es_valido) {
                mundos.push_back(nombre);
            } else {
                std::cerr << "[mundos] '" << nombre << "' no es valido y no se ofrece\n";
            }
        } catch (const std::exception& excepcion) {
            std::cerr << "[mundos] no se pudo abrir '" << nombre << "': " << excepcion.what()
                      << "\n";
        }
    }
    std::sort(mundos.begin(), mundos.end());
    return mundos;
}

bool CatalogoDeMundos::es_valido(const std::string& nombre_mundo) const {
    if (nombre_mundo.empty() || nombre_mundo == NOMBRE_DEL_MUNDO_ESTANDAR) {
        return true;
    }
    const std::vector<std::string> mundos = listar();
    return std::find(mundos.begin(), mundos.end(), nombre_mundo) != mundos.end();
}
