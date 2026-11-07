#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <cstdint>
#include <optional>
#include <string>

#include "../../../common/estado_de_descanso.h"
#include "../../../common/mensajes/mensajes.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_accion.h"
#include "../inventario/inventario.h"
#include "../notificador.h"
#include "../objetos/catalogo_de_items.h"

// Dónde está la cama en la que durmió por última vez: el mapa y la casilla de arriba a la
// izquierda.
struct UbicacionDeCama {
    std::string mapa;
    int columna = 0;
    int fila = 0;
};

// Datos lógicos de un jugador en la partida como su inventario, qué ranura tiene elegida, qué cofre
// está mirando y qué acción está ejecutando. El cuerpo (posición, dirección, colisiones) vive en
// Jugadores, dentro del Sector donde esté. Acá podría ir también lo que maneje las stats y la plata
// (?)
//
// Un Personaje se crea la primera vez que el jugador entra a la partida y no se borra nunca, si el
// jugador se desconecta queda con "conectado" en false (sin cuerpo en ningún sector) y conserva su
// inventario. Al volver se lo coloca de nuevo en el mundo, sin repetir los ítems iniciales.
//
// Acá también están las pocas reglas del inventario que necesitan saber de la barra y del tacho.
// Todo lo demás del inventario lo resuelve Inventario.
struct Personaje {
    uint32_t id_jugador = 0;

    // El mapa donde está su cuerpo. Solo lo escribe GestorDeSectores, el valor inicial del
    // constructor no cuenta porque colocar_en_aparicion() lo pisa.
    std::string mapa_actual;
    bool conectado = false;
    int id_cofre_abierto = 0;  // 0 si no tiene ningún cofre abierto
    std::string mapa_cofre_abierto;
    Inventario inventario;
    uint8_t ranura_activa = 0;
    TipoAccion accion_actual = TipoAccion::NINGUNA;
    uint16_t ticks_restantes_accion = 0;
    AparienciaPersonaje apariencia;

    // Vida del jugador en combate (ver "Un día en el juego" del enunciado). Arranca a tope y se
    // repone entera al dormir; por ahora el único lado implementado es "la mazmorra le pega al
    // jugador": el reposo al dormir y el desmayo automático a las 2am los va a manejar el ciclo de
    // día, que todavía no existe. VIDA_MAXIMA toma el valor de VIDA_MAXIMA_JUGADOR (común con el
    // cliente) para que el servidor y la barra de vida del HUD nunca se desincronicen.
    static constexpr int VIDA_MAXIMA = VIDA_MAXIMA_JUGADOR;
    int vida = VIDA_MAXIMA;

    // Energía del jugador. El enunciado la gasta con cualquier acción y la liga a la velocidad de
    // movimiento al 10%; por ahora lo único implementado es que atacar con la espada la gasta
    // (ver AccionAtacar); el resto de las acciones y el efecto en la velocidad quedan para cuando
    // se sume el ciclo de día completo. ENERGIA_MAXIMA toma ENERGIA_MAXIMA_JUGADOR por la misma
    // razón que VIDA_MAXIMA.
    static constexpr int ENERGIA_MAXIMA = ENERGIA_MAXIMA_JUGADOR;
    int energia = ENERGIA_MAXIMA;

    // El fin del día (ver ControladorDelDia). Acostado en una cama o desmayado, el jugador espera
    // el día siguiente y tiene que dar el OK. "ultima_cama" es donde se despierta si se desmaya.
    EstadoDeDescanso descanso = EstadoDeDescanso::DESPIERTO;
    bool dio_el_ok = false;
    std::optional<UbicacionDeCama> ultima_cama;

    Personaje(
        uint32_t id, const std::string& mapa_inicial, const CatalogoDeItems& catalogo,
        uint8_t cantidad_ranuras_barra);

    // Traduce el índice que mandó el cliente (INDICE_RANURA_ACTIVA o una ranura real).
    uint8_t resolver_ranura(uint8_t indice_ranura) const;

    // Una copia de la ranura, ya resuelta la ranura activa. Si el índice no existe, viene vacía.
    RanuraInventario ranura(uint8_t indice_ranura) const;

    bool seleccionar_ranura(uint8_t indice_ranura);

    // Mueve entre ranuras de la barra. Si el destino es el tacho, elimina el ítem.
    bool mover_item(uint8_t origen, uint8_t destino, uint16_t cantidad);

    bool quitar_de_ranura(uint8_t indice_ranura, uint16_t cantidad);

    bool esta_ocupado() const;

    // Al 20% de vida o menos el jugador golpea a la mitad de efectivo (regla del enunciado), así
    // que esto es lo que hay que multiplicar al daño que reparte con la espada.
    double multiplicador_de_ataque() const;

    // Si está a tope de vida (no puede subir más) o a cero (no baja de ahí).
    bool esta_inconsciente() const;

    // Le aplica daño recibido en combate. No hace nada si "danio" no es positivo o si ya está
    // inconsciente.
    void recibir_golpe(int danio);

    // Descuenta energía gastada en una acción (por ahora, atacar). No baja de 0.
    void gastar_energia(int cantidad);

    // El mensaje con la barra completa tal como lo espera el cliente. Se manda con
    // notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario()).
    ActualizacionInventario actualizacion_de_inventario() const;

    Personaje(const Personaje&) = delete;
    Personaje& operator=(const Personaje&) = delete;
    Personaje(Personaje&&) = default;
    Personaje& operator=(Personaje&&) = default;
};

#endif
