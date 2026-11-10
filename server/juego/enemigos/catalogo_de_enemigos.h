#ifndef CATALOGO_DE_ENEMIGOS_H
#define CATALOGO_DE_ENEMIGOS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "../../../common/tipo_enemigo.h"
#include "../../configuracion/configuracion_servidor_datos.h"

// Qué se potencia cuando un vampiro entra en frenesí (al morir su pareja).
enum class HabilidadDeFrenesi : uint8_t {
    NINGUNA,
    VELOCIDAD,  // la vampira: tarda menos en cruzar cada casilla
    DANIO,      // el vampiro: pega más fuerte
};

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

    // Ticks de juego que tarda en cruzar una casilla (cuanto más alto, más lento). 0 significa que
    // no se mueve. A 40 ticks/seg, 20 ticks son 2 casillas por segundo.
    uint32_t ticks_por_casilla = 0;

    // A cuántas casillas (en línea recta, contando diagonales) detecta a un jugador y empieza a
    // perseguirlo. Más lejos que eso se queda quieto.
    int radio_de_vision = 0;

    // Mientras no ve a nadie, deambula al azar sin alejarse de donde apareció más que esta cantidad
    // de casillas. 0 significa que se queda quieto hasta ver a un jugador.
    int radio_de_deambulo = 0;

    // Las Skulls no se pueden matar: el jugador tiene que huir. Con esto en true, los golpes no
    // les hacen nada.
    bool invulnerable = false;

    // Oro que suelta al morir (ver sección "Oro" del enunciado: "todos los enemigos pueden
    // droppear oro al morir"). Se sortea un entero uniforme en [oro_minimo, oro_maximo]; ambos en
    // cero significa que ese tipo no deja oro.
    uint16_t oro_minimo = 0;
    uint16_t oro_maximo = 0;

    // Probabilidad (0.0 a 1.0) de que al morir deje además una plata (los Orcos: 20%).
    double probabilidad_de_plata = 0.0;

    // Vampiros: muertos no reviven solos (ticks_para_revivir se cuenta recién desde que cae
    // sangre), y la sangre cae cuando un jugador queda gravemente herido o inconsciente a
    // radio_de_sangre casillas o menos de su cadáver.
    bool revive_con_sangre = false;
    int radio_de_sangre = 0;

    // Vampiros: qué se multiplica, y por cuánto, mientras su pareja está muerta.
    HabilidadDeFrenesi habilidad_de_frenesi = HabilidadDeFrenesi::NINGUNA;
    double multiplicador_de_frenesi = 1.0;
};

// Arma la definición con los números de combate comunes a todos los tipos (vida, daño, ticks,
// radios y oro) sacados de su sección del servidor.toml. Cada tipo parte de acá y agrega lo suyo.
DefinicionDeEnemigo definicion_de_combate(
    TipoEnemigo tipo, const std::string& clave, const ConfiguracionEnemigoServidor& configuracion);

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
