#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

#include "../common/modo_uso.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/acciones/accion_colocar.h"
#include "../server/juego/acciones/accion_cosechar.h"
#include "../server/juego/acciones/accion_plantar.h"
#include "../server/juego/acciones/accion_regar.h"
#include "../server/juego/controladores/controlador_de_cultivos.h"
#include "../server/juego/cultivos/cultivos.h"
#include "../server/juego/cultivos/huerta.h"
#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/mundo.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/sector/gestor_de_sectores.h"
#include "../server/persistencia/serializador_partida.h"

// Las pruebas de la Huerta no dependen de nada externo. Las de las acciones y el controlador leen
// config/servidor.toml y los mapas de Assets/, así que corren desde la raíz del repositorio.

namespace {

// Con estos tiempos, la cebolla de verdeo (3 riegos por día) tiene turnos que empiezan a los 0,
// 400 y 800 minutos del día (6 am, 12:40 y 19:20), y la mitad del primero es el minuto 200.
const int CEBOLLA_TURNO_2 = 400;
const int CEBOLLA_TURNO_3 = 800;
const int MITAD_DEL_PRIMER_TURNO_DE_LA_CEBOLLA = 200;
const Casilla AQUI{3, 4};

ConfiguracionCultivosServidor sin_suerte() {
    ConfiguracionCultivosServidor configuracion;
    configuracion.probabilidad_de_subir_calidad = 0.0;
    configuracion.probabilidad_de_bajar_calidad = 0.0;
    return configuracion;
}

class Cultivos : public ::testing::Test {
protected:
    CatalogoDeCultivos catalogo;
    ConfiguracionCultivosServidor configuracion = sin_suerte();
    Huerta huerta{catalogo, configuracion};

    // Riega en cada turno de la cebolla apenas empieza y termina el día.
    void un_dia_bien_regado_de_cebolla() {
        for (int minuto : {0, CEBOLLA_TURNO_2, CEBOLLA_TURNO_3}) {
            huerta.avanzar_hasta(minuto);
            ASSERT_TRUE(huerta.regar(AQUI)) << minuto;
        }
        huerta.terminar_el_dia();
        huerta.avanzar_hasta(0);
    }

    // Riega todos los turnos del día del cultivo de AQUI (para los de 1 o 2 riegos).
    void un_dia_bien_regado(int riegos_por_dia) {
        for (int turno = 0; turno < riegos_por_dia; ++turno) {
            huerta.avanzar_hasta(turno * 1200 / riegos_por_dia);
            ASSERT_TRUE(huerta.regar(AQUI)) << turno;
        }
        huerta.terminar_el_dia();
        huerta.avanzar_hasta(0);
    }
};

}  // namespace

TEST_F(Cultivos, RecienPlantadaPideAguaYAlRegarlaDejaDePedir) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    EXPECT_TRUE(huerta.pide_agua(AQUI));

    EXPECT_TRUE(huerta.regar(AQUI));
    EXPECT_FALSE(huerta.pide_agua(AQUI));
    EXPECT_FALSE(huerta.regar(AQUI));
}

TEST_F(Cultivos, NoSePlantaDosVecesEnLaMismaCasilla) {
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::PAPA));
    EXPECT_FALSE(huerta.plantar(AQUI, TipoCultivo::FRUTILLA));
}

TEST_F(Cultivos, LaCebollaPideAguaEnCadaUnoDeSusTresTurnos) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    ASSERT_TRUE(huerta.regar(AQUI));

    huerta.avanzar_hasta(CEBOLLA_TURNO_2 - 10);
    EXPECT_FALSE(huerta.pide_agua(AQUI));
    huerta.avanzar_hasta(CEBOLLA_TURNO_2);
    EXPECT_TRUE(huerta.pide_agua(AQUI));
    ASSERT_TRUE(huerta.regar(AQUI));
    huerta.avanzar_hasta(CEBOLLA_TURNO_3);
    EXPECT_TRUE(huerta.pide_agua(AQUI));
}

TEST_F(Cultivos, BienRegadaMaduraEnLosDiasDelEnunciado) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));

    un_dia_bien_regado_de_cebolla();
    EXPECT_FALSE(huerta.esta_madura(AQUI));
    un_dia_bien_regado_de_cebolla();
    EXPECT_TRUE(huerta.esta_madura(AQUI));
    EXPECT_FALSE(huerta.pide_agua(AQUI));
}

TEST_F(Cultivos, SiLeFaltaUnRiegoEseDiaNoCrece) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    huerta.regar(AQUI);
    huerta.avanzar_hasta(CEBOLLA_TURNO_2);
    huerta.avanzar_hasta(CEBOLLA_TURNO_3);
    huerta.regar(AQUI);
    huerta.terminar_el_dia();
    huerta.avanzar_hasta(0);

    EXPECT_EQ(huerta.cultivo_en(AQUI)->dias_de_crecimiento, 0);
}

TEST_F(Cultivos, LaPapaToleraSaltearUnRiegoPorDia) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::PAPA));
    huerta.regar(AQUI);
    huerta.terminar_el_dia();
    huerta.avanzar_hasta(0);
    EXPECT_EQ(huerta.cultivo_en(AQUI)->dias_de_crecimiento, 1);

    huerta.avanzar_hasta(600);
    huerta.terminar_el_dia();
    huerta.avanzar_hasta(0);
    EXPECT_EQ(huerta.cultivo_en(AQUI)->dias_de_crecimiento, 1);
}

TEST_F(Cultivos, SiTodosSeVanADormirTempranoLosTurnosQueNoEmpezaronNoCuentan) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    huerta.regar(AQUI);
    huerta.avanzar_hasta(CEBOLLA_TURNO_2);
    huerta.regar(AQUI);
    huerta.avanzar_hasta(CEBOLLA_TURNO_3 - 10);
    huerta.terminar_el_dia();

    EXPECT_EQ(huerta.cultivo_en(AQUI)->dias_de_crecimiento, 1);
    EXPECT_EQ(huerta.cultivo_en(AQUI)->faltas, 0);
}

TEST_F(Cultivos, SinFaltasNiSuerteSeCosechaPerfectaYSeSaca) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    un_dia_bien_regado_de_cebolla();
    un_dia_bien_regado_de_cebolla();

    const std::optional<Cosecha> cosecha = huerta.cosechar(AQUI);
    ASSERT_TRUE(cosecha.has_value());
    EXPECT_EQ(cosecha->producto, TipoItem::CEBOLLA_DE_VERDEO);
    EXPECT_EQ(cosecha->calidad, Calidad::PERFECTA);
    EXPECT_FALSE(huerta.hay_cultivo(AQUI));
}

TEST_F(Cultivos, NoSeCosechaLoQueNoEstaMaduro) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    EXPECT_FALSE(huerta.cosechar(AQUI).has_value());
    EXPECT_TRUE(huerta.hay_cultivo(AQUI));
}

TEST_F(Cultivos, UnRiegoTardioBajaUnNivel) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    huerta.avanzar_hasta(MITAD_DEL_PRIMER_TURNO_DE_LA_CEBOLLA + 10);
    ASSERT_TRUE(huerta.regar(AQUI));  // en la segunda mitad del turno
    huerta.avanzar_hasta(CEBOLLA_TURNO_2);
    huerta.regar(AQUI);
    huerta.avanzar_hasta(CEBOLLA_TURNO_3);
    huerta.regar(AQUI);
    huerta.terminar_el_dia();
    huerta.avanzar_hasta(0);
    un_dia_bien_regado_de_cebolla();

    EXPECT_EQ(huerta.cosechar(AQUI)->calidad, Calidad::INCREIBLE);
}

TEST_F(Cultivos, CadaDiaSinCosecharBajaUnNivel) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    un_dia_bien_regado_de_cebolla();
    un_dia_bien_regado_de_cebolla();
    huerta.terminar_el_dia();
    huerta.terminar_el_dia();

    EXPECT_EQ(huerta.cosechar(AQUI)->calidad, Calidad::EXCELENTE);
}

TEST_F(Cultivos, LaCalidadNoBajaDeBase) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CEBOLLA_DE_VERDEO));
    un_dia_bien_regado_de_cebolla();
    un_dia_bien_regado_de_cebolla();
    for (int dia = 0; dia < 10; ++dia) {
        huerta.terminar_el_dia();
    }
    EXPECT_EQ(huerta.cosechar(AQUI)->calidad, Calidad::BASE);
}

TEST_F(Cultivos, LaSuerteSubeOBajaUnNivel) {
    ConfiguracionCultivosServidor siempre_sube = sin_suerte();
    siempre_sube.probabilidad_de_subir_calidad = 1.0;
    Huerta con_suerte(catalogo, siempre_sube);
    con_suerte.avanzar_hasta(0);
    ASSERT_TRUE(con_suerte.plantar(AQUI, TipoCultivo::CALABAZA));
    for (int dia = 0; dia < 7; ++dia) {
        con_suerte.regar(AQUI);
        con_suerte.terminar_el_dia();
        con_suerte.avanzar_hasta(0);
    }
    con_suerte.terminar_el_dia();
    EXPECT_EQ(con_suerte.cosechar(AQUI)->calidad, Calidad::PERFECTA);
}

TEST_F(Cultivos, LaFrutillaVuelveADarCadaDosDias) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::FRUTILLA));
    for (int dia = 0; dia < 4; ++dia) {
        un_dia_bien_regado(2);
    }
    ASSERT_TRUE(huerta.esta_madura(AQUI));
    ASSERT_TRUE(huerta.cosechar(AQUI).has_value());

    EXPECT_TRUE(huerta.hay_cultivo(AQUI));
    EXPECT_FALSE(huerta.esta_madura(AQUI));
    EXPECT_TRUE(huerta.pide_agua(AQUI));
    un_dia_bien_regado(2);
    EXPECT_FALSE(huerta.esta_madura(AQUI));
    un_dia_bien_regado(2);
    EXPECT_TRUE(huerta.esta_madura(AQUI));
}

TEST_F(Cultivos, LaCasillaParaElClienteLlevaElCultivoYSuEstado) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::CALABAZA));
    CasillaTratada casilla{static_cast<uint32_t>(AQUI.columna), static_cast<uint32_t>(AQUI.fila)};
    huerta.completar(casilla);
    EXPECT_EQ(casilla.cultivo, TipoCultivo::CALABAZA);
    EXPECT_EQ(casilla.etapa, 0);
    EXPECT_TRUE(casilla.pide_agua);
    EXPECT_FALSE(casilla.regada);

    huerta.regar(AQUI);
    huerta.completar(casilla);
    EXPECT_TRUE(casilla.regada);
    EXPECT_FALSE(casilla.pide_agua);

    for (int dia = 0; dia < 7; ++dia) {
        huerta.terminar_el_dia();
        huerta.avanzar_hasta(0);
        huerta.regar(AQUI);
    }
    huerta.completar(casilla);
    EXPECT_TRUE(casilla.madura);
    EXPECT_EQ(casilla.etapa, ETAPA_DE_CULTIVO_MAXIMA);
}

TEST_F(Cultivos, SeGuardaYSeVuelveACargar) {
    huerta.avanzar_hasta(0);
    ASSERT_TRUE(huerta.plantar(AQUI, TipoCultivo::PAPA));
    un_dia_bien_regado(2);
    std::vector<DatosCultivoGuardado> guardados;
    huerta.agregar_guardados_a("granja", guardados);
    ASSERT_EQ(guardados.size(), 1u);
    EXPECT_EQ(guardados[0].tipo, TipoCultivo::PAPA);
    EXPECT_EQ(guardados[0].dias_de_crecimiento, 1);

    Huerta otra(catalogo, configuracion);
    otra.cargar(guardados[0]);
    otra.avanzar_hasta(0);
    EXPECT_EQ(otra.cultivo_en(AQUI)->dias_de_crecimiento, 1);
    EXPECT_TRUE(otra.pide_agua(AQUI));
}

TEST(GuardadoDeCultivos, ElArchivoConservaLosCultivos) {
    DatosPartidaGuardada partida;
    partida.codigo = "P1";
    DatosCultivoGuardado cultivo;
    cultivo.id_mapa = "granja";
    cultivo.columna = 3;
    cultivo.fila = 4;
    cultivo.tipo = TipoCultivo::FRUTILLA;
    cultivo.dias_de_crecimiento = 2;
    cultivo.madura = true;
    cultivo.faltas = 3;
    partida.mundo.cultivos.push_back(cultivo);

    std::stringstream archivo;
    SerializadorPartida::serializar(archivo, partida);
    EXPECT_EQ(SerializadorPartida::deserializar(archivo), partida);
}

// Las acciones y el controlador, con el mapa de la granja.
namespace {

const uint32_t JUGADOR = 1;

const RecursosDeJuego& recursos() {
    static const RecursosDeJuego recursos_de_juego = [] {
        const ConfiguracionServidor configuracion("config/servidor.toml");
        std::vector<ConfiguracionMapaServidor> mapas;
        for (const ConfiguracionMapaServidor& mapa : configuracion.mapas()) {
            if (mapa.id == "granja") {
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
    std::vector<TierraCambiada> tierra;

    void a_jugador(uint32_t, const EventoServidor& evento) override {
        if (const TierraCambiada* cambio = std::get_if<TierraCambiada>(&evento)) {
            tierra.push_back(*cambio);
        }
    }
    void a_todos(const EventoServidor&) override {
    }
};

// La primera casilla cultivable de la granja que se pueda tratar.
Casilla una_casilla_cultivable(const Sector& granja) {
    for (int fila = 0; fila < 200; ++fila) {
        for (int columna = 0; columna < 200; ++columna) {
            if (granja.tierra.se_puede_tratar(Casilla{columna, fila})) {
                return Casilla{columna, fila};
            }
        }
    }
    return Casilla{-1, -1};
}

class AccionesDeCultivos : public ::testing::Test {
protected:
    SinAvisos notificador;
    Sector granja{"granja", recursos(), 40};
    Personaje personaje{
        JUGADOR,
        "granja",
        recursos().catalogo_de_items,
        recursos().juego.inventario.cantidad_ranuras_barra};
    ResolverDeTransicion sin_transiciones = [](Personaje&, const Transicion&) {};
    Casilla casilla = una_casilla_cultivable(granja);

    void SetUp() override {
        ASSERT_GE(casilla.columna, 0);
        personaje.inventario.agregar_item(TipoItem::SEMILLA_PAPA, 5);
        granja.jugadores.agregar_en(JUGADOR, casilla.columna * 16.0, (casilla.fila - 1) * 16.0);
    }

    ContextoDeAccion contexto(RanuraInventario item, uint8_t ranura, ModoUso modo) {
        return ContextoDeAccion{
            JUGADOR,
            item,
            ranura,
            casilla,
            granja,
            personaje,
            recursos(),
            notificador,
            modo,
            sin_transiciones};
    }

    std::optional<uint8_t> ranura_con(TipoItem tipo) const {
        for (uint8_t i = 0; i < recursos().juego.inventario.cantidad_ranuras_barra; ++i) {
            if (personaje.ranura(i).tipo == tipo) {
                return i;
            }
        }
        return std::nullopt;
    }

    int cantidad_de(TipoItem tipo) const {
        int total = 0;
        for (uint8_t i = 0; i < recursos().juego.inventario.cantidad_ranuras_barra; ++i) {
            if (personaje.ranura(i).tipo == tipo) {
                total += personaje.ranura(i).cantidad;
            }
        }
        return total;
    }

    bool plantar_papa() {
        const std::optional<uint8_t> ranura = ranura_con(TipoItem::SEMILLA_PAPA);
        if (!ranura.has_value()) {
            return false;
        }
        ContextoDeAccion c = contexto(personaje.ranura(*ranura), *ranura, ModoUso::SECUNDARIO);
        AccionPlantar plantar;
        return plantar.aplica(c) && plantar.ejecutar(c);
    }
};

}  // namespace

TEST_F(AccionesDeCultivos, SinTierraTratadaNoSePlanta) {
    EXPECT_FALSE(plantar_papa());
}

TEST_F(AccionesDeCultivos, PlantarGastaUnaSemillaYAvisaLaCasilla) {
    ASSERT_TRUE(granja.tratar_tierra(notificador, casilla));
    const int antes = cantidad_de(TipoItem::SEMILLA_PAPA);
    ASSERT_GT(antes, 0);

    ASSERT_TRUE(plantar_papa());
    EXPECT_EQ(cantidad_de(TipoItem::SEMILLA_PAPA), antes - 1);
    ASSERT_FALSE(notificador.tierra.empty());
    EXPECT_EQ(notificador.tierra.back().casilla.cultivo, TipoCultivo::PAPA);
    EXPECT_TRUE(notificador.tierra.back().casilla.pide_agua);
    EXPECT_FALSE(plantar_papa());  // ya está ocupada
}

TEST_F(AccionesDeCultivos, LaRegaderaRiegaYSoloAlQuePideAgua) {
    ASSERT_TRUE(granja.tratar_tierra(notificador, casilla));
    ASSERT_TRUE(plantar_papa());
    const std::optional<uint8_t> regadera = ranura_con(TipoItem::REGADERA);
    ASSERT_TRUE(regadera.has_value());

    ContextoDeAccion c = contexto(personaje.ranura(*regadera), *regadera, ModoUso::PRINCIPAL);
    AccionRegar regar;
    ASSERT_TRUE(regar.aplica(c));
    EXPECT_TRUE(regar.ejecutar(c));
    EXPECT_TRUE(notificador.tierra.back().casilla.regada);
    EXPECT_FALSE(regar.ejecutar(c));
}

TEST_F(AccionesDeCultivos, AlCosecharElProductoVaAlInventario) {
    ASSERT_TRUE(granja.tratar_tierra(notificador, casilla));
    ASSERT_TRUE(plantar_papa());
    for (int dia = 0; dia < 3; ++dia) {
        granja.avanzar_cultivos(notificador, 0);
        granja.huerta.regar(casilla);
        granja.avanzar_cultivos(notificador, 600);
        granja.huerta.regar(casilla);
        granja.terminar_el_dia_de_los_cultivos(notificador);
    }
    ASSERT_TRUE(granja.huerta.esta_madura(casilla));

    ContextoDeAccion c = contexto(personaje.ranura(0), 0, ModoUso::SECUNDARIO);
    AccionCosechar cosechar;
    ASSERT_TRUE(cosechar.aplica(c));
    ASSERT_TRUE(cosechar.ejecutar(c));
    EXPECT_EQ(cantidad_de(TipoItem::PAPA), 1);
    EXPECT_FALSE(granja.huerta.hay_cultivo(casilla));
    EXPECT_TRUE(granja.tierra.esta_tratada(casilla));  // la tierra queda tratada
}

TEST_F(AccionesDeCultivos, ConElPicoSeVuelveATierraComunYSePierdeLaPlanta) {
    ASSERT_TRUE(granja.tratar_tierra(notificador, casilla));
    ASSERT_TRUE(plantar_papa());
    ASSERT_TRUE(granja.volver_tierra_a_comun(notificador, casilla));
    EXPECT_FALSE(granja.huerta.hay_cultivo(casilla));
}

TEST_F(AccionesDeCultivos, NoSeColocaUnCofreEncimaDeUnaPlanta) {
    ASSERT_TRUE(granja.tratar_tierra(notificador, casilla));
    ASSERT_TRUE(plantar_papa());
    personaje.inventario.agregar_item(TipoItem::COFRE, 1);
    const std::optional<uint8_t> cofre = ranura_con(TipoItem::COFRE);
    ASSERT_TRUE(cofre.has_value());
    granja.jugadores.quitar(JUGADOR);

    ContextoDeAccion c = contexto(personaje.ranura(*cofre), *cofre, ModoUso::SECUNDARIO);
    EXPECT_FALSE(AccionColocar().se_puede_colocar(c));
}

TEST(ControladorDeCultivos, AlEmpezarUnDiaNuevoCrecenLasPlantasRegadas) {
    SinAvisos notificador;
    std::map<uint32_t, Personaje> personajes;
    GestorDeSectores sectores(recursos(), 40, personajes, notificador);
    ControladorDeCultivos controlador(sectores, notificador);
    Sector& granja = sectores.en("granja");
    const Casilla casilla = una_casilla_cultivable(granja);
    ASSERT_TRUE(granja.tratar_tierra(notificador, casilla));

    HoraDelJuego hora;
    controlador.actualizar(hora);
    ASSERT_TRUE(granja.plantar(notificador, casilla, TipoCultivo::CALABAZA));
    ASSERT_TRUE(granja.regar(notificador, casilla));

    hora.hora = 22;
    controlador.actualizar(hora);
    EXPECT_EQ(granja.huerta.cultivo_en(casilla)->dias_de_crecimiento, 0);

    hora.dia = 2;
    hora.hora = 6;
    controlador.actualizar(hora);
    EXPECT_EQ(granja.huerta.cultivo_en(casilla)->dias_de_crecimiento, 1);
    EXPECT_TRUE(granja.huerta.pide_agua(casilla));  // el día nuevo vuelve a pedir agua
}

TEST(CheatDeSemillas, ElMundoLeDaLasSemillasAlJugador) {
    SinAvisos notificador;
    Mundo mundo(recursos(), notificador, 40);
    mundo.conectar(JUGADOR, AparienciaPersonaje{});
    EXPECT_EQ(mundo.dar_items(JUGADOR, TipoItem::SEMILLA_FRUTILLA, 10), 10);
    EXPECT_EQ(mundo.dar_items(99, TipoItem::SEMILLA_FRUTILLA, 10), 0);  // no está conectado
}

TEST(GuardadoDeCultivos, AlRetomarLaPartidaVuelvenLasPlantas) {
    SinAvisos notificador;
    std::map<uint32_t, Personaje> personajes;
    GestorDeSectores sectores(recursos(), 40, personajes, notificador);
    const Casilla casilla = una_casilla_cultivable(sectores.en("granja"));

    DatosMundoGuardado guardado;
    DatosMapaGuardado granja;
    granja.id_mapa = "granja";
    granja.casillas_modificadas.push_back(ModificacionCasilla{
        static_cast<uint32_t>(casilla.columna),
        static_cast<uint32_t>(casilla.fila),
        static_cast<uint8_t>(TipoModificacionCasilla::TIERRA_TRATADA)});
    guardado.mapas.push_back(granja);
    DatosCultivoGuardado cultivo;
    cultivo.id_mapa = "granja";
    cultivo.columna = static_cast<uint32_t>(casilla.columna);
    cultivo.fila = static_cast<uint32_t>(casilla.fila);
    cultivo.tipo = TipoCultivo::CALABAZA;
    cultivo.dias_de_crecimiento = 3;
    guardado.cultivos.push_back(cultivo);

    Mundo mundo(recursos(), notificador, 40, guardado);
    const std::vector<DatosCultivoGuardado> cultivos = mundo.obtener_cultivos_guardados();
    ASSERT_EQ(cultivos.size(), 1u);
    EXPECT_EQ(cultivos[0], cultivo);
}
