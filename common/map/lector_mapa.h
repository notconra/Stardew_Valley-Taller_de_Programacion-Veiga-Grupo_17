#ifndef __LECTOR_MAPA_H__
#define __LECTOR_MAPA_H__

#include <string>
#include <vector>
#include <stdexcept>

struct EntidadInicial {
    std::string tipo;
    int x;
    int y;
};

struct DatosMapa {
    std::string zona;
    int ancho;
    int alto;
    std::vector<int> terreno;
    std::vector<EntidadInicial> entidades;
};

class LectorMapa {
public:
    LectorMapa() = delete;
    ~LectorMapa() = delete;
    LectorMapa(const LectorMapa&) = delete;
    LectorMapa& operator=(const LectorMapa&) = delete;

    static DatosMapa leer(const std::string& ruta_archivo);
};

#endif // __LECTOR_MAPA_H__