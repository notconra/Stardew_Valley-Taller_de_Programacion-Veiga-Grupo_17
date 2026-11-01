#include "objetos_del_mundo.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

CatalogoDeObjetos::CatalogoDeObjetos(const std::vector<DefinicionDeObjeto>& definiciones) {
    for (const DefinicionDeObjeto& definicion : definiciones) {
        if (definicion.tipo == TipoObjeto::NINGUNO) {
            throw std::runtime_error("objetos del mundo: hay una definición sin tipo");
        }
        if (definicion.clave.empty()) {
            throw std::runtime_error("objetos del mundo: hay una definición sin clave");
        }
        if (definiciones_por_tipo.contains(definicion.tipo)) {
            throw std::runtime_error(
                "objetos del mundo: '" + definicion.clave + "' repite un tipo ya definido");
        }
        if (buscar_por_clave(definicion.clave) != nullptr) {
            throw std::runtime_error(
                "objetos del mundo: la clave '" + definicion.clave +
                "' está definida más de una vez");
        }
        definiciones_por_tipo.emplace(definicion.tipo, definicion);
    }
}

const DefinicionDeObjeto* CatalogoDeObjetos::buscar(TipoObjeto tipo) const {
    const auto encontrado = definiciones_por_tipo.find(tipo);
    return encontrado == definiciones_por_tipo.end() ? nullptr : &encontrado->second;
}

const DefinicionDeObjeto* CatalogoDeObjetos::buscar_por_clave(const std::string& clave) const {
    for (const auto& entrada : definiciones_por_tipo) {
        if (entrada.second.clave == clave) {
            return &entrada.second;
        }
    }
    return nullptr;
}

GestorDeObjetos::GestorDeObjetos(
    const std::string& id_del_mapa, const CatalogoDeObjetos& catalogo_de_objetos,
    Colisiones& colisiones_del_mapa)
    : id_mapa(id_del_mapa), catalogo(catalogo_de_objetos), colisiones(colisiones_del_mapa) {
}

int GestorDeObjetos::colocar(TipoObjeto tipo, int columna, int fila) {
    // esta_libre también devuelve false si la casilla cae fuera del mapa.
    if (catalogo.buscar(tipo) == nullptr || !colisiones.esta_libre(columna, fila)) {
        return 0;
    }

    const int id = siguiente_id++;
    objetos_por_id.emplace(id, ObjetoDelMundo{id, columna, fila, tipo});
    colisiones.colocar(columna, fila);
    return id;
}

void GestorDeObjetos::agregar_a(EstadoJuego& estado) const {
    for (const auto& entrada : objetos_por_id) {
        const ObjetoDelMundo& objeto = entrada.second;
        estado.objetos.push_back(EstadoObjetoDelMundo{
            static_cast<uint32_t>(objeto.id),
            id_mapa,
            static_cast<uint32_t>(objeto.columna),
            static_cast<uint32_t>(objeto.fila),
            objeto.tipo});
    }
}

ResultadoDeRotura GestorDeObjetos::romper(int id) {
    ResultadoDeRotura resultado;
    const auto encontrado = objetos_por_id.find(id);
    if (encontrado == objetos_por_id.end()) {
        return resultado;
    }

    const ObjetoDelMundo objeto = encontrado->second;
    colisiones.eliminar(objeto.columna, objeto.fila);
    objetos_por_id.erase(encontrado);
    resultado.se_rompio = true;

    // colocar solo acepta tipos del catálogo, así que la definición siempre existe.
    const DefinicionDeObjeto& definicion = *catalogo.buscar(objeto.tipo);
    if (definicion.producto == TipoItem::NINGUNO) {
        return resultado;
    }

    std::uniform_real_distribution<double> sorteo(0.0, 1.0);
    if (sorteo(motor) < definicion.probabilidad_de_no_dejar) {
        return resultado;
    }

    resultado.producto = definicion.producto;
    resultado.cantidad = definicion.cantidad;
    return resultado;
}

const ObjetoDelMundo* GestorDeObjetos::objeto_en(int columna, int fila) const {
    for (const auto& entrada : objetos_por_id) {
        const ObjetoDelMundo& objeto = entrada.second;
        if (objeto.columna == columna && objeto.fila == fila) {
            return &objeto;
        }
    }
    return nullptr;
}

int GestorDeObjetos::cantidad_de_objetos() const {
    return static_cast<int>(objetos_por_id.size());
}
