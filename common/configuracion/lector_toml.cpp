#include "lector_toml.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace lector_toml {

toml::table cargar_archivo(const std::string& ruta) {
    try {
        return toml::parse_file(ruta);
    } catch (const toml::parse_error& error) {
        throw std::runtime_error(
            "no se pudo leer la configuración '" + ruta + "': " + std::string(error.description()) +
            " (línea " + std::to_string(error.source().begin.line) + ")");
    }
}

const toml::table& exigir_tabla(const toml::table& archivo, const std::string& nombre) {
    const toml::table* tabla = archivo[nombre].as_table();
    if (tabla == nullptr) {
        throw std::runtime_error("configuración: falta la sección [" + nombre + "]");
    }
    return *tabla;
}

const toml::array& exigir_arreglo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) {
    const toml::array* valores = tabla[clave].as_array();
    if (valores == nullptr) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    return *valores;
}

const toml::table& tabla_en(
    const toml::array& lista, std::size_t indice, const std::string& seccion) {
    const toml::table* tabla = lista[indice].as_table();
    if (tabla == nullptr) {
        throw std::runtime_error("configuración: la entrada '" + seccion + "' debe ser una tabla");
    }
    return *tabla;
}

int leer_entero(const toml::table& tabla, const std::string& clave, const std::string& seccion) {
    const auto valor = tabla[clave].value<int>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    return *valor;
}

int leer_entero_positivo(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) {
    const int valor = leer_entero(tabla, clave, seccion);
    if (valor <= 0) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] debe ser mayor que cero");
    }
    return valor;
}

int leer_entero_en_rango(
    const toml::table& tabla, const std::string& clave, const std::string& seccion, int minimo,
    int maximo) {
    const int valor = leer_entero(tabla, clave, seccion);
    if (valor < minimo || valor > maximo) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] debe estar entre " +
            std::to_string(minimo) + " y " + std::to_string(maximo));
    }
    return valor;
}

int leer_entero_opcional(
    const toml::table& tabla, const std::string& clave, int valor_por_defecto) {
    const toml::node* nodo = tabla.get(clave);
    if (nodo == nullptr) {
        return valor_por_defecto;
    }
    const auto valor = nodo->value<int>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' debe ser un número entero");
    }
    return *valor;
}

int leer_entero_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    int valor_por_defecto, int minimo, int maximo) {
    if (tabla.get(clave) == nullptr) {
        return valor_por_defecto;
    }
    return leer_entero_en_rango(tabla, clave, seccion, minimo, maximo);
}

double leer_decimal_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    double valor_por_defecto, double minimo, double maximo) {
    const toml::node* nodo = tabla.get(clave);
    if (nodo == nullptr) {
        return valor_por_defecto;
    }
    const auto valor = nodo->value<double>();
    if (!valor || *valor < minimo || *valor > maximo) {
        std::ostringstream mensaje;
        mensaje << "configuración: la clave '" << clave << "' de [" << seccion
                << "] debe ser un número entre " << minimo << " y " << maximo;
        throw std::runtime_error(mensaje.str());
    }
    return *valor;
}

bool leer_booleano(const toml::table& tabla, const std::string& clave, const std::string& seccion) {
    const auto valor = tabla[clave].value<bool>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    return *valor;
}

bool leer_booleano_opcional(
    const toml::table& tabla, const std::string& clave, bool valor_por_defecto) {
    const toml::node* nodo = tabla.get(clave);
    if (nodo == nullptr) {
        return valor_por_defecto;
    }
    const auto valor = nodo->value<bool>();
    if (!valor) {
        throw std::runtime_error("configuración: la clave '" + clave + "' debe ser un booleano");
    }
    return *valor;
}

std::string leer_texto(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) {
    const auto valor = tabla[clave].value<std::string>();
    if (!valor) {
        throw std::runtime_error(
            "configuración: falta o es inválida la clave '" + clave + "' de [" + seccion + "]");
    }
    if (valor->empty()) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] no puede estar vacía");
    }
    return *valor;
}

std::string leer_texto_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    const std::string& valor_por_defecto) {
    if (tabla.get(clave) == nullptr) {
        return valor_por_defecto;
    }
    return leer_texto(tabla, clave, seccion);
}

std::vector<std::string> leer_textos(
    const toml::table& tabla, const std::string& clave, const std::string& seccion) {
    const toml::array& valores = exigir_arreglo(tabla, clave, seccion);

    std::vector<std::string> textos;
    textos.reserve(valores.size());
    for (const auto& valor : valores) {
        const auto texto = valor.value<std::string>();
        if (!texto || texto->empty()) {
            throw std::runtime_error(
                "configuración: la clave '" + clave + "' de [" + seccion +
                "] debe contener textos no vacíos");
        }
        textos.push_back(*texto);
    }

    if (textos.empty()) {
        throw std::runtime_error(
            "configuración: la clave '" + clave + "' de [" + seccion + "] no puede estar vacía");
    }
    return textos;
}

std::vector<std::string> leer_textos_opcional(
    const toml::table& tabla, const std::string& clave, const std::string& seccion,
    const std::vector<std::string>& valor_por_defecto) {
    if (tabla.get(clave) == nullptr) {
        return valor_por_defecto;
    }
    return leer_textos(tabla, clave, seccion);
}

}  // namespace lector_toml
