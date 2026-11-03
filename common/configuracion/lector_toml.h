#ifndef LECTOR_TOML_H
#define LECTOR_TOML_H

#include <cstddef>
#include <string>
#include <vector>

#include "../externo/tomlplusplus/toml.hpp"

// Auxiliares de lectura y validación compartidos por los parsers de configuración del servidor y
// del cliente. Todos lanzan std::runtime_error con un mensaje que indica la sección y la clave, así
// el error de configuración se ve igual en los dos binarios.
//
// "seccion" es el nombre sin corchetes (por ejemplo "juego.drops"); el mensaje lo muestra entre
// corchetes. Las funciones "_opcional" devuelven el valor por defecto si la clave no está, pero si
// está la validan igual que las obligatorias.
namespace lector_toml {

// Parsea el archivo y traduce el error de toml++ a runtime_error con la línea.
toml::table cargar_archivo(const std::string& ruta);

const toml::table& exigir_tabla(const toml::table& archivo, const std::string& nombre);

const toml::array& exigir_arreglo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

// El elemento "indice" de un arreglo de tablas ([[mapas]], por ejemplo). "seccion" es como se
// nombra esa entrada en el mensaje de error (por ejemplo "mapas[2]").
const toml::table& tabla_en(
    const toml::array& lista, std::size_t indice, const std::string& seccion);

int leer_entero(const toml::table& tabla, const std::string& clave, const std::string& seccion);

int leer_entero_positivo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Entero obligatorio que tiene que estar entre "minimo" y "maximo" (ambos incluidos).
int leer_entero_en_rango(
    const toml::table& tabla, const std::string& clave, const std::string& seccion, int minimo,
    int maximo);

// Si la clave no está, devuelve el valor por defecto; si está, exige que sea un entero.
int leer_entero_opcional(const toml::table& tabla, const std::string& clave, int valor_por_defecto);

// Igual que leer_entero_en_rango pero con valor por defecto si la clave no está.
int leer_entero_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    int valor_por_defecto, int minimo, int maximo);

// Decimal opcional entre "minimo" y "maximo" (un entero de TOML también sirve).
double leer_decimal_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    double valor_por_defecto, double minimo, double maximo);

bool leer_booleano(const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Exige que la clave exista y no esté vacía.
std::string leer_texto(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Si la clave está, exige que no esté vacía; si no está, devuelve el valor por defecto.
std::string leer_texto_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    const std::string& valor_por_defecto);

// Exige un arreglo no vacío de textos no vacíos.
std::vector<std::string> leer_textos(
    const toml::table& tabla, const std::string& clave, const std::string& seccion);

// Si la clave está, igual que leer_textos; si no está, devuelve el valor por defecto.
std::vector<std::string> leer_textos_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    const std::vector<std::string>& valor_por_defecto);

}  // namespace lector_toml

#endif
