#ifndef SEGUIMIENTO_DE_PELEA_H
#define SEGUIMIENTO_DE_PELEA_H

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "../../../common/logro_de_pelea.h"
#include "../enemigos/gestor_de_enemigos.h"

// Lleva la cuenta de lo que hace cada jugador en la mazmorra para saber cuándo consigue un logro
// del set de pelea del centro comunitario. Cada logro lo tiene que conseguir un mismo jugador:
//  - Esqueletos: matar al menos un esqueleto en tres pisos seguidos de una misma bajada. Si pasa
//    por un piso sin matar ninguno, la racha arranca de nuevo.
//  - Vampiros: dejar muertos a la vez a los dos de una pareja, los dos con su golpe final.
//  - Orcos: matar a cuatro orcos de un mismo grupo.
//  - Calavera: bajar por la escalera de un piso con una calavera sin haber recibido daño de
//    enemigos en ese piso (las trampas no cuentan).
// Una muerte es de quien dio el golpe final. Al salir de la mazmorra, desmayarse o desconectarse
// se pierde todo el avance. Nada de esto se guarda con la partida.
//
// No conoce a los personajes ni al centro: quien lo usa (ControladorDelCentro) le avisa lo que
// pasa y cumple los logros que devuelve.
class SeguimientoDePelea {
private:
    struct Avance {
        int piso = 0;
        int racha_de_esqueletos = 0;
        int ultimo_piso_con_esqueleto = 0;
        bool recibio_danio_en_el_piso = false;
    };

    struct Muerte {
        uint32_t id_jugador = 0;
        int piso = 0;
        int id_enemigo = 0;
        int grupo = 0;
    };

    std::map<uint32_t, Avance> avances;
    std::vector<Muerte> muertes;

    bool mato_un_esqueleto(Avance& avance, int piso) const;
    bool mato_un_orco(uint32_t id_jugador, int piso, const Enemigo& muerto);
    bool mato_un_vampiro(
        uint32_t id_jugador, int piso, const Enemigo& muerto, const GestorDeEnemigos& enemigos);

    void anotar_muerte(uint32_t id_jugador, int piso, const Enemigo& muerto);
    bool lo_mato(uint32_t id_jugador, int piso, int id_enemigo) const;

public:
    // Dónde está el jugador: el piso de la mazmorra, o 0 si está afuera, desmayado o desconectado
    // (ahí pierde su avance). Si acaba de bajar del piso anterior sin que un enemigo lo lastimara,
    // devuelve ese piso, para que quien llama vea si tenía una calavera.
    std::optional<int> ubicar(uint32_t id_jugador, int piso);

    // Un enemigo lastimó al jugador.
    void recibio_danio_de_un_enemigo(uint32_t id_jugador);

    // El jugador dio el golpe final a "muerto" en ese piso. "enemigos" son los del piso. Devuelve
    // el logro que consiguió con esa muerte, si consiguió alguno.
    std::optional<LogroDePelea> mato_a(
        uint32_t id_jugador, int piso, const Enemigo& muerto, const GestorDeEnemigos& enemigos);

    // Pierde todo el avance del jugador.
    void reiniciar(uint32_t id_jugador);
};

#endif
