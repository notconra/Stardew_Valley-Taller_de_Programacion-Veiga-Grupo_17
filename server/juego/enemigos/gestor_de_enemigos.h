#ifndef GESTOR_DE_ENEMIGOS_H
#define GESTOR_DE_ENEMIGOS_H

#include <map>
#include <utility>
#include <vector>

#include "../../../common/mensajes/mensajes.h"
#include "../geometria.h"
#include "catalogo_de_enemigos.h"
#include "enemigo.h"

// Resultado de golpear a un enemigo. "golpeo" es false si el id no existe o ya estaba muerto (o es
// invulnerable); "murio" es true si este golpe en particular lo mató, para que quien ataca sepa
// cuándo soltar su drop de oro.
struct ResultadoDeGolpeEnemigo {
    bool golpeo = false;
    bool murio = false;
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
// perseguir ni atacar al jugador: esa IA la va a tener un controlador aparte que en cada tick
// recorra los enemigos de acá, los mueva y llame a golpear() cuando corresponda.
class GestorDeEnemigos {
private:
    const CatalogoDeEnemigos& catalogo;
    std::map<int, Enemigo> enemigos_por_id;
    int siguiente_id = 1;  // no se reinicia, así un id viejo nunca apunta a otro enemigo

public:
    explicit GestorDeEnemigos(const CatalogoDeEnemigos& catalogo_de_enemigos);

    // Coloca un enemigo del tipo dado en cada una de las casillas indicadas, con la vida llena.
    // Devuelve cuántos colocó (0 si el tipo no está en el catálogo).
    int poblar_en(const std::vector<Casilla>& casillas, TipoEnemigo tipo);

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
