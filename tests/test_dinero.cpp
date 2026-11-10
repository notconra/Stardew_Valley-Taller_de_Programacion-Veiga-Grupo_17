#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "../server/configuracion/configuracion_servidor.h"
#include "../server/juego/chat/comando_dinero.h"
#include "../server/juego/chat/contexto_de_comando_de_chat.h"
#include "../server/juego/chat/respuestas_de_chat.h"
#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/mundo.h"
#include "../server/juego/notificador.h"
#include "../server/juego/objetos/catalogo_de_items.h"
#include "../server/juego/recursos/recursos_de_juego.h"

// Pruebas sobre la billetera de monedas de oro del jugador (regla del enunciado: el jugador
// arranca con una cantidad aleatoria entre 0 y 100 monedas de oro, y puede recibir o gastar
// monedas según disponga de saldo suficiente).

namespace {

const uint32_t ID_JUGADOR = 1;
const uint8_t CANTIDAD_RANURAS_BARRA = 10;
const char* MAPA_INICIAL = "granja";

Personaje crear_personaje(const CatalogoDeItems& catalogo, uint32_t id = ID_JUGADOR) {
    return Personaje(id, MAPA_INICIAL, catalogo, CANTIDAD_RANURAS_BARRA);
}

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

class NotificadorMudo : public Notificador {
public:
    void a_jugador(uint32_t /*id_jugador*/, const EventoServidor& /*evento*/) override {
    }
    void a_todos(const EventoServidor& /*evento*/) override {
    }
};

class RespuestasDePrueba : public RespuestasDeChat {
public:
    std::string ultimo_mensaje;
    uint32_t ultimo_destinatario = 0;

    void responder_a_jugador(uint32_t id_jugador, const std::string& texto) override {
        ultimo_destinatario = id_jugador;
        ultimo_mensaje = texto;
    }

    void responder_como_comerciante(
        uint32_t /*id_jugador*/, const std::string& /*nombre*/,
        const std::string& /*texto*/) override {
    }
};

}  // namespace

TEST(Dinero, ElJugadorEmpiezaConMonedasEntreCeroYCien) {
    const CatalogoDeItems catalogo;
    std::set<uint32_t> valores_obtenidos;

    // Se verifica en múltiples creaciones que todas las monedas caigan dentro del rango [0, 100]
    // y que no sea un valor fijo constante.
    for (int i = 0; i < 50; ++i) {
        const Personaje personaje = crear_personaje(catalogo, i + 1);
        EXPECT_GE(personaje.monedas, Personaje::MONEDAS_INICIALES_MIN);
        EXPECT_LE(personaje.monedas, Personaje::MONEDAS_INICIALES_MAX);
        valores_obtenidos.insert(personaje.monedas);
    }

    EXPECT_GT(valores_obtenidos.size(), 1u);
}

TEST(Dinero, RecibirMonedasSumaAlSaldoActual) {
    const CatalogoDeItems catalogo;
    Personaje personaje = crear_personaje(catalogo);
    personaje.monedas = 25;

    personaje.recibir_monedas(50);
    EXPECT_EQ(personaje.monedas, 75u);

    personaje.recibir_monedas(10);
    EXPECT_EQ(personaje.monedas, 85u);
}

TEST(Dinero, GastarMonedasConSaldoSuficienteDescuentaYDevuelveTrue) {
    const CatalogoDeItems catalogo;
    Personaje personaje = crear_personaje(catalogo);
    personaje.monedas = 100;

    const bool exito = personaje.gastar_monedas(40);
    EXPECT_TRUE(exito);
    EXPECT_EQ(personaje.monedas, 60u);
}

TEST(Dinero, GastarExactamenteElSaldoDejaLasMonedasEnCero) {
    const CatalogoDeItems catalogo;
    Personaje personaje = crear_personaje(catalogo);
    personaje.monedas = 50;

    const bool exito = personaje.gastar_monedas(50);
    EXPECT_TRUE(exito);
    EXPECT_EQ(personaje.monedas, 0u);
}

TEST(Dinero, GastarMonedasSinSaldoSuficienteNoModificaElSaldoYDevuelveFalse) {
    const CatalogoDeItems catalogo;
    Personaje personaje = crear_personaje(catalogo);
    personaje.monedas = 30;

    const bool exito = personaje.gastar_monedas(31);
    EXPECT_FALSE(exito);
    EXPECT_EQ(personaje.monedas, 30u);
}

TEST(Dinero, GastarSinTenerMonedasFallaYPermaneceEnCero) {
    const CatalogoDeItems catalogo;
    Personaje personaje = crear_personaje(catalogo);
    personaje.monedas = 0;

    const bool exito = personaje.gastar_monedas(1);
    EXPECT_FALSE(exito);
    EXPECT_EQ(personaje.monedas, 0u);
}

TEST(Dinero, MundoOperacionesConMonedas) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    AparienciaPersonaje apariencia;
    mundo.conectar(ID_JUGADOR, apariencia);

    const uint32_t saldo_inicial = mundo.obtener_monedas(ID_JUGADOR);
    EXPECT_GE(saldo_inicial, Personaje::MONEDAS_INICIALES_MIN);
    EXPECT_LE(saldo_inicial, Personaje::MONEDAS_INICIALES_MAX);

    EXPECT_TRUE(mundo.sumar_monedas(ID_JUGADOR, 150));
    EXPECT_EQ(mundo.obtener_monedas(ID_JUGADOR), saldo_inicial + 150);

    EXPECT_TRUE(mundo.gastar_monedas(ID_JUGADOR, 50));
    EXPECT_EQ(mundo.obtener_monedas(ID_JUGADOR), saldo_inicial + 100);

    EXPECT_FALSE(mundo.gastar_monedas(ID_JUGADOR, 1'000'000));
    EXPECT_EQ(mundo.obtener_monedas(ID_JUGADOR), saldo_inicial + 100);

    // Jugador no conectado devuelve false o 0
    EXPECT_FALSE(mundo.sumar_monedas(999, 10));
    EXPECT_FALSE(mundo.gastar_monedas(999, 10));
    EXPECT_EQ(mundo.obtener_monedas(999), 0u);
}

TEST(Dinero, ComandoDineroReconoceSoloPalabraClaveDinero) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    ComandoDinero comando{mundo};

    EXPECT_TRUE(comando.reconoce("dinero"));
    EXPECT_FALSE(comando.reconoce("oro"));
    EXPECT_FALSE(comando.reconoce("monedas"));
    EXPECT_FALSE(comando.reconoce("plata"));
}

TEST(Dinero, ComandoDineroConsultarSaldo) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    AparienciaPersonaje apariencia;
    mundo.conectar(ID_JUGADOR, apariencia);

    ComandoDinero comando{mundo};
    RespuestasDePrueba respuestas;

    ContextoDeComandoDeChat contexto_vacio{ID_JUGADOR, "Jugador1", "dinero", {}, "", respuestas};
    comando.ejecutar(contexto_vacio);

    const uint32_t saldo = mundo.obtener_monedas(ID_JUGADOR);
    EXPECT_EQ(respuestas.ultimo_destinatario, ID_JUGADOR);
    EXPECT_EQ(respuestas.ultimo_mensaje, "Tenes " + std::to_string(saldo) + " monedas de oro.");

    ContextoDeComandoDeChat
        contexto_saldo{ID_JUGADOR, "Jugador1", "dinero", {"saldo"}, "", respuestas};
    comando.ejecutar(contexto_saldo);
    EXPECT_EQ(respuestas.ultimo_mensaje, "Tenes " + std::to_string(saldo) + " monedas de oro.");
}

TEST(Dinero, ComandoDineroRecibirMonedas) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    AparienciaPersonaje apariencia;
    mundo.conectar(ID_JUGADOR, apariencia);

    const uint32_t saldo_inicial = mundo.obtener_monedas(ID_JUGADOR);
    ComandoDinero comando{mundo};
    RespuestasDePrueba respuestas;

    ContextoDeComandoDeChat
        contexto{ID_JUGADOR, "Jugador1", "dinero", {"recibir", "500"}, "", respuestas};
    comando.ejecutar(contexto);

    const uint32_t saldo_esperado = saldo_inicial + 500;
    EXPECT_EQ(mundo.obtener_monedas(ID_JUGADOR), saldo_esperado);
    EXPECT_EQ(
        respuestas.ultimo_mensaje,
        "Recibiste 500 monedas. Saldo actual: " + std::to_string(saldo_esperado) + " monedas.");
}

TEST(Dinero, ComandoDineroGastarMonedasConExito) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    AparienciaPersonaje apariencia;
    mundo.conectar(ID_JUGADOR, apariencia);

    mundo.sumar_monedas(ID_JUGADOR, 200);
    const uint32_t saldo_previo = mundo.obtener_monedas(ID_JUGADOR);

    ComandoDinero comando{mundo};
    RespuestasDePrueba respuestas;

    ContextoDeComandoDeChat
        contexto{ID_JUGADOR, "Jugador1", "dinero", {"gastar", "150"}, "", respuestas};
    comando.ejecutar(contexto);

    const uint32_t saldo_esperado = saldo_previo - 150;
    EXPECT_EQ(mundo.obtener_monedas(ID_JUGADOR), saldo_esperado);
    EXPECT_EQ(
        respuestas.ultimo_mensaje,
        "Gastaste 150 monedas. Saldo restante: " + std::to_string(saldo_esperado) + " monedas.");
}

TEST(Dinero, ComandoDineroGastarMonedasSinSaldoSuficiente) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    AparienciaPersonaje apariencia;
    mundo.conectar(ID_JUGADOR, apariencia);

    const uint32_t saldo_actual = mundo.obtener_monedas(ID_JUGADOR);

    ComandoDinero comando{mundo};
    RespuestasDePrueba respuestas;

    ContextoDeComandoDeChat
        contexto{ID_JUGADOR, "Jugador1", "dinero", {"gastar", "1000000"}, "", respuestas};
    comando.ejecutar(contexto);

    EXPECT_EQ(mundo.obtener_monedas(ID_JUGADOR), saldo_actual);
    EXPECT_EQ(
        respuestas.ultimo_mensaje,
        "Saldo insuficiente. Tenes " + std::to_string(saldo_actual) +
            " monedas y queres gastar 1000000.");
}

TEST(Dinero, ComandoDineroValidacionDeArgumentosYAyuda) {
    NotificadorMudo notificador;
    Mundo mundo{recursos(), notificador, 40};
    AparienciaPersonaje apariencia;
    mundo.conectar(ID_JUGADOR, apariencia);

    ComandoDinero comando{mundo};
    RespuestasDePrueba respuestas;

    // Falta cantidad en recibir
    ContextoDeComandoDeChat
        sin_cantidad{ID_JUGADOR, "Jugador1", "dinero", {"recibir"}, "", respuestas};
    comando.ejecutar(sin_cantidad);
    EXPECT_EQ(respuestas.ultimo_mensaje, "Uso: /dinero recibir <cantidad>");

    // Cantidad no numérica
    ContextoDeComandoDeChat
        cantidad_invalida{ID_JUGADOR, "Jugador1", "dinero", {"recibir", "mucho"}, "", respuestas};
    comando.ejecutar(cantidad_invalida);
    EXPECT_EQ(
        respuestas.ultimo_mensaje,
        "La cantidad debe ser un numero entero positivo menor a 100 millones.");

    // Cantidad cero
    ContextoDeComandoDeChat
        cantidad_cero{ID_JUGADOR, "Jugador1", "dinero", {"gastar", "0"}, "", respuestas};
    comando.ejecutar(cantidad_cero);
    EXPECT_EQ(respuestas.ultimo_mensaje, "La cantidad debe ser un numero entero positivo.");

    // Subcomando desconocido
    ContextoDeComandoDeChat
        subcomando_invalido{ID_JUGADOR, "Jugador1", "dinero", {"inventar"}, "", respuestas};
    comando.ejecutar(subcomando_invalido);
    EXPECT_EQ(
        respuestas.ultimo_mensaje,
        "Uso: /dinero [saldo | recibir <cantidad> | gastar <cantidad>]");
}
