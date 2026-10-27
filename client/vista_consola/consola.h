#ifndef CONSOLA_H
#define CONSOLA_H

#include "./../../common/mensajes/mensajes.h"
#include "./../red/conexion_con_servidor.h"
#include "interprete_de_comandos.h"

class Consola {
private:
    ConexionConServidor& conexion;
    InterpreteDeComandos interprete;

    bool imprimir_eventos(EntradaExitosaAPartida& partida);

    bool atender_entrada();

public:
    explicit Consola(ConexionConServidor& conexion_conectada);

    bool ejecutar_lobby(EntradaExitosaAPartida& partida);

    Consola(const Consola&) = delete;
    Consola& operator=(const Consola&) = delete;
};

#endif
