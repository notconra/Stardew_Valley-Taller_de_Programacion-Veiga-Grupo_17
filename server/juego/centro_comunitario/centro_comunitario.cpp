#include "centro_comunitario.h"

#include <algorithm>
#include <string>
#include <vector>

CentroComunitario::CentroComunitario(const CatalogoDeLotes& catalogo_de_lotes)
    : catalogo(catalogo_de_lotes) {
    for (const auto& [tipo, definicion] : catalogo.definiciones()) {
        estados[tipo].aportado.assign(definicion.requisitos.size(), 0);
    }
}

EstadoDeLote* CentroComunitario::estado_de(TipoLote tipo) {
    auto encontrado = estados.find(tipo);
    return encontrado == estados.end() ? nullptr : &encontrado->second;
}

int CentroComunitario::requisito_para(TipoLote tipo, const RanuraInventario& item) const {
    const DefinicionDeLote* definicion = catalogo.buscar(tipo);
    auto estado = estados.find(tipo);
    if (definicion == nullptr || estado == estados.end() || estado->second.completo ||
        item.tipo == TipoItem::NINGUNO || item.cantidad == 0) {
        return -1;
    }
    for (std::size_t i = 0; i < definicion->requisitos.size(); ++i) {
        const Requisito& requisito = definicion->requisitos[i];
        const bool le_falta = estado->second.aportado[i] < requisito.cantidad;
        if (requisito.tipo == TipoDeRequisito::ITEM && requisito.item == item.tipo && le_falta &&
            item.calidad >= requisito.calidad_minima) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

uint16_t CentroComunitario::cuanto_acepta(TipoLote tipo, const RanuraInventario& item) const {
    const int indice = requisito_para(tipo, item);
    if (indice < 0) {
        return 0;
    }
    const Requisito& requisito = catalogo.buscar(tipo)->requisitos[indice];
    const uint32_t falta = requisito.cantidad - estados.at(tipo).aportado[indice];
    return static_cast<uint16_t>(std::min<uint32_t>(falta, item.cantidad));
}

uint16_t CentroComunitario::donar_item(TipoLote tipo, const RanuraInventario& item, uint32_t dia) {
    const uint16_t tomadas = cuanto_acepta(tipo, item);
    if (tomadas == 0) {
        return 0;
    }
    estado_de(tipo)->aportado[requisito_para(tipo, item)] += tomadas;
    completar_si_corresponde(tipo, dia);
    return tomadas;
}

uint32_t CentroComunitario::monedas_pedidas(TipoLote tipo) const {
    const DefinicionDeLote* definicion = catalogo.buscar(tipo);
    if (definicion == nullptr || esta_completo(tipo)) {
        return 0;
    }
    for (const Requisito& requisito : definicion->requisitos) {
        if (requisito.tipo == TipoDeRequisito::DINERO) {
            return requisito.cantidad;
        }
    }
    return 0;
}

bool CentroComunitario::donar_monedas(TipoLote tipo, uint32_t dia) {
    if (monedas_pedidas(tipo) == 0) {
        return false;
    }
    const DefinicionDeLote* definicion = catalogo.buscar(tipo);
    EstadoDeLote* estado = estado_de(tipo);
    for (std::size_t i = 0; i < definicion->requisitos.size(); ++i) {
        if (definicion->requisitos[i].tipo == TipoDeRequisito::DINERO) {
            estado->aportado[i] = definicion->requisitos[i].cantidad;
        }
    }
    completar_si_corresponde(tipo, dia);
    return true;
}

bool CentroComunitario::cumplir_logro(LogroDePelea logro, uint32_t dia) {
    const DefinicionDeLote* definicion = catalogo.buscar(TipoLote::PELEA);
    EstadoDeLote* estado = estado_de(TipoLote::PELEA);
    if (definicion == nullptr || estado == nullptr) {
        return false;
    }
    for (std::size_t i = 0; i < definicion->requisitos.size(); ++i) {
        const Requisito& requisito = definicion->requisitos[i];
        if (requisito.tipo == TipoDeRequisito::LOGRO && requisito.logro == logro &&
            estado->aportado[i] < requisito.cantidad) {
            estado->aportado[i] = requisito.cantidad;
            completar_si_corresponde(TipoLote::PELEA, dia);
            return true;
        }
    }
    return false;
}

bool CentroComunitario::completar(TipoLote tipo, uint32_t dia) {
    const DefinicionDeLote* definicion = catalogo.buscar(tipo);
    EstadoDeLote* estado = estado_de(tipo);
    if (definicion == nullptr || estado == nullptr || estado->completo) {
        return false;
    }
    for (std::size_t i = 0; i < definicion->requisitos.size(); ++i) {
        estado->aportado[i] = definicion->requisitos[i].cantidad;
    }
    completar_si_corresponde(tipo, dia);
    return true;
}

void CentroComunitario::completar_si_corresponde(TipoLote tipo, uint32_t dia) {
    const DefinicionDeLote* definicion = catalogo.buscar(tipo);
    EstadoDeLote* estado = estado_de(tipo);
    if (definicion == nullptr || estado == nullptr || estado->completo) {
        return;
    }
    for (std::size_t i = 0; i < definicion->requisitos.size(); ++i) {
        if (estado->aportado[i] < definicion->requisitos[i].cantidad) {
            return;
        }
    }
    estado->completo = true;
    estado->dia_completado = dia;
    lotes_recien_completados.push_back(tipo);

    const bool estan_todos = std::all_of(estados.begin(), estados.end(), [](const auto& par) {
        return par.second.completo;
    });
    if (estan_todos && !terminado) {
        terminado = true;
        fin_sin_avisar = true;
    }
}

bool CentroComunitario::esta_completo(TipoLote tipo) const {
    auto encontrado = estados.find(tipo);
    return encontrado != estados.end() && encontrado->second.completo;
}

bool CentroComunitario::esta_terminado() const {
    return terminado;
}

uint32_t CentroComunitario::aportado(TipoLote tipo, std::size_t indice_requisito) const {
    auto encontrado = estados.find(tipo);
    if (encontrado == estados.end() || indice_requisito >= encontrado->second.aportado.size()) {
        return 0;
    }
    return encontrado->second.aportado[indice_requisito];
}

bool CentroComunitario::hay_precios_del_lote_de_dinero(uint32_t dia) const {
    auto encontrado = estados.find(TipoLote::DINERO);
    return encontrado != estados.end() && encontrado->second.completo &&
           dia == encontrado->second.dia_completado + 1;
}

std::string CentroComunitario::mapa_para(const std::string& destino) const {
    if (destino == MAPA_DEL_CENTRO && terminado) {
        return MAPA_DEL_CENTRO_ARREGLADO;
    }
    return destino;
}

std::vector<TipoLote> CentroComunitario::tomar_lotes_completados() {
    std::vector<TipoLote> completados;
    completados.swap(lotes_recien_completados);
    return completados;
}

bool CentroComunitario::tomar_fin_del_juego() {
    const bool hay_que_avisar = fin_sin_avisar;
    fin_sin_avisar = false;
    return hay_que_avisar;
}

DatosCentroGuardado CentroComunitario::guardar() const {
    DatosCentroGuardado guardado;
    guardado.terminado = terminado;
    for (const auto& [tipo, estado] : estados) {
        guardado.lotes.push_back(
            DatosLoteGuardado{tipo, estado.aportado, estado.completo, estado.dia_completado});
    }
    return guardado;
}

void CentroComunitario::cargar(const DatosCentroGuardado& guardado) {
    for (const DatosLoteGuardado& lote : guardado.lotes) {
        EstadoDeLote* estado = estado_de(lote.tipo);
        if (estado == nullptr) {
            continue;
        }
        for (std::size_t i = 0; i < estado->aportado.size() && i < lote.aportado.size(); ++i) {
            estado->aportado[i] = lote.aportado[i];
        }
        estado->completo = lote.completo;
        estado->dia_completado = lote.dia_completado;
    }
    terminado = guardado.terminado;
    lotes_recien_completados.clear();
    fin_sin_avisar = false;
}
