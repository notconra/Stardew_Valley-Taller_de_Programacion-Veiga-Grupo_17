#ifndef CONTROLADOR_DE_CULTIVOS_H
#define CONTROLADOR_DE_CULTIVOS_H

#include <cstdint>
#include <optional>

#include "../../../common/mensajes/mensajes.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// El paso del tiempo para las plantas de todos los sectores. Mira la hora del ControladorDelDia:
// cuando cambia, abre y cierra los turnos de riego, y cuando empieza un día nuevo hace crecer lo
// que se regó (ver Huerta).
class ControladorDeCultivos {
private:
    GestorDeSectores& sectores;
    Notificador& notificador;
    std::optional<uint32_t> dia_actual;
    int minuto_actual = -1;

    // Minutos de juego desde las 6 am (de 0 a 1200).
    int minutos_desde_el_inicio(const HoraDelJuego& hora) const;

public:
    ControladorDeCultivos(GestorDeSectores& sectores, Notificador& notificador);

    // Se llama en cada tick, después de que avanzó el reloj.
    void actualizar(const HoraDelJuego& hora);
};

#endif
