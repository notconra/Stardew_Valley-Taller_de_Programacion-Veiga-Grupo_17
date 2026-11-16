#ifndef MUNDO_H
#define MUNDO_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../common/mensajes/mensajes.h"
#include "../persistencia/datos_partida_guardada.h"
#include "controladores/controlador_de_acciones.h"
#include "controladores/controlador_de_cofres.h"
#include "controladores/controlador_de_cultivos.h"
#include "controladores/controlador_de_dia.h"
#include "controladores/controlador_de_drops.h"
#include "controladores/controlador_de_enemigos.h"
#include "controladores/controlador_de_jugador.h"
#include "controladores/controlador_de_mazmorra.h"
#include "controladores/controlador_de_mercado.h"
#include "controladores/controlador_de_regalos.h"
#include "controladores/controlador_de_trampas.h"
#include "controladores/controlador_del_centro.h"
#include "jugadores/personaje.h"
#include "mapa/mapa.h"
#include "notificador.h"
#include "npcs/gestor_de_npcs.h"
#include "recursos/recursos_de_juego.h"
#include "sector/gestor_de_sectores.h"

// Router del juego. Es dueño de los datos compartidos (sectores y personajes) y de los
// controladores, recibe los comandos de red y el tick, y los reparte llamando a cada controlador
// explícitamente. Para sumar un feature nuevo se agregaría un controlador, un miembro acá y una
// línea en atender() o actualizar().
class Mundo {
private:
    const RecursosDeJuego& recursos;
    Notificador& notificador;
    std::map<uint32_t, Personaje> personajes;
    std::map<uint32_t, DatosJugadorGuardado> datos_jugadores;
    GestorDeSectores sectores;
    GestorDeNpcs gestor_de_npcs;

    ControladorDeJugador controlador_de_jugador;
    ControladorDeMercado controlador_de_mercado;
    ControladorDelDia controlador_del_dia;
    ControladorDeAcciones controlador_de_acciones;
    ControladorDeCofres controlador_de_cofres;
    ControladorDeDrops controlador_de_drops;
    ControladorDeMazmorra controlador_de_mazmorra;
    ControladorDeEnemigos controlador_de_enemigos;
    ControladorDeTrampas controlador_de_trampas;
    ControladorDeCultivos controlador_de_cultivos;
    ControladorDeRegalos controlador_de_regalos;
    ControladorDelCentro controlador_del_centro;

    void resolver_transicion(Personaje& personaje, const Transicion& transicion);

    // Responde el diálogo del NPC si el jugador está cerca. El texto lo elige el servidor según
    // el día y la hora.
    void hablar_con_npc(Personaje& personaje, const HablarConNpc& pedido);

    // Si la transición se usa con clic derecho en vez de activarse al pisarla: las que tienen un
    // destino de destinos_con_clic (la puerta de la casa, la escalera para bajar de nivel). El
    // resto se activa al pisarla.
    bool requiere_clic(const Transicion& transicion) const;

    // Mueve a quienes pisan una transición que no pide clic.
    void resolver_transiciones_automaticas();
    // Con poca energía los jugadores caminan más lento: se revisa cada tick antes de moverlos.
    void ajustar_velocidad_por_energia();

    // Saca al personaje del mundo sin borrarlo, cada controlador limpia lo suyo y se le quita el
    // cuerpo del sector.
    // Lo que se guarda del personaje y no depende de dónde está su cuerpo: inventario, apariencia,
    // vida, energía y última cama.
    void completar_datos_guardados(const Personaje& personaje, DatosJugadorGuardado& datos) const;

    // Lo inverso, al cargar una partida guardada.
    void restaurar(Personaje& personaje, const DatosJugadorGuardado& guardado) const;

    void dar_de_baja(Personaje& personaje);

public:
    Mundo(
        const RecursosDeJuego& recursos, Notificador& notificador, uint8_t ticks_por_segundo,
        const DatosMundoGuardado& datos_guardados = {});

    // El jugador entra a la partida. Si es la primera vez se crea su Personaje con los ítems
    // iniciales, si ya existía (volvió después de desconectarse o restaurado de partida) conserva
    // todo. "nombre" vacío deja el que tenía.
    void conectar(
        uint32_t id_jugador, const AparienciaPersonaje& apariencia, const std::string& nombre = "");

    // El jugador se fue. Su Personaje queda guardado.
    void desconectar(uint32_t id_jugador);

    void atender(uint32_t id_jugador, const ComandoCliente& comando);

    // Le da ítems a un jugador conectado (para los cheats) y le avisa cómo quedó su inventario.
    // Devuelve cuántos entraron.
    int dar_items(uint32_t id_jugador, TipoItem tipo, uint16_t cantidad);
    void actualizar();

    // Operaciones sobre la billetera de monedas del jugador.
    bool sumar_monedas(uint32_t id_jugador, uint32_t cantidad);
    bool gastar_monedas(uint32_t id_jugador, uint32_t cantidad);
    uint32_t obtener_monedas(uint32_t id_jugador) const;

    // Devuelve los datos guardados o en vivo de un jugador (posición, inventario, etc.).
    std::optional<DatosJugadorGuardado> obtener_datos_jugador(uint32_t id_jugador) const;
    std::vector<std::string> obtener_niveles_mazmorra() const;
    std::vector<DatosMapaGuardado> obtener_mapas_guardados() const;
    std::vector<DatosCofreGuardado> obtener_cofres_guardados() const;
    std::vector<DatosMuebleGuardado> obtener_muebles_guardados() const;
    std::vector<DatosAnimalGuardado> obtener_animales_guardados() const;
    std::vector<uint32_t> obtener_ids_vacas_listas_para_ordeniar() const;
    std::vector<DatosCultivoGuardado> obtener_cultivos_guardados() const;
    DatosCentroGuardado obtener_centro_guardado() const;
    uint32_t obtener_dia() const;

    void cambiar_estacion(Estacion estacion);
    void cambiar_lluvia(bool llueve);
    bool esta_lloviendo() const;

    // Si terminó un día desde la última vez que se preguntó (todos dieron el OK). Es cuando hay
    // que guardar la partida; devuelve true una sola vez por día.
    bool tomar_dia_terminado();

    // Comandos de prueba y cheats de animales / tiempo
    bool agregar_vaca_en_granja_cerca(uint32_t id_jugador);
    bool agregar_gallina_en_granja_cerca(uint32_t id_jugador);
    std::string estado_animales_granja() const;
    void avanzar_dia();
    void avanzar_minutos(uint32_t minutos);
    HoraDelJuego hora_actual() const;

    // Centro comunitario. Donan desde el inventario o la billetera de un jugador conectado.
    uint16_t donar_al_centro(uint32_t id_jugador, TipoLote lote, uint8_t indice_ranura);
    bool donar_monedas_al_centro(uint32_t id_jugador, TipoLote lote);
    const CentroComunitario& centro_comunitario() const;

    Mundo(const Mundo&) = delete;
    Mundo& operator=(const Mundo&) = delete;
};

#endif
