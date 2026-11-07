#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <stdexcept>
#include <variant>
#include <vector>

#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/controladores/controlador_de_dia.h"
#include "../server/juego/mundo.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/sector/gestor_de_sectores.h"
#include "../server/juego/tiempo/reloj.h"

namespace {

// Con un día de 1 minuto real y 20 ticks por segundo, el día son 1200 ticks: cada tick es justo
// un minuto de juego, y cada aviso (10 minutos de juego) son 10 ticks.
const double UN_MINUTO = 1.0;
const int TICKS_POR_SEGUNDO = 20;
const int TICKS_POR_DIA = 1200;
const int DIAS_POR_ESTACION = 28;

HoraDelJuego hora(uint32_t dia, uint8_t hora_del_dia, uint8_t minuto) {
    HoraDelJuego hora_del_juego;
    hora_del_juego.dia = dia;
    hora_del_juego.hora = hora_del_dia;
    hora_del_juego.minuto = minuto;
    hora_del_juego.dia_de_la_estacion = static_cast<uint8_t>(dia);
    return hora_del_juego;
}

void ir_al_dia(Reloj& reloj, uint32_t dia) {
    while (reloj.hora().dia < dia) {
        if (reloj.avanzar() == CambioDelReloj::FIN_DEL_DIA) {
            reloj.empezar_dia_siguiente();
        }
    }
}
struct Avisos {
    int horas = 0;
    int dias = 0;
};

Avisos avanzar(Reloj& reloj, int ticks) {
    Avisos avisos;
    for (int i = 0; i < ticks; ++i) {
        const CambioDelReloj cambio = reloj.avanzar();
        if (cambio == CambioDelReloj::HORA) {
            ++avisos.horas;
        } else if (cambio == CambioDelReloj::FIN_DEL_DIA) {
            ++avisos.dias;
        }
    }
    return avisos;
}

class NotificadorDePrueba : public Notificador {
public:
    std::vector<HoraDelJuego> a_todos_recibidas;
    std::vector<HoraDelJuego> a_un_jugador_recibidas;

    void a_jugador(uint32_t, const EventoServidor& evento) override {
        if (const HoraDelJuego* recibida = std::get_if<HoraDelJuego>(&evento)) {
            a_un_jugador_recibidas.push_back(*recibida);
        }
    }

    void a_todos(const EventoServidor& evento) override {
        if (const HoraDelJuego* recibida = std::get_if<HoraDelJuego>(&evento)) {
            a_todos_recibidas.push_back(*recibida);
        }
    }
};

}  // namespace

TEST(Reloj, ArrancaElDiaUnoALasSeis) {
    const Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);
    EXPECT_EQ(reloj.hora(), hora(1, 6, 0));
}

TEST(Reloj, LaHoraAvanzaDeADiezMinutos) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);

    avanzar(reloj, 9);
    EXPECT_EQ(reloj.hora(), hora(1, 6, 0));

    EXPECT_EQ(reloj.avanzar(), CambioDelReloj::HORA);
    EXPECT_EQ(reloj.hora(), hora(1, 6, 10));

    avanzar(reloj, 6 * 60 - 10);
    EXPECT_EQ(reloj.hora(), hora(1, 12, 0));
}

TEST(Reloj, PasaLaMedianocheSinCambiarDeDia) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);
    avanzar(reloj, 18 * 60);
    EXPECT_EQ(reloj.hora(), hora(1, 0, 0));

    avanzar(reloj, 90);
    EXPECT_EQ(reloj.hora(), hora(1, 1, 30));
}

TEST(Reloj, ALasDosSeFrenaHastaQueEmpieceElDiaSiguiente) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);
    avanzar(reloj, TICKS_POR_DIA - 1);
    EXPECT_EQ(reloj.hora(), hora(1, 1, 50));
    EXPECT_FALSE(reloj.termino_el_dia());

    EXPECT_EQ(reloj.avanzar(), CambioDelReloj::FIN_DEL_DIA);
    EXPECT_TRUE(reloj.termino_el_dia());
    EXPECT_EQ(reloj.hora(), hora(1, 2, 0));

    const Avisos esperando = avanzar(reloj, 500);
    EXPECT_EQ(esperando.horas, 0);
    EXPECT_EQ(esperando.dias, 0);
    EXPECT_EQ(reloj.hora(), hora(1, 2, 0));

    reloj.empezar_dia_siguiente();
    EXPECT_FALSE(reloj.termino_el_dia());
    EXPECT_EQ(reloj.hora(), hora(2, 6, 0));
}

TEST(Reloj, EmpezarElDiaSiguienteAntesDeLasDosTambienVaALasSeis) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);
    avanzar(reloj, 300);
    reloj.empezar_dia_siguiente();
    EXPECT_EQ(reloj.hora(), hora(2, 6, 0));
}

TEST(Reloj, AvisaCientoVeinteVecesPorDia) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);
    const Avisos avisos = avanzar(reloj, TICKS_POR_DIA);

    EXPECT_EQ(avisos.horas, 119);
    EXPECT_EQ(avisos.dias, 1);
}

TEST(Reloj, UnDiaDeCatorceMinutosDuraLoQueDebe) {
    Reloj reloj(14.0, 40, DIAS_POR_ESTACION);
    const Avisos antes = avanzar(reloj, 33599);
    EXPECT_EQ(antes.dias, 0);
    EXPECT_EQ(reloj.avanzar(), CambioDelReloj::FIN_DEL_DIA);
}

TEST(Reloj, NoAceptaDuracionesInvalidas) {
    EXPECT_THROW(Reloj(0.0, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION), std::invalid_argument);
    EXPECT_THROW(Reloj(UN_MINUTO, 0, DIAS_POR_ESTACION), std::invalid_argument);
    EXPECT_THROW(Reloj(UN_MINUTO, TICKS_POR_SEGUNDO, 0), std::invalid_argument);
}

TEST(Reloj, ArrancaEnElPrimerDiaDeLaPrimaveraDelAnioUno) {
    const HoraDelJuego hora_del_juego =
        Reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION).hora();
    EXPECT_EQ(hora_del_juego.dia_de_la_estacion, 1);
    EXPECT_EQ(hora_del_juego.estacion, Estacion::PRIMAVERA);
    EXPECT_EQ(hora_del_juego.anio, 1);
}

TEST(Reloj, DespuesDelDia28DeLaPrimaveraVieneElVerano) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, DIAS_POR_ESTACION);

    ir_al_dia(reloj, 28);
    EXPECT_EQ(reloj.hora().dia_de_la_estacion, 28);
    EXPECT_EQ(reloj.hora().estacion, Estacion::PRIMAVERA);

    ir_al_dia(reloj, 29);
    EXPECT_EQ(reloj.hora().dia_de_la_estacion, 1);
    EXPECT_EQ(reloj.hora().estacion, Estacion::VERANO);
    EXPECT_EQ(reloj.hora().anio, 1);
}

TEST(Reloj, LasEstacionesVanEnOrdenYDespuesDelInviernoEmpiezaOtroAnio) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO, 1);
    const Estacion orden[] = {
        Estacion::PRIMAVERA,
        Estacion::VERANO,
        Estacion::OTONO,
        Estacion::INVIERNO};
    for (uint32_t dia = 1; dia <= 4; ++dia) {
        ir_al_dia(reloj, dia);
        EXPECT_EQ(reloj.hora().estacion, orden[dia - 1]) << "día " << dia;
        EXPECT_EQ(reloj.hora().anio, 1);
    }

    ir_al_dia(reloj, 5);
    EXPECT_EQ(reloj.hora().estacion, Estacion::PRIMAVERA);
    EXPECT_EQ(reloj.hora().anio, 2);
    EXPECT_EQ(reloj.hora().dia, 5u);
}

TEST(ConfiguracionDelDia, ElTomlPideUnDiaDeCatorceMinutos) {
    const ConfiguracionServidor configuracion("config/servidor.toml");
    EXPECT_EQ(configuracion.juego().dia.velocidad, "medio");
    EXPECT_DOUBLE_EQ(configuracion.juego().dia.duracion_en_minutos, 14.0);
    EXPECT_EQ(configuracion.juego().dia.dias_por_estacion, 28);
}

TEST(ControladorDelDia, AvisaATodosCadaDiezMinutosYAlQueEntra) {
    const ConfiguracionServidor configuracion_del_servidor("config/servidor.toml");
    std::vector<ConfiguracionMapaServidor> mapas;
    for (const ConfiguracionMapaServidor& mapa : configuracion_del_servidor.mapas()) {
        if (mapa.id == "granja") {
            mapas.push_back(mapa);
        }
    }
    const RecursosDeJuego recursos(
        mapas,
        "granja",
        configuracion_del_servidor.juego(),
        ConfiguracionMazmorraServidor{});
    NotificadorDePrueba notificador;
    std::map<uint32_t, Personaje> personajes;
    GestorDeSectores sectores(recursos, TICKS_POR_SEGUNDO, personajes, notificador);
    ConfiguracionDiaServidor configuracion;
    configuracion.duracion_en_minutos = UN_MINUTO;
    ControladorDelDia controlador(
        configuracion,
        TICKS_POR_SEGUNDO,
        sectores,
        personajes,
        recursos,
        notificador,
        [](Personaje&) {});

    for (int i = 0; i < 25; ++i) {
        controlador.actualizar();
    }
    ASSERT_EQ(notificador.a_todos_recibidas.size(), 2u);
    EXPECT_EQ(notificador.a_todos_recibidas[0], hora(1, 6, 10));
    EXPECT_EQ(notificador.a_todos_recibidas[1], hora(1, 6, 20));
}

TEST(ControladorDelDia, ElQueEntraALaPartidaRecibeLaHora) {
    const ConfiguracionServidor configuracion("config/servidor.toml");
    std::vector<ConfiguracionMapaServidor> mapas;
    for (const ConfiguracionMapaServidor& mapa : configuracion.mapas()) {
        if (mapa.id == "granja") {
            mapas.push_back(mapa);
        }
    }
    const RecursosDeJuego recursos(
        mapas,
        "granja",
        configuracion.juego(),
        ConfiguracionMazmorraServidor{});
    NotificadorDePrueba notificador;
    Mundo mundo(recursos, notificador, 40);

    mundo.conectar(1);
    ASSERT_EQ(notificador.a_un_jugador_recibidas.size(), 1u);
    EXPECT_EQ(notificador.a_un_jugador_recibidas[0], hora(1, 6, 0));
}
