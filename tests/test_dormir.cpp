#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "../common/modo_uso.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/acciones/accion_dormir.h"
#include "../server/juego/controladores/controlador_de_dia.h"
#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/muebles/muebles.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/sector/gestor_de_sectores.h"

// Estas pruebas leen config/servidor.toml y los mapas de Assets/, así que corren desde la raíz del
// repositorio.

namespace {
const double UN_MINUTO = 1.0;
const int TICKS_POR_SEGUNDO = 20;
const int TICKS_POR_DIA = 1200;
const Casilla CAMA{2, 4};
const Casilla AL_PIE_DE_LA_CAMA{2, 7};
const Casilla OTRA_CAMA{6, 4};
const Casilla AL_PIE_DE_LA_OTRA_CAMA{6, 7};
const uint32_t PRIMERO = 1;
const uint32_t SEGUNDO = 2;

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

class AvisosDeDescanso : public Notificador {
public:
    std::map<uint32_t, EstadoDelDescanso> ultimo_descanso;

    void a_jugador(uint32_t id_jugador, const EventoServidor& evento) override {
        if (const EstadoDelDescanso* descanso = std::get_if<EstadoDelDescanso>(&evento)) {
            ultimo_descanso[id_jugador] = *descanso;
        }
    }
    void a_todos(const EventoServidor&) override {
    }
};

ConfiguracionDiaServidor dia_de_un_minuto() {
    ConfiguracionDiaServidor configuracion;
    configuracion.duracion_en_minutos = UN_MINUTO;
    return configuracion;
}

class Dormir : public ::testing::Test {
protected:
    AvisosDeDescanso notificador;
    std::map<uint32_t, Personaje> personajes;
    GestorDeSectores sectores{recursos(), TICKS_POR_SEGUNDO, personajes, notificador};
    int sacados_de_la_mazmorra = 0;
    ControladorDelDia dia{
        dia_de_un_minuto(),
        TICKS_POR_SEGUNDO,
        sectores,
        personajes,
        recursos(),
        notificador,
        [this](Personaje&) { ++sacados_de_la_mazmorra; }};
    ResolverDeTransicion sin_transiciones = [](Personaje&, const Transicion&) {};

    void SetUp() override {
        ASSERT_NE(
            sectores.en(MAPA_DE_LA_CASA).objetos.colocar(TipoObjeto::CAMA, CAMA.columna, CAMA.fila),
            0);
    }

    Personaje& entrar(uint32_t id_jugador) {
        Personaje& personaje = personajes
                                   .try_emplace(
                                       id_jugador,
                                       id_jugador,
                                       "granja",
                                       recursos().catalogo_de_items,
                                       recursos().juego.inventario.cantidad_ranuras_barra)
                                   .first->second;
        personaje.conectado = true;
        sectores.colocar_en_aparicion(personaje, "granja");
        sectores.trasladar_a_casilla(personaje, MAPA_DE_LA_CASA, 5, 8);
        dia.al_conectarse(personaje);
        return personaje;
    }

    bool acostar(Personaje& personaje, const Casilla& objetivo = CAMA) {
        ContextoDeAccion contexto{
            personaje.id_jugador,
            personaje.ranura(INDICE_RANURA_ACTIVA),
            INDICE_RANURA_ACTIVA,
            objetivo,
            sectores.en(personaje.mapa_actual),
            personaje,
            recursos(),
            notificador,
            ModoUso::SECUNDARIO,
            sin_transiciones};
        AccionDormir accion(personajes);
        return accion.aplica(contexto) && accion.ejecutar(contexto);
    }

    void dar_el_ok(Personaje& personaje) {
        dia.atender(personaje, ComandoCliente{DarElOk{}});
    }

    void avanzar(int ticks) {
        for (int i = 0; i < ticks; ++i) {
            dia.actualizar();
        }
    }

    std::optional<Casilla> casilla_de(const Personaje& personaje) const {
        return sectores.en(personaje.mapa_actual)
            .jugadores.casilla_del_jugador(personaje.id_jugador);
    }
};

}  // namespace

TEST_F(Dormir, ConClicDerechoEnCualquierCasillaDeLaCamaSeAcuesta) {
    Personaje& personaje = entrar(PRIMERO);

    ASSERT_TRUE(acostar(personaje, Casilla{3, 6}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::ACOSTADO);
    EXPECT_FALSE(personaje.dio_el_ok);
    ASSERT_TRUE(personaje.ultima_cama.has_value());
    EXPECT_EQ(personaje.ultima_cama->mapa, MAPA_DE_LA_CASA);
    EXPECT_EQ(personaje.ultima_cama->columna, CAMA.columna);
    EXPECT_EQ(personaje.ultima_cama->fila, CAMA.fila);
}

TEST_F(Dormir, DosJugadoresNoSePuedenAcostarEnLaMismaCama) {
    Personaje& primero = entrar(PRIMERO);
    Personaje& segundo = entrar(SEGUNDO);

    ASSERT_TRUE(acostar(primero));
    EXPECT_FALSE(acostar(segundo, Casilla{3, 5}));
    EXPECT_EQ(segundo.descanso, EstadoDeDescanso::DESPIERTO);
    EXPECT_FALSE(segundo.ultima_cama.has_value());
}

TEST_F(Dormir, CuandoElOtroSeLevantaLaCamaQuedaLibre) {
    Personaje& primero = entrar(PRIMERO);
    Personaje& segundo = entrar(SEGUNDO);
    ASSERT_TRUE(acostar(primero));
    ASSERT_FALSE(dia.atender(primero, ComandoCliente{Moverse{1, 0}}));

    EXPECT_TRUE(acostar(segundo));
}

TEST_F(Dormir, SiElQueEstabaAcostadoSeDesconectaLaCamaQuedaLibre) {
    Personaje& primero = entrar(PRIMERO);
    Personaje& segundo = entrar(SEGUNDO);
    ASSERT_TRUE(acostar(primero));
    dia.al_desconectarse(primero);
    sectores.quitar(primero);
    primero.conectado = false;

    EXPECT_TRUE(acostar(segundo));
}

TEST_F(Dormir, DosDesmayadosConLaMismaUltimaCamaNoAparecenUnoEncimaDelOtro) {
    Personaje& primero = entrar(PRIMERO);
    Personaje& segundo = entrar(SEGUNDO);
    ASSERT_TRUE(acostar(primero));
    ASSERT_FALSE(dia.atender(primero, ComandoCliente{Moverse{1, 0}}));
    ASSERT_TRUE(acostar(segundo));
    ASSERT_FALSE(dia.atender(segundo, ComandoCliente{Moverse{1, 0}}));

    primero.energia = 0;
    segundo.energia = 0;
    avanzar(1);
    dar_el_ok(primero);
    dar_el_ok(segundo);
    avanzar(1);

    ASSERT_EQ(dia.hora().dia, 2u);
    ASSERT_TRUE(casilla_de(primero).has_value());
    ASSERT_TRUE(casilla_de(segundo).has_value());
    EXPECT_FALSE(
        casilla_de(primero)->columna == casilla_de(segundo)->columna &&
        casilla_de(primero)->fila == casilla_de(segundo)->fila);
}

TEST_F(Dormir, FueraDeLaCamaNoSeAcuesta) {
    Personaje& personaje = entrar(PRIMERO);
    EXPECT_FALSE(acostar(personaje, Casilla{5, 5}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::DESPIERTO);
}

TEST_F(Dormir, ElAcostadoVeCuantosDieronElOk) {
    Personaje& primero = entrar(PRIMERO);
    entrar(SEGUNDO);

    ASSERT_TRUE(acostar(primero));
    avanzar(1);
    EstadoDelDescanso esperado{EstadoDeDescanso::ACOSTADO, false, 0, 2};
    EXPECT_EQ(notificador.ultimo_descanso[PRIMERO], esperado);

    dar_el_ok(primero);
    avanzar(1);
    esperado = EstadoDelDescanso{EstadoDeDescanso::ACOSTADO, true, 1, 2};
    EXPECT_EQ(notificador.ultimo_descanso[PRIMERO], esperado);
    EXPECT_EQ(notificador.ultimo_descanso[SEGUNDO].estado, EstadoDeDescanso::DESPIERTO);
}

TEST_F(Dormir, MientrasUnoSigaDespiertoNoPasaElDia) {
    Personaje& primero = entrar(PRIMERO);
    entrar(SEGUNDO);
    ASSERT_TRUE(acostar(primero));
    dar_el_ok(primero);

    avanzar(100);
    EXPECT_EQ(dia.hora().dia, 1u);
    EXPECT_EQ(primero.descanso, EstadoDeDescanso::ACOSTADO);
}

TEST_F(Dormir, AcostadoSinOkNoAlcanzaHayQueDarElOk) {
    Personaje& personaje = entrar(PRIMERO);
    ASSERT_TRUE(acostar(personaje));

    avanzar(100);
    EXPECT_EQ(dia.hora().dia, 1u);
}

TEST_F(Dormir, CuandoTodosDieronElOkSeDespiertanALasSeisDelDiaSiguiente) {
    Personaje& primero = entrar(PRIMERO);
    Personaje& segundo = entrar(SEGUNDO);
    ASSERT_NE(
        sectores.en(MAPA_DE_LA_CASA)
            .objetos.colocar(TipoObjeto::CAMA, OTRA_CAMA.columna, OTRA_CAMA.fila),
        0);
    primero.vida = 10;
    segundo.energia = 5;
    ASSERT_TRUE(acostar(primero));
    ASSERT_TRUE(acostar(segundo, OTRA_CAMA));
    avanzar(30);
    dar_el_ok(primero);
    avanzar(1);
    EXPECT_EQ(dia.hora().dia, 1u);
    dar_el_ok(segundo);
    avanzar(1);

    EXPECT_EQ(dia.hora().dia, 2u);
    EXPECT_EQ(dia.hora().hora, 6);
    EXPECT_EQ(dia.hora().minuto, 0);
    const std::vector<std::pair<const Personaje*, Casilla>> esperados{
        {&primero, AL_PIE_DE_LA_CAMA},
        {&segundo, AL_PIE_DE_LA_OTRA_CAMA}};
    for (const auto& [personaje, al_pie] : esperados) {
        EXPECT_EQ(personaje->descanso, EstadoDeDescanso::DESPIERTO);
        EXPECT_FALSE(personaje->dio_el_ok);
        EXPECT_EQ(personaje->vida, Personaje::VIDA_MAXIMA);
        EXPECT_EQ(personaje->energia, Personaje::ENERGIA_MAXIMA);
        EXPECT_EQ(personaje->mapa_actual, MAPA_DE_LA_CASA);
        ASSERT_TRUE(casilla_de(*personaje).has_value());
        EXPECT_EQ(casilla_de(*personaje)->columna, al_pie.columna);
        EXPECT_EQ(casilla_de(*personaje)->fila, al_pie.fila);
    }
    EXPECT_EQ(sacados_de_la_mazmorra, 2);
}

TEST_F(Dormir, MoverseLevantaAlQueTodaviaNoDioElOk) {
    Personaje& personaje = entrar(PRIMERO);
    ASSERT_TRUE(acostar(personaje));

    EXPECT_TRUE(dia.atender(personaje, ComandoCliente{Moverse{0, 0}}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::ACOSTADO);

    EXPECT_FALSE(dia.atender(personaje, ComandoCliente{Moverse{1, 0}}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::DESPIERTO);
}

TEST_F(Dormir, ConElOkDadoYaNoSePuedeMoverNiUsarItems) {
    Personaje& personaje = entrar(PRIMERO);
    entrar(SEGUNDO);
    ASSERT_TRUE(acostar(personaje));
    dar_el_ok(personaje);

    EXPECT_TRUE(dia.atender(personaje, ComandoCliente{Moverse{1, 0}}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::ACOSTADO);
    EXPECT_TRUE(dia.atender(personaje, ComandoCliente{UsarItem{}}));
}

TEST_F(Dormir, DespiertoElOkNoHaceNada) {
    Personaje& personaje = entrar(PRIMERO);
    dar_el_ok(personaje);
    EXPECT_FALSE(personaje.dio_el_ok);
    EXPECT_FALSE(dia.atender(personaje, ComandoCliente{Moverse{1, 0}}));
}

TEST_F(Dormir, SinEnergiaSeDesmayaYTambienTieneQueDarElOk) {
    Personaje& personaje = entrar(PRIMERO);
    personaje.energia = 0;
    avanzar(1);
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::DESMAYADO);
    EXPECT_EQ(notificador.ultimo_descanso[PRIMERO].estado, EstadoDeDescanso::DESMAYADO);

    EXPECT_TRUE(dia.atender(personaje, ComandoCliente{Moverse{1, 0}}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::DESMAYADO);
    avanzar(50);
    EXPECT_EQ(dia.hora().dia, 1u);

    dar_el_ok(personaje);
    avanzar(1);
    EXPECT_EQ(dia.hora().dia, 2u);
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::DESPIERTO);
    EXPECT_EQ(personaje.vida, Personaje::VIDA_MAXIMA / 2);
    EXPECT_EQ(personaje.energia, Personaje::ENERGIA_MAXIMA / 2);
}

TEST_F(Dormir, SinVidaSeDesmaya) {
    Personaje& personaje = entrar(PRIMERO);
    personaje.vida = 0;
    avanzar(1);
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::DESMAYADO);
}

TEST_F(Dormir, ElQueNuncaDurmioSeDespiertaEnLaEntradaDeLaCasa) {
    Personaje& personaje = entrar(PRIMERO);
    sectores.trasladar_a_casilla(personaje, "granja", 20, 20);
    personaje.energia = 0;
    avanzar(1);
    dar_el_ok(personaje);
    avanzar(1);

    EXPECT_EQ(personaje.mapa_actual, MAPA_DE_LA_CASA);
    ASSERT_TRUE(casilla_de(personaje).has_value());
    EXPECT_EQ(casilla_de(personaje)->columna, 3);
    EXPECT_EQ(casilla_de(personaje)->fila, 9);
}

TEST_F(Dormir, ElDesmayadoSeDespiertaEnSuUltimaCama) {
    Personaje& personaje = entrar(PRIMERO);
    ASSERT_TRUE(acostar(personaje));
    dar_el_ok(personaje);
    avanzar(1);
    ASSERT_EQ(dia.hora().dia, 2u);

    sectores.trasladar_a_casilla(personaje, "granja", 20, 20);
    personaje.vida = 0;
    avanzar(1);
    dar_el_ok(personaje);
    avanzar(1);

    EXPECT_EQ(dia.hora().dia, 3u);
    EXPECT_EQ(personaje.mapa_actual, MAPA_DE_LA_CASA);
    EXPECT_EQ(casilla_de(personaje)->columna, AL_PIE_DE_LA_CAMA.columna);
    EXPECT_EQ(casilla_de(personaje)->fila, AL_PIE_DE_LA_CAMA.fila);
    EXPECT_EQ(personaje.vida, Personaje::VIDA_MAXIMA / 2);
}

TEST_F(Dormir, SiLevantaronLaCamaSeDespiertaEnLaEntrada) {
    Personaje& personaje = entrar(PRIMERO);
    ASSERT_TRUE(acostar(personaje));
    Sector& casa = sectores.en(MAPA_DE_LA_CASA);
    ASSERT_TRUE(casa.objetos.retirar(casa.objetos.objeto_en(CAMA.columna, CAMA.fila)->id));
    dar_el_ok(personaje);
    avanzar(1);

    EXPECT_FALSE(personaje.ultima_cama.has_value());
    EXPECT_EQ(casilla_de(personaje)->columna, 3);
    EXPECT_EQ(casilla_de(personaje)->fila, 9);
}

TEST_F(Dormir, ALasDosLosDespiertosSeDesmayanYTodosTienenQueDarElOk) {
    Personaje& despierto = entrar(PRIMERO);
    Personaje& acostado = entrar(SEGUNDO);
    ASSERT_TRUE(acostar(acostado));

    avanzar(TICKS_POR_DIA - 1);
    EXPECT_EQ(despierto.descanso, EstadoDeDescanso::DESPIERTO);
    avanzar(1);

    EXPECT_EQ(despierto.descanso, EstadoDeDescanso::DESMAYADO);
    EXPECT_EQ(acostado.descanso, EstadoDeDescanso::ACOSTADO);
    EXPECT_EQ(notificador.ultimo_descanso[PRIMERO].estado, EstadoDeDescanso::DESMAYADO);
    EXPECT_EQ(dia.hora().hora, 2);
    avanzar(500);
    EXPECT_EQ(dia.hora().dia, 1u);
    EXPECT_EQ(dia.hora().hora, 2);
    EXPECT_EQ(dia.hora().minuto, 0);

    dar_el_ok(acostado);
    avanzar(1);
    EXPECT_EQ(dia.hora().dia, 1u);
    dar_el_ok(despierto);
    avanzar(1);

    EXPECT_EQ(dia.hora().dia, 2u);
    EXPECT_EQ(dia.hora().hora, 6);
    EXPECT_EQ(despierto.energia, Personaje::ENERGIA_MAXIMA / 2);
    EXPECT_EQ(acostado.energia, Personaje::ENERGIA_MAXIMA);
    EXPECT_EQ(despierto.descanso, EstadoDeDescanso::DESPIERTO);
    EXPECT_EQ(acostado.descanso, EstadoDeDescanso::DESPIERTO);
}

TEST_F(Dormir, DespuesDeLasDosElAcostadoYaNoSePuedeLevantar) {
    Personaje& personaje = entrar(PRIMERO);
    ASSERT_TRUE(acostar(personaje));
    avanzar(TICKS_POR_DIA);

    EXPECT_TRUE(dia.atender(personaje, ComandoCliente{Moverse{1, 0}}));
    EXPECT_EQ(personaje.descanso, EstadoDeDescanso::ACOSTADO);
}

TEST_F(Dormir, ElQueEntraDespuesDeLasDosApareceDesmayado) {
    Personaje& primero = entrar(PRIMERO);
    avanzar(TICKS_POR_DIA);
    ASSERT_EQ(primero.descanso, EstadoDeDescanso::DESMAYADO);

    Personaje& segundo = entrar(SEGUNDO);
    avanzar(1);
    EXPECT_EQ(segundo.descanso, EstadoDeDescanso::DESMAYADO);
    EXPECT_EQ(notificador.ultimo_descanso[SEGUNDO].estado, EstadoDeDescanso::DESMAYADO);
}

TEST_F(Dormir, ElQueSeDesconectaNoCuenta) {
    Personaje& primero = entrar(PRIMERO);
    Personaje& segundo = entrar(SEGUNDO);
    ASSERT_TRUE(acostar(primero));
    dar_el_ok(primero);
    avanzar(1);
    EXPECT_EQ(dia.hora().dia, 1u);

    dia.al_desconectarse(segundo);
    sectores.quitar(segundo);
    segundo.conectado = false;
    avanzar(1);
    EXPECT_EQ(dia.hora().dia, 2u);
}

TEST_F(Dormir, SinNadieConectadoElDiaNoSeSaltea) {
    avanzar(10);
    EXPECT_EQ(dia.hora().dia, 1u);
}
