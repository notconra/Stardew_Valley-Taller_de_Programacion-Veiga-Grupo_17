#ifndef CONTROLADOR_DE_MERCADO_H
#define CONTROLADOR_DE_MERCADO_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../jugadores/personaje.h"
#include "../mercado/puesto_de_mercado.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Atiende los comandos del mercado, qué puesto tiene abierto cada jugador y cada compra o venta.
class ControladorDeMercado {
private:
    Notificador& notificador;
    GestorDeSectores& sectores;
    std::string mapa_del_mercado;

    std::map<std::string, std::unique_ptr<PuestoDeMercado>> puestos;
    std::map<uint32_t, std::string> puesto_abierto_por_jugador;

    void agregar_puesto(std::unique_ptr<PuestoDeMercado> puesto);
    PuestoDeMercado* puesto_actual_de(const Personaje& personaje);

    // Cuántos animales de cada tipo hay hoy en la granja (lo que muestra la ventana).
    std::map<TipoAnimal, uint16_t> animales_en_la_granja() const;

    void refrescar_ventana(
        const Personaje& personaje, PuestoDeMercado& puesto, const HoraDelJuego& hora);

    void notificar_transaccion(
        Personaje& personaje, PuestoDeMercado& puesto, const HoraDelJuego& hora);

    void procesar_compra_item(
        Personaje& personaje, PuestoDeMercado& puesto, const ComprarEnMercado& cmd,
        const HoraDelJuego& hora);
    void procesar_venta_item(
        Personaje& personaje, PuestoDeMercado& puesto, const VenderEnMercado& cmd,
        const HoraDelJuego& hora);
    void procesar_compra_animal(
        Personaje& personaje, PuestoDeMercado& puesto, const ComprarAnimalMercado& cmd,
        const HoraDelJuego& hora);
    void procesar_venta_animal(
        Personaje& personaje, PuestoDeMercado& puesto, const VenderAnimalMercado& cmd,
        const HoraDelJuego& hora);

    // Devuelve el puesto con el que el jugador puede operar ahora, o nullptr (avisando si el
    // mercado está cerrado).
    PuestoDeMercado* puesto_para_operar(const Personaje& personaje, const HoraDelJuego& hora);

public:
    ControladorDeMercado(
        const ConfiguracionMercadoServidor& config, GestorDeSectores& sectores,
        Notificador& notificador);

    // Si el mapa es el del mercado y está fuera de horario, el jugador no puede entrar.
    bool puede_entrar(const std::string& mapa_destino, const HoraDelJuego& hora) const;
    void avisar_mercado_cerrado(const Personaje& personaje) const;

    void abrir_mercado(
        Personaje& personaje, const std::string& id_puesto, const HoraDelJuego& hora);
    void cerrar_mercado(Personaje& personaje);

    // Devuelve false si el comando no es del mercado (o no hay nada que cerrar).
    bool atender(Personaje& personaje, const ComandoCliente& comando, const HoraDelJuego& hora);

    // El jugador cambió de mapa: si tenía un catálogo abierto, se le cierra.
    void al_cambiar_de_mapa(Personaje& personaje);
    void al_desconectarse(Personaje& personaje);
};

#endif
