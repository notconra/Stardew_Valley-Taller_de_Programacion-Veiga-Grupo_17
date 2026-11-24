#ifndef DEFINICION_DE_MUNDO_H
#define DEFINICION_DE_MUNDO_H

#include <cstddef>
#include <string>
#include <vector>

#include "mapa_editable.h"

class DefinicionDeMundo {
private:
    std::string nombre_del_mundo;
    MapaEditable sector_granja;
    MapaEditable sector_pueblo;
    MapaEditable sector_mercado;
    std::vector<MapaEditable> niveles_mazmorra;

    static constexpr size_t CANTIDAD_DE_NIVELES_MAZMORRA = 10;
    static constexpr int COLUMNAS_MAPA_GRANJA = 34;
    static constexpr int FILAS_MAPA_GRANJA = 30;
    static constexpr int ANCHO_MAPA_EXTERIORES = 35;
    static constexpr int ALTO_MAPA_EXTERIORES = 35;
    static constexpr int ANCHO_MAPA_MAZMORRA = 25;
    static constexpr int ALTO_MAPA_MAZMORRA = 25;

public:
    DefinicionDeMundo(
        const std::string& nombre, const MapaEditable& granja, const MapaEditable& pueblo,
        const std::vector<MapaEditable>& niveles);

    DefinicionDeMundo(
        const std::string& nombre, const MapaEditable& granja, const MapaEditable& pueblo,
        const MapaEditable& mercado, const std::vector<MapaEditable>& niveles);

    const std::string& obtener_nombre() const;
    void establecer_nombre(const std::string& nuevo_nombre);

    const MapaEditable& obtener_sector_granja() const;
    MapaEditable& obtener_sector_granja_modificable();

    const MapaEditable& obtener_sector_pueblo() const;
    MapaEditable& obtener_sector_pueblo_modificable();

    const MapaEditable& obtener_sector_mercado() const;
    MapaEditable& obtener_sector_mercado_modificable();

    size_t obtener_cantidad_de_niveles_mazmorra() const;
    const MapaEditable& obtener_nivel_mazmorra(size_t indice_nivel) const;
    MapaEditable& obtener_nivel_mazmorra_modificable(size_t indice_nivel);

    void establecer_sector_granja(const MapaEditable& granja);
    void establecer_sector_pueblo(const MapaEditable& pueblo);
    void establecer_sector_mercado(const MapaEditable& mercado);
    void establecer_niveles_mazmorra(const std::vector<MapaEditable>& niveles);

    static MapaEditable crear_sector_mercado_por_defecto();
    static DefinicionDeMundo crear_mundo_por_defecto(const std::string& nombre = "Mundo Nuevo");
    static const std::vector<std::string>& ids_sectores_mazmorra();
    static std::string nombre_archivo_caverna(size_t indice_nivel);
};

#endif  // DEFINICION_DE_MUNDO_H
