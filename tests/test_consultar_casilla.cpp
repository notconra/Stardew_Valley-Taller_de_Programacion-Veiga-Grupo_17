#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/controladores/controlador_de_acciones.h"
#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/muebles/muebles.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/sector/gestor_de_sectores.h"

// Estas pruebas leen config/servidor.toml y los mapas de Assets/, así que corren desde la raíz del
// repositorio (el target test del Makefile ejecuta ./build/tests desde ahí).

namespace {

const uint32_t ID_JUGADOR = 1;
const Casilla CULTIVABLE{13, 12};

const RecursosDeJuego& recursos() {
    static const RecursosDeJuego recursos_de_juego = [] {
        const ConfiguracionServidor configuracion("config/servidor.toml");
        std::vector<ConfiguracionMapaServidor> mapas;
        for (const ConfiguracionMapaServidor& mapa : configuracion.mapas()) {
            if (mapa.id == "granja" || mapa.id == MAPA_DE_LA_CASA) {
                mapas.push_back(mapa);
            }
        }
        return RecursosDeJuego(
            mapas,
            "granja",
            configuracion.juego(),
            ConfiguracionMazmorraServidor{});
    }();
    return recursos_de_juego;
}

class NotificadorDePrueba : public Notificador {
public:
    std::optional<CasillaConsultada> respuesta;

    void a_jugador(uint32_t, const EventoServidor& evento) override {
        if (const CasillaConsultada* consultada = std::get_if<CasillaConsultada>(&evento)) {
            respuesta = *consultada;
        }
    }
    void a_todos(const EventoServidor&) override {
    }
};

class ConsultarCasillaTest : public ::testing::Test {
protected:
    NotificadorDePrueba notificador;
    std::map<uint32_t, Personaje> personajes;
    Personaje& personaje = personajes
                               .try_emplace(
                                   ID_JUGADOR, ID_JUGADOR, "granja", recursos().catalogo_de_items,
                                   recursos().juego.inventario.cantidad_ranuras_barra)
                               .first->second;
    GestorDeSectores sectores{recursos(), 40, personajes, notificador};
    ControladorDeAcciones acciones{
        sectores, personajes, recursos(), notificador, [](Personaje&, const Transicion&) {}};

    // Lleva al jugador al mapa con los pies centrados en (columna, fila).
    void parar_en(const std::string& mapa, int columna, int fila) {
        const ConfiguracionMovimientoJugador& movimiento = recursos().juego.movimiento;
        sectores.colocar_en_aparicion(personaje, "granja");
        sectores.trasladar(
            personaje,
            mapa,
            columna * 16 + (16 - movimiento.hitbox_ancho) / 2.0 - movimiento.hitbox_x,
            (fila + 1) * 16 - movimiento.hitbox_y - movimiento.hitbox_alto);
    }

    void elegir(TipoItem tipo) {
        for (uint8_t i = 0; i < recursos().juego.inventario.cantidad_ranuras_barra; ++i) {
            if (personaje.ranura(i).tipo == tipo) {
                personaje.seleccionar_ranura(i);
                return;
            }
        }
        FAIL() << "el personaje no tiene ese ítem";
    }

    CasillaConsultada consultar(int columna, int fila) {
        notificador.respuesta.reset();
        acciones.atender(
            personaje,
            ComandoCliente{
                ConsultarCasilla{static_cast<uint32_t>(columna), static_cast<uint32_t>(fila)}});
        EXPECT_TRUE(notificador.respuesta.has_value());
        return notificador.respuesta.value_or(CasillaConsultada{});
    }
};

}  // namespace

TEST_F(ConsultarCasillaTest, ConLaCamaSeMarcaVerdeDondeEntra) {
    parar_en(MAPA_DE_LA_CASA, 5, 5);
    elegir(TipoItem::CAMA);

    const CasillaConsultada respuesta = consultar(6, 5);
    EXPECT_EQ(respuesta.columna, 6u);
    EXPECT_EQ(respuesta.fila, 5u);
    EXPECT_EQ(respuesta.ancho, ANCHO_DE_LA_CAMA);
    EXPECT_EQ(respuesta.alto, ALTO_DE_LA_CAMA);
    EXPECT_EQ(respuesta.marca, MarcaDeCasilla::VALIDA);
}

TEST_F(ConsultarCasillaTest, ConLaCamaSeMarcaRojoDondeNoEntra) {
    parar_en(MAPA_DE_LA_CASA, 5, 5);
    elegir(TipoItem::CAMA);

    EXPECT_EQ(consultar(5, 5).marca, MarcaDeCasilla::INVALIDA);
    EXPECT_EQ(consultar(5, 4).marca, MarcaDeCasilla::INVALIDA);
    EXPECT_EQ(consultar(9, 9).marca, MarcaDeCasilla::INVALIDA);
    EXPECT_EQ(consultar(9, 9).alto, ALTO_DE_LA_CAMA);
}

TEST_F(ConsultarCasillaTest, ConLaCamaFueraDeLaCasaSeMarcaRojo) {
    parar_en("granja", 13, 11);
    elegir(TipoItem::CAMA);

    EXPECT_EQ(consultar(14, 11).marca, MarcaDeCasilla::INVALIDA);
}

TEST_F(ConsultarCasillaTest, LaPalaMarcaSoloLaTierraQueSePuedeTratar) {
    parar_en("granja", CULTIVABLE.columna, CULTIVABLE.fila - 1);
    elegir(TipoItem::PALA);

    const CasillaConsultada respuesta = consultar(CULTIVABLE.columna, CULTIVABLE.fila);
    EXPECT_EQ(respuesta.marca, MarcaDeCasilla::VALIDA);
    EXPECT_EQ(respuesta.ancho, 1);
    EXPECT_EQ(respuesta.alto, 1);

    sectores.en("granja").tratar_tierra(notificador, CULTIVABLE);
    EXPECT_EQ(consultar(CULTIVABLE.columna, CULTIVABLE.fila).marca, MarcaDeCasilla::NINGUNA);
}

TEST_F(ConsultarCasillaTest, ElPicoMarcaLaTierraTratada) {
    parar_en("granja", CULTIVABLE.columna, CULTIVABLE.fila - 1);
    elegir(TipoItem::PICO);
    EXPECT_EQ(consultar(CULTIVABLE.columna, CULTIVABLE.fila).marca, MarcaDeCasilla::NINGUNA);

    sectores.en("granja").tratar_tierra(notificador, CULTIVABLE);
    EXPECT_EQ(consultar(CULTIVABLE.columna, CULTIVABLE.fila).marca, MarcaDeCasilla::VALIDA);
}

TEST_F(ConsultarCasillaTest, LejosOConOtroItemNoSeMarcaNada) {
    parar_en("granja", CULTIVABLE.columna, CULTIVABLE.fila - 1);

    elegir(TipoItem::PALA);
    EXPECT_EQ(consultar(CULTIVABLE.columna, CULTIVABLE.fila + 3).marca, MarcaDeCasilla::NINGUNA);

    elegir(TipoItem::ESPADA);
    EXPECT_EQ(consultar(CULTIVABLE.columna, CULTIVABLE.fila).marca, MarcaDeCasilla::NINGUNA);
}
