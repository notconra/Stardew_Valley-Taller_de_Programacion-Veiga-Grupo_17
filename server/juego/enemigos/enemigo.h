#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <cstdint>

#include "../../../common/tipo_enemigo.h"
#include "../geometria.h"

// Un enemigo colocado en el mapa de un nivel de la mazmorra. Ocupa una sola casilla como posición
// lógica; acá solo vive el estado de combate (vida, si está muerto esperando revivir).
//
// No sabe perseguir ni golpear al jugador: esa IA (moverse, atacar cuerpo a cuerpo o a distancia,
// huir del jugador en el caso de las Skulls) la va a tener un controlador aparte que recorra los
// enemigos de GestorDeEnemigos en cada tick. Acá solo están los datos y las reglas de "recibir un
// golpe" que son comunes a todos los tipos.
struct Enemigo {
    int id = 0;
    TipoEnemigo tipo = TipoEnemigo::NINGUNO;
    Casilla posicion{};
    int vida = 0;
    int vida_maxima = 0;

    // Las Skulls no se pueden matar: el jugador debe huir. Con esto en true, recibir_golpe() no
    // hace nada.
    bool invulnerable = false;

    // Ticks que faltan para que reviva, válido solo mientras está muerto (vida == 0). Lo fija
    // GestorDeEnemigos::golpear() al matarlo, a partir de la definición de su tipo; 0 significa
    // que no revive.
    uint32_t ticks_para_revivir = 0;

    // Ticks que faltan para que pueda volver a golpear a un jugador. En 0 está listo para atacar;
    // GestorDeEnemigos::atacar_adyacentes() lo recarga con el cooldown de su tipo cada vez que
    // conecta un golpe.
    uint32_t ticks_para_poder_atacar = 0;

    // Ticks que faltan para que termine la animación de ataque (para EstadoEnemigo::atacando).
    // Mientras sea > 0, GestorDeEnemigos::estado_de() lo manda en true. Es independiente de
    // ticks_para_poder_atacar: una cosa es el cooldown del golpe, otra cuánto dura el gesto visual.
    uint32_t ticks_de_animacion_ataque = 0;

    // Ticks que faltan para que termine la animación de morir (para EstadoEnemigo::muriendo). Se
    // carga al morir (ver GestorDeEnemigos::golpear()) y se cuenta en paralelo a
    // ticks_para_revivir: mientras es > 0 el cliente dibuja el cadáver cayendo; en 0 ya no dibuja
    // nada hasta que vuelva a estar vivo.
    uint32_t ticks_de_animacion_muerte = 0;

    bool esta_vivo() const;

    // Le aplica daño. Si es invulnerable, ya está muerto o el daño no es positivo, no hace nada.
    // Devuelve true si este golpe puntual lo mató (vida pasó de positiva a cero).
    bool recibir_golpe(int danio);
};

#endif
