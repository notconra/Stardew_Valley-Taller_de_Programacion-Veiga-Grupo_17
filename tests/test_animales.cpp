#include <gtest/gtest.h>

#include <vector>

#include "../common/tipo_animal.h"
#include "../common/tipo_item.h"
#include "../server/configuracion/configuracion_servidor.h"
#include "../server/juego/animales/gallina.h"
#include "../server/juego/animales/gestor_de_animales.h"
#include "../server/juego/animales/vaca.h"
#include "../server/juego/drops/drops.h"
#include "../server/juego/objetos/catalogo_de_items.h"
#include "../server/juego/recursos/recursos_de_juego.h"

namespace {
const RecursosDeJuego& recursos_test() {
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

Colisiones crear_colisiones_test() {
    return Colisiones(recursos_test().mapas.at("granja"));
}

Jugadores crear_jugadores_test(const Colisiones& colisiones) {
    const RecursosDeJuego& recursos = recursos_test();
    return Jugadores(
        recursos.mapas.at("granja"),
        colisiones,
        10,
        recursos.juego.movimiento,
        recursos.juego.aparicion);
}
}  // namespace

TEST(Animales, VacaCicloDeProduccionDeLecheCadaDosDias) {
    Vaca vaca(1, 10, 10, 0);
    EXPECT_FALSE(vaca.esta_lista_para_ordeniar());
    EXPECT_EQ(vaca.dias_acumulados, 0u);

    // Día 1: no lista todavía
    vaca.avanzar_dia();
    EXPECT_FALSE(vaca.esta_lista_para_ordeniar());
    EXPECT_EQ(vaca.dias_acumulados, 1u);

    // Día 2: lista para ordeñar
    vaca.avanzar_dia();
    EXPECT_TRUE(vaca.esta_lista_para_ordeniar());
    EXPECT_EQ(vaca.dias_acumulados, 2u);
}

TEST(Animales, VacaOrdeniarReiniciaCicloYDesactivaFlag) {
    Vaca vaca(1, 10, 10, 0);
    vaca.avanzar_dia();
    vaca.avanzar_dia();
    ASSERT_TRUE(vaca.esta_lista_para_ordeniar());

    // Se ordeña la vaca con éxito
    EXPECT_TRUE(vaca.ordeniar());
    EXPECT_FALSE(vaca.esta_lista_para_ordeniar());
    EXPECT_EQ(vaca.dias_acumulados, 0u);

    // Un segundo ordeñe inmediato no debe tener leche
    EXPECT_FALSE(vaca.ordeniar());
}

TEST(Animales, VacaLimiteDeCincoPorGranja) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);

    for (uint32_t i = 0; i < Vaca::LIMITE_POR_GRANJA; ++i) {
        EXPECT_TRUE(gestor.agregar_vaca(10 + static_cast<int>(i * 2), 10));
    }
    EXPECT_EQ(gestor.cantidad_vacas(), 5);

    // La sexta vaca debe ser rechazada por límite de tope
    EXPECT_FALSE(gestor.agregar_vaca(22, 22));
    EXPECT_EQ(gestor.cantidad_vacas(), 5);
}

TEST(Animales, GallinaProduccionDiariaAutomaticaDeHuevos) {
    Gallina gallina(1, 5, 5, 0);

    // Todos los días avanza su ciclo
    gallina.avanzar_dia();
    EXPECT_EQ(gallina.dias_acumulados, 1u);

    gallina.avanzar_dia();
    EXPECT_EQ(gallina.dias_acumulados, 2u);
}

TEST(Animales, AgregarConVarianteEspecifica) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);
    EXPECT_TRUE(gestor.agregar_vaca(15, 15, 1));
    const Vaca* vaca = gestor.buscar_vaca(1);
    ASSERT_NE(vaca, nullptr);
    EXPECT_EQ(vaca->variante, 1);

    EXPECT_TRUE(gestor.agregar_gallina(20, 20, 3));
    const Gallina* gallina = gestor.buscar_gallina(2);
    ASSERT_NE(gallina, nullptr);
    EXPECT_EQ(gallina->variante, 3);
}

TEST(Animales, GallinaLimiteDeDiezPorGranja) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);

    for (uint32_t i = 0; i < Gallina::LIMITE_POR_GRANJA; ++i) {
        EXPECT_TRUE(gestor.agregar_gallina(10 + static_cast<int>(i), 10));
    }
    EXPECT_EQ(gestor.cantidad_gallinas(), 10);

    // La onceava gallina debe ser rechazada por límite de tope
    EXPECT_FALSE(gestor.agregar_gallina(30, 30));
    EXPECT_EQ(gestor.cantidad_gallinas(), 10);
}

TEST(Animales, CatalogoContieneLecheYHuevosConPreciosDelEnunciado) {
    const CatalogoDeItems catalogo;

    const DefinicionItem* def_leche = catalogo.buscar(TipoItem::LECHE);
    ASSERT_NE(def_leche, nullptr);
    EXPECT_EQ(def_leche->nombre, "Leche");
    EXPECT_EQ(def_leche->categoria, CategoriaItem::RECURSO);
    EXPECT_EQ(def_leche->precio_de_venta, 30u);
    EXPECT_EQ(def_leche->maximo_apilable, 50u);

    const DefinicionItem* def_huevo = catalogo.buscar(TipoItem::HUEVO);
    ASSERT_NE(def_huevo, nullptr);
    EXPECT_EQ(def_huevo->nombre, "Huevo");
    EXPECT_EQ(def_huevo->categoria, CategoriaItem::RECURSO);
    EXPECT_EQ(def_huevo->precio_de_venta, 3u);
    EXPECT_EQ(def_huevo->maximo_apilable, 50u);
}

TEST(Animales, VariantesAleatoriasDeRazasEnRangoValido) {
    for (int i = 0; i < 20; ++i) {
        Colisiones colisiones = crear_colisiones_test();
        GestorDeAnimales gestor(colisiones);
        EXPECT_TRUE(gestor.agregar_vaca(15, 15));
        const Vaca* vaca = gestor.buscar_vaca(1);
        ASSERT_NE(vaca, nullptr);
        EXPECT_LT(vaca->variante, Vaca::CANTIDAD_VARIANTES);

        EXPECT_TRUE(gestor.agregar_gallina(20, 20));
        const Gallina* gallina = gestor.buscar_gallina(2);
        ASSERT_NE(gallina, nullptr);
        EXPECT_LT(gallina->variante, Gallina::CANTIDAD_VARIANTES);
    }
}

TEST(Animales, GranjaProvisoriaArrancaConUnaVacaYUnaGallina) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);
    gestor.inicializar_granja_provisoria();

    EXPECT_EQ(gestor.cantidad_vacas(), 1);
    EXPECT_EQ(gestor.cantidad_gallinas(), 1);
    EXPECT_EQ(gestor.cantidad_total(), 2);
}

TEST(Animales, GestorAvanzarDiaDepositaHuevoComoDropEnElPiso) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);
    EXPECT_TRUE(gestor.agregar_gallina(12, 14));

    Drops drops;
    gestor.avanzar_dia(drops);

    const Drop* drop = drops.buscar(1);
    ASSERT_NE(drop, nullptr);
    EXPECT_EQ(drop->tipo, TipoItem::HUEVO);
    EXPECT_EQ(drop->cantidad, 1);
    EXPECT_EQ(drop->calidad, Calidad::BASE);
    EXPECT_EQ(drop->columna, 12);
    EXPECT_EQ(drop->fila, 14);
}

TEST(Animales, GestorOrdeniarVacaAlSegundoDia) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);
    EXPECT_TRUE(gestor.agregar_vaca(10, 10));

    Drops drops;
    // Día 1
    gestor.avanzar_dia(drops);
    EXPECT_FALSE(gestor.ordeniar_vaca(1));

    // Día 2
    gestor.avanzar_dia(drops);
    EXPECT_TRUE(gestor.ordeniar_vaca(1));

    // Ya no tiene leche
    EXPECT_FALSE(gestor.ordeniar_vaca(1));
}

TEST(Animales, ColisionesVacaOcupaDosPorDosYGallinaUnoPorUno) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);

    // (15, 15) y alrededores son pasto abierto en la granja
    EXPECT_TRUE(gestor.agregar_vaca(15, 15));
    const Vaca* vaca = gestor.buscar_vaca(1);
    ASSERT_NE(vaca, nullptr);
    EXPECT_TRUE(colisiones.bloquea(vaca->rectangulo_de_colision()));

    EXPECT_TRUE(gestor.agregar_gallina(20, 20));
    const Gallina* gallina = gestor.buscar_gallina(2);
    ASSERT_NE(gallina, nullptr);
    EXPECT_TRUE(colisiones.bloquea(gallina->rectangulo_de_colision()));

    // No se puede agregar otro animal encima de las casillas de colision
    EXPECT_FALSE(gestor.agregar_gallina(15, 16));
    EXPECT_FALSE(gestor.agregar_vaca(19, 19));
}

TEST(Animales, RutinaHorariaALasSietePMQuietosYALasOchoPMDuermen) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);
    Jugadores jugadores = crear_jugadores_test(colisiones);
    EXPECT_TRUE(gestor.agregar_vaca(10, 10));
    EXPECT_TRUE(gestor.agregar_gallina(15, 15));

    HoraDelJuego hora_tarde;
    hora_tarde.hora = 19;
    hora_tarde.minuto = 0;
    gestor.actualizar(hora_tarde, 50, 50, jugadores);

    const Vaca* vaca = gestor.buscar_vaca(1);
    const Gallina* gallina = gestor.buscar_gallina(2);
    ASSERT_NE(vaca, nullptr);
    ASSERT_NE(gallina, nullptr);
    EXPECT_FALSE(vaca->caminando);
    EXPECT_FALSE(vaca->durmiendo);
    EXPECT_FALSE(gallina->caminando);
    EXPECT_FALSE(gallina->durmiendo);

    HoraDelJuego hora_noche;
    hora_noche.hora = 20;
    hora_noche.minuto = 0;
    gestor.actualizar(hora_noche, 50, 50, jugadores);
    EXPECT_TRUE(vaca->durmiendo);
    EXPECT_FALSE(vaca->caminando);
    EXPECT_TRUE(gallina->durmiendo);
    EXPECT_FALSE(gallina->caminando);

    // Ordeñar una vaca dormida sigue funcionando si tiene leche
    Drops drops;
    gestor.avanzar_dia(drops);
    gestor.avanzar_dia(drops);
    EXPECT_TRUE(vaca->esta_lista_para_ordeniar());
    EXPECT_TRUE(gestor.ordeniar_vaca(1));
    EXPECT_FALSE(vaca->esta_lista_para_ordeniar());
}

TEST(Animales, BuscarVacaPorCasillaDosPorDos) {
    Colisiones colisiones = crear_colisiones_test();
    GestorDeAnimales gestor(colisiones);
    EXPECT_TRUE(gestor.agregar_vaca(10, 10));

    EXPECT_NE(gestor.vaca_en(10, 10), nullptr);
    EXPECT_NE(gestor.vaca_en(11, 10), nullptr);
    EXPECT_NE(gestor.vaca_en(10, 11), nullptr);
    EXPECT_NE(gestor.vaca_en(11, 11), nullptr);

    EXPECT_EQ(gestor.vaca_en(9, 10), nullptr);
    EXPECT_EQ(gestor.vaca_en(12, 10), nullptr);
    EXPECT_EQ(gestor.vaca_en(10, 12), nullptr);
}
