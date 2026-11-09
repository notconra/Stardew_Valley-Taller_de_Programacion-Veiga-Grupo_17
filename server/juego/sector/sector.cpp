#include "sector.h"

#include <optional>
#include <vector>

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

    TierraCambiada evento;
    evento.casilla =
        CasillaTratada{static_cast<uint32_t>(casilla.columna), static_cast<uint32_t>(casilla.fila)};
    evento.tratada = true;
    avisar(notificador, evento);
    return true;
}

bool Sector::volver_tierra_a_comun(Notificador& notificador, const Casilla& casilla) {
    if (!tierra.volver_a_comun(casilla)) {
        return false;
    }

    TierraCambiada evento;
    evento.casilla =
        CasillaTratada{static_cast<uint32_t>(casilla.columna), static_cast<uint32_t>(casilla.fila)};
    evento.tratada = false;
    avisar(notificador, evento);
    return true;
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
