#ifndef GESTOR_DE_ANIMALES_H
#define GESTOR_DE_ANIMALES_H

#include <cstdint>
#include <iterator>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "../../../common/mensajes/mensajes_partida.h"
#include "../../persistencia/datos_partida_guardada.h"
#include "../colisiones/colisiones.h"
#include "../drops/drops.h"
#include "../jugadores/jugadores.h"
#include "gallina.h"
#include "vaca.h"

class GestorDeAnimales {
private:
    uint32_t siguiente_id = 1;
    Colisiones& colisiones;
    std::map<uint32_t, Vaca> vacas;
    std::map<uint32_t, Gallina> gallinas;

    uint8_t sortear_variante_vaca();
    uint8_t sortear_variante_gallina();

    static std::pair<int, int> desplazamiento_segun(Direccion direccion);
    static DatosAnimalGuardado datos_guardados_de(const Animal& animal);

    bool puede_ubicarse(const Animal& animal) const;
    void ocupar_colisiones(const Animal& animal);
    void liberar_colisiones(const Animal& animal);
    bool casillas_libres(const std::vector<Casilla>& casillas) const;

    void actualizar_rutina(
        Animal& animal, const HoraDelJuego& hora, std::vector<EstadoAnimal>& actualizados);
    void decidir_movimiento(
        Animal& animal, int ancho_mapa, int alto_mapa, const Jugadores& jugadores,
        std::vector<EstadoAnimal>& actualizados);
    bool intentar_mover(
        Animal& animal, Direccion dir, int ancho_mapa, int alto_mapa, const Jugadores& jugadores);

    void actualizar_animal(
        Animal& animal, const HoraDelJuego& hora, int ancho_mapa, int alto_mapa,
        const Jugadores& jugadores, std::vector<EstadoAnimal>& actualizados);

    Vaca* buscar_vaca_en(uint32_t columna, uint32_t fila);
    Gallina* buscar_gallina_en(uint32_t columna, uint32_t fila);

    // Agrega un animal de esa especie en la casilla indicada si entra en el mapa y está libre. El
    // id solo se gasta si el animal se agregó.
    template <typename Especie>
    std::optional<EstadoAnimal> colocar(
        std::map<uint32_t, Especie>& animales, const Casilla& casilla, uint8_t variante,
        int ancho_mapa, int alto_mapa) {
        const uint32_t id = siguiente_id;
        Especie animal(
            id,
            static_cast<uint32_t>(casilla.columna),
            static_cast<uint32_t>(casilla.fila),
            variante);
        const bool entra = casilla.columna + animal.ancho_en_casillas <= ancho_mapa &&
                           casilla.fila + animal.alto_en_casillas <= alto_mapa;
        if (!entra || !puede_ubicarse(animal)) {
            return std::nullopt;
        }
        ++siguiente_id;
        ocupar_colisiones(animal);
        animales.emplace(id, animal);
        return estado_de(animal);
    }

    // Saca de la granja el animal con el id más alto (el que se agregó último) y devuelve su id.
    template <typename Animales>
    std::optional<uint32_t> quitar_el_ultimo_de(Animales& animales) {
        if (animales.empty()) {
            return std::nullopt;
        }
        const auto ultimo = std::prev(animales.end());
        const uint32_t id = ultimo->first;
        liberar_colisiones(ultimo->second);
        animales.erase(ultimo);
        return id;
    }

public:
    explicit GestorDeAnimales(Colisiones& colisiones_mapa);

    // Intentan agregar un animal a la granja sorteando su variante. Devuelven false si se supera el
    // límite.
    bool agregar_vaca(uint32_t columna, uint32_t fila);
    // Agrega una vaca asignando una variante específica (para persistencia o tests).
    bool agregar_vaca(uint32_t columna, uint32_t fila, uint8_t variante);

    // Intentan agregar una gallina a la granja sorteando su variante. Devuelven false si se supera
    // el límite.
    bool agregar_gallina(uint32_t columna, uint32_t fila);
    // Agrega una gallina asignando una variante específica (para persistencia o tests).
    bool agregar_gallina(uint32_t columna, uint32_t fila, uint8_t variante);

    // Agrega un animal del tipo pedido en la casilla libre más cercana a (columna, fila).
    // Devuelve su estado, o nullopt si no hay lugar o la granja llegó al límite de ese tipo.
    std::optional<EstadoAnimal> agregar_cerca_de(
        TipoAnimal tipo, int columna, int fila, int ancho_mapa, int alto_mapa);

    // Saca de la granja el último animal agregado de ese tipo. Devuelve su id, o nullopt si
    // no había ninguno.
    std::optional<uint32_t> quitar_ultimo(TipoAnimal tipo);

    // Avanza un día para todos los animales de la granja:
    // - Las vacas acumulan días hacia el ciclo de 2 días de leche.
    // - Las gallinas ponen un huevo en el suelo como Drop de forma automática.
    std::vector<EstadoDrop> avanzar_dia(Drops& drops);

    // Intenta ordeñar una vaca por su id. Devuelve true si tenía leche y se completó la extracción.
    bool ordeniar_vaca(uint32_t id_vaca);

    // Intenta ordeñar la vaca ubicada en la casilla (columna, fila).
    // Devuelve true si había una vaca lista para ordeñar y asigna su id a id_vaca_ordenada.
    bool ordeniar_vaca_en(uint32_t columna, uint32_t fila, uint32_t& id_vaca_ordenada);

    // Consulta si hay una vaca lista para ordeñar en dicha casilla
    bool vaca_lista_en(uint32_t columna, uint32_t fila) const;

    // Consulta de vaca por casilla (cualquiera de sus casillas 2x2)
    const Vaca* vaca_en(uint32_t columna, uint32_t fila) const;

    // Consulta de gallina por casilla (1x1)
    const Gallina* gallina_en(uint32_t columna, uint32_t fila) const;

    // Actualiza movimiento aleatorio y rutina según la hora del día. Devuelve los animales que
    // cambiaron de estado o posición.
    std::vector<EstadoAnimal> actualizar(
        const HoraDelJuego& hora, int ancho_mapa, int alto_mapa, const Jugadores& jugadores);

    static EstadoAnimal estado_de(const Animal& animal);

    // Agrega el estado actual de los animales al snapshot del sector
    void agregar_a(EstadoJuego& estado) const;

    // Guarda y restaura el estado de los animales en el archivo de partida
    void agregar_animales_guardados_a(std::vector<DatosAnimalGuardado>& lista) const;
    void cargar_animales_guardados(
        const std::vector<DatosAnimalGuardado>& lista,
        const std::vector<uint32_t>& vacas_para_ordeniar);

    int cantidad_vacas() const;
    int cantidad_gallinas() const;
    int cantidad_total() const;
    int cantidad_de(TipoAnimal tipo) const;
    int cantidad_vacas_listas_para_ordeniar() const;
    std::vector<uint32_t> obtener_ids_vacas_listas_para_ordeniar() const;

    const Vaca* buscar_vaca(uint32_t id) const;
    const Gallina* buscar_gallina(uint32_t id) const;
    const Animal* buscar(uint32_t id) const;
};

#endif
