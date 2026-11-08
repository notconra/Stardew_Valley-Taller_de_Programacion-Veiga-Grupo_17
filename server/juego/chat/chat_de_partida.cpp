#include "chat_de_partida.h"

#include <cctype>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

ChatDePartida::ChatDePartida(Notificador& notificador_de_la_partida)
    : notificador(notificador_de_la_partida) {
}

void ChatDePartida::conectar(
    uint32_t id_jugador, const std::string& nombre_jugador, ColorDeJugador color) {
    const bool es_nuevo = jugadores.find(id_jugador) == jugadores.end();
    jugadores[id_jugador] = JugadorEnChat{nombre_jugador, color, std::string()};
    if (es_nuevo) {
        avisar_a_los_demas(id_jugador, nombre_jugador + " se conectó.");
    }
}

void ChatDePartida::desconectar(uint32_t id_jugador) {
    auto jugador = jugadores.find(id_jugador);
    if (jugador == jugadores.end()) {
        return;
    }
    const std::string nombre = jugador->second.nombre;
    jugadores.erase(jugador);
    avisar_a_los_demas(id_jugador, nombre + " se desconectó.");
}

void ChatDePartida::avisar_a_los_demas(uint32_t id_excluido, const std::string& texto) {
    for (const auto& [id_jugador, jugador] : jugadores) {
        if (id_jugador != id_excluido) {
            enviar_sistema(id_jugador, texto);
        }
    }
}

void ChatDePartida::iniciar_conversacion(
    uint32_t id_jugador, const std::string& nombre_interlocutor) {
    auto jugador = jugadores.find(id_jugador);
    if (jugador != jugadores.end()) {
        jugador->second.interlocutor = nombre_interlocutor;
    }
}

void ChatDePartida::terminar_conversacion(uint32_t id_jugador) {
    auto jugador = jugadores.find(id_jugador);
    if (jugador != jugadores.end()) {
        jugador->second.interlocutor.clear();
    }
}

std::string ChatDePartida::interlocutor_de(uint32_t id_jugador) const {
    auto jugador = jugadores.find(id_jugador);
    return jugador == jugadores.end() ? std::string() : jugador->second.interlocutor;
}

void ChatDePartida::agregar_comando(std::unique_ptr<ComandoDeChat> comando) {
    if (comando) {
        comandos.push_back(std::move(comando));
    }
}

void ChatDePartida::recibir(uint32_t id_jugador, const EnviarMensajeDeChat& mensaje) {
    auto emisor = jugadores.find(id_jugador);
    if (emisor == jugadores.end()) {
        return;
    }

    const std::string texto = sin_espacios_extremos(mensaje.texto);
    if (texto.empty()) {
        return;
    }
    if (texto.size() > LARGO_MAXIMO_DE_MENSAJE_DE_CHAT) {
        enviar_sistema(
            id_jugador,
            "El mensaje es demasiado largo: el máximo es " +
                std::to_string(LARGO_MAXIMO_DE_MENSAJE_DE_CHAT) + " caracteres.");
        return;
    }

    if (texto.front() == '/') {
        ejecutar_comando(id_jugador, emisor->second, texto);
        return;
    }
    if (texto.front() == '@') {
        const size_t separador = texto.find_first_of(" \t\n\r", 1);
        const std::string destinatario =
            texto.substr(1, separador == std::string::npos ? std::string::npos : separador - 1);
        const std::string cuerpo = separador == std::string::npos
                                       ? std::string()
                                       : sin_espacios_extremos(texto.substr(separador + 1));
        if (destinatario.empty()) {
            enviar_sistema(id_jugador, "Escribí el nombre del jugador después de la arroba.");
            return;
        }
        if (cuerpo.empty()) {
            enviar_sistema(id_jugador, "Escribí un mensaje después del nombre del jugador.");
            return;
        }
        enviar_privado(id_jugador, emisor->second, destinatario, cuerpo);
        return;
    }

    enviar_global(emisor->second, texto);
}

void ChatDePartida::enviar_global(const JugadorEnChat& emisor, const std::string& texto) {
    notificador.a_todos(
        MensajeDeChat{CanalDeChat::GLOBAL, emisor.nombre, std::string(), texto, emisor.color});
}

void ChatDePartida::enviar_privado(
    uint32_t id_emisor, const JugadorEnChat& emisor, const std::string& destinatario,
    const std::string& texto) {
    const JugadoresEnChat::const_iterator destino = buscar_por_nombre(destinatario);
    if (destino == jugadores.end()) {
        enviar_sistema(
            id_emisor,
            "No se pudo enviar el mensaje: \"" + destinatario + "\" no está conectado.");
        return;
    }

    const MensajeDeChat mensaje{
        CanalDeChat::PRIVADO,
        emisor.nombre,
        destino->second.nombre,
        texto,
        emisor.color};
    notificador.a_jugador(id_emisor, mensaje);
    if (destino->first != id_emisor) {
        notificador.a_jugador(destino->first, mensaje);
    }
}

void ChatDePartida::enviar_sistema(uint32_t id_jugador, const std::string& texto) {
    notificador.a_jugador(
        id_jugador,
        MensajeDeChat{CanalDeChat::SISTEMA, "Sistema", std::string(), texto});
}

void ChatDePartida::responder_a_jugador(uint32_t id_jugador, const std::string& texto) {
    enviar_sistema(id_jugador, texto);
}

void ChatDePartida::responder_como_comerciante(
    uint32_t id_jugador, const std::string& nombre_comerciante, const std::string& texto) {
    notificador.a_jugador(
        id_jugador,
        MensajeDeChat{CanalDeChat::COMERCIANTE, nombre_comerciante, std::string(), texto});
}

void ChatDePartida::ejecutar_comando(
    uint32_t id_jugador, const JugadorEnChat& emisor, const std::string& texto) {
    std::vector<std::string> partes = separar_argumentos(texto.substr(1));
    if (partes.empty()) {
        enviar_sistema(id_jugador, "Falta el nombre del comando después de la barra.");
        return;
    }

    const std::string palabra_clave = a_minusculas(partes.front());
    partes.erase(partes.begin());

    for (const std::unique_ptr<ComandoDeChat>& comando : comandos) {
        if (!comando->reconoce(palabra_clave)) {
            continue;
        }
        try {
            comando->ejecutar(ContextoDeComandoDeChat{
                id_jugador,
                emisor.nombre,
                palabra_clave,
                partes,
                emisor.interlocutor,
                *this});
        } catch (const std::exception&) {
            enviar_sistema(id_jugador, "No se pudo ejecutar /" + palabra_clave + ".");
        }
        return;
    }

    enviar_sistema(id_jugador, "Comando desconocido: /" + palabra_clave + ".");
}

JugadoresEnChat::const_iterator ChatDePartida::buscar_por_nombre(
    const std::string& nombre_buscado) const {
    const std::string objetivo = a_minusculas(nombre_buscado);
    for (JugadoresEnChat::const_iterator jugador = jugadores.begin(); jugador != jugadores.end();
         ++jugador) {
        if (a_minusculas(jugador->second.nombre) == objetivo) {
            return jugador;
        }
    }
    return jugadores.end();
}

std::string ChatDePartida::sin_espacios_extremos(const std::string& texto) {
    const size_t inicio = texto.find_first_not_of(" \t\n\r");
    if (inicio == std::string::npos) {
        return std::string();
    }
    const size_t fin = texto.find_last_not_of(" \t\n\r");
    return texto.substr(inicio, fin - inicio + 1);
}

std::string ChatDePartida::a_minusculas(const std::string& texto) {
    std::string resultado = texto;
    for (char& caracter : resultado) {
        caracter = static_cast<char>(std::tolower(static_cast<unsigned char>(caracter)));
    }
    return resultado;
}

std::vector<std::string> ChatDePartida::separar_argumentos(const std::string& texto) {
    std::vector<std::string> argumentos;
    size_t posicion = 0;
    while (posicion < texto.size()) {
        posicion = texto.find_first_not_of(" \t\n\r", posicion);
        if (posicion == std::string::npos) {
            break;
        }
        size_t fin = texto.find_first_of(" \t\n\r", posicion);
        if (fin == std::string::npos) {
            fin = texto.size();
        }
        argumentos.push_back(texto.substr(posicion, fin - posicion));
        posicion = fin;
    }
    return argumentos;
}
