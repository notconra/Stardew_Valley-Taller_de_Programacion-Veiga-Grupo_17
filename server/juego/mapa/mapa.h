#ifndef MAPA_H
#define MAPA_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../../configuracion/configuracion_servidor_datos.h"
#include "../geometria.h"
#include "propiedad_de_casilla.h"

// La capa del TMX que dibuja el piso. De ahí salen las casillas cultivables.
#define CAPA_DE_PISO "Back"

// Valor que usa Tiled en las propiedades booleanas de los tiles.
#define VALOR_VERDADERO_TMX "T"

// Los bits altos de un id de tile guardan si está espejado o rotado. El id real es el resto.
#define MASCARA_ID_DE_TILE 0x1FFFFFFF

// A dónde lleva una transición: el mapa destino y la casilla donde aparece el jugador ahí.
struct Transicion {
    std::string mapa_destino;
    int columna_destino = 0;
    int fila_destino = 0;
};

// El terreno tal como lo necesita el servidor, qué propiedades tiene cada casilla.
// Se carga una sola vez y después es de solo lectura. El cliente carga el mismo TMX por su cuenta
// para dibujarlo. No sabe nada de jugadores ni de cosas colocadas porque eso es de cada partida.
class Mapa {
private:
    int ancho_en_casillas = 0;
    int alto_en_casillas = 0;
    int ancho_de_casilla = 0;
    int alto_de_casilla = 0;
    std::vector<uint8_t> propiedades;  // un byte de flags por casilla, fila por fila

    // Zonas (en píxeles) que llevan a otro mapa. Pocas por mapa, alcanza con recorrerlas.
    std::vector<std::pair<Rectangulo, Transicion>> transiciones;

    // Suma "propiedad" a toda casilla no vacía de la capa. Lanza std::runtime_error si la capa
    // no coincide con el tamaño del mapa.
    void marcar(const std::vector<int>& capa, uint8_t propiedad, const std::string& nombre_capa);

    // Suma "propiedad" a las casillas de la capa cuyo tile es uno de "ids". Lanza
    // std::runtime_error si la capa no coincide con el tamaño del mapa.
    void marcar_si_es_alguno(
        const std::vector<int>& capa, const std::vector<int>& ids, uint8_t propiedad,
        const std::string& nombre_capa);

    // Ajusta la colisión de las casillas de cada transición. Las de los destinos indicados quedan
    // bloqueadas, el resto se liberan para poder pisarlas.
    void ajustar_colisiones_de_transiciones(
        const std::vector<std::string>& destinos_que_conservan_colision);

public:
    // Lanza std::runtime_error si no puede leer el TMX o la capa de colisiones.
    // "destinos_que_conservan_colision" son los mapa_destino cuyas casillas no se liberan.
    explicit Mapa(
        const ConfiguracionMapaServidor& configuracion,
        const std::vector<std::string>& destinos_que_conservan_colision = {});

    int cantidad_de_columnas() const;
    int cantidad_de_filas() const;
    int ancho_de_casilla_en_pixeles() const;
    int alto_de_casilla_en_pixeles() const;
    int ancho_en_pixeles() const;
    int alto_en_pixeles() const;

    bool esta_dentro(int columna, int fila) const;

    // Si la casilla tiene alguna de las propiedades pedidas. Fuera del mapa devuelve false.
    bool tiene(int columna, int fila, uint8_t propiedad) const;

    // La transición que toca la zona dada (el hitbox del jugador), si hay alguna. Devuelve
    // nullptr si no toca ninguna. Si el jugador tocara dos a la vez (no debería pasar si los
    // objetos de Tiled no se superponen), devuelve la primera que encuentra.
    const Transicion* transicion_en(const Rectangulo& zona) const;

    // La transición que ocupa esa casilla (cualquier superposición con su rectángulo cuenta), o
    // nullptr si no hay ninguna o la casilla está fuera del mapa. Es lo que usa el clic sobre
    // puertas, salidas y escaleras.
    const Transicion* transicion_en_casilla(int columna, int fila) const;

    // Si la casilla es parte de una transición (una puerta, una escalera) o está pegada a una,
    // también en diagonal. Ahí no se pueden poner objetos: taparían la entrada o la salida, o la
    // casilla donde aparece el que llega.
    bool cerca_de_una_transicion(int columna, int fila) const;

    // Todas las zonas de transición del mapa, para validarlas al cargar los recursos.
    const std::vector<std::pair<Rectangulo, Transicion>>& obtener_transiciones() const;
};

#endif
