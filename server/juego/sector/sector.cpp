#include "sector.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <vector>

#include "../azar.h"
#include "../muebles/muebles.h"

const ObjetoDelMundo* Sector::objeto_al_frente(uint32_t id_jugador) const {
    const std::optional<Casilla> casilla = jugadores.casilla_de_enfrente(id_jugador);
    if (!casilla.has_value()) {
        return nullptr;
    }
    const ObjetoDelMundo* objeto = objetos.objeto_en(casilla->columna, casilla->fila);
    if (objeto == nullptr || objeto->tipo == TipoObjeto::NINGUNO) {
        return nullptr;
    }
    return objeto;
}

const ObjetoDelMundo* Sector::cofre_al_frente(uint32_t id_jugador) const {
    const ObjetoDelMundo* objeto = objeto_al_frente(id_jugador);
    if (objeto == nullptr || objeto->tipo != TipoObjeto::COFRE) {
        return nullptr;
    }
    return objeto;
}

void Sector::avisar(Notificador& notificador, const EventoServidor& evento) const {
    const std::vector<uint32_t> ids = jugadores.ids();
    for (uint32_t id_jugador : ids) {
        notificador.a_jugador(id_jugador, evento);
    }
}

bool Sector::tratar_tierra(Notificador& notificador, const Casilla& casilla) {
    if (!tierra.tratar(casilla)) {
        return false;
    }
    avisar_casilla(notificador, casilla);
    return true;
}

bool Sector::volver_tierra_a_comun(Notificador& notificador, const Casilla& casilla) {
    if (!tierra.volver_a_comun(casilla)) {
        return false;
    }
    huerta.quitar(casilla);  // lo que estaba plantado se pierde
    avisar_casilla(notificador, casilla);
    return true;
}

bool Sector::se_puede_plantar(const Casilla& casilla) const {
    return tierra.esta_tratada(casilla) && !huerta.hay_cultivo(casilla);
}

bool Sector::plantar(Notificador& notificador, const Casilla& casilla, TipoCultivo tipo) {
    if (!se_puede_plantar(casilla) || !huerta.plantar(casilla, tipo)) {
        return false;
    }
    avisar_casilla(notificador, casilla);
    return true;
}

bool Sector::regar(Notificador& notificador, const Casilla& casilla) {
    if (!huerta.regar(casilla)) {
        return false;
    }
    avisar_casilla(notificador, casilla);
    return true;
}

std::optional<Cosecha> Sector::cosechar(Notificador& notificador, const Casilla& casilla) {
    std::optional<Cosecha> cosecha = huerta.cosechar(casilla);
    if (cosecha.has_value()) {
        avisar_casilla(notificador, casilla);
    }
    return cosecha;
}

void Sector::avanzar_cultivos(Notificador& notificador, int minuto_del_dia) {
    for (const Casilla& casilla : huerta.avanzar_hasta(minuto_del_dia)) {
        avisar_casilla(notificador, casilla);
    }
}

void Sector::terminar_el_dia_de_los_cultivos(Notificador& notificador) {
    for (const Casilla& casilla : huerta.terminar_el_dia()) {
        avisar_casilla(notificador, casilla);
    }
}

void Sector::avisar_casilla(Notificador& notificador, const Casilla& casilla) const {
    TierraCambiada evento;
    evento.casilla =
        CasillaTratada{static_cast<uint32_t>(casilla.columna), static_cast<uint32_t>(casilla.fila)};
    evento.tratada = tierra.esta_tratada(casilla);
    huerta.completar(evento.casilla);
    avisar(notificador, evento);
}

void Sector::cargar_cultivo(const DatosCultivoGuardado& guardado) {
    huerta.cargar(guardado);
}

void Sector::agregar_cultivos_guardados_a(std::vector<DatosCultivoGuardado>& lista) const {
    huerta.agregar_guardados_a(id, lista);
}

void Sector::registrar_manejadores_de_modificaciones() {
    manejadores_modificaciones[TipoModificacionCasilla::TIERRA_TRATADA] =
        [this](uint32_t columna, uint32_t fila) {
            tierra.cargar(static_cast<int>(columna), static_cast<int>(fila));
        };
}

void Sector::cargar_casillas_modificadas(const std::vector<ModificacionCasilla>& modificaciones) {
    for (const auto& mod : modificaciones) {
        auto manejador =
            manejadores_modificaciones.find(static_cast<TipoModificacionCasilla>(mod.propiedad));
        if (manejador != manejadores_modificaciones.end()) {
            manejador->second(mod.columna, mod.fila);
        }
    }
}

void Sector::agregar_modificaciones_a(std::vector<ModificacionCasilla>& modificaciones) const {
    tierra.agregar_modificaciones_a(modificaciones);
}

void Sector::cargar_cofre(
    uint32_t columna, uint32_t fila, const std::vector<RanuraInventario>& ranuras) {
    const int id_objeto =
        objetos.colocar(TipoObjeto::COFRE, static_cast<int>(columna), static_cast<int>(fila));
    if (id_objeto > 0) {
        cofres.cargar_contenido(id_objeto, ranuras);
    }
}

void Sector::agregar_cofres_guardados_a(std::vector<DatosCofreGuardado>& lista) const {
    for (const auto& objeto : objetos.obtener_objetos_de_tipo(TipoObjeto::COFRE)) {
        DatosCofreGuardado cofre;
        cofre.id_mapa = id;
        cofre.columna = static_cast<uint32_t>(objeto.columna);
        cofre.fila = static_cast<uint32_t>(objeto.fila);
        cofre.inventario = cofres.obtener_contenido(objeto.id);
        lista.push_back(cofre);
    }
}

void Sector::cargar_mueble(TipoObjeto tipo, uint32_t columna, uint32_t fila) {
    objetos.colocar(tipo, static_cast<int>(columna), static_cast<int>(fila));
}

void Sector::agregar_muebles_guardados_a(std::vector<DatosMuebleGuardado>& lista) const {
    for (const DefinicionDeObjeto& mueble : Muebles::definiciones()) {
        for (const auto& objeto : objetos.obtener_objetos_de_tipo(mueble.tipo)) {
            DatosMuebleGuardado guardado;
            guardado.id_mapa = id;
            guardado.tipo = objeto.tipo;
            guardado.columna = static_cast<uint32_t>(objeto.columna);
            guardado.fila = static_cast<uint32_t>(objeto.fila);
            lista.push_back(guardado);
        }
    }
}

int Sector::colocar_objeto(Notificador& notificador, TipoObjeto tipo, int columna, int fila) {
    const int id = objetos.colocar(tipo, columna, fila);
    if (id == 0) {
        return 0;
    }

    ObjetoColocado evento;
    evento.id = static_cast<uint32_t>(id);
    evento.columna = static_cast<uint32_t>(columna);
    evento.fila = static_cast<uint32_t>(fila);
    evento.tipo = tipo;
    avisar(notificador, evento);
    return id;
}

void Sector::retirar_objeto(Notificador& notificador, int id_objeto) {
    objetos.retirar(id_objeto);

    ObjetoRetirado evento;
    evento.id = static_cast<uint32_t>(id_objeto);
    avisar(notificador, evento);
}

ResultadoDeGolpe Sector::golpear_objeto(
    Notificador& notificador, int id_objeto, TipoItem herramienta) {
    const ResultadoDeGolpe golpe = objetos.golpear(id_objeto, herramienta);
    if (golpe.golpeo && golpe.rotura.se_rompio) {
        ObjetoRetirado evento;
        evento.id = static_cast<uint32_t>(id_objeto);
        avisar(notificador, evento);
    }
    return golpe;
}

ResultadoDeGolpeEnemigo Sector::golpear_enemigo(
    Notificador& notificador, int id_enemigo, int danio) {
    const ResultadoDeGolpeEnemigo golpe = enemigos.golpear(id_enemigo, danio);
    if (golpe.golpeo) {
        const Enemigo* enemigo = enemigos.buscar(id_enemigo);
        if (enemigo != nullptr) {
            EnemigoActualizado evento;
            evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
            avisar(notificador, evento);
        }
        for (int id_afectado : golpe.afectados) {
            const Enemigo* afectado = enemigos.buscar(id_afectado);
            if (afectado != nullptr) {
                EnemigoActualizado evento;
                evento.enemigo = GestorDeEnemigos::estado_de(*afectado);
                avisar(notificador, evento);
            }
        }
    }
    return golpe;
}

int Sector::soltar(
    Notificador& notificador, TipoItem tipo, uint16_t cantidad, Calidad calidad, int columna,
    int fila) {
    if (!mapa.esta_dentro(columna, fila)) {
        return 0;
    }

    const int id = drops.dejar(tipo, cantidad, calidad, columna, fila);
    if (id == 0) {
        return 0;
    }

    EstadoDrop estado;
    estado.id = static_cast<uint32_t>(id);
    estado.columna = static_cast<uint32_t>(columna);
    estado.fila = static_cast<uint32_t>(fila);
    estado.tipo = tipo;
    estado.cantidad = cantidad;
    estado.calidad = calidad;

    DropAparecido evento;
    evento.drop = estado;
    avisar(notificador, evento);
    return id;
}

int Sector::aparecer_grupo_persiguiendo(
    Notificador& notificador, uint32_t id_jugador, int distancia, int cantidad, TipoEnemigo tipo) {
    const std::optional<Casilla> origen = jugadores.casilla_del_jugador(id_jugador);
    if (!origen.has_value() || cantidad <= 0) {
        return 0;
    }
    std::vector<Casilla> ocupadas_por_jugadores;
    for (uint32_t otro : jugadores.ids()) {
        const std::optional<Casilla> casilla = jugadores.casilla_del_jugador(otro);
        if (casilla.has_value()) {
            ocupadas_por_jugadores.push_back(*casilla);
        }
    }
    const auto lejania = [&origen](const Casilla& casilla) {
        return std::max(
            std::abs(casilla.columna - origen->columna),
            std::abs(casilla.fila - origen->fila));
    };
    const auto se_puede_aparecer = [&](const Casilla& casilla) {
        return enemigos.enemigo_vivo_en(casilla.columna, casilla.fila) == nullptr &&
               std::find(ocupadas_por_jugadores.begin(), ocupadas_por_jugadores.end(), casilla) ==
                   ocupadas_por_jugadores.end();
    };

    const int alcance = distancia + 3;
    std::vector<Casilla> visitadas{*origen};
    std::vector<Casilla> candidatas;
    std::vector<Casilla> mas_lejanas;
    for (size_t i = 0; i < visitadas.size(); ++i) {
        const Casilla actual = visitadas[i];
        const int lejos = lejania(actual);
        if (lejos >= 2 && se_puede_aparecer(actual)) {
            (lejos >= distancia ? candidatas : mas_lejanas).push_back(actual);
        }
        const Casilla vecinas[] = {
            {actual.columna + 1, actual.fila},
            {actual.columna - 1, actual.fila},
            {actual.columna, actual.fila + 1},
            {actual.columna, actual.fila - 1}};
        for (const Casilla& vecina : vecinas) {
            if (lejania(vecina) > alcance ||
                std::find(visitadas.begin(), visitadas.end(), vecina) != visitadas.end() ||
                !mapa.esta_dentro(vecina.columna, vecina.fila) ||
                !colisiones.esta_libre(vecina.columna, vecina.fila)) {
                continue;
            }
            visitadas.push_back(vecina);
        }
    }
    if (candidatas.empty()) {
        candidatas = mas_lejanas;
    }
    if (candidatas.empty()) {
        return 0;
    }
    const Casilla ancla =
        candidatas[static_cast<size_t>(Azar::entero(0, static_cast<int>(candidatas.size()) - 1))];
    std::sort(candidatas.begin(), candidatas.end(), [&ancla](const Casilla& a, const Casilla& b) {
        const auto d = [&ancla](const Casilla& c) {
            return std::abs(c.columna - ancla.columna) + std::abs(c.fila - ancla.fila);
        };
        return d(a) < d(b);
    });
    if (static_cast<int>(candidatas.size()) > cantidad) {
        candidatas.resize(static_cast<size_t>(cantidad));
    }

    const int colocados = enemigos.poblar_grupo_en(candidatas, tipo);
    for (const Casilla& casilla : candidatas) {
        const Enemigo* enemigo = enemigos.enemigo_vivo_en(casilla.columna, casilla.fila);
        if (enemigo == nullptr) {
            continue;
        }
        enemigos.perseguir(enemigo->id, id_jugador);
        EnemigoActualizado evento;
        evento.enemigo = GestorDeEnemigos::estado_de(*enemigo);
        avisar(notificador, evento);
    }
    return colocados;
}
