#include "objetos_del_mundo.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <random>
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
        if (definicion.item_colocable != TipoItem::NINGUNO &&
            buscar_por_item(definicion.item_colocable) != nullptr) {
            throw std::runtime_error(
                "objetos del mundo: '" + definicion.clave +
                "' usa un ítem que ya coloca otro objeto");
        }
        if (definicion.probabilidad_de_no_dejar < 0.0 ||
            definicion.probabilidad_de_no_dejar > 1.0) {
            throw std::runtime_error(
                "objetos del mundo: '" + definicion.clave +
                "' tiene una probabilidad de no dejar producto fuera de [0, 1]");
        }
        if (definicion.producto != TipoItem::NINGUNO && definicion.cantidad < 1) {
            throw std::runtime_error(
                "objetos del mundo: '" + definicion.clave +
                "' deja un producto con cantidad menor a uno");
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

const DefinicionDeObjeto* CatalogoDeObjetos::buscar_por_item(TipoItem item) const {
    if (item == TipoItem::NINGUNO) {
        return nullptr;
    }
    for (const auto& entrada : definiciones_por_tipo) {
        if (entrada.second.item_colocable == item) {
            return &entrada.second;
        }
    }
    return nullptr;
}

const std::map<TipoObjeto, DefinicionDeObjeto>& CatalogoDeObjetos::definiciones() const {
    return definiciones_por_tipo;
}

GestorDeObjetos::GestorDeObjetos(
    const std::string& id_del_mapa, const Mapa& mapa_de_partida,
    const CatalogoDeObjetos& catalogo_de_objetos, Colisiones& colisiones_del_mapa)
    : id_mapa(id_del_mapa),
      mapa(mapa_de_partida),
      catalogo(catalogo_de_objetos),
      colisiones(colisiones_del_mapa),
      id_por_casilla(
          static_cast<std::size_t>(mapa_de_partida.cantidad_de_columnas()) *
              mapa_de_partida.cantidad_de_filas(),
          0) {
}

std::size_t GestorDeObjetos::indice(int columna, int fila) const {
    return static_cast<std::size_t>(fila) * mapa.cantidad_de_columnas() + columna;
}

int GestorDeObjetos::colocar(TipoObjeto tipo, int columna, int fila) {
    // esta_libre también devuelve false si la casilla cae fuera del mapa.
    if (catalogo.buscar(tipo) == nullptr || !colisiones.esta_libre(columna, fila)) {
        return 0;
    }

    const int id = siguiente_id++;
    objetos_por_id.emplace(id, ObjetoDelMundo{id, columna, fila, tipo});
    id_por_casilla[indice(columna, fila)] = id;
    colisiones.colocar(columna, fila);
    return id;
}

ResultadoDeGolpe GestorDeObjetos::golpear(int id, TipoItem herramienta_usada) {
    ResultadoDeGolpe res;
    auto it = objetos_por_id.find(id);
    if (it == objetos_por_id.end())
        return res;

    ObjetoDelMundo& objeto = it->second;
    const DefinicionDeObjeto* def = catalogo.buscar(objeto.tipo);
    if (!def || def->herramienta == TipoItem::NINGUNO || def->herramienta != herramienta_usada) {
        return res;  // Herramienta incorrecta o el objeto no se puede golpear
    }

    res.golpeo = true;
    objeto.golpes_recibidos++;
    if (objeto.golpes_recibidos >= def->golpes_para_picar) {
        res.rotura = romper(id);
    }
    return res;
}

int GestorDeObjetos::poblar_al_azar(
    const std::vector<PesoDeObjeto>& pesos, int cantidad, uint8_t propiedad_requerida) {
    if (cantidad < 0) {
        throw std::invalid_argument(
            "objetos del mundo: la cantidad a colocar no puede ser negativa");
    }

    std::vector<Casilla> candidatas;
    for (int fila = 0; fila < mapa.cantidad_de_filas(); ++fila) {
        for (int columna = 0; columna < mapa.cantidad_de_columnas(); ++columna) {
            const bool sirve_el_terreno = propiedad_requerida == PropiedadDeCasilla::NINGUNA ||
                                          mapa.tiene(columna, fila, propiedad_requerida);
            if (sirve_el_terreno && colisiones.esta_libre(columna, fila)) {
                candidatas.push_back(Casilla{columna, fila});
            }
        }
    }

    std::shuffle(candidatas.begin(), candidatas.end(), motor);
    if (static_cast<int>(candidatas.size()) > cantidad) {
        candidatas.resize(static_cast<size_t>(cantidad));
    }
    return colocar_al_azar(candidatas, pesos);
}

int GestorDeObjetos::poblar_en(
    const std::vector<Casilla>& casillas, const std::vector<PesoDeObjeto>& pesos) {
    return colocar_al_azar(casillas, pesos);
}

int GestorDeObjetos::colocar_al_azar(
    const std::vector<Casilla>& casillas, const std::vector<PesoDeObjeto>& pesos) {
    if (pesos.empty()) {
        throw std::invalid_argument("objetos del mundo: hace falta al menos un tipo con peso");
    }

    std::vector<double> valores;
    double total = 0.0;
    for (const PesoDeObjeto& entrada : pesos) {
        if (catalogo.buscar(entrada.tipo) == nullptr) {
            throw std::invalid_argument(
                "objetos del mundo: se pidió poblar con un tipo inexistente");
        }
        if (!std::isfinite(entrada.peso) || entrada.peso < 0.0) {
            throw std::invalid_argument("objetos del mundo: los pesos tienen que ser >= 0");
        }
        valores.push_back(entrada.peso);
        total += entrada.peso;
    }
    if (total <= 0.0) {
        throw std::invalid_argument("objetos del mundo: todos los pesos están en cero");
    }

    std::discrete_distribution<std::size_t> sorteo_de_tipo(valores.begin(), valores.end());
    int colocados = 0;
    for (const Casilla& casilla : casillas) {
        if (colocar(pesos[sorteo_de_tipo(motor)].tipo, casilla.columna, casilla.fila) != 0) {
            ++colocados;
        }
    }
    return colocados;
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

    // colocar solo acepta tipos del catálogo, así que la definición siempre existe.
    const DefinicionDeObjeto& definicion = *catalogo.buscar(encontrado->second.tipo);
    retirar(id);
    resultado.se_rompio = true;

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

bool GestorDeObjetos::retirar(int id) {
    const auto encontrado = objetos_por_id.find(id);
    if (encontrado == objetos_por_id.end()) {
        return false;
    }

    const ObjetoDelMundo& objeto = encontrado->second;
    id_por_casilla[indice(objeto.columna, objeto.fila)] = 0;
    colisiones.eliminar(objeto.columna, objeto.fila);
    objetos_por_id.erase(encontrado);
    return true;
}

int GestorDeObjetos::retirar_objetos_naturales() {
    std::vector<int> ids;
    for (const auto& entrada : objetos_por_id) {
        // colocar solo acepta tipos del catálogo, así que la definición siempre existe.
        if (catalogo.buscar(entrada.second.tipo)->item_colocable == TipoItem::NINGUNO) {
            ids.push_back(entrada.first);
        }
    }
    for (const int id : ids) {
        retirar(id);
    }
    return static_cast<int>(ids.size());
}

void GestorDeObjetos::fijar_semilla(std::mt19937::result_type semilla) {
    motor.seed(semilla);
}

const ObjetoDelMundo* GestorDeObjetos::objeto_en(int columna, int fila) const {
    if (!mapa.esta_dentro(columna, fila)) {
        return nullptr;
    }
    const int id = id_por_casilla[indice(columna, fila)];
    return id == 0 ? nullptr : &objetos_por_id.at(id);
}

std::vector<ObjetoDelMundo> GestorDeObjetos::obtener_objetos_de_tipo(TipoObjeto tipo) const {
    std::vector<ObjetoDelMundo> resultado;
    for (const auto& [id, objeto] : objetos_por_id) {
        if (objeto.tipo == tipo) {
            resultado.push_back(objeto);
        }
    }
    return resultado;
}

int GestorDeObjetos::cantidad_de_objetos() const {
    return static_cast<int>(objetos_por_id.size());
}
