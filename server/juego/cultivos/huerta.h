#ifndef HUERTA_H
#define HUERTA_H

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../../common/calidad.h"
#include "../../../common/mensajes/mensajes.h"
#include "../../configuracion/configuracion_servidor_datos.h"
#include "../../persistencia/datos_partida_guardada.h"
#include "../geometria.h"
#include "cultivos.h"

struct Cultivo {
    TipoCultivo tipo = TipoCultivo::NINGUNO;
    int dias_de_crecimiento = 0;
    bool madura = false;
    int turno = -1;
    bool pide_agua = false;
    int inicio_del_turno = 0;
    int fin_del_turno = 0;
    int riegos_perdidos_hoy = 0;
    int faltas = 0;
    bool ya_dio_fruto = false;
};

struct Cosecha {
    TipoItem producto = TipoItem::NINGUNO;
    Calidad calidad = Calidad::BASE;
};

// Las plantas de UN sector, como TierraTratada guarda la tierra. Sabe las reglas de los cultivos:
// el día se parte en turnos de riego, cada turno la planta pide agua, al terminar el día crece si
// no le faltaron riegos, y al cosecharla da su producto con una calidad que depende de las faltas
// y de la suerte. Que la casilla esté tratada lo controla el Sector antes de plantar.
//
// El tiempo lo pone ControladorDeCultivos: cada vez que cambia la hora llama a avanzar_hasta() y
// al empezar un día nuevo, a terminar_el_dia().
class Huerta {
private:
    const CatalogoDeCultivos& catalogo;
    const ConfiguracionCultivosServidor& configuracion;
    std::map<std::pair<int, int>, Cultivo> cultivos;
    int minuto_del_dia = 0;

    const DefinicionDeCultivo& definicion_de(const Cultivo& cultivo) const;

    // El turno al que pertenece un minuto del día para ese cultivo, y sus límites.
    int turno_en(const DefinicionDeCultivo& definicion, int minuto) const;
    int inicio_de_turno(const DefinicionDeCultivo& definicion, int turno) const;

    // Arranca el turno de ahora: la planta pide agua desde "desde" hasta el fin del turno.
    void empezar_turno(Cultivo& cultivo, int desde) const;

    // Si en el turno que termina no se regó, es un riego perdido.
    void cerrar_turno(Cultivo& cultivo) const;

    Calidad calidad_al_cosechar(const Cultivo& cultivo) const;

    Cultivo* buscar(const Casilla& casilla);

public:
    Huerta(const CatalogoDeCultivos& catalogo, const ConfiguracionCultivosServidor& configuracion);

    const Cultivo* cultivo_en(const Casilla& casilla) const;
    bool hay_cultivo(const Casilla& casilla) const;

    // Planta en esa casilla. Devuelve false si ya hay algo o el cultivo no existe.
    bool plantar(const Casilla& casilla, TipoCultivo tipo);

    // Si regar() haría algo: hay una planta que en este turno todavía pide agua.
    bool pide_agua(const Casilla& casilla) const;

    // Riega la planta. Si se hace en la segunda mitad del turno, cuenta como riego tardío.
    // Devuelve false si no había nada que regar.
    bool regar(const Casilla& casilla);

    bool esta_madura(const Casilla& casilla) const;

    // Cosecha una planta madura. La frutilla queda y vuelve a dar; las demás se sacan. nullopt si
    // no había nada para cosechar.
    std::optional<Cosecha> cosechar(const Casilla& casilla);

    // Saca lo que haya plantado (cuando la tierra vuelve a ser común).
    bool quitar(const Casilla& casilla);

    // La hora avanzó hasta ese minuto del día (desde las 6 am). Cierra los turnos que terminaron
    // y abre los nuevos. Devuelve las casillas que cambiaron.
    std::vector<Casilla> avanzar_hasta(int minuto);

    // Terminó el día: las plantas a las que no les faltaron riegos crecen, y las maduras que no se
    // cosecharon suman una falta. Devuelve las casillas que cambiaron.
    std::vector<Casilla> terminar_el_dia();

    // Completa una casilla de tierra con lo que tiene plantado, para mandarla al cliente.
    void completar(CasillaTratada& casilla) const;
    void completar(std::vector<CasillaTratada>& casillas) const;

    // Guardar y cargar la partida.
    void agregar_guardados_a(
        const std::string& id_mapa, std::vector<DatosCultivoGuardado>& lista) const;
    void cargar(const DatosCultivoGuardado& guardado);
};

#endif
