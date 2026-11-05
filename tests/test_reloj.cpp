#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <variant>
#include <vector>

#include "../server/configuracion/configuracion_servidor.h"
#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/controladores/controlador_de_dia.h"
#include "../server/juego/mundo.h"
#include "../server/juego/notificador.h"
#include "../server/juego/recursos/recursos_de_juego.h"
#include "../server/juego/tiempo/reloj.h"

namespace {
// Con un día de 1 minuto real y 20 ticks por segundo, el día son 1200 ticks: cada tick es justo
// un minuto de juego, y cada aviso (10 minutos de juego) son 10 ticks.
const double UN_MINUTO = 1.0;
const int TICKS_POR_SEGUNDO = 20;
const int TICKS_POR_DIA = 1200;

HoraDelJuego hora(uint32_t dia, uint8_t hora_del_dia, uint8_t minuto) {
    HoraDelJuego hora_del_juego;
    hora_del_juego.dia = dia;
    hora_del_juego.hora = hora_del_dia;
    hora_del_juego.minuto = minuto;
    return hora_del_juego;
}

// Avanza el reloj "ticks" veces y devuelve cuántas veces avisó cada cambio.
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
        } else if (cambio == CambioDelReloj::DIA_NUEVO) {
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

// Test que prueban el funcionamiento del reloj, si cambia correctamente, si cambia cuando tiene que
// cambiar o no, si acepta o no ticks invalidos entre otros.

TEST(Reloj, ArrancaElDiaUnoALasSeis) {
    const Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO);
    EXPECT_EQ(reloj.hora(), hora(1, 6, 0));
}

TEST(Reloj, LaHoraAvanzaDeADiezMinutos) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO);

    avanzar(reloj, 9);
    EXPECT_EQ(reloj.hora(), hora(1, 6, 0));

    EXPECT_EQ(reloj.avanzar(), CambioDelReloj::HORA);
    EXPECT_EQ(reloj.hora(), hora(1, 6, 10));

    avanzar(reloj, 6 * 60 - 10);
    EXPECT_EQ(reloj.hora(), hora(1, 12, 0));
}

TEST(Reloj, PasaLaMedianocheSinCambiarDeDia) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO);
    avanzar(reloj, 18 * 60);
    EXPECT_EQ(reloj.hora(), hora(1, 0, 0));

    avanzar(reloj, 90);
    EXPECT_EQ(reloj.hora(), hora(1, 1, 30));
}

TEST(Reloj, ALasDosEmpiezaElDiaSiguienteALasSeis) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO);
    avanzar(reloj, TICKS_POR_DIA - 1);
    EXPECT_EQ(reloj.hora(), hora(1, 1, 50));

    EXPECT_EQ(reloj.avanzar(), CambioDelReloj::DIA_NUEVO);
    EXPECT_EQ(reloj.hora(), hora(2, 6, 0));
}

TEST(Reloj, AvisaCientoVeinteVecesPorDia) {
    Reloj reloj(UN_MINUTO, TICKS_POR_SEGUNDO);
    const Avisos avisos = avanzar(reloj, 3 * TICKS_POR_DIA);
    EXPECT_EQ(avisos.horas, 3 * 119);
    EXPECT_EQ(avisos.dias, 3);
    EXPECT_EQ(reloj.hora(), hora(4, 6, 0));
}

TEST(Reloj, UnDiaDeCatorceMinutosDuraLoQueDebe) {
    Reloj reloj(14.0, 40);
    const Avisos antes = avanzar(reloj, 33599);
    EXPECT_EQ(antes.dias, 0);
    EXPECT_EQ(reloj.avanzar(), CambioDelReloj::DIA_NUEVO);
}

TEST(Reloj, NoAceptaDuracionesInvalidas) {
    EXPECT_THROW(Reloj(0.0, TICKS_POR_SEGUNDO), std::invalid_argument);
    EXPECT_THROW(Reloj(UN_MINUTO, 0), std::invalid_argument);
}

TEST(ConfiguracionDelDia, ElTomlPideUnDiaDeCatorceMinutos) {
    const ConfiguracionServidor configuracion("config/servidor.toml");
    EXPECT_EQ(configuracion.juego().dia.velocidad, "medio");
    EXPECT_DOUBLE_EQ(configuracion.juego().dia.duracion_en_minutos, 14.0);
}

TEST(ControladorDelDia, AvisaATodosCadaDiezMinutosYAlQueEntra) {
    NotificadorDePrueba notificador;
    ConfiguracionDiaServidor configuracion;
    configuracion.duracion_en_minutos = UN_MINUTO;
    ControladorDelDia controlador(configuracion, TICKS_POR_SEGUNDO, notificador);

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
