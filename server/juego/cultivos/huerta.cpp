#include "huerta.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../azar.h"

// El día de juego va de las 6 am a las 2 am: 1200 minutos.
#define MINUTOS_DEL_DIA 1200
#define CALIDAD_MAS_ALTA static_cast<int>(Calidad::PERFECTA)
#define CALIDAD_MAS_BAJA static_cast<int>(Calidad::BASE)

Huerta::Huerta(
    const CatalogoDeCultivos& catalogo_de_cultivos,
    const ConfiguracionCultivosServidor& configuracion_de_cultivos)
    : catalogo(catalogo_de_cultivos), configuracion(configuracion_de_cultivos) {
}

const DefinicionDeCultivo& Huerta::definicion_de(const Cultivo& cultivo) const {
    const DefinicionDeCultivo* definicion = catalogo.buscar(cultivo.tipo);
    if (definicion == nullptr) {
        throw std::logic_error("hay plantado un cultivo que no está en el catálogo");
    }
    return *definicion;
}

int Huerta::turno_en(const DefinicionDeCultivo& definicion, int minuto) const {
    const int turno = minuto * definicion.riegos_por_dia / MINUTOS_DEL_DIA;
    return std::clamp(turno, 0, definicion.riegos_por_dia - 1);
}

int Huerta::inicio_de_turno(const DefinicionDeCultivo& definicion, int turno) const {
    return turno * MINUTOS_DEL_DIA / definicion.riegos_por_dia;
}

void Huerta::empezar_turno(Cultivo& cultivo, int desde) const {
    const DefinicionDeCultivo& definicion = definicion_de(cultivo);
    cultivo.turno = turno_en(definicion, desde);
    cultivo.pide_agua = true;
    cultivo.inicio_del_turno = desde;
    cultivo.fin_del_turno = inicio_de_turno(definicion, cultivo.turno + 1);
}

void Huerta::cerrar_turno(Cultivo& cultivo) const {
    if (cultivo.pide_agua) {
        ++cultivo.riegos_perdidos_hoy;
        cultivo.faltas += configuracion.faltas_por_riego_perdido;
        cultivo.pide_agua = false;
    }
}

Cultivo* Huerta::buscar(const Casilla& casilla) {
    const auto encontrado = cultivos.find({casilla.columna, casilla.fila});
    return encontrado == cultivos.end() ? nullptr : &encontrado->second;
}

const Cultivo* Huerta::cultivo_en(const Casilla& casilla) const {
    const auto encontrado = cultivos.find({casilla.columna, casilla.fila});
    return encontrado == cultivos.end() ? nullptr : &encontrado->second;
}

bool Huerta::hay_cultivo(const Casilla& casilla) const {
    return cultivo_en(casilla) != nullptr;
}

bool Huerta::plantar(const Casilla& casilla, TipoCultivo tipo) {
    if (hay_cultivo(casilla) || catalogo.buscar(tipo) == nullptr) {
        return false;
    }
    Cultivo cultivo;
    cultivo.tipo = tipo;
    empezar_turno(cultivo, minuto_del_dia);
    cultivos[{casilla.columna, casilla.fila}] = cultivo;
    return true;
}

bool Huerta::pide_agua(const Casilla& casilla) const {
    const Cultivo* cultivo = cultivo_en(casilla);
    return cultivo != nullptr && cultivo->pide_agua;
}

bool Huerta::regar(const Casilla& casilla) {
    Cultivo* cultivo = buscar(casilla);
    if (cultivo == nullptr || !cultivo->pide_agua) {
        return false;
    }
    const int mitad_del_turno =
        cultivo->inicio_del_turno + (cultivo->fin_del_turno - cultivo->inicio_del_turno) / 2;
    if (minuto_del_dia > mitad_del_turno) {
        cultivo->faltas += configuracion.faltas_por_riego_tardio;
    }
    cultivo->pide_agua = false;
    return true;
}

bool Huerta::esta_madura(const Casilla& casilla) const {
    const Cultivo* cultivo = cultivo_en(casilla);
    return cultivo != nullptr && cultivo->madura;
}

Calidad Huerta::calidad_al_cosechar(const Cultivo& cultivo) const {
    int nivel = std::max(CALIDAD_MAS_BAJA, CALIDAD_MAS_ALTA - cultivo.faltas);
    if (Azar::con_probabilidad(configuracion.probabilidad_de_subir_calidad)) {
        ++nivel;
    } else if (Azar::con_probabilidad(configuracion.probabilidad_de_bajar_calidad)) {
        --nivel;
    }
    return static_cast<Calidad>(std::clamp(nivel, CALIDAD_MAS_BAJA, CALIDAD_MAS_ALTA));
}

std::optional<Cosecha> Huerta::cosechar(const Casilla& casilla) {
    Cultivo* cultivo = buscar(casilla);
    if (cultivo == nullptr || !cultivo->madura) {
        return std::nullopt;
    }
    const DefinicionDeCultivo& definicion = definicion_de(*cultivo);
    const Cosecha cosecha{definicion.producto, calidad_al_cosechar(*cultivo)};

    if (definicion.dias_para_volver_a_dar <= 0) {
        cultivos.erase({casilla.columna, casilla.fila});
        return cosecha;
    }
    cultivo->madura = false;
    cultivo->dias_de_crecimiento =
        std::max(0, definicion.dias_para_madurar - definicion.dias_para_volver_a_dar);
    cultivo->faltas = 0;
    empezar_turno(*cultivo, minuto_del_dia);
    return cosecha;
}

bool Huerta::quitar(const Casilla& casilla) {
    return cultivos.erase({casilla.columna, casilla.fila}) > 0;
}

std::vector<Casilla> Huerta::avanzar_hasta(int minuto) {
    minuto_del_dia = std::clamp(minuto, 0, MINUTOS_DEL_DIA);
    std::vector<Casilla> cambiaron;
    for (auto& [posicion, cultivo] : cultivos) {
        if (cultivo.madura) {
            continue;
        }
        const DefinicionDeCultivo& definicion = definicion_de(cultivo);
        const int turno = turno_en(definicion, minuto_del_dia);
        if (turno == cultivo.turno) {
            continue;
        }
        if (cultivo.turno >= 0) {
            cerrar_turno(cultivo);
        }
        empezar_turno(cultivo, inicio_de_turno(definicion, turno));
        cambiaron.push_back(Casilla{posicion.first, posicion.second});
    }
    return cambiaron;
}

std::vector<Casilla> Huerta::terminar_el_dia() {
    std::vector<Casilla> cambiaron;
    for (auto& [posicion, cultivo] : cultivos) {
        if (cultivo.madura) {
            cultivo.faltas += configuracion.faltas_por_dia_sin_cosechar;
        } else {
            const DefinicionDeCultivo& definicion = definicion_de(cultivo);
            cerrar_turno(cultivo);
            if (cultivo.riegos_perdidos_hoy <= definicion.riegos_que_tolera) {
                ++cultivo.dias_de_crecimiento;
                cultivo.madura = cultivo.dias_de_crecimiento >= definicion.dias_para_madurar;
            }
        }
        cultivo.riegos_perdidos_hoy = 0;
        cultivo.turno = -1;
        cultivo.pide_agua = false;
        cambiaron.push_back(Casilla{posicion.first, posicion.second});
    }
    minuto_del_dia = 0;
    return cambiaron;
}

void Huerta::completar(CasillaTratada& casilla) const {
    const Cultivo* cultivo =
        cultivo_en(Casilla{static_cast<int>(casilla.columna), static_cast<int>(casilla.fila)});
    if (cultivo == nullptr) {
        casilla.cultivo = TipoCultivo::NINGUNO;
        casilla.etapa = 0;
        casilla.madura = false;
        casilla.regada = false;
        casilla.pide_agua = false;
        return;
    }
    const DefinicionDeCultivo& definicion = definicion_de(*cultivo);
    casilla.cultivo = cultivo->tipo;
    casilla.madura = cultivo->madura;
    casilla.etapa = cultivo->madura
                        ? ETAPA_DE_CULTIVO_MAXIMA
                        : static_cast<uint8_t>(std::min(
                              ETAPA_DE_CULTIVO_MAXIMA,
                              cultivo->dias_de_crecimiento * (ETAPA_DE_CULTIVO_MAXIMA + 1) /
                                  definicion.dias_para_madurar));
    casilla.pide_agua = cultivo->pide_agua;
    casilla.regada = !cultivo->madura && cultivo->turno >= 0 && !cultivo->pide_agua;
}

void Huerta::completar(std::vector<CasillaTratada>& casillas) const {
    for (CasillaTratada& casilla : casillas) {
        completar(casilla);
    }
}

void Huerta::agregar_guardados_a(
    const std::string& id_mapa, std::vector<DatosCultivoGuardado>& lista) const {
    for (const auto& [posicion, cultivo] : cultivos) {
        DatosCultivoGuardado guardado;
        guardado.id_mapa = id_mapa;
        guardado.columna = static_cast<uint32_t>(posicion.first);
        guardado.fila = static_cast<uint32_t>(posicion.second);
        guardado.tipo = cultivo.tipo;
        guardado.dias_de_crecimiento = static_cast<uint16_t>(cultivo.dias_de_crecimiento);
        guardado.madura = cultivo.madura;
        guardado.faltas = static_cast<uint16_t>(cultivo.faltas);
        lista.push_back(guardado);
    }
}

void Huerta::cargar(const DatosCultivoGuardado& guardado) {
    if (catalogo.buscar(guardado.tipo) == nullptr) {
        return;
    }
    Cultivo cultivo;
    cultivo.tipo = guardado.tipo;
    cultivo.dias_de_crecimiento = guardado.dias_de_crecimiento;
    cultivo.madura = guardado.madura;
    cultivo.faltas = guardado.faltas;
    cultivos[{static_cast<int>(guardado.columna), static_cast<int>(guardado.fila)}] = cultivo;
}
