#ifndef CONTROLADOR_DE_ENEMIGOS_H
#define CONTROLADOR_DE_ENEMIGOS_H

#include <cstdint>
#include <map>

#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// El otro lado del combate: por ahora los enemigos no se mueven, pero si un jugador se para al
// lado de uno vivo, lo golpea. GestorDeEnemigos sabe "quién está listo para pegar y a quién le
// pega" (ver atacar_adyacentes()); este controlador arma las posiciones de los jugadores de cada
// sector, aplica el daño resultante sobre el Personaje correspondiente y avisa con
// EnemigoActualizado para que el cliente dispare la animación de ataque de ese enemigo.
class ControladorDeEnemigos {
private:
    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    Notificador& notificador;

    void atacar_en_sector(Sector& sector);

public:
    ControladorDeEnemigos(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        Notificador& notificador);

    // Un tick: en cada sector, los enemigos listos para pegar que tengan un jugador al lado le
    // bajan la vida.
    void actualizar();
};

#endif
