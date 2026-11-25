#ifndef ENEMIGO_H
#define ENEMIGO_H

#include <cstdint>

#include "../../../common/direccion.h"
#include "../../../common/tipo_enemigo.h"
#include "../geometria.h"

// Un enemigo colocado en el mapa de un nivel de la mazmorra. Ocupa una sola casilla como posición
// lógica; acá solo vive el estado de combate (vida, si está muerto esperando revivir).
//
// No sabe perseguir ni golpear al jugador: eso lo resuelve GestorDeEnemigos (mover_hacia() y
// atacar_adyacentes()), que ControladorDeEnemigos llama en cada tick. Acá solo están los datos y
// las reglas de "recibir un golpe" que son comunes a todos los tipos.
struct Enemigo {
    int id = 0;
    TipoEnemigo tipo = TipoEnemigo::NINGUNO;
    Casilla posicion{};
    Casilla origen{};  // donde apareció: de ahí no se aleja más que su radio de deambulo
    // Grupo con el que apareció (orcos). 0 si apareció solo. Es único dentro del sector.
    int grupo = 0;
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

    // Hacia dónde mira: hacia donde camina, o hacia el jugador al que le pega.
    Direccion mirando = Direccion::ABAJO;

    // Vampiros: muerto y esperando que caiga sangre cerca para empezar a contar ticks_para_revivir.
    bool espera_sangre = false;
    // Vampiros: ya cayó sangre sobre el cadáver y está contando para revivir (el cliente la dibuja).
    bool con_sangre = false;
    // Vampiros: su pareja está muerta, así que su habilidad está multiplicada.
    bool en_frenesi = false;

    // Lo que tarda (en ms) el último paso que dio, para que el cliente lo deslice a esa velocidad.
    uint16_t ms_por_casilla = 0;

    // Jugador al que persigue sin importar su radio de visión (ej: la manada que se avecina al
    // picar plata). 0 = ninguno: usa la regla normal de visión. Se borra si ese jugador deja de
    // estar en el sector o cae inconsciente.
    uint32_t jugador_perseguido = 0;

    // Ticks que faltan para que pueda dar el próximo paso (ver GestorDeEnemigos::mover_hacia()).
    uint32_t ticks_para_moverse = 0;

    bool esta_vivo() const;

    // Le aplica daño. Si es invulnerable, ya está muerto o el daño no es positivo, no hace nada.
    // Devuelve true si este golpe puntual lo mató (vida pasó de positiva a cero).
    bool recibir_golpe(int danio);
};

#endif