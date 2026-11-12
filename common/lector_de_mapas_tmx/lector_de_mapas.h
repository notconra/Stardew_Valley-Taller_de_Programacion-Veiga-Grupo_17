#ifndef LECTOR_DE_MAPAS_H
#define LECTOR_DE_MAPAS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "externo/tinyxml2/tinyxml2.h"

// Los bits altos de un id de tile guardan si está espejado o rotado. El id real es el resto.
#define MASCARA_GID 0x1FFFFFFF

// Tamaño del mapa en cantidad de casillas y tamaño de cada casilla en píxeles.
struct TamanioMapa {
    int ancho_en_casillas = 0;
    int alto_en_casillas = 0;
    int ancho_de_casilla = 0;
    int alto_de_casilla = 0;
};

// Una transición tal como está en el TMX: la zona que la dispara y a dónde lleva.
struct TransicionTMX {
    int x = 0;
    int y = 0;
    int ancho = 0;
    int alto = 0;
    std::string mapa_destino;
    int columna_destino = 0;
    int fila_destino = 0;
};

// Un puesto de venta tal como está en el TMX (objeto con tipo = "puesto" en la capa de
// interacciones). Tiene la zona donde se hace clic, el identificador con el que la configuración le
// asigna catálogo y horario, y la casilla desde donde se atiende.
struct PuestoTMX {
    int x = 0;
    int y = 0;
    int ancho = 0;
    int alto = 0;
    std::string id;  // propiedad id_puesto
    int columna_atencion = 0;
    int fila_atencion = 0;
    bool tiene_atencion = false;  // false si falta columna_atencion o fila_atencion
};

// Un objeto de una capa de objetos de Tiled, tal cual está en el TMX, con su zona y sus
// propiedades (nombre -> valor).
class ObjetoTMX {
private:
    int pos_x;
    int pos_y;
    int ancho_de_zona;
    int alto_de_zona;
    std::map<std::string, std::string> propiedades;

public:
    ObjetoTMX(
        int x, int y, int ancho, int alto, const std::map<std::string, std::string>& propiedades);

    int x() const;
    int y() const;
    int ancho() const;
    int alto() const;

    bool tiene_propiedad(const std::string& propiedad) const;

    // El valor de la propiedad como texto, o "" si el objeto no la tiene.
    std::string texto_de(const std::string& propiedad) const;

    // El valor de la propiedad como entero, o 0 si el objeto no la tiene.
    int entero_de(const std::string& propiedad) const;
};

// Lee un archivo TMX de Tiled. Lo usan el servidor (para las colisiones) y el cliente (para
// dibujar), así los dos ven exactamente el mismo mapa.
class LectorDeMapasTMX {
private:
    std::string ruta_archivo;

    // Carga el archivo en "documento" y devuelve su nodo raíz <map>. Si falla la lectura o no
    // hay <map>, imprime un error y devuelve nullptr. Las tres funciones públicas arrancan así.
    tinyxml2::XMLElement* abrir_mapa(tinyxml2::XMLDocument& documento) const;

    // Lee todos los objetos de la capa de objetos con ese nombre.
    // Es lo único que conoce la estructura XML de una capa de objetos.
    std::vector<ObjetoTMX> leer_objetos(
        tinyxml2::XMLElement* nodo_mapa, const std::string& nombre_capa) const;

    // Tiled puede guardar decimales en x, y, width y height. Se redondea para abajo.
    int leer_entero_de_objeto(tinyxml2::XMLElement* nodo, const char* atributo) const;

public:
    explicit LectorDeMapasTMX(const std::string& ruta);

    // Devuelve el tamaño del mapa. Si falla la lectura, devuelve ambos valores en 0.
    TamanioMapa leer_tamanio() const;

    // Devuelve los identificadores de las casillas de la capa, fila por fila.
    // Si falla la lectura o no existe la capa, devuelve un vector vacío.
    std::vector<int> cargar_capa(const std::string& nombre_capa) const;

    std::vector<std::uint32_t> cargar_capa_con_flags(const std::string& nombre_capa) const;

    // Devuelve las transiciones de la capa de objetos indicada (ver clase Mapa).
    // Si no existe la capa, devuelve un vector vacío. Ignora objetos que no tengan
    // las propiedades necesarias (mapa_destino, columna_destino, fila_destino).
    std::vector<TransicionTMX> cargar_transiciones(const std::string& nombre_capa) const;

    // Devuelve los puestos de la capa de objetos indicada (todo objeto con la propiedad
    // tipo = "puesto"). Los objetos de otro tipo se ignoran (la capa puede traer otras
    // interacciones).
    std::vector<PuestoTMX> cargar_puestos(const std::string& nombre_capa) const;

    // Devuelve los identificadores globales (firstgid + id) de los tiles de todos los tilesets
    // del mapa que tienen la propiedad con ese valor, por ejemplo Diggable = T. Sirve para marcar
    // casillas según el dibujo que tienen. Si falla la lectura, devuelve un vector vacío.
    std::vector<int> ids_con_propiedad(
        const std::string& propiedad, const std::string& valor) const;
};

#endif
