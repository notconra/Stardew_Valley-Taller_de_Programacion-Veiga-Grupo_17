#include "mundo.h"

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

Mundo::Mundo(
    const RecursosDeJuego& recursos_de_juego, Notificador& notificador_de_juego,
    uint8_t ticks_por_segundo, const DatosMundoGuardado& datos_guardados)
    : recursos(recursos_de_juego),
      notificador(notificador_de_juego),
      sectores(recursos_de_juego, ticks_por_segundo, personajes, notificador_de_juego),
      gestor_de_npcs(recursos_de_juego, sectores, ticks_por_segundo),
      controlador_de_jugador(sectores, notificador_de_juego),
      controlador_de_mercado(recursos_de_juego.mercado, sectores, notificador_de_juego),
      controlador_del_dia(
          recursos_de_juego.juego.dia, ticks_por_segundo, sectores, personajes, recursos_de_juego,
          notificador_de_juego,
          [this](Personaje& personaje) { controlador_de_mazmorra.al_desconectarse(personaje); }),
      controlador_de_acciones(
          sectores, personajes, recursos_de_juego, notificador_de_juego, controlador_de_mercado,
          controlador_del_dia,
          [this](Personaje& personaje, const Transicion& transicion) {
              resolver_transicion(personaje, transicion);
          }),
      controlador_de_cofres(sectores, personajes, notificador_de_juego),
      controlador_de_drops(
          sectores, personajes, notificador_de_juego, recursos_de_juego.juego.drops),
      controlador_de_mazmorra(recursos_de_juego, sectores, datos_guardados.niveles_mazmorra),
      controlador_de_enemigos(sectores, personajes, notificador_de_juego),
      controlador_de_trampas(sectores, personajes, notificador_de_juego),
      controlador_de_cultivos(sectores, notificador_de_juego),
      controlador_de_regalos(personajes, recursos_de_juego.catalogo_de_items, notificador_de_juego),
      controlador_del_centro(
          personajes, sectores, recursos_de_juego, notificador_de_juego,
          [this](TipoAnimal animal) {
              return animal == TipoAnimal::VACA ? agregar_vaca_en_granja_cerca(0)
                                                : agregar_gallina_en_granja_cerca(0);
          },
          datos_guardados.centro) {
    sectores.cargar_mapas_guardados(datos_guardados.mapas);
    sectores.cargar_cultivos_guardados(datos_guardados.cultivos);
    sectores.cargar_cofres_guardados(datos_guardados.cofres);
    sectores.cargar_muebles_guardados(datos_guardados.muebles);
    sectores.cargar_animales_guardados(
        datos_guardados.animales,
        datos_guardados.vacas_para_ordeniar);
    if (datos_guardados.dia > 0) {
        controlador_del_dia.continuar_en_el_dia(datos_guardados.dia);
    }
    for (const auto& guardado : datos_guardados.jugadores) {
        datos_jugadores[guardado.id_jugador] = guardado;
        std::string mapa_destino =
            guardado.mapa_actual.empty() ? recursos.mapa_inicial : guardado.mapa_actual;
        auto [it, nuevo] = personajes.try_emplace(
            guardado.id_jugador,
            guardado.id_jugador,
            mapa_destino,
            recursos.catalogo_de_items,
            recursos.juego.inventario.cantidad_ranuras_barra);
        restaurar(it->second, guardado);
    }
}

void Mundo::completar_datos_guardados(
    const Personaje& personaje, DatosJugadorGuardado& datos) const {
    datos.id_jugador = personaje.id_jugador;
    datos.ranura_activa = personaje.ranura_activa;
    datos.inventario = personaje.inventario.todas_las_ranuras();
    datos.apariencia = personaje.apariencia;
    datos.vida = static_cast<uint16_t>(std::max(personaje.vida, 0));
    datos.energia = static_cast<uint16_t>(std::max(personaje.energia, 0));
    datos.monedas = personaje.monedas;
    datos.tiene_mejora_de_pelea = personaje.tiene_mejora_de_pelea;
    datos.regalos_pendientes = personaje.buzon.pendientes();
    datos.tiene_cama = personaje.ultima_cama.has_value();
    datos.mapa_cama = datos.tiene_cama ? personaje.ultima_cama->mapa : "";
    datos.columna_cama =
        datos.tiene_cama ? static_cast<uint32_t>(personaje.ultima_cama->columna) : 0;
    datos.fila_cama = datos.tiene_cama ? static_cast<uint32_t>(personaje.ultima_cama->fila) : 0;
}

void Mundo::restaurar(Personaje& personaje, const DatosJugadorGuardado& guardado) const {
    personaje.ranura_activa = guardado.ranura_activa;
    personaje.apariencia = guardado.apariencia;
    if (!guardado.inventario.empty()) {
        personaje.inventario.cargar_ranuras(guardado.inventario);
    }
    personaje.tiene_mejora_de_pelea = guardado.tiene_mejora_de_pelea;
    personaje.vida = std::min<int>(guardado.vida, personaje.vida_maxima());
    personaje.energia = std::min<int>(guardado.energia, personaje.energia_maxima());
    personaje.monedas = guardado.monedas;
    personaje.nombre = guardado.nombre_jugador;
    personaje.buzon = Buzon(guardado.regalos_pendientes);
    if (guardado.tiene_cama) {
        personaje.ultima_cama = UbicacionDeCama{
            guardado.mapa_cama,
            static_cast<int>(guardado.columna_cama),
            static_cast<int>(guardado.fila_cama)};
    }
}

void Mundo::dar_de_baja(Personaje& personaje) {
    if (sectores.existe(personaje.mapa_actual)) {
        const JugadorEnPartida* jugador_en_partida =
            sectores.en(personaje.mapa_actual).jugadores.buscar(personaje.id_jugador);
        if (jugador_en_partida != nullptr) {
            DatosJugadorGuardado& datos = datos_jugadores[personaje.id_jugador];
            completar_datos_guardados(personaje, datos);
            datos.mapa_actual = personaje.mapa_actual;
            datos.x = jugador_en_partida->x;
            datos.y = jugador_en_partida->y;
            datos.mirando = jugador_en_partida->mirando;
        }
    }
    controlador_de_mazmorra.al_desconectarse(personaje);
    controlador_de_cofres.al_desconectarse(personaje);
    controlador_de_acciones.al_desconectarse(personaje);
    controlador_del_dia.al_desconectarse(personaje);
    controlador_de_mercado.al_desconectarse(personaje);
    sectores.quitar(personaje);
    personaje.conectado = false;
}

void Mundo::conectar(
    uint32_t id_jugador, const AparienciaPersonaje& apariencia, const std::string& nombre) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end()) {
        encontrado = personajes
                         .try_emplace(
                             id_jugador,
                             id_jugador,
                             recursos.mapa_inicial,
                             recursos.catalogo_de_items,
                             recursos.juego.inventario.cantidad_ranuras_barra)
                         .first;
        encontrado->second.apariencia = apariencia;
    }
    Personaje& personaje = encontrado->second;
    if (!nombre.empty()) {
        personaje.nombre = nombre;
    }
    if (personaje.esta_inconsciente()) {
        return;
    }

    if (personaje.conectado) {
        dar_de_baja(personaje);
    }

    personaje.conectado = true;
    notificador.a_jugador(id_jugador, personaje.actualizacion_de_inventario());
    controlador_del_dia.al_conectarse(personaje);
    controlador_de_regalos.al_conectarse(personaje);
    controlador_del_centro.al_conectarse(personaje);

    auto guardado = datos_jugadores.find(id_jugador);
    if (guardado != datos_jugadores.end() && !guardado->second.mapa_actual.empty() &&
        sectores.existe(guardado->second.mapa_actual)) {
        sectores.colocar_en(
            personaje,
            guardado->second.mapa_actual,
            guardado->second.x,
            guardado->second.y,
            guardado->second.mirando);
    } else {
        sectores.colocar_en_aparicion(personaje, recursos.mapa_inicial);
    }
}

void Mundo::desconectar(uint32_t id_jugador) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return;
    }
    dar_de_baja(encontrado->second);
    controlador_de_regalos.al_desconectarse();
}

void Mundo::atender(uint32_t id_jugador, const ComandoCliente& comando) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return;
    }
    Personaje& personaje = encontrado->second;

    if (const HablarConNpc* pedido = std::get_if<HablarConNpc>(&comando)) {
        hablar_con_npc(personaje, *pedido);
        return;
    }
    if (controlador_del_dia.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_jugador.atender(personaje, comando)) {
        return;
    }
    // Va antes que las acciones: el clic secundario sobre un lote del centro lo abre.
    if (controlador_del_centro.atender(personaje, comando, obtener_dia())) {
        return;
    }
    if (controlador_de_acciones.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_cofres.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_drops.atender(personaje, comando)) {
        return;
    }
    if (controlador_de_mercado.atender(personaje, comando, controlador_del_dia.hora())) {
        return;
    }
    if (controlador_de_regalos.atender(personaje, comando)) {
        return;
    }
}

void Mundo::hablar_con_npc(Personaje& personaje, const HablarConNpc& pedido) {
    if (!sectores.existe(personaje.mapa_actual)) {
        return;
    }
    const std::optional<Casilla> casilla =
        sectores.en(personaje.mapa_actual).jugadores.casilla_del_jugador(personaje.id_jugador);
    if (!casilla.has_value()) {
        return;
    }

    const std::optional<std::string> texto = gestor_de_npcs.dialogo(
        pedido.id_npc,
        *casilla,
        personaje.mapa_actual,
        controlador_del_dia.hora());
    if (!texto.has_value()) {
        return;
    }
    notificador.a_jugador(personaje.id_jugador, DialogoNpc{pedido.id_npc, *texto});
}

void Mundo::resolver_transicion(Personaje& personaje, const Transicion& transicion) {
    if (!controlador_de_mercado.puede_entrar(transicion.mapa_destino, controlador_del_dia.hora())) {
        controlador_de_mercado.avisar_mercado_cerrado(personaje);
        return;
    }
    if (controlador_de_mazmorra.resolver_transicion(personaje, transicion)) {
        controlador_de_mercado.al_cambiar_de_mapa(personaje);
        return;
    }
    // La puerta del pueblo lleva al centro en ruinas, o al arreglado si ya se completaron todos los
    // lotes. Los dos mapas tienen la entrada en el mismo lugar.
    const std::string destino = controlador_del_centro.mapa_para(transicion.mapa_destino);
    if (!sectores.existe(destino)) {
        return;
    }
    sectores.trasladar_a_casilla(
        personaje,
        destino,
        transicion.columna_destino,
        transicion.fila_destino);
    controlador_de_mercado.al_cambiar_de_mapa(personaje);
}

bool Mundo::requiere_clic(const Transicion& transicion) const {
    const std::vector<std::string>& destinos = recursos.juego.transiciones.destinos_con_clic;
    return std::find(destinos.begin(), destinos.end(), transicion.mapa_destino) != destinos.end();
}

void Mundo::resolver_transiciones_automaticas() {
    for (const JugadorEnTransicion& en_transicion : sectores.jugadores_en_transicion()) {
        auto encontrado = personajes.find(en_transicion.id_jugador);
        if (encontrado == personajes.end() || requiere_clic(en_transicion.transicion)) {
            continue;
        }
        resolver_transicion(encontrado->second, en_transicion.transicion);
    }
}

void Mundo::ajustar_velocidad_por_energia() {
    for (auto& [id_jugador, personaje] : personajes) {
        if (personaje.conectado && sectores.existe(personaje.mapa_actual)) {
            sectores.en(personaje.mapa_actual)
                .jugadores.fijar_velocidad(id_jugador, personaje.multiplicador_de_velocidad());
        }
    }
}

int Mundo::dar_items(uint32_t id_jugador, TipoItem tipo, uint16_t cantidad) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return 0;
    }
    Personaje& personaje = encontrado->second;
    const uint16_t sin_lugar = personaje.inventario.agregar_item(tipo, cantidad);
    notificador.a_jugador(id_jugador, personaje.actualizacion_de_inventario());
    return cantidad - sin_lugar;
}

void Mundo::actualizar() {
    controlador_del_dia.actualizar();
    controlador_de_cultivos.actualizar(controlador_del_dia.hora());
    gestor_de_npcs.actualizar(controlador_del_dia.hora());
    ajustar_velocidad_por_energia();
    sectores.actualizar(controlador_del_dia.hora());
    resolver_transiciones_automaticas();
    controlador_de_enemigos.actualizar();
    controlador_de_trampas.actualizar();
    sectores.difundir_jugadores();
    sectores.difundir_npcs();
    controlador_de_acciones.actualizar();
    controlador_de_cofres.actualizar();
    controlador_de_drops.actualizar();
    controlador_del_centro.actualizar();
}

std::optional<DatosJugadorGuardado> Mundo::obtener_datos_jugador(uint32_t id_jugador) const {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado != personajes.end()) {
        const Personaje& personaje = encontrado->second;
        if (personaje.conectado && sectores.existe(personaje.mapa_actual)) {
            const JugadorEnPartida* cuerpo =
                sectores.en(personaje.mapa_actual).jugadores.buscar(id_jugador);
            if (cuerpo != nullptr) {
                DatosJugadorGuardado datos;
                completar_datos_guardados(personaje, datos);
                datos.mapa_actual = personaje.mapa_actual;
                datos.x = cuerpo->x;
                datos.y = cuerpo->y;
                datos.mirando = cuerpo->mirando;
                return datos;
            }
        }
    }

    auto guardado = datos_jugadores.find(id_jugador);
    if (guardado == datos_jugadores.end()) {
        return std::nullopt;
    }
    DatosJugadorGuardado datos = guardado->second;
    if (encontrado != personajes.end()) {
        completar_datos_guardados(encontrado->second, datos);
    }
    return datos;
}

std::vector<std::string> Mundo::obtener_niveles_mazmorra() const {
    return controlador_de_mazmorra.obtener_niveles();
}

std::vector<DatosMapaGuardado> Mundo::obtener_mapas_guardados() const {
    return sectores.obtener_mapas_guardados();
}

std::vector<DatosCofreGuardado> Mundo::obtener_cofres_guardados() const {
    return sectores.obtener_cofres_guardados();
}

std::vector<DatosMuebleGuardado> Mundo::obtener_muebles_guardados() const {
    return sectores.obtener_muebles_guardados();
}

std::vector<DatosAnimalGuardado> Mundo::obtener_animales_guardados() const {
    return sectores.obtener_animales_guardados();
}

std::vector<uint32_t> Mundo::obtener_ids_vacas_listas_para_ordeniar() const {
    if (!sectores.existe("granja")) {
        return {};
    }
    return sectores.en("granja").animales.obtener_ids_vacas_listas_para_ordeniar();
}

std::vector<DatosCultivoGuardado> Mundo::obtener_cultivos_guardados() const {
    return sectores.obtener_cultivos_guardados();
}

void Mundo::cambiar_lluvia(bool llueve) {
    controlador_del_dia.cambiar_lluvia(llueve);
}

bool Mundo::esta_lloviendo() const {
    return controlador_del_dia.esta_lloviendo();
}

void Mundo::cambiar_estacion(Estacion estacion) {
    controlador_del_dia.cambiar_estacion(estacion);
}

DatosCentroGuardado Mundo::obtener_centro_guardado() const {
    return controlador_del_centro.obtener_guardado();
}

uint16_t Mundo::donar_al_centro(uint32_t id_jugador, TipoLote lote, uint8_t indice_ranura) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return 0;
    }
    return controlador_del_centro
        .donar_item(encontrado->second, lote, indice_ranura, obtener_dia());
}

bool Mundo::donar_monedas_al_centro(uint32_t id_jugador, TipoLote lote) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return false;
    }
    return controlador_del_centro.donar_monedas(encontrado->second, lote, obtener_dia());
}

const CentroComunitario& Mundo::centro_comunitario() const {
    return controlador_del_centro.estado();
}

uint32_t Mundo::obtener_dia() const {
    return controlador_del_dia.hora().dia;
}

bool Mundo::tomar_dia_terminado() {
    return controlador_del_dia.tomar_dia_terminado();
}

bool Mundo::agregar_vaca_en_granja_cerca(uint32_t id_jugador) {
    if (!sectores.existe("granja")) {
        return false;
    }
    Sector& granja = sectores.en("granja");
    uint32_t centro_x = 20;
    uint32_t centro_y = 15;
    auto it_jugador = personajes.find(id_jugador);
    if (it_jugador != personajes.end() && it_jugador->second.conectado &&
        it_jugador->second.mapa_actual == "granja") {
        const auto casilla = granja.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            centro_x = static_cast<uint32_t>(std::max(casilla->columna, 0));
            centro_y = static_cast<uint32_t>(std::max(casilla->fila, 0));
        }
    }
    for (int radio = 1; radio <= 10; ++radio) {
        for (int dx = -radio; dx <= radio; ++dx) {
            for (int dy = -radio; dy <= radio; ++dy) {
                if (std::abs(dx) != radio && std::abs(dy) != radio) {
                    continue;
                }
                int col = static_cast<int>(centro_x) + dx;
                int fila = static_cast<int>(centro_y) + dy;
                if (col >= 0 && col + 1 < granja.mapa.cantidad_de_columnas() && fila >= 0 &&
                    fila + 1 < granja.mapa.cantidad_de_filas()) {
                    if (granja.agregar_vaca(
                            notificador,
                            static_cast<uint32_t>(col),
                            static_cast<uint32_t>(fila))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Mundo::agregar_gallina_en_granja_cerca(uint32_t id_jugador) {
    if (!sectores.existe("granja")) {
        return false;
    }
    Sector& granja = sectores.en("granja");
    uint32_t centro_x = 22;
    uint32_t centro_y = 15;
    auto it_jugador = personajes.find(id_jugador);
    if (it_jugador != personajes.end() && it_jugador->second.conectado &&
        it_jugador->second.mapa_actual == "granja") {
        const auto casilla = granja.jugadores.casilla_del_jugador(id_jugador);
        if (casilla.has_value()) {
            centro_x = static_cast<uint32_t>(std::max(casilla->columna, 0));
            centro_y = static_cast<uint32_t>(std::max(casilla->fila, 0));
        }
    }
    for (int radio = 1; radio <= 10; ++radio) {
        for (int dx = -radio; dx <= radio; ++dx) {
            for (int dy = -radio; dy <= radio; ++dy) {
                if (std::abs(dx) != radio && std::abs(dy) != radio) {
                    continue;
                }
                int col = static_cast<int>(centro_x) + dx;
                int fila = static_cast<int>(centro_y) + dy;
                if (col >= 0 && col < granja.mapa.cantidad_de_columnas() && fila >= 0 &&
                    fila < granja.mapa.cantidad_de_filas()) {
                    if (granja.agregar_gallina(
                            notificador,
                            static_cast<uint32_t>(col),
                            static_cast<uint32_t>(fila))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

std::string Mundo::estado_animales_granja() const {
    if (!sectores.existe("granja")) {
        return "No existe el sector granja.";
    }
    const Sector& granja = sectores.en("granja");
    int total_vacas = granja.animales.cantidad_vacas();
    int vacas_listas = granja.animales.cantidad_vacas_listas_para_ordeniar();
    int total_gallinas = granja.animales.cantidad_gallinas();
    return "Granja: " + std::to_string(total_vacas) + " vacas (" + std::to_string(vacas_listas) +
           " listas para ordeniar), " + std::to_string(total_gallinas) + " gallinas.";
}

void Mundo::avanzar_dia() {
    controlador_del_dia.avanzar_dia();
}

void Mundo::avanzar_minutos(uint32_t minutos) {
    controlador_del_dia.avanzar_minutos(minutos);
}

HoraDelJuego Mundo::hora_actual() const {
    return controlador_del_dia.hora();
}

bool Mundo::sumar_monedas(uint32_t id_jugador, uint32_t cantidad) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return false;
    }
    encontrado->second.recibir_monedas(cantidad);
    return true;
}

bool Mundo::gastar_monedas(uint32_t id_jugador, uint32_t cantidad) {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return false;
    }
    return encontrado->second.gastar_monedas(cantidad);
}

uint32_t Mundo::obtener_monedas(uint32_t id_jugador) const {
    auto encontrado = personajes.find(id_jugador);
    if (encontrado == personajes.end() || !encontrado->second.conectado) {
        return 0;
    }
    return encontrado->second.monedas;
}
