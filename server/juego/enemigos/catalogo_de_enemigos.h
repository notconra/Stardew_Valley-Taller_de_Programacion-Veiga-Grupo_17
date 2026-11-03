#ifndef CATALOGO_DE_ENEMIGOS_H
#define CATALOGO_DE_ENEMIGOS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../common/tipo_enemigo.h"

// Cómo es un tipo de enemigo: cuánta vida tiene, cuánto daño hace y, si puede morir, cuánto tarda
// en revivir. El combate en sí (cuerpo a cuerpo o a distancia, perseguir, huir) no está acá: esto
// es solo la hoja de datos que arma el catálogo al arrancar el servidor, análoga a
// DefinicionDeObjeto para los objetos del mundo.
struct DefinicionDeEnemigo {
    TipoEnemigo tipo = TipoEnemigo::NINGUNO;
    std::string clave;  // identificador de texto, único entre todas las definiciones
    int vida_maxima = 0;
    int danio = 0;

    // Ticks de juego entre un golpe y el siguiente del mismo enemigo (mientras el jugador se
    // quede al lado). Tiene que ser mayor que cero: si no, golpearía todos los ticks.
    uint32_t ticks_entre_ataques = 1;

    // Ticks de juego hasta que vuelve a aparecer con la vida llena después de morir. 0 significa
    // que no revive (como los Orcos, que solo pueden dropear plata al morir).
    uint32_t ticks_para_revivir = 0;

    // Las Skulls no se pueden matar: el jugador tiene que huir. Con esto en true, los golpes no
    // les hacen nada.
    bool invulnerable = false;

    // Oro que suelta al morir (ver sección "Oro" del enunciado: "todos los enemigos pueden
    // droppear oro al morir"). Se sortea un entero uniforme en [oro_minimo, oro_maximo]; ambos en
    // cero significa que ese tipo no deja oro directamente (por ejemplo, si solo puede dejar
    // plata, como el Orco).
    uint16_t oro_minimo = 0;
    uint16_t oro_maximo = 0;
};

// Todos los tipos de enemigo que existen. Se arma una sola vez al arrancar (RecursosDeJuego) y
// después solo se consulta, igual que CatalogoDeObjetos.
class CatalogoDeEnemigos {
private:
    std::map<TipoEnemigo, DefinicionDeEnemigo> definiciones_por_tipo;

public:
    // Lanza std::runtime_error si una definición no tiene tipo o clave, o si hay tipos o claves
    // repetidos.
    explicit CatalogoDeEnemigos(const std::vector<DefinicionDeEnemigo>& definiciones);

    // Devuelve nullptr si el tipo no existe en el catálogo.
    const DefinicionDeEnemigo* buscar(TipoEnemigo tipo) const;

    const std::map<TipoEnemigo, DefinicionDeEnemigo>& definiciones() const;
};

#endif
