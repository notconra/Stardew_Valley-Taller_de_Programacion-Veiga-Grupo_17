#include "gestor_de_enemigos.h"

#include <cstdlib>

namespace {
// Cuánto dura el gesto de ataque del lado visual, en ticks. Es independiente del cooldown entre
// golpes (ticks_entre_ataques, que es por tipo): esto solo controla cuánto se ve la animación.
constexpr uint32_t DURACION_ANIMACION_ATAQUE_TICKS = 16;

// Cuánto dura la animación de morir antes de que el cadáver deje de dibujarse (a 40 ticks/seg,
// unos 600ms).
constexpr uint32_t DURACION_ANIMACION_MUERTE_TICKS = 36;
}  // namespace

GestorDeEnemigos::GestorDeEnemigos(const CatalogoDeEnemigos& catalogo_de_enemigos)
    : catalogo(catalogo_de_enemigos) {
}

int GestorDeEnemigos::poblar_en(const std::vector<Casilla>& casillas, TipoEnemigo tipo) {
    const DefinicionDeEnemigo* definicion = catalogo.buscar(tipo);
    if (definicion == nullptr) {
        return 0;
    }

    int colocados = 0;
    for (const Casilla& casilla : casillas) {
        Enemigo enemigo;
        enemigo.id = siguiente_id++;
        enemigo.tipo = tipo;
        enemigo.posicion = casilla;
        enemigo.vida_maxima = definicion->vida_maxima;
        enemigo.vida = definicion->vida_maxima;
        enemigo.invulnerable = definicion->invulnerable;
        enemigos_por_id.emplace(enemigo.id, enemigo);
        ++colocados;
    }
    return colocados;
}

ResultadoDeGolpeEnemigo GestorDeEnemigos::golpear(int id, int danio) {
    ResultadoDeGolpeEnemigo resultado;
    auto it = enemigos_por_id.find(id);
    if (it == enemigos_por_id.end() || !it->second.esta_vivo()) {
        return resultado;
    }

    Enemigo& enemigo = it->second;
    resultado.golpeo = true;  // el golpe conectó; si es invulnerable, igual no le baja la vida
    if (enemigo.recibir_golpe(danio)) {
        resultado.murio = true;
        const DefinicionDeEnemigo* definicion = catalogo.buscar(enemigo.tipo);
        enemigo.ticks_para_revivir = definicion != nullptr ? definicion->ticks_para_revivir : 0;
        enemigo.ticks_de_animacion_muerte = DURACION_ANIMACION_MUERTE_TICKS;
    }
    return resultado;
}

std::vector<int> GestorDeEnemigos::actualizar() {
    std::vector<int> cambiaron;
    for (auto& [id, enemigo] : enemigos_por_id) {
        if (enemigo.esta_vivo()) {
            if (enemigo.ticks_para_poder_atacar > 0) {
                --enemigo.ticks_para_poder_atacar;
            }
            if (enemigo.ticks_de_animacion_ataque > 0) {
                --enemigo.ticks_de_animacion_ataque;
                if (enemigo.ticks_de_animacion_ataque == 0) {
                    cambiaron.push_back(id);
                }
            }
            continue;
        }

        // Muerto: la animación de morir y el reloj de revivir corren en paralelo. Si terminó de
        // morir sin que haya revivido todavía, el cliente tiene que dejar de dibujar el cadáver;
        // si en este mismo tick además revive, el "vida llena" de abajo ya lo cubre.
        bool cambio_visible = false;
        if (enemigo.ticks_de_animacion_muerte > 0) {
            --enemigo.ticks_de_animacion_muerte;
            if (enemigo.ticks_de_animacion_muerte == 0) {
                cambio_visible = true;
            }
        }
        if (enemigo.ticks_para_revivir > 0) {
            --enemigo.ticks_para_revivir;
            if (enemigo.ticks_para_revivir == 0) {
                enemigo.vida = enemigo.vida_maxima;
                cambio_visible = true;
            }
        }
        if (cambio_visible) {
            cambiaron.push_back(id);
        }
    }
    return cambiaron;
}

namespace {

bool son_adyacentes(const Casilla& a, const Casilla& b) {
    const int distancia_columnas = std::abs(a.columna - b.columna);
    const int distancia_filas = std::abs(a.fila - b.fila);
    return distancia_columnas <= 1 && distancia_filas <= 1 &&
           (a.columna != b.columna || a.fila != b.fila);
}

}  // namespace

std::vector<GolpeDeEnemigo> GestorDeEnemigos::atacar_adyacentes(
    const std::vector<std::pair<uint32_t, Casilla>>& jugadores) {
    std::vector<GolpeDeEnemigo> golpes;
    for (auto& [id, enemigo] : enemigos_por_id) {
        if (!enemigo.esta_vivo() || enemigo.ticks_para_poder_atacar > 0) {
            continue;
        }
        for (const auto& [id_jugador, casilla] : jugadores) {
            if (!son_adyacentes(enemigo.posicion, casilla)) {
                continue;
            }
            const DefinicionDeEnemigo* definicion = catalogo.buscar(enemigo.tipo);
            const int danio = definicion != nullptr ? definicion->danio : 0;
            golpes.push_back(GolpeDeEnemigo{static_cast<uint32_t>(id), id_jugador, danio});
            enemigo.ticks_para_poder_atacar =
                definicion != nullptr ? definicion->ticks_entre_ataques : 1;
            enemigo.ticks_de_animacion_ataque = DURACION_ANIMACION_ATAQUE_TICKS;
            break; 
        }
    }
    return golpes;
}

void GestorDeEnemigos::limpiar() {
    enemigos_por_id.clear();
}

const Enemigo* GestorDeEnemigos::buscar(int id) const {
    auto it = enemigos_por_id.find(id);
    return it == enemigos_por_id.end() ? nullptr : &it->second;
}

const Enemigo* GestorDeEnemigos::enemigo_vivo_en(int columna, int fila) const {
    for (const auto& [id, enemigo] : enemigos_por_id) {
        if (enemigo.esta_vivo() && enemigo.posicion.columna == columna &&
            enemigo.posicion.fila == fila) {
            return &enemigo;
        }
    }
    return nullptr;
}

int GestorDeEnemigos::cantidad() const {
    return static_cast<int>(enemigos_por_id.size());
}

void GestorDeEnemigos::agregar_a(EstadoJuego& estado) const {
    for (const auto& [id, enemigo] : enemigos_por_id) {
        if (enemigo.esta_vivo() || enemigo.ticks_de_animacion_muerte > 0) {
            estado.enemigos.push_back(estado_de(enemigo));
        }
    }
}

EstadoEnemigo GestorDeEnemigos::estado_de(const Enemigo& enemigo) {
    EstadoEnemigo estado;
    estado.id = static_cast<uint32_t>(enemigo.id);
    estado.tipo = enemigo.tipo;
    estado.columna = static_cast<uint32_t>(enemigo.posicion.columna);
    estado.fila = static_cast<uint32_t>(enemigo.posicion.fila);
    estado.vida = static_cast<uint16_t>(enemigo.vida);
    estado.vida_maxima = static_cast<uint16_t>(enemigo.vida_maxima);
    estado.atacando = enemigo.ticks_de_animacion_ataque > 0;
    estado.muriendo = enemigo.ticks_de_animacion_muerte > 0;
    return estado;
}
