#include <gtest/gtest.h>

#include <cstdint>
#include <set>

#include "../server/juego/jugadores/personaje.h"
#include "../server/juego/objetos/catalogo_de_items.h"

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
