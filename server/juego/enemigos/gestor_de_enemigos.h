#ifndef GESTOR_DE_ENEMIGOS_H
#define GESTOR_DE_ENEMIGOS_H

#include <map>
#include <utility>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../colisiones/colisiones.h"
#include "../geometria.h"
#include "catalogo_de_enemigos.h"
#include "enemigo.h"

// Resultado de golpear a un enemigo. "golpeo" es false si el id no existe o ya estaba muerto (o es
// invulnerable); "murio" es true si este golpe en particular lo mató, para que quien ataca sepa
// cuándo soltar su drop de oro.
struct ResultadoDeGolpeEnemigo {
    bool golpeo = false;
    bool murio = false;
    // Otros enemigos que cambiaron por esta muerte (la pareja de un vampiro que entró en frenesí),
    // para avisarle al cliente.
    std::vector<int> afectados;
};

// Un golpe que un enemigo le dio a un jugador. Lo arma GestorDeEnemigos::atacar_adyacentes();
// aplicarlo a Personaje::recibir_golpe() es responsabilidad de quien lo llama (el controlador de
// enemigos sabe de personajes, GestorDeEnemigos solo de enemigos).
struct GolpeDeEnemigo {
    uint32_t id_enemigo = 0;
    uint32_t id_jugador = 0;
    int danio = 0;
};

// Los enemigos (vivos y muertos esperando revivir) de UN sector. Les asigna id, les aplica golpes
// y hace avanzar el reloj de revivir, igual de independiente de la red que Drops. No sabe
// perseguir ni atacar por su cuenta: ControladorDeEnemigos lo hace andar en cada tick con
// mover_hacia() y atacar_adyacentes().
class GestorDeEnemigos {
private:
    const CatalogoDeEnemigos& catalogo;
    std::map<int, Enemigo> enemigos_por_id;
    int siguiente_id = 1;  // no se reinicia, así un id viejo nunca apunta a otro enemigo
    int siguiente_grupo = 1;  // tampoco se reinicia, por el set de pelea ("del mismo grupo")
    uint32_t ms_por_tick;

    // Cuántos ticks tarda en dar un paso, con el frenesí aplicado.
    uint32_t ticks_por_paso(const Enemigo& enemigo, const DefinicionDeEnemigo& definicion) const;
    // Cuánto daño hace un golpe, con el frenesí aplicado.
    int danio_de(const Enemigo& enemigo, const DefinicionDeEnemigo& definicion) const;
    // Arranca un paso de "ticks" ticks: carga la espera y el tiempo que ve el cliente.
    void dar_paso(Enemigo& enemigo, uint32_t ticks);
    // Recalcula el frenesí de los vivos del grupo (en frenesí si algún otro del grupo está muerto)
    // y agrega a "cambiaron" los que cambiaron.
    void recalcular_frenesi(int grupo, std::vector<int>& cambiaron);

    // Un paso al azar (o un rato de descanso) de un enemigo que no ve a ningún jugador. Devuelve
    // true si se movió.
    bool deambular(
        Enemigo& enemigo, const DefinicionDeEnemigo& definicion, const Colisiones& colisiones);
    // A quién persigue: el jugador que tiene marcado o, si no, el más cercano dentro de su radio de
    // visión. Devuelve nullptr si no ve a nadie; "distancia" queda con la distancia a ese objetivo.
    const Casilla* elegir_objetivo(
        Enemigo& enemigo, const DefinicionDeEnemigo& definicion,
        const std::vector<std::pair<uint32_t, Casilla>>& jugadores, int& distancia) const;
    // Da un paso hacia el objetivo (por el eje más lejano o, si está bloqueado, por el otro).
    // Devuelve true si se movió.
    bool acercarse(
        Enemigo& enemigo, const DefinicionDeEnemigo& definicion, const Casilla& objetivo,
        const std::vector<std::pair<uint32_t, Casilla>>& jugadores, const Colisiones& colisiones);

public:
    explicit GestorDeEnemigos(
        const CatalogoDeEnemigos& catalogo_de_enemigos, uint8_t ticks_por_segundo = 40);

    // Coloca un enemigo del tipo dado en cada una de las casillas indicadas, con la vida llena.
    // Todos quedan en el "grupo" indicado (0 = sin grupo). Devuelve cuántos colocó (0 si el tipo no
    // está en el catálogo).
    int poblar_en(const std::vector<Casilla>& casillas, TipoEnemigo tipo, int grupo = 0);

    // Igual que poblar_en(), pero todos los colocados quedan en un mismo grupo nuevo (orcos).
    int poblar_grupo_en(const std::vector<Casilla>& casillas, TipoEnemigo tipo);

    // Coloca una pareja (los dos en un mismo grupo nuevo), como los vampiros. Devuelve cuántos
    // colocó (0 si alguno de los tipos no existe en el catálogo).
    int poblar_pareja(const Casilla& primera, TipoEnemigo tipo_primera, const Casilla& segunda,
        TipoEnemigo tipo_segunda);

    // Los muertos que reviven con sangre (vampiros) y tienen a uno de "heridos" a su radio de
    // sangre o menos: cae sangre sobre ellos y empiezan a contar para revivir. Devuelve sus ids.
    std::vector<int> derramar_sangre(const std::vector<Casilla>& heridos);

    // Hace que ese enemigo persiga al jugador aunque esté fuera de su radio de visión.
    void perseguir(int id, uint32_t id_jugador);

    // Golpea a un enemigo con "danio" puntos de daño. Si lo mata, le carga el reloj de revivir
    // según su definición (0 si ese tipo no revive).
    ResultadoDeGolpeEnemigo golpear(int id, int danio);

    // Avanza un tick el reloj de revivir de los enemigos muertos (al llegar a cero, revive con la
    // vida llena), el cooldown de ataque de los vivos y la duración de su animación de ataque. Se
    // llama una vez por tick del juego (ver GestorDeSectores::actualizar()). Devuelve los ids de
    // los que cambiaron de un modo que el cliente necesita ver (revivieron, o se les terminó la
    // animación de ataque), para que quien llama les avise a los jugadores del sector (acá no se
    // conoce al Notificador).
    std::vector<int> actualizar();

    // Cada enemigo vivo que se mueva (ticks_por_casilla > 0) y esté listo para dar un paso busca
    // al jugador más cercano dentro de su radio de visión; si no hay ninguno, deambula al azar
    // cerca de donde apareció. Si lo ve, avanza una casilla hacia él (en las 4
    // direcciones, sin diagonales), por el eje en el que más lejos está o, si ese está bloqueado,
    // por el otro. No pisa paredes, objetos, otros enemigos vivos ni jugadores, y se frena al
    // quedar al lado (ahí le toca pegar). "jugadores" son los (id, casilla) de este sector.
    // Devuelve los ids de los que se movieron, para avisarle a los jugadores del sector.
    std::vector<int> mover_hacia(
        const std::vector<std::pair<uint32_t, Casilla>>& jugadores, const Colisiones& colisiones);

    // Todo enemigo vivo y listo para golpear (cooldown en cero) que tenga a un jugador en una
    // casilla adyacente (las 8 direcciones) le pega al primero que encuentre y recarga su
    // cooldown. "jugadores" son los (id, casilla) de los jugadores presentes en este sector; no
    // hace falta filtrar a los desconectados, esto no los conoce. Movimiento y de quién es cada
    // casilla los resuelve quien llama, acá solo está la regla de "quién ataca a quién".
    std::vector<GolpeDeEnemigo> atacar_adyacentes(
        const std::vector<std::pair<uint32_t, Casilla>>& jugadores);

    // Borra todos los enemigos del sector. La usa la mazmorra cuando se vacía y se vuelve a
    // sortear, igual que Drops::limpiar().
    void limpiar();

    // El enemigo con ese id, o nullptr si no existe. El puntero deja de ser válido si se llama a
    // limpiar().
    const Enemigo* buscar(int id) const;

    // El primer enemigo vivo en esa casilla, o nullptr si no hay ninguno. Lo usa AccionAtacar para
    // resolver a qué le pega el jugador. El puntero deja de ser válido si se llama a limpiar().
    const Enemigo* enemigo_vivo_en(int columna, int fila) const;

    int cantidad() const;

    // Agrega todos los enemigos vivos al estado que se manda al cliente al entrar al sector. Los
    // muertos no se agregan: todavía no revivieron, no hay nada que dibujar.
    void agregar_a(EstadoJuego& estado) const;

    // El EstadoEnemigo (para la red) de un enemigo puntual. Lo usa Sector para armar el evento
    // EnemigoActualizado después de golpear() o cuando GestorDeSectores detecta una revivida.
    static EstadoEnemigo estado_de(const Enemigo& enemigo);

    GestorDeEnemigos(const GestorDeEnemigos&) = delete;
    GestorDeEnemigos& operator=(const GestorDeEnemigos&) = delete;
};

#endif