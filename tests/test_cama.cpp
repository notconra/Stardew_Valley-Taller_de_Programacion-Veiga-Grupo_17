#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "../common/modo_uso.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/acciones/accion_colocar.h"
#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/muebles/muebles.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/sector/sector.h"

// Estas pruebas leen config/servidor.toml y los mapas de Assets/, así que corren desde la raíz del
// repositorio (el target test del Makefile ejecuta ./build/tests desde ahí).

namespace {
const Casilla CONTRA_LA_PARED{2, 4};
const uint32_t ID_JUGADOR = 1;

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

class SinAvisos : public Notificador {
public:
    void a_jugador(uint32_t, const EventoServidor&) override {
    }
    void a_todos(const EventoServidor&) override {
    }
};

std::optional<uint8_t> ranura_de_la_cama(const Personaje& personaje) {
    for (uint8_t i = 0; i < recursos().juego.inventario.cantidad_ranuras_barra; ++i) {
        if (personaje.ranura(i).tipo == TipoItem::CAMA) {
            return i;
        }
    }
    return std::nullopt;
}

class ColocarCama : public ::testing::Test {
protected:
    SinAvisos notificador;
    Personaje personaje{
        ID_JUGADOR,
        "granja",
        recursos().catalogo_de_items,
        recursos().juego.inventario.cantidad_ranuras_barra};
    ResolverDeTransicion sin_transiciones = [](Personaje&, const Transicion&) {};
    AccionColocar accion;

    bool colocar(Sector& sector, const Casilla& objetivo) {
        const std::optional<uint8_t> ranura = ranura_de_la_cama(personaje);
        if (!ranura.has_value()) {
            return false;
        }
        ContextoDeAccion contexto{
            ID_JUGADOR,
            personaje.ranura(*ranura),
            *ranura,
            objetivo,
            sector,
            personaje,
            recursos(),
            notificador,
            ModoUso::SECUNDARIO,
            sin_transiciones};
        return accion.aplica(contexto) && accion.ejecutar(contexto);
    }
};

}  // namespace

TEST(Cama, CadaPersonajeEmpiezaConUnaCama) {
    const Personaje personaje(
        ID_JUGADOR,
        "granja",
        recursos().catalogo_de_items,
        recursos().juego.inventario.cantidad_ranuras_barra);
    ASSERT_TRUE(ranura_de_la_cama(personaje).has_value());
    EXPECT_EQ(personaje.ranura(*ranura_de_la_cama(personaje)).cantidad, 1);
}

TEST(Cama, NoSePuedeTirarAlTacho) {
    EXPECT_FALSE(recursos().catalogo_de_items.se_puede_eliminar(TipoItem::CAMA));
}

TEST_F(ColocarCama, SePoneContraLaParedYBloqueaDosPorTres) {
    Sector casa(MAPA_DE_LA_CASA, recursos(), 40);

    ASSERT_TRUE(colocar(casa, CONTRA_LA_PARED));
    EXPECT_FALSE(ranura_de_la_cama(personaje).has_value());

    const ObjetoDelMundo* cama =
        casa.objetos.objeto_en(CONTRA_LA_PARED.columna, CONTRA_LA_PARED.fila);
    ASSERT_NE(cama, nullptr);
    EXPECT_EQ(cama->tipo, TipoObjeto::CAMA);
    for (int columna = 2; columna <= 3; ++columna) {
        for (int fila = 4; fila <= 6; ++fila) {
            EXPECT_EQ(casa.objetos.objeto_en(columna, fila), cama) << columna << ", " << fila;
            EXPECT_FALSE(casa.colisiones.esta_libre(columna, fila)) << columna << ", " << fila;
        }
    }
    EXPECT_TRUE(casa.colisiones.esta_libre(4, 4));
    EXPECT_TRUE(casa.colisiones.esta_libre(2, 7));
}

TEST_F(ColocarCama, EnElMedioSePuedePasarPorDetrasDeLaCabecera) {
    Sector casa(MAPA_DE_LA_CASA, recursos(), 40);

    ASSERT_TRUE(colocar(casa, Casilla{5, 6}));
    EXPECT_TRUE(casa.colisiones.esta_libre(5, 5));
    EXPECT_TRUE(casa.colisiones.esta_libre(6, 5));
    EXPECT_FALSE(casa.colisiones.esta_libre(5, 6));
    EXPECT_FALSE(casa.colisiones.esta_libre(6, 8));
}

TEST_F(ColocarCama, NoSePuedePonerFueraDeLaCasa) {
    Sector granja("granja", recursos(), 40);

    EXPECT_FALSE(colocar(granja, Casilla{13, 12}));
    EXPECT_TRUE(ranura_de_la_cama(personaje).has_value());
    EXPECT_EQ(granja.objetos.cantidad_de_objetos(), 0);
}

TEST_F(ColocarCama, NoSePuedePonerSobreLaParedNiFueraDelPiso) {
    Sector casa(MAPA_DE_LA_CASA, recursos(), 40);

    EXPECT_FALSE(colocar(casa, Casilla{2, 3}));
    EXPECT_FALSE(colocar(casa, Casilla{10, 4}));
    EXPECT_FALSE(colocar(casa, Casilla{2, 9}));
    EXPECT_TRUE(ranura_de_la_cama(personaje).has_value());
    EXPECT_EQ(casa.objetos.cantidad_de_objetos(), 0);
}

TEST_F(ColocarCama, NoSePuedePonerEncimaDeOtraCama) {
    Sector casa(MAPA_DE_LA_CASA, recursos(), 40);
    ASSERT_TRUE(colocar(casa, CONTRA_LA_PARED));

    personaje.inventario.agregar_item(TipoItem::CAMA, 1);
    EXPECT_FALSE(colocar(casa, Casilla{3, 5}));
    EXPECT_TRUE(colocar(casa, Casilla{4, 4}));
    EXPECT_EQ(casa.objetos.cantidad_de_objetos(), 2);
}

TEST_F(ColocarCama, AlLevantarlaVuelveElItemYSeLiberanLasCasillas) {
    Sector casa(MAPA_DE_LA_CASA, recursos(), 40);
    ASSERT_TRUE(colocar(casa, CONTRA_LA_PARED));
    const int id = casa.objetos.objeto_en(CONTRA_LA_PARED.columna, CONTRA_LA_PARED.fila)->id;

    const ResultadoDeRotura rotura = casa.objetos.romper(id);
    EXPECT_TRUE(rotura.se_rompio);
    EXPECT_EQ(rotura.producto, TipoItem::CAMA);
    EXPECT_EQ(rotura.cantidad, 1);
    for (int columna = 2; columna <= 3; ++columna) {
        for (int fila = 4; fila <= 6; ++fila) {
            EXPECT_TRUE(casa.colisiones.esta_libre(columna, fila)) << columna << ", " << fila;
        }
    }
}

TEST_F(ColocarCama, NoSePuedePonerPegadaALaPuerta) {
    Sector casa(MAPA_DE_LA_CASA, recursos(), 40);
    EXPECT_FALSE(colocar(casa, Casilla{3, 8}));
    EXPECT_FALSE(colocar(casa, Casilla{5, 8}));
    EXPECT_TRUE(colocar(casa, Casilla{7, 8}));
}
