#include "tipo_herramienta_editor.h"

#include <map>
#include <string>

#include "tipo_entidad.h"

struct DimensionesEntidad {
    int ancho;
    int alto;
};

static constexpr int DIMENSION_POR_DEFECTO = 1;
static constexpr int ANCHO_CABANIA = 9;
static constexpr int ALTO_CABANIA = 9;
static constexpr int ANCHO_ENTRADA_CAVERNA = 5;
static constexpr int ALTO_ENTRADA_CAVERNA = 7;
static constexpr int ANCHO_PUESTO = 1;
static constexpr int ALTO_PUESTO = 1;
static constexpr int ANCHO_CENTRO_COMUNITARIO = 12;
static constexpr int ALTO_CENTRO_COMUNITARIO = 10;
static constexpr int ANCHO_MERCADO = 10;
static constexpr int ALTO_MERCADO = 10;
static constexpr int ANCHO_ESCALERA = 1;
static constexpr int ALTO_ESCALERA = 1;
static constexpr int ANCHO_SALIDA_CAVERNA = 1;
static constexpr int ALTO_SALIDA_CAVERNA = 1;
static constexpr int ANCHO_TRANSICION = 2;
static constexpr int ALTO_TRANSICION = 2;

static constexpr const char* NOMBRE_CABANIA = "Cabania";
static constexpr const char* NOMBRE_ENTRADA_CAVERNA = "Entrada Caverna";
static constexpr const char* NOMBRE_PUESTO_MARNIE = "Puesto Marnie";
static constexpr const char* NOMBRE_PUESTO_CLINT = "Puesto Clint";
static constexpr const char* NOMBRE_PUESTO_PIERRE = "Puesto Pierre";
static constexpr const char* NOMBRE_PUESTO_ROBIN = "Puesto Robin";
static constexpr const char* NOMBRE_CENTRO_COMUNITARIO = "Centro Comunitario";
static constexpr const char* NOMBRE_MERCADO = "Mercado";
static constexpr const char* NOMBRE_ESCALERA = "Escalera Mazmorra";
static constexpr const char* NOMBRE_SALIDA_CAVERNA = "Salida Caverna";
static constexpr const char* NOMBRE_TRANSICION_PUEBLO = "Salida a Pueblo";
static constexpr const char* NOMBRE_TRANSICION_GRANJA = "Salida a Granja";
static constexpr const char* NOMBRE_DESCONOCIDA = "Desconocida";

static const std::map<TipoEntidad, std::string> NOMBRES_ENTIDADES = {
    {TipoEntidad::CABANIA, NOMBRE_CABANIA},
    {TipoEntidad::ENTRADA_CAVERNA, NOMBRE_ENTRADA_CAVERNA},
    {TipoEntidad::PUESTO_MARNIE, NOMBRE_PUESTO_MARNIE},
    {TipoEntidad::PUESTO_CLINT, NOMBRE_PUESTO_CLINT},
    {TipoEntidad::PUESTO_PIERRE, NOMBRE_PUESTO_PIERRE},
    {TipoEntidad::PUESTO_ROBIN, NOMBRE_PUESTO_ROBIN},
    {TipoEntidad::CENTRO_COMUNITARIO, NOMBRE_CENTRO_COMUNITARIO},
    {TipoEntidad::MERCADO, NOMBRE_MERCADO},
    {TipoEntidad::ESCALERA_NIVEL, NOMBRE_ESCALERA},
    {TipoEntidad::SALIDA_CAVERNA, NOMBRE_SALIDA_CAVERNA},
    {TipoEntidad::TRANSICION_PUEBLO, NOMBRE_TRANSICION_PUEBLO},
    {TipoEntidad::TRANSICION_GRANJA, NOMBRE_TRANSICION_GRANJA}};

static const std::map<TipoEntidad, DimensionesEntidad> DIMENSIONES_ENTIDADES = {
    {TipoEntidad::CABANIA, {ANCHO_CABANIA, ALTO_CABANIA}},
    {TipoEntidad::ENTRADA_CAVERNA, {ANCHO_ENTRADA_CAVERNA, ALTO_ENTRADA_CAVERNA}},
    {TipoEntidad::PUESTO_MARNIE, {ANCHO_PUESTO, ALTO_PUESTO}},
    {TipoEntidad::PUESTO_CLINT, {ANCHO_PUESTO, ALTO_PUESTO}},
    {TipoEntidad::PUESTO_PIERRE, {ANCHO_PUESTO, ALTO_PUESTO}},
    {TipoEntidad::PUESTO_ROBIN, {ANCHO_PUESTO, ALTO_PUESTO}},
    {TipoEntidad::CENTRO_COMUNITARIO, {ANCHO_CENTRO_COMUNITARIO, ALTO_CENTRO_COMUNITARIO}},
    {TipoEntidad::MERCADO, {ANCHO_MERCADO, ALTO_MERCADO}},
    {TipoEntidad::ESCALERA_NIVEL, {ANCHO_ESCALERA, ALTO_ESCALERA}},
    {TipoEntidad::SALIDA_CAVERNA, {ANCHO_SALIDA_CAVERNA, ALTO_SALIDA_CAVERNA}},
    {TipoEntidad::TRANSICION_PUEBLO, {ANCHO_TRANSICION, ALTO_TRANSICION}},
    {TipoEntidad::TRANSICION_GRANJA, {ANCHO_TRANSICION, ALTO_TRANSICION}}};

std::string ConfiguracionEntidadesEditor::obtener_nombre(TipoEntidad entidad) {
    auto iterador = NOMBRES_ENTIDADES.find(entidad);
    if (iterador != NOMBRES_ENTIDADES.end()) {
        return iterador->second;
    }
    return NOMBRE_DESCONOCIDA;
}

void ConfiguracionEntidadesEditor::consultar_dimensiones_por_defecto(
    TipoEntidad entidad, int& ancho, int& alto) {
    auto iterador = DIMENSIONES_ENTIDADES.find(entidad);
    if (iterador != DIMENSIONES_ENTIDADES.end()) {
        ancho = iterador->second.ancho;
        alto = iterador->second.alto;
    } else {
        ancho = DIMENSION_POR_DEFECTO;
        alto = DIMENSION_POR_DEFECTO;
    }
}
