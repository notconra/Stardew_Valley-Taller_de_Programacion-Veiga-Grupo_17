#include "gestor_de_enemigos.h"

#include <algorithm>
#include <cstdlib>

namespace {
constexpr uint32_t DURACION_ANIMACION_ATAQUE_TICKS = 16;
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
        enemigo.origen = casilla;
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
    resultado.golpeo = true;
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
            if (enemigo.ticks_para_moverse > 0) {
                --enemigo.ticks_para_moverse;
            }
            if (enemigo.ticks_de_animacion_ataque > 0) {
                --enemigo.ticks_de_animacion_ataque;
                if (enemigo.ticks_de_animacion_ataque == 0) {
                    cambiaron.push_back(id);
                }
            }
            continue;
        }

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

int signo(int valor) {
    return (valor > 0) - (valor < 0);
}

// Hacia dónde hay que mirar para ver al otro: el eje con más distancia manda.
Direccion direccion_hacia(const Casilla& desde, const Casilla& hasta) {
    const int dc = hasta.columna - desde.columna;
    const int df = hasta.fila - desde.fila;
    if (std::abs(dc) > std::abs(df)) {
        return dc > 0 ? Direccion::DERECHA : Direccion::IZQUIERDA;
    }
    return df > 0 ? Direccion::ABAJO : Direccion::ARRIBA;
}

bool son_adyacentes(const Casilla& a, const Casilla& b) {
    const int distancia_columnas = std::abs(a.columna - b.columna);
    const int distancia_filas = std::abs(a.fila - b.fila);
    return distancia_columnas <= 1 && distancia_filas <= 1 &&
           (a.columna != b.columna || a.fila != b.fila);
}

}  // namespace

bool GestorDeEnemigos::deambular(
    Enemigo& enemigo, const DefinicionDeEnemigo& definicion, const Colisiones& colisiones) {
    if (definicion.radio_de_deambulo <= 0) {
        return false;
    }
    const uint32_t paso = definicion.ticks_por_casilla;

    // Camina a ratos: cuatro de cada diez veces se queda parado un rato en vez de dar un paso.
    if (azar() % 10 < 4) {
        enemigo.ticks_para_moverse = paso + (azar() % (2 * paso));
        return false;
    }

    static constexpr Casilla PASOS[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    static constexpr Direccion DIRECCIONES[4] = {
        Direccion::ARRIBA, Direccion::ABAJO, Direccion::IZQUIERDA, Direccion::DERECHA};
    const int elegido = static_cast<int>(azar() % 4);
    const Casilla destino{
        enemigo.posicion.columna + PASOS[elegido].columna,
        enemigo.posicion.fila + PASOS[elegido].fila};
    const bool muy_lejos =
        std::abs(destino.columna - enemigo.origen.columna) > definicion.radio_de_deambulo ||
        std::abs(destino.fila - enemigo.origen.fila) > definicion.radio_de_deambulo;
    if (muy_lejos || !colisiones.esta_libre(destino.columna, destino.fila) ||
        enemigo_vivo_en(destino.columna, destino.fila) != nullptr) {
        enemigo.ticks_para_moverse = paso / 2;  // reintenta pronto, con otra dirección
        return false;
    }
    enemigo.mirando = DIRECCIONES[elegido];
    enemigo.posicion = destino;
    enemigo.ticks_para_moverse = paso;
    return true;
}

std::vector<int> GestorDeEnemigos::mover_hacia(
    const std::vector<std::pair<uint32_t, Casilla>>& jugadores, const Colisiones& colisiones) {
    std::vector<int> movidos;
    for (auto& [id, enemigo] : enemigos_por_id) {
        const DefinicionDeEnemigo* definicion = catalogo.buscar(enemigo.tipo);
        if (!enemigo.esta_vivo() || definicion == nullptr || definicion->ticks_por_casilla == 0 ||
            enemigo.ticks_para_moverse > 0) {
            continue;
        }

        // El jugador más cercano dentro del radio de visión.
        const Casilla* objetivo = nullptr;
        int distancia_al_objetivo = definicion->radio_de_vision + 1;
        for (const auto& [id_jugador, casilla] : jugadores) {
            const int distancia = std::max(
                std::abs(casilla.columna - enemigo.posicion.columna),
                std::abs(casilla.fila - enemigo.posicion.fila));
            if (distancia < distancia_al_objetivo) {
                distancia_al_objetivo = distancia;
                objetivo = &casilla;
            }
        }
        if (objetivo == nullptr) {
            if (deambular(enemigo, *definicion, colisiones)) {
                movidos.push_back(id);
            }
            continue;
        }
        if (distancia_al_objetivo <= 1) {
            continue;  // ya está al lado y le toca pegar
        }

        const int dc = objetivo->columna - enemigo.posicion.columna;
        const int df = objetivo->fila - enemigo.posicion.fila;
        const Casilla paso_horizontal{enemigo.posicion.columna + signo(dc), enemigo.posicion.fila};
        const Casilla paso_vertical{enemigo.posicion.columna, enemigo.posicion.fila + signo(df)};
        // Primero el eje donde está más lejos; un eje con distancia 0 no es un paso válido.
        const bool horizontal_primero = std::abs(dc) >= std::abs(df);
        const Casilla* pasos[2] = {
            horizontal_primero ? &paso_horizontal : &paso_vertical,
            horizontal_primero ? &paso_vertical : &paso_horizontal};

        for (const Casilla* paso : pasos) {
            if (*paso == enemigo.posicion || !colisiones.esta_libre(paso->columna, paso->fila) ||
                enemigo_vivo_en(paso->columna, paso->fila) != nullptr) {
                continue;
            }
            const bool ocupada_por_jugador = std::any_of(
                jugadores.begin(), jugadores.end(), [paso](const auto& jugador) {
                    return jugador.second == *paso;
                });
            if (ocupada_por_jugador) {
                continue;
            }
            enemigo.mirando = direccion_hacia(enemigo.posicion, *paso);
            enemigo.posicion = *paso;
            enemigo.ticks_para_moverse = definicion->ticks_por_casilla;
            movidos.push_back(id);
            break;
        }
    }
    return movidos;
}

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
            enemigo.mirando = direccion_hacia(enemigo.posicion, casilla);
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
    estado.mirando = enemigo.mirando;
    return estado;
}