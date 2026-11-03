#ifndef LECTOR_TOML_H
#define LECTOR_TOML_H

#include <string>
#include <vector>

#include "../externo/tomlplusplus/toml.hpp"

// Auxiliares de lectura y validación compartidos por los parsers de configuración del servidor y
// del cliente. Todos lanzan std::runtime_error con un mensaje que indica la sección y la clave, así
// el error de configuración se ve igual en los dos binarios.
namespace lector_toml {

// Parsea el archivo y traduce el error de toml++ a runtime_error con la línea.
toml::table cargar_archivo(const std::string& ruta);

const toml::table& exigir_tabla(const toml::table& archivo, const std::string& nombre);

const toml::array& exigir_arreglo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

int leer_entero(const toml::table& tabla, const std::string& clave, const std::string& seccion);

int leer_entero_positivo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Si la clave no está, devuelve el valor por defecto; si está, exige que sea un entero.
int leer_entero_opcional(const toml::table& tabla, const std::string& clave, int valor_por_defecto);

bool leer_booleano(const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Si la clave no está, devuelve el valor por defecto; si está, exige que sea un booleano.
bool leer_booleano_opcional(
    const toml::table& tabla, const std::string& clave, bool valor_por_defecto);

// Exige que la clave exista y no esté vacía.
std::string leer_texto(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Exige un arreglo no vacío de textos no vacíos.
std::vector<std::string> leer_textos(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

}  // namespace lector_toml

#endif
