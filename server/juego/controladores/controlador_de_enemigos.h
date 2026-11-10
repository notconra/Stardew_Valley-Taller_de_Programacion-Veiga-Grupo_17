#ifndef CONTROLADOR_DE_ENEMIGOS_H
#define CONTROLADOR_DE_ENEMIGOS_H

#include <cstdint>
#include <map>

#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// El otro lado del combate: los enemigos persiguen al jugador cercano (ver mover_hacia()) y, si
// quedan al lado de uno, lo golpean. GestorDeEnemigos sabe "hacia dónde avanza cada uno" y "quién
// está listo para pegar y a quién le pega" (ver atacar_adyacentes()); este controlador arma las
// posiciones de los jugadores de cada sector, aplica el daño resultante sobre el Personaje
// correspondiente y avisa con EnemigoActualizado para que el cliente vea cada paso y cada golpe.
class ControladorDeEnemigos {
private:
    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    Notificador& notificador;

    void atacar_en_sector(Sector& sector);
    // Revisa si algún jugador del sector está gravemente herido o caído cerca de un vampiro muerto.
    void derramar_sangre_en_sector(Sector& sector);

public:
    ControladorDeEnemigos(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        Notificador& notificador);

    // Un tick: en cada sector, los enemigos avanzan hacia el jugador más cercano y los que quedan
    // listos para pegar con un jugador al lado le bajan la vida.
    void actualizar();
};

#endif
