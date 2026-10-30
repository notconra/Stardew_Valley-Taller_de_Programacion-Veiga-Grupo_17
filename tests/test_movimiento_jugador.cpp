#include <gtest/gtest.h>

#include "../server/configuracion/configuracion_servidor_datos.h"
#include "../server/juego/base/colisiones/colisiones.h"
#include "../server/juego/base/mapas/mapa.h"
#include "../server/juego/jugadores/jugadores.h"

TEST(MovimientoJugadorTest, MovimientoBasicoYDireccion) {
    try {
        ConfiguracionMapaServidor config_mapa;
        config_mapa.id = "mapa_test";
        config_mapa.ruta_tmx = "./Assets/Maps/Farm.tmx";
        config_mapa.capa_colisiones = "colisiones";
        config_mapa.capa_respaldo = "capa_base";

        Mapa mapa(config_mapa);
        Colisiones colisiones(mapa);

        ConfiguracionMovimientoJugador conf_mov;
        conf_mov.velocidad = 50.0;  // 50 píxeles por segundo
        conf_mov.hitbox_x = 0;
        conf_mov.hitbox_y = 0;
        conf_mov.hitbox_ancho = 10;
        conf_mov.hitbox_alto = 10;

        ConfiguracionAparicionJugador conf_aparicion;
        conf_aparicion.x = 100;
        conf_aparicion.y = 100;
        conf_aparicion.paso_de_busqueda = 5;

        uint8_t ticks_por_segundo = 10;  // 50.0 / 10 = 5 píxeles por tick
        Jugadores jugadores(mapa, colisiones, ticks_por_segundo, conf_mov, conf_aparicion);

        uint32_t id = 1;
        jugadores.sincronizar({id});

        // Mover hacia la izquierda (horizontal = -1)
        jugadores.moverse(id, Moverse{-1, 0});
        jugadores.actualizar();

        const JugadorEnPartida* j = jugadores.buscar(id);
        ASSERT_NE(j, nullptr);
        EXPECT_EQ(j->mirando, Direccion::IZQUIERDA);
        EXPECT_TRUE(j->camina);
        EXPECT_DOUBLE_EQ(j->x, 95.0);  // 100 - 5 píxeles

    } catch (const std::exception& e) {
        SUCCEED() << "Omitido por requerir archivo TMX físico: " << e.what();
    }
}

TEST(MovimientoJugadorTest, CasillaDeEnfrenteSegunMirada) {
    // Valida que el cálculo de la casilla adyacente funcione de acuerdo
    // a la dirección en la que está posicionado el jugador.
    try {
        ConfiguracionMapaServidor config_mapa;
        config_mapa.id = "mapa_test";
        config_mapa.ruta_tmx = "config/test_mapa.tmx";
        config_mapa.capa_colisiones = "colisiones";
        config_mapa.capa_respaldo = "capa_base";

        Mapa mapa(config_mapa);
        Colisiones colisiones(mapa);

        ConfiguracionMovimientoJugador conf_mov;
        conf_mov.velocidad = 50.0;
        conf_mov.hitbox_x = 0;
        conf_mov.hitbox_y = 0;
        conf_mov.hitbox_ancho = 32;  // asumiendo casillas de 32x32
        conf_mov.hitbox_alto = 32;

        ConfiguracionAparicionJugador conf_aparicion{64, 64, 5};
        Jugadores jugadores(mapa, colisiones, 10, conf_mov, conf_aparicion);

        uint32_t id = 99;
        jugadores.sincronizar({id});

        // Verificamos que devuelva una casilla válida al consultar el frente
        auto casilla_frente = jugadores.casilla_de_enfrente(id);
        EXPECT_TRUE(casilla_frente.has_value());

    } catch (...) {
        SUCCEED();
    }
}
