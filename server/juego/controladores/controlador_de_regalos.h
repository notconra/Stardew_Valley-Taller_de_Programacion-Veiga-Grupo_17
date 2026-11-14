#ifndef CONTROLADOR_DE_REGALOS_H
#define CONTROLADOR_DE_REGALOS_H

#include <cstdint>
#include <map>

#include "../../../common/mensajes/mensajes.h"
#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../objetos/catalogo_de_items.h"

// Regalos entre jugadores de la partida. Un regalo sale de la barra del que lo manda y queda en el
// buzón del destinatario (esté conectado o no) hasta que lo abre: recién ahí pasa a su inventario.
// Las herramientas no se pueden regalar.
class ControladorDeRegalos {
private:
    std::map<uint32_t, Personaje>& personajes;
    const CatalogoDeItems& catalogo;
    Notificador& notificador;

    void enviar(Personaje& remitente, const EnviarRegalo& pedido);
    void abrir(Personaje& personaje);

    JugadoresDeLaPartida jugadores_de_la_partida() const;
    void avisar_regalos_pendientes(const Personaje& personaje);

public:
    ControladorDeRegalos(
        std::map<uint32_t, Personaje>& personajes, const CatalogoDeItems& catalogo,
        Notificador& notificador);

    // EnviarRegalo y AbrirRegalos. Devuelve true si lo consumió.
    bool atender(Personaje& personaje, const ComandoCliente& comando);

    // Al entrar: la lista de jugadores a todos (cambió quién está conectado) y sus regalos al que
    // entró.
    void al_conectarse(const Personaje& personaje);

    // Al irse: la lista de jugadores a todos.
    void al_desconectarse();
};

#endif
