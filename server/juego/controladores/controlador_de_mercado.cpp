#include "controlador_de_mercado.h"

#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../../../common/horario_del_mercado.h"
#include "../mercado/puesto_clint.h"
#include "../mercado/puesto_marnie.h"
#include "../mercado/puesto_pierre.h"
#include "../mercado/puesto_robin.h"

constexpr uint16_t TODO_EL_STACK = 0;                // al vender, 0 significa "todo el stack"
constexpr const char* MAPA_DE_LA_GRANJA = "granja";  // el mapa donde viven los animales

ControladorDeMercado::ControladorDeMercado(
    const ConfiguracionMercadoServidor& config, GestorDeSectores& sectores,
    Notificador& notificador)
    : notificador(notificador), sectores(sectores), mapa_del_mercado(config.mapa) {
    agregar_puesto(std::make_unique<PuestoPierre>(config));
    agregar_puesto(std::make_unique<PuestoClint>(config));
    agregar_puesto(std::make_unique<PuestoMarnie>(config));
    agregar_puesto(std::make_unique<PuestoRobin>(config));
}

void ControladorDeMercado::agregar_puesto(std::unique_ptr<PuestoDeMercado> puesto) {
    const std::string id = puesto->id();
    puestos[id] = std::move(puesto);
}

PuestoDeMercado* ControladorDeMercado::puesto_actual_de(const Personaje& personaje) {
    const auto abierto = puesto_abierto_por_jugador.find(personaje.id_jugador);
    if (abierto == puesto_abierto_por_jugador.end()) {
        return nullptr;
    }
    return puestos.at(abierto->second).get();
}

std::map<TipoAnimal, uint16_t> ControladorDeMercado::animales_en_la_granja() const {
    if (!sectores.existe(MAPA_DE_LA_GRANJA)) {
        return {};
    }
    const GestorDeAnimales& animales = sectores.en(MAPA_DE_LA_GRANJA).animales;
    return {
        {TipoAnimal::VACA, static_cast<uint16_t>(animales.cantidad_vacas())},
        {TipoAnimal::GALLINA, static_cast<uint16_t>(animales.cantidad_gallinas())}};
}

void ControladorDeMercado::refrescar_ventana(
    const Personaje& personaje, PuestoDeMercado& puesto, const HoraDelJuego& hora) {
    notificador.a_jugador(
        personaje.id_jugador,
        puesto.armar_actualizacion(personaje, hora.dia, animales_en_la_granja()));
}

void ControladorDeMercado::notificar_transaccion(
    Personaje& personaje, PuestoDeMercado& puesto, const HoraDelJuego& hora) {
    notificador.a_jugador(personaje.id_jugador, personaje.actualizacion_de_inventario());
    refrescar_ventana(personaje, puesto, hora);
}

bool ControladorDeMercado::puede_entrar(
    const std::string& mapa_destino, const HoraDelJuego& hora) const {
    return mapa_destino != mapa_del_mercado || mercado_esta_abierto(hora.hora);
}

void ControladorDeMercado::avisar_mercado_cerrado(const Personaje& personaje) const {
    notificador.a_jugador(personaje.id_jugador, AvisoEnPantalla{MENSAJE_DE_MERCADO_CERRADO});
}

void ControladorDeMercado::abrir_mercado(
    Personaje& personaje, const std::string& id_puesto, const HoraDelJuego& hora) {
    if (personaje.mapa_actual != mapa_del_mercado) {
        return;
    }
    if (!mercado_esta_abierto(hora.hora)) {
        avisar_mercado_cerrado(personaje);
        return;
    }

    const auto puesto = puestos.find(id_puesto);
    if (puesto == puestos.end()) {
        return;
    }
    puesto_abierto_por_jugador[personaje.id_jugador] = id_puesto;
    refrescar_ventana(personaje, *puesto->second, hora);
}

void ControladorDeMercado::cerrar_mercado(Personaje& personaje) {
    puesto_abierto_por_jugador.erase(personaje.id_jugador);
    notificador.a_jugador(personaje.id_jugador, CerrarVentanaMercado{});
}

void ControladorDeMercado::procesar_compra_item(
    Personaje& personaje, PuestoDeMercado& puesto, const ComprarEnMercado& cmd,
    const HoraDelJuego& hora) {
    const std::vector<ProductoEnVenta> catalogo = puesto.obtener_productos_en_venta(hora.dia);
    if (cmd.cantidad == 0 || cmd.indice_item >= catalogo.size()) {
        return;
    }

    const ProductoEnVenta& producto = catalogo[cmd.indice_item];
    if (producto.es_animal()) {
        return;
    }
    if (!personaje.inventario.puede_agregar(producto.tipo_item, cmd.cantidad, Calidad::BASE)) {
        return;
    }
    if (!personaje.gastar_monedas(producto.precio_que_paga_el_jugador * cmd.cantidad)) {
        return;
    }

    personaje.inventario.agregar_item(producto.tipo_item, cmd.cantidad, Calidad::BASE);
    notificar_transaccion(personaje, puesto, hora);
}

void ControladorDeMercado::procesar_venta_item(
    Personaje& personaje, PuestoDeMercado& puesto, const VenderEnMercado& cmd,
    const HoraDelJuego& hora) {
    if (!personaje.inventario.indice_valido(cmd.ranura_inventario)) {
        return;
    }

    const RanuraInventario ranura = personaje.inventario.ranura(cmd.ranura_inventario);
    if (ranura.tipo == TipoItem::NINGUNO || ranura.cantidad == 0) {
        return;
    }

    const uint32_t precio_unitario =
        puesto.precio_que_recibe_el_jugador(ranura.tipo, ranura.calidad);
    if (precio_unitario == 0) {
        return;  // El puesto no compra ese ítem.
    }

    const bool pide_todo = (cmd.cantidad == TODO_EL_STACK || cmd.cantidad > ranura.cantidad);
    const uint16_t a_vender = pide_todo ? ranura.cantidad : cmd.cantidad;
    if (!personaje.quitar_de_ranura(cmd.ranura_inventario, a_vender)) {
        return;
    }

    personaje.recibir_monedas(precio_unitario * a_vender);
    notificar_transaccion(personaje, puesto, hora);
}

void ControladorDeMercado::procesar_compra_animal(
    Personaje& personaje, PuestoDeMercado& puesto, const ComprarAnimalMercado& cmd,
    const HoraDelJuego& hora) {
    const ConfiguracionAnimalServidor* animal = puesto.animal(cmd.tipo_animal);
    if (animal == nullptr || cmd.cantidad == 0 || !sectores.existe(MAPA_DE_LA_GRANJA)) {
        return;
    }

    Sector& granja = sectores.en(MAPA_DE_LA_GRANJA);
    const int tope = static_cast<int>(animal->tope_por_granja);
    bool compro_alguno = false;
    for (uint16_t i = 0; i < cmd.cantidad; ++i) {
        if (granja.animales.cantidad_de(cmd.tipo_animal) >= tope ||
            !personaje.gastar_monedas(animal->precio_de_compra)) {
            break;
        }
        if (!granja.agregar_animal(notificador, cmd.tipo_animal)) {
            personaje.recibir_monedas(animal->precio_de_compra);
            break;
        }
        compro_alguno = true;
    }
    if (compro_alguno) {
        notificar_transaccion(personaje, puesto, hora);
    }
}

void ControladorDeMercado::procesar_venta_animal(
    Personaje& personaje, PuestoDeMercado& puesto, const VenderAnimalMercado& cmd,
    const HoraDelJuego& hora) {
    const ConfiguracionAnimalServidor* animal = puesto.animal(cmd.tipo_animal);
    if (animal == nullptr || cmd.cantidad == 0 || !sectores.existe(MAPA_DE_LA_GRANJA)) {
        return;
    }

    Sector& granja = sectores.en(MAPA_DE_LA_GRANJA);
    bool vendio_alguno = false;
    for (uint16_t i = 0; i < cmd.cantidad; ++i) {
        if (!granja.retirar_animal(notificador, cmd.tipo_animal)) {
            break;
        }
        personaje.recibir_monedas(animal->precio_de_venta);
        vendio_alguno = true;
    }
    if (vendio_alguno) {
        notificar_transaccion(personaje, puesto, hora);
    }
}

PuestoDeMercado* ControladorDeMercado::puesto_para_operar(
    const Personaje& personaje, const HoraDelJuego& hora) {
    PuestoDeMercado* puesto = puesto_actual_de(personaje);
    if (puesto == nullptr) {
        return nullptr;
    }
    if (!mercado_esta_abierto(hora.hora)) {
        avisar_mercado_cerrado(personaje);
        return nullptr;
    }
    return puesto;
}

bool ControladorDeMercado::atender(
    Personaje& personaje, const ComandoCliente& comando, const HoraDelJuego& hora) {
    if (std::holds_alternative<CerrarMercado>(comando)) {
        if (puesto_actual_de(personaje) == nullptr) {
            return false;
        }
        cerrar_mercado(personaje);
        return true;
    }

    if (const ComprarEnMercado* cmd = std::get_if<ComprarEnMercado>(&comando)) {
        if (PuestoDeMercado* puesto = puesto_para_operar(personaje, hora)) {
            procesar_compra_item(personaje, *puesto, *cmd, hora);
        }
        return true;
    }
    if (const VenderEnMercado* cmd = std::get_if<VenderEnMercado>(&comando)) {
        if (PuestoDeMercado* puesto = puesto_para_operar(personaje, hora)) {
            procesar_venta_item(personaje, *puesto, *cmd, hora);
        }
        return true;
    }
    if (const ComprarAnimalMercado* cmd = std::get_if<ComprarAnimalMercado>(&comando)) {
        if (PuestoDeMercado* puesto = puesto_para_operar(personaje, hora)) {
            procesar_compra_animal(personaje, *puesto, *cmd, hora);
        }
        return true;
    }
    if (const VenderAnimalMercado* cmd = std::get_if<VenderAnimalMercado>(&comando)) {
        if (PuestoDeMercado* puesto = puesto_para_operar(personaje, hora)) {
            procesar_venta_animal(personaje, *puesto, *cmd, hora);
        }
        return true;
    }
    return false;
}

void ControladorDeMercado::al_cambiar_de_mapa(Personaje& personaje) {
    if (puesto_actual_de(personaje) != nullptr) {
        cerrar_mercado(personaje);
    }
}

void ControladorDeMercado::al_desconectarse(Personaje& personaje) {
    puesto_abierto_por_jugador.erase(personaje.id_jugador);
}
