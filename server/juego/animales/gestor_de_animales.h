#ifndef GESTOR_DE_ANIMALES_H
#define GESTOR_DE_ANIMALES_H

#include <map>
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

    // NOTA PROVISORIA: Inicializa 1 vaca y 1 gallina para permitir pruebas y visualización
    // en tiempo de ejecución mientras no existan los comerciantes.
    // Eliminar una vez que se implemente la compra de animales en el mercado con Marnie.
    void inicializar_granja_provisoria();

    // Avanza un día para todos los animales de la granja:
    // - Las vacas acumulan días hacia el ciclo de 2 días de leche.
    // - Las gallinas ponen un huevo en el suelo como Drop de forma automática.
    void avanzar_dia(Drops& drops);

    // Intenta ordeñar una vaca por su id. Devuelve true si tenía leche y se completó la extracción.
    bool ordeniar_vaca(uint32_t id_vaca);

    // Consulta de vaca por casilla (cualquiera de sus casillas 2x2)
    const Vaca* vaca_en(uint32_t columna, uint32_t fila) const;
    Vaca* buscar_vaca_en(uint32_t columna, uint32_t fila);

    // Consulta de gallina por casilla (1x1)
    const Gallina* gallina_en(uint32_t columna, uint32_t fila) const;
    Gallina* buscar_gallina_en(uint32_t columna, uint32_t fila);

    // Actualiza movimiento aleatorio y rutina según la hora del día. Devuelve los animales que
    // cambiaron de estado o posición.
    std::vector<EstadoAnimal> actualizar(
        const HoraDelJuego& hora, int ancho_mapa, int alto_mapa, const Jugadores& jugadores);

    static EstadoAnimal estado_de(const Animal& animal);

    // Agrega el estado actual de los animales al snapshot del sector
    void agregar_a(EstadoJuego& estado) const;

    // Guarda y restaura el estado de los animales en el archivo de partida
    void agregar_animales_guardados_a(std::vector<DatosAnimalGuardado>& lista) const;
    void cargar_animales_guardados(const std::vector<DatosAnimalGuardado>& lista);

    int cantidad_vacas() const;
    int cantidad_gallinas() const;
    int cantidad_total() const;

    const Vaca* buscar_vaca(uint32_t id) const;
    const Gallina* buscar_gallina(uint32_t id) const;
    const Animal* buscar(uint32_t id) const;

    const std::map<uint32_t, Vaca>& obtener_vacas() const;
    const std::map<uint32_t, Gallina>& obtener_gallinas() const;
};

#endif
