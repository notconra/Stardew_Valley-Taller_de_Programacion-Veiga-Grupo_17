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

void Sector::agregar_animales_guardados_a(std::vector<DatosAnimalGuardado>& lista) const {
    if (id == "granja") {
        animales.agregar_animales_guardados_a(lista);
    }
}

void Sector::cargar_animales_guardados(
    const std::vector<DatosAnimalGuardado>& lista,
    const std::vector<uint32_t>& vacas_para_ordeniar) {
    if (id == "granja") {
        animales.cargar_animales_guardados(lista, vacas_para_ordeniar);
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

void Sector::avanzar_dia(Notificador& notificador) {
    animales.avanzar_dia(drops);
    std::vector<uint32_t> ids_vacas = animales.obtener_ids_vacas_listas_para_ordeniar();
    for (uint32_t id_vaca : ids_vacas) {
        VacaOrdeniable evento;
        evento.id_vaca = id_vaca;
        evento.lista = true;
        avisar(notificador, evento);
    }
}

void Sector::avanzar_dia() {
    animales.avanzar_dia(drops);
}

bool Sector::vaca_lista_en(uint32_t columna, uint32_t fila) const {
    return animales.vaca_lista_en(columna, fila);
}

bool Sector::ordeniar_vaca(Notificador& notificador, uint32_t columna, uint32_t fila) {
    uint32_t id_vaca = 0;
    if (!animales.ordeniar_vaca_en(columna, fila, id_vaca)) {
        return false;
    }
    VacaOrdeniable evento;
    evento.id_vaca = id_vaca;
    evento.lista = false;
    avisar(notificador, evento);
    return true;
}

bool Sector::agregar_vaca(Notificador& notificador, uint32_t columna, uint32_t fila) {
    if (!animales.agregar_vaca(columna, fila)) {
        return false;
    }
    const Vaca* vaca = animales.vaca_en(columna, fila);
    if (vaca) {
        AnimalActualizado evento;
        evento.animal = GestorDeAnimales::estado_de(*vaca);
        avisar(notificador, evento);
    }
    return true;
}

bool Sector::agregar_gallina(Notificador& notificador, uint32_t columna, uint32_t fila) {
    if (!animales.agregar_gallina(columna, fila)) {
        return false;
    }
    const Gallina* gallina = animales.gallina_en(columna, fila);
    if (gallina) {
        AnimalActualizado evento;
        evento.animal = GestorDeAnimales::estado_de(*gallina);
        avisar(notificador, evento);
    }
    return true;
}

void Sector::actualizar(const HoraDelJuego& hora, Notificador& notificador) {
    if (id == "granja") {
        std::vector<EstadoAnimal> actualizados = animales.actualizar(
            hora,
            mapa.cantidad_de_columnas(),
            mapa.cantidad_de_filas(),
            jugadores);
        for (const auto& animal : actualizados) {
            AnimalActualizado evento;
            evento.animal = animal;
            avisar(notificador, evento);
        }
    }
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
