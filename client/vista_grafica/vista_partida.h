#ifndef VISTA_PARTIDA_H
#define VISTA_PARTIDA_H

#include <cstdint>
#include <map>
#include <string>

#include "../../common/mensajes/mensajes.h"
#include "../configuracion/configuracion_cliente_datos.h"
#include "../red/conexion_con_servidor.h"
#include "camara.h"
#include "mapa_grafico.h"
#include "personaje.h"
#include "renderizador.h"

class VistaPartida {
private:
    ConexionConServidor& conexion;
    const ConfiguracionVista& configuracion;

    uint32_t id_jugador = 0;
    Moverse intencion_enviada{};
    EstadoJuego ultimo_estado_juego{};
    std::string mapa_actual;

    void enviar_intencion_si_cambio(const Moverse& intencion);
    bool recibir_eventos();

    // Busca la configuración del mapa con ese id. Lanza std::runtime_error si no existe.
    const ConfiguracionMapa& configuracion_de(const std::string& id_mapa) const;

    int ancho_del_mapa_en_pantalla(const MapaGrafico& mapa) const;
    int alto_del_mapa_en_pantalla(const MapaGrafico& mapa) const;

    void dibujar(
        Renderizador& renderizador, const MapaGrafico& mapa, const Personaje& personaje,
        Camara& camara);

public:
    VistaPartida(ConexionConServidor& conexion_conectada, const ConfiguracionVista& configuracion);

    bool ejecutar(const EntradaExitosaAPartida& partida);

    VistaPartida(const VistaPartida&) = delete;
    VistaPartida& operator=(const VistaPartida&) = delete;
};

#endif