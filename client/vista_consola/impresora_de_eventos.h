#ifndef IMPRESORA_DE_EVENTOS_H
#define IMPRESORA_DE_EVENTOS_H

#include <atomic>
#include <string>

#include "./../../common/mensajes/mensajes.h"
#include "./../../common/thread.h"
#include "./../red/conexion_con_servidor.h"

class ImpresoraDeEventos : public Thread {
private:
    ConexionConServidor& conexion;
    std::atomic<bool> conexion_cortada;
    std::atomic<bool> dentro_de_partida;
    EntradaExitosaAPartida partida{};

    void run() override;

    void registrar_fase(const EventoServidor& evento);

    static void escribir(const std::string& texto);

public:
    explicit ImpresoraDeEventos(ConexionConServidor& conexion_conectada);

    bool conexion_perdida() const;

    bool en_partida() const;

    EntradaExitosaAPartida partida_actual() const;

    ImpresoraDeEventos(const ImpresoraDeEventos&) = delete;
    ImpresoraDeEventos& operator=(const ImpresoraDeEventos&) = delete;
};

#endif
