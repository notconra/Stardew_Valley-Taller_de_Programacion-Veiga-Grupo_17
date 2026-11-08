#ifndef MODELO_DE_CHAT_H
#define MODELO_DE_CHAT_H

#include <cstddef>
#include <deque>
#include <string>

#include "../../../common/mensajes/mensajes_partida.h"

// Estado del chat del lado del cliente, sin SDL ni Qt. La vista solo lo consulta y le avisa lo que
// hace el usuario. Guarda:
//  - los mensajes recibidos, acotados para no crecer sin límite (se descartan los más viejos),
//  - cuántos no se leyeron (suma solo con el chat cerrado y se pone en cero al cerrarlo),
//  - si el jugador está escribiendo y el texto que lleva,
//  - cuánto subió el historial con la rueda (0 es "pegado al último mensaje").
//
// El historial es de la partida actual, al cambiar de partida o volver al lobby, se llama a
// limpiar().
class ModeloDeChat {
public:
    explicit ModeloDeChat(size_t maximo_de_mensajes);

    void recibir(const MensajeDeChat& mensaje);

    // Con mensajes sin leer, deja el primero de ellos arriba de la ventana; si entran todos, queda
    // pegada al último. "mensajes_visibles" es cuántos mensajes, contando desde el primero sin
    // leer, entran en el panel. Lo calcula la vista porque depende de cuánto mide cada uno.
    void abrir(size_t mensajes_visibles);
    void cerrar();
    bool esta_abierto() const;
    size_t cantidad_sin_leer() const;

    // Borra historial, texto en edición, contador y desplazamiento. Para empezar otra partida.
    void limpiar();

    // Historial en orden de llegada.
    const std::deque<MensajeDeChat>& mensajes() const;

    // Mientras esta_escribiendo() sea true, el juego tiene que ignorar el movimiento y las
    // acciones.
    void empezar_a_escribir();
    void terminar_de_escribir();
    bool esta_escribiendo() const;
    const std::string& texto_en_edicion() const;

    // Agrega texto tal como llega de SDL (UTF-8). Ignora lo que no entre en
    // LARGO_MAXIMO_DE_MENSAJE_DE_CHAT.
    void agregar_texto(const std::string& texto_nuevo);
    void borrar_caracter();

    // Devuelve el texto sin espacios en los extremos y lo borra. Cadena vacía si no había nada.
    std::string confirmar_y_limpiar();

    // Un valor positivo sube (mensajes más viejos) y uno negativo baja. El resultado se acota a
    // [0, desplazamiento_maximo], que calcula la vista.
    void desplazar(int mensajes, size_t desplazamiento_maximo);
    void ir_al_final();
    size_t desplazamiento() const;

private:
    size_t maximo_de_mensajes;
    std::deque<MensajeDeChat> historial;
    size_t mensajes_sin_leer = 0;
    bool abierto = false;
    bool escribiendo = false;
    std::string texto;
    size_t posicion_del_historial = 0;
};

#endif
