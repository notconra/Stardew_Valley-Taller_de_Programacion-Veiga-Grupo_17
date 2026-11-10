#include "gestor_de_enemigos.h"

#include <algorithm>
#include <cstdlib>

#include "../azar.h"

namespace {
constexpr uint32_t DURACION_ANIMACION_ATAQUE_TICKS = 16;
constexpr uint32_t DURACION_ANIMACION_MUERTE_TICKS = 36;
// Probabilidad de que un enemigo que no ve a nadie se quede un rato quieto en vez de dar un paso.
constexpr double PROBABILIDAD_DE_DESCANSAR = 0.4;
}  // namespace

GestorDeEnemigos::GestorDeEnemigos(
    const CatalogoDeEnemigos& catalogo_de_enemigos, uint8_t ticks_por_segundo)
    : catalogo(catalogo_de_enemigos),
      ms_por_tick(ticks_por_segundo > 0 ? 1000u / ticks_por_segundo : 25u) {
}

uint32_t GestorDeEnemigos::ticks_por_paso(
    const Enemigo& enemigo, const DefinicionDeEnemigo& definicion) const {
    if (enemigo.en_frenesi && definicion.habilidad_de_frenesi == HabilidadDeFrenesi::VELOCIDAD) {
        const auto ticks = static_cast<uint32_t>(
            static_cast<double>(definicion.ticks_por_casilla) / definicion.multiplicador_de_frenesi);
        return std::max<uint32_t>(1, ticks);
    }
    return definicion.ticks_por_casilla;
}

int GestorDeEnemigos::danio_de(const Enemigo& enemigo, const DefinicionDeEnemigo& definicion) const {
    if (enemigo.en_frenesi && definicion.habilidad_de_frenesi == HabilidadDeFrenesi::DANIO) {
        return static_cast<int>(definicion.danio * definicion.multiplicador_de_frenesi);
    }
    return definicion.danio;
}

void GestorDeEnemigos::dar_paso(Enemigo& enemigo, uint32_t ticks) {
    enemigo.ticks_para_moverse = ticks;
    enemigo.ms_por_casilla = static_cast<uint16_t>(std::min<uint32_t>(ticks * ms_por_tick, 65535));
}

void GestorDeEnemigos::recalcular_frenesi(int grupo, std::vector<int>& cambiaron) {
    if (grupo == 0) {
        return;
    }
    const int muertos = static_cast<int>(std::count_if(
        enemigos_por_id.begin(), enemigos_por_id.end(), [grupo](const auto& entrada) {
            return entrada.second.grupo == grupo && !entrada.second.esta_vivo();
        }));
    // Un vivo está en frenesí si hay algún otro del grupo muerto (y él está vivo, así que no
    // cuenta entre los muertos).
    const bool en_frenesi = muertos > 0;
    for (auto& [id, enemigo] : enemigos_por_id) {
        if (enemigo.grupo != grupo || !enemigo.esta_vivo()) {
            continue;
        }
        const DefinicionDeEnemigo* definicion = catalogo.buscar(enemigo.tipo);
        if (definicion == nullptr ||
            definicion->habilidad_de_frenesi == HabilidadDeFrenesi::NINGUNA) {
            continue;
        }
        if (en_frenesi != enemigo.en_frenesi) {
            enemigo.en_frenesi = en_frenesi;
            cambiaron.push_back(id);
        }
    }
}

int GestorDeEnemigos::poblar_pareja(
    const Casilla& primera, TipoEnemigo tipo_primera, const Casilla& segunda,
    TipoEnemigo tipo_segunda) {
    if (catalogo.buscar(tipo_primera) == nullptr || catalogo.buscar(tipo_segunda) == nullptr) {
        return 0;
    }
    const int grupo = siguiente_grupo++;
    return poblar_en({primera}, tipo_primera, grupo) + poblar_en({segunda}, tipo_segunda, grupo);
}

std::vector<int> GestorDeEnemigos::derramar_sangre(const std::vector<Casilla>& heridos) {
    std::vector<int> ensangrentados;
    if (heridos.empty()) {
        return ensangrentados;
    }
    for (auto& [id, enemigo] : enemigos_por_id) {
        if (enemigo.esta_vivo() || !enemigo.espera_sangre) {
            continue;
        }
        const DefinicionDeEnemigo* definicion = catalogo.buscar(enemigo.tipo);
        if (definicion == nullptr) {
            continue;
        }
        const bool hay_herido_cerca =
            std::any_of(heridos.begin(), heridos.end(), [&](const Casilla& herido) {
                return distancia_en_casillas(herido, enemigo.posicion) <=
                       definicion->radio_de_sangre;
            });
        if (!hay_herido_cerca) {
            continue;
        }
        enemigo.espera_sangre = false;
        enemigo.con_sangre = true;
        enemigo.ticks_para_revivir = std::max<uint32_t>(1, definicion->ticks_para_revivir);
        ensangrentados.push_back(id);
    }
    return ensangrentados;
}

int GestorDeEnemigos::poblar_grupo_en(const std::vector<Casilla>& casillas, TipoEnemigo tipo) {
    return poblar_en(casillas, tipo, siguiente_grupo++);
}

void GestorDeEnemigos::perseguir(int id, uint32_t id_jugador) {
    const auto encontrado = enemigos_por_id.find(id);
    if (encontrado != enemigos_por_id.end()) {
        encontrado->second.jugador_perseguido = id_jugador;
    }
}

int GestorDeEnemigos::poblar_en(
    const std::vector<Casilla>& casillas, TipoEnemigo tipo, int grupo) {
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
        enemigo.grupo = grupo;
        enemigo.vida_maxima = definicion->vida_maxima;
        enemigo.vida = definicion->vida_maxima;
        enemigo.invulnerable = definicion->invulnerable;
        enemigo.ms_por_casilla = static_cast<uint16_t>(
            std::min<uint32_t>(definicion->ticks_por_casilla * ms_por_tick, 65535));
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
        const bool con_sangre = definicion != nullptr && definicion->revive_con_sangre;
        enemigo.ticks_para_revivir =
            (definicion != nullptr && !con_sangre) ? definicion->ticks_para_revivir : 0;
        enemigo.espera_sangre = con_sangre;
        enemigo.en_frenesi = false;
        enemigo.ticks_de_animacion_muerte = DURACION_ANIMACION_MUERTE_TICKS;
        recalcular_frenesi(enemigo.grupo, resultado.afectados);
    }
    return resultado;
}

std::vector<int> GestorDeEnemigos::actualizar() {
    std::vector<int> cambiaron;
    std::vector<int> revividos_en_grupo;
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
                enemigo.con_sangre = false;
                cambio_visible = true;
                revividos_en_grupo.push_back(enemigo.grupo);
            }
        }
        if (cambio_visible) {
            cambiaron.push_back(id);
        }
    }
    for (int grupo : revividos_en_grupo) {
        recalcular_frenesi(grupo, cambiaron);
    }
    return cambiaron;
}

namespace {

int signo(int valor) {
    return (valor > 0) - (valor < 0);
}

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
    const uint32_t paso = ticks_por_paso(enemigo, definicion);

    if (Azar::con_probabilidad(PROBABILIDAD_DE_DESCANSAR)) {
        enemigo.ticks_para_moverse = static_cast<uint32_t>(Azar::entero(paso, 3 * paso - 1));
        return false;
    }

    static constexpr Casilla PASOS[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    static constexpr Direccion DIRECCIONES[4] = {
        Direccion::ARRIBA, Direccion::ABAJO, Direccion::IZQUIERDA, Direccion::DERECHA};
    const int elegido = Azar::entero(0, 3);
    const Casilla destino{
        enemigo.posicion.columna + PASOS[elegido].columna,
        enemigo.posicion.fila + PASOS[elegido].fila};
    const bool muy_lejos =
        std::abs(destino.columna - enemigo.origen.columna) > definicion.radio_de_deambulo ||
        std::abs(destino.fila - enemigo.origen.fila) > definicion.radio_de_deambulo;
    if (muy_lejos || !colisiones.esta_libre(destino.columna, destino.fila) ||
        enemigo_vivo_en(destino.columna, destino.fila) != nullptr) {
        enemigo.ticks_para_moverse = paso / 2;
        return false;
    }
    enemigo.mirando = DIRECCIONES[elegido];
    enemigo.posicion = destino;
    dar_paso(enemigo, paso);
    return true;
}

const Casilla* GestorDeEnemigos::elegir_objetivo(
    Enemigo& enemigo, const DefinicionDeEnemigo& definicion,
    const std::vector<std::pair<uint32_t, Casilla>>& jugadores, int& distancia) const {
    // Si tiene un jugador marcado (la manada que se avecina), va por él sin mirar la visión.
    if (enemigo.jugador_perseguido != 0) {
        const auto perseguido = std::find_if(
            jugadores.begin(), jugadores.end(), [&enemigo](const auto& jugador) {
                return jugador.first == enemigo.jugador_perseguido;
            });
        if (perseguido != jugadores.end()) {
            distancia = distancia_en_casillas(perseguido->second, enemigo.posicion);
            return &perseguido->second;
        }
        enemigo.jugador_perseguido = 0;  // ya no está en el sector: vuelve a la regla normal
    }

    const Casilla* objetivo = nullptr;
    distancia = definicion.radio_de_vision + 1;
    for (const auto& [id_jugador, casilla] : jugadores) {
        const int lejos = distancia_en_casillas(casilla, enemigo.posicion);
        if (lejos < distancia) {
            distancia = lejos;
            objetivo = &casilla;
        }
    }
    return objetivo;
}

bool GestorDeEnemigos::acercarse(
    Enemigo& enemigo, const DefinicionDeEnemigo& definicion, const Casilla& objetivo,
    const std::vector<std::pair<uint32_t, Casilla>>& jugadores, const Colisiones& colisiones) {
    const int dc = objetivo.columna - enemigo.posicion.columna;
    const int df = objetivo.fila - enemigo.posicion.fila;
    const Casilla paso_horizontal{enemigo.posicion.columna + signo(dc), enemigo.posicion.fila};
    const Casilla paso_vertical{enemigo.posicion.columna, enemigo.posicion.fila + signo(df)};
    const bool horizontal_primero = std::abs(dc) >= std::abs(df);
    const Casilla pasos[2] = {
        horizontal_primero ? paso_horizontal : paso_vertical,
        horizontal_primero ? paso_vertical : paso_horizontal};

    for (const Casilla& paso : pasos) {
        if (paso == enemigo.posicion || !colisiones.esta_libre(paso.columna, paso.fila) ||
            enemigo_vivo_en(paso.columna, paso.fila) != nullptr) {
            continue;
        }
        const bool ocupada_por_jugador = std::any_of(
            jugadores.begin(), jugadores.end(),
            [&paso](const auto& jugador) { return jugador.second == paso; });
        if (ocupada_por_jugador) {
            continue;
        }
        enemigo.mirando = direccion_hacia(enemigo.posicion, paso);
        enemigo.posicion = paso;
        dar_paso(enemigo, ticks_por_paso(enemigo, definicion));
        return true;
    }
    return false;
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

        int distancia = 0;
        const Casilla* objetivo = elegir_objetivo(enemigo, *definicion, jugadores, distancia);
        bool se_movio = false;
        if (objetivo == nullptr) {
            se_movio = deambular(enemigo, *definicion, colisiones);
        } else if (distancia > 1) {  // a distancia 1 ya está al lado: le toca pegar
            se_movio = acercarse(enemigo, *definicion, *objetivo, jugadores, colisiones);
        }
        if (se_movio) {
            movidos.push_back(id);
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
            const int danio = definicion != nullptr ? danio_de(enemigo, *definicion) : 0;
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
    estado.ms_por_casilla = enemigo.ms_por_casilla;
    estado.frenesi = enemigo.en_frenesi;
    estado.sangre = enemigo.con_sangre;
    return estado;
}
