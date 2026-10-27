#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>

#include "../../common/mensajes/mensajes.h"
#include "../red/conexion_con_servidor.h"
#include "camara.h"
#include "mapa_grafico.h"
#include "personaje.h"
#include "renderizador.h"

// Vista de la partida con SDL. Es un cliente "dummy", lee el teclado, avisa al servidor la
// intención y dibuja lo que el servidor dice en el último EstadoJuego. No calcula posiciones ni
// mueve nada por su cuenta.
class VistaPartida {
private:
    ConexionConServidor& conexion;

    uint32_t id_jugador = 0;
    Moverse intencion_enviada{};
    EstadoJuego ultimo_estado_juego{};
    Camara camara;

    void enviar_intencion_si_cambio(const Moverse& intencion);

    // Vacía los eventos que llegaron y se queda con el último EstadoJuego. Devuelve false si la
    // partida terminó (hay que volver al lobby).
    bool recibir_eventos();

    void dibujar(Renderizador& renderizador, const MapaGrafico& mapa, const Personaje& personaje);

public:
    explicit VistaPartida(ConexionConServidor& conexion_conectada);

    // Abre la ventana, bloquea hasta que termina la partida y la cierra. Devuelve true si hay que
    // volver al lobby (el jugador salió con ESC o la partida terminó) y false si el usuario
    // cerró la ventana (hay que cerrar el cliente). SDL se inicializa y se libera acá adentro.
    // Lanza ErrorConexionCerrada si el servidor se cae y std::runtime_error si falla SDL o un
    // recurso gráfico.
    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif
