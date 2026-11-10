#ifndef GESTOR_DE_TRAMPAS_H
#define GESTOR_DE_TRAMPAS_H

#include <cstdint>
#include <utility>
#include <vector>

#include "../../../common/mensajes/mensajes_partida.h"
#include "../../../common/tipo_trampa.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../geometria.h"

// Unos pinches en el suelo de un nivel. Su ciclo dura ticks_inactiva + ticks_activa; "desfase"
// corre el comienzo para que los de un mismo nivel no salgan todos juntos.
struct Trampa {
    int id = 0;
    TipoTrampa tipo = TipoTrampa::PINCHES;
    Casilla casilla{0, 0};
    int desfase = 0;
    bool activa = false;
    // Los jugadores que ya lastimó en la activación en curso: a cada uno le pega una sola vez.
    std::vector<uint32_t> lastimados;
};

// Un golpe de trampa que hay que aplicarle a un jugador.
struct GolpeDeTrampa {
    uint32_t id_jugador;
    int danio;
};

// Lo que pasó en un tick: qué trampas cambiaron de estado y a quién hay que lastimar.
struct ResultadoDeTick {
    std::vector<int> cambiadas;  // ids de las que salieron o se guardaron
    std::vector<GolpeDeTrampa> golpes;
};

// Las trampas de un sector. No conoce a los Personajes: avanza el ciclo de cada una y dice a quién
// lastimar; aplicar el daño y avisar a los clientes lo hace ControladorDeTrampas.
class GestorDeTrampas {
private:
    std::vector<Trampa> trampas;
    int siguiente_id = 1;
    uint64_t tick = 0;
    ConfiguracionTrampasServidor config;

    bool deberia_estar_activa(const Trampa& trampa) const;

public:
    void limpiar();
    void configurar(const ConfiguracionTrampasServidor& configuracion);

    // "desfase" en ticks: en qué momento de su ciclo arranca.
    void agregar(const Casilla& casilla, int desfase);

    // Cuántos ticks dura un ciclo completo (guardada + afuera).
    int duracion_del_ciclo() const;

    // Avanza un tick con los jugadores del sector (id y casilla que pisan).
    ResultadoDeTick actualizar(const std::vector<std::pair<uint32_t, Casilla>>& jugadores);

    bool vacio() const;
    const Trampa* buscar(int id) const;
    const std::vector<Trampa>& todas() const;

    static EstadoTrampa estado_de(const Trampa& trampa);
    void agregar_a(EstadoJuego& estado) const;
};

#endif
