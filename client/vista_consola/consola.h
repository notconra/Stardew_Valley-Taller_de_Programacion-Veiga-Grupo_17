#ifndef CONSOLA_H
#define CONSOLA_H

#include "./../../common/mensajes/mensajes.h"
#include "./../red/conexion_con_servidor.h"
#include "impresora_de_eventos.h"
#include "interprete_de_comandos.h"

// Interfaz de terminal del cliente. TEMPORAL: se borra cuando estén las vistas de Qt y SDL.
class Consola {
private:
    ConexionConServidor& conexion;
    InterpreteDeComandos interprete;
    ImpresoraDeEventos impresora;

    bool leer_comandos(bool en_partida);

public:
    explicit Consola(ConexionConServidor& conexion_conectada);

    ~Consola();

    bool ejecutar_lobby(EntradaExitosaAPartida& partida);

    bool ejecutar_partida(const EntradaExitosaAPartida& partida);

    Consola(const Consola&) = delete;
    Consola& operator=(const Consola&) = delete;
};

#endif
