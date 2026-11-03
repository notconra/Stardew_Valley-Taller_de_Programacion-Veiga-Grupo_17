#include "controlador_de_mazmorra.h"

#include <random>
#include <string>
#include <utility>
#include <vector>

#include "./../mazmorra/cargador_mazmorra.h"
#include "./../mazmorra/minerales.h"

ControladorDeMazmorra::ControladorDeMazmorra(
    const RecursosDeJuego& recursos, GestorDeSectores& sectores)
    : recursos(recursos), sectores(sectores) {
    if (recursos.mazmorra.mapas.empty()) {
        return;
    }

    std::random_device dispositivo;
    std::mt19937 generador(dispositivo());
    niveles_de_mazmorra = CargadorMazmorra::elegir_niveles(recursos.mazmorra.mapas, generador);

    std::vector<GeografiaNivel> geografias;
    geografias.reserve(niveles_de_mazmorra.size());
    for (const std::string& id_mapa : niveles_de_mazmorra) {
        geografias.push_back(recursos.geografias_de_mazmorra.at(id_mapa));
    }
    mazmorra.emplace(
        std::move(geografias),
        recursos.mazmorra.enemigos_por_nivel,
        recursos.mazmorra.minerales_por_nivel,
        std::move(generador));
}

void ControladorDeMazmorra::limpiar_drops_de_niveles() {
    for (const std::string& id_mapa : niveles_de_mazmorra) {
        sectores.en(id_mapa).drops.limpiar();
    }
}

void ControladorDeMazmorra::poblar_niveles() {
    // Un nivel se vuelve a poblar recién cuando la mazmorra quedó vacía, así que los drops viejos
    // ya no le sirven a nadie y se descartan con el resto del contenido procedural.
    limpiar_drops_de_niveles();

    for (size_t i = 0; i < niveles_de_mazmorra.size(); ++i) {
        const int numero_de_nivel = static_cast<int>(i) + 1;
        GestorDeObjetos& objetos = sectores.en(niveles_de_mazmorra.at(i)).objetos;

        objetos.retirar_objetos_naturales();
        objetos.poblar_en(
            mazmorra->nivel(numero_de_nivel).obtener_celdas_de_objetos(),
            Minerales::pesos(numero_de_nivel, recursos.mazmorra.minerales));
    }
}

void ControladorDeMazmorra::llevar_al_nivel_actual(Personaje& personaje) {
    const uint32_t id_jugador = personaje.id_jugador;
    const std::string& id_nivel = niveles_de_mazmorra.at(mazmorra->numero_de_nivel(id_jugador) - 1);
    const Casilla entrada =
        mazmorra->nivel_actual_del_jugador(id_jugador).obtener_geografia().entrada;
    const Sector& destino = sectores.en(id_nivel);

    // Se aparece parado en la casilla de entrada, con los pies centrados en ella.
    const ConfiguracionMovimientoJugador& movimiento = recursos.juego.movimiento;
    const int ancho = destino.mapa.ancho_de_casilla_en_pixeles();
    const int alto = destino.mapa.alto_de_casilla_en_pixeles();
    const double x =
        entrada.columna * ancho + (ancho - movimiento.hitbox_ancho) / 2.0 - movimiento.hitbox_x;
    const double y = (entrada.fila + 1) * alto - movimiento.hitbox_y - movimiento.hitbox_alto;

    sectores.trasladar(personaje, id_nivel, x, y);
}

void ControladorDeMazmorra::entrar(Personaje& personaje) {
    if (mazmorra->esta_adentro(personaje.id_jugador)) {
        return;
    }
    const bool estaba_vacia = mazmorra->esta_vacia();
    mazmorra->entrar(personaje.id_jugador);
    if (estaba_vacia) {
        poblar_niveles();
    }
    llevar_al_nivel_actual(personaje);
}

void ControladorDeMazmorra::bajar(Personaje& personaje) {
    if (mazmorra->esta_adentro(personaje.id_jugador) && mazmorra->bajar(personaje.id_jugador)) {
        llevar_al_nivel_actual(personaje);
    }
}

void ControladorDeMazmorra::salir_de_la_mazmorra(Personaje& personaje) {
    if (!mazmorra || !mazmorra->esta_adentro(personaje.id_jugador)) {
        return;
    }
    mazmorra->salir(personaje.id_jugador);
    if (mazmorra->esta_vacia()) {
        limpiar_drops_de_niveles();
    }
}

bool ControladorDeMazmorra::resolver_transicion(
    Personaje& personaje, const Transicion& transicion) {
    const std::string& destino = transicion.mapa_destino;
    const bool es_entrada_interna = destino == DESTINO_ENTRADA;
    const bool es_escalera = destino == DESTINO_ESCALERA;
    const bool es_puerta_de_la_mazmorra = destino == recursos.mazmorra.destino;
    if (!mazmorra) {
        return es_entrada_interna || es_escalera || es_puerta_de_la_mazmorra;
    }
    if (es_entrada_interna) {
        return true;
    }
    if (es_escalera) {
        bajar(personaje);
        return true;
    }
    if (es_puerta_de_la_mazmorra) {
        entrar(personaje);
        return true;
    }
    salir_de_la_mazmorra(personaje);
    return false;
}

void ControladorDeMazmorra::al_desconectarse(Personaje& personaje) {
    salir_de_la_mazmorra(personaje);
}
