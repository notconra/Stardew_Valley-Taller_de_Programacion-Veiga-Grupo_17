#ifndef CATALOGO_DE_MUNDOS_H
#define CATALOGO_DE_MUNDOS_H

#include <string>
#include <vector>

// Carpeta (relativa a donde se ejecuta el servidor) con un subdirectorio por mundo del editor.
#define CARPETA_DE_MUNDOS "mundos"

// Nombre con el que el cliente pide el mundo de siempre (también vale el nombre vacío).
#define NOMBRE_DEL_MUNDO_ESTANDAR "Mundo Estandar"

// Los mundos armados con el editor que tiene el servidor. Se lee la carpeta cada vez, así aparece
// un mundo nuevo sin reiniciar el servidor. Un mundo es válido si el editor lo puede abrir y pasa
// su validación (no le falta la caverna, los puestos, las puertas...).
class CatalogoDeMundos {
public:
    // Los nombres de los mundos válidos, ordenados.
    std::vector<std::string> listar() const;

    // Si se puede crear una partida con ese mundo: el estándar o uno de listar().
    bool es_valido(const std::string& nombre_mundo) const;
};

#endif
