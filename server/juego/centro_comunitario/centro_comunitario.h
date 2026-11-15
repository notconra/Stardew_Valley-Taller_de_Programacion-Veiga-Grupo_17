#ifndef CENTRO_COMUNITARIO_H
#define CENTRO_COMUNITARIO_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../common/logro_de_pelea.h"
#include "../../../common/ranura_inventario.h"
#include "../../../common/tipo_lote.h"
#include "../../persistencia/datos_partida_guardada.h"
#include "catalogo_lotes.h"

// El id del mapa al que lleva la puerta del pueblo y el del centro ya arreglado, que se usa en su
// lugar cuando se completaron todos los lotes.
#define MAPA_DEL_CENTRO "centro_comunitario"
#define MAPA_DEL_CENTRO_ARREGLADO "centro_comunitario_arreglado"

// Lo que se lleva aportado a un lote y si ya se completó.
struct EstadoDeLote {
    std::vector<uint32_t> aportado;
    bool completo = false;
    uint32_t dia_completado = 0;
};

// El estado del centro comunitario de una partida, común a todos los jugadores: cuánto se donó a
// cada lote, cuáles se completaron y si ya se completaron todos. Solo sabe las reglas de los lotes;
// sacar los ítems del inventario, cobrar las monedas o repartir los premios lo hace quien lo usa.
//
// Cuando un lote se completa queda anotado para tomar_lotes_completados(), y cuando se completa el
// último, para tomar_fin_del_juego(). Así quien lo usa reacciona una sola vez a cada cosa.
class CentroComunitario {
private:
    const CatalogoDeLotes& catalogo;
    std::map<TipoLote, EstadoDeLote> estados;
    bool terminado = false;
    std::vector<TipoLote> lotes_recien_completados;
    bool fin_sin_avisar = false;

    EstadoDeLote* estado_de(TipoLote tipo);

    // El requisito de ítem pendiente que acepta ese ítem, o -1 si ninguno.
    int requisito_para(TipoLote tipo, const RanuraInventario& item) const;

    // Si todos los requisitos del lote están cumplidos, lo completa.
    void completar_si_corresponde(TipoLote tipo, uint32_t dia);

public:
    explicit CentroComunitario(const CatalogoDeLotes& catalogo_de_lotes);

    // Cuántas unidades de ese ítem tomaría el lote: lo que le falta al requisito, hasta lo que
    // haya en la ranura. 0 si el lote está completo, no pide ese ítem o la calidad no alcanza.
    uint16_t cuanto_acepta(TipoLote tipo, const RanuraInventario& item) const;

    // Suma al lote lo que acepte de ese ítem y devuelve cuántas unidades tomó. Sacarlas del
    // inventario le toca a quien llama. "dia" es el día del juego, por si el lote se completa.
    uint16_t donar_item(TipoLote tipo, const RanuraInventario& item, uint32_t dia);

    // Las monedas que pide el lote, o 0 si no pide monedas o ya está completo.
    uint32_t monedas_pedidas(TipoLote tipo) const;

    // Completa un lote de monedas. Cobrarlas le toca a quien llama, antes. Devuelve false si el
    // lote no pide monedas o ya estaba completo.
    bool donar_monedas(TipoLote tipo, uint32_t dia);

    // Marca un logro del lote de pelea. Devuelve false si ya estaba cumplido.
    bool cumplir_logro(LogroDePelea logro, uint32_t dia);

    bool esta_completo(TipoLote tipo) const;
    bool esta_terminado() const;

    // Cuánto se lleva aportado a un requisito del lote (en el orden del catálogo).
    uint32_t aportado(TipoLote tipo, std::size_t indice_requisito) const;

    // Si en ese día rige el premio del lote de dinero: es solo el día siguiente a completarlo.
    bool hay_precios_del_lote_de_dinero(uint32_t dia) const;

    // El mapa al que hay que llevar a quien entra al centro: las ruinas mientras falten lotes y el
    // centro arreglado cuando están todos. Cualquier otro destino se devuelve igual.
    std::string mapa_para(const std::string& destino) const;

    // Los lotes que se completaron desde la última vez que se preguntó.
    std::vector<TipoLote> tomar_lotes_completados();

    // true una sola vez: cuando se completó el último lote.
    bool tomar_fin_del_juego();

    DatosCentroGuardado guardar() const;

    // Lo guardado reemplaza el estado. Los lotes completos al cargar no se vuelven a anunciar.
    void cargar(const DatosCentroGuardado& guardado);
};

#endif
