#ifndef EXPORTADOR_DE_MUNDO_TMX_H
#define EXPORTADOR_DE_MUNDO_TMX_H

#include <string>

#include "definicion_de_mundo.h"
#include "tipo_entidad.h"

class ExportadorDeMundoTMX {
private:
    static void obtener_coordenadas_de_entidad_requerida(
        const MapaEditable& mapa, TipoEntidad tipo, const std::string& mensaje_error,
        int& columna_resultado, int& fila_resultado);
    static void calcular_destino_entrada_borde(
        const MapaEditable& mapa_destino, const EntidadDelMapa& entidad_transicion,
        int& columna_destino, int& fila_destino);

public:
    static constexpr const char* NOMBRE_ARCHIVO_GRANJA = "granja.tmx";
    static constexpr const char* NOMBRE_ARCHIVO_PUEBLO = "pueblo.tmx";
    static constexpr const char* NOMBRE_ARCHIVO_MERCADO = "mercado.tmx";

    static bool exportar_mundo_completo(
        const std::string& ruta_directorio_destino, const DefinicionDeMundo& mundo);
};

#endif  // EXPORTADOR_DE_MUNDO_TMX_H
