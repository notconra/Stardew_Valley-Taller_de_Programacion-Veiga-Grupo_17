#include "enemigo.h"

bool Enemigo::esta_vivo() const {
    return vida > 0;
}

bool Enemigo::recibir_golpe(int danio) {
    if (invulnerable || !esta_vivo() || danio <= 0) {
        return false;
    }
    vida = (danio >= vida) ? 0 : vida - danio;
    return !esta_vivo();
}
