#ifndef CONTROLADOR_DE_TRAMPAS_H
#define CONTROLADOR_DE_TRAMPAS_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "../jugadores/personaje.h"
#include "../notificador.h"
#include "../sector/gestor_de_sectores.h"

// Los pinches de la mazmorra se activan por tiempo. En cada tick avanza su ciclo, avisa con
// TrampaActualizada los que salieron o se guardaron y le baja la vida a quien esté parado encima.
class ControladorDeTrampas {
private:
    GestorDeSectores& sectores;
    std::map<uint32_t, Personaje>& personajes;
    Notificador& notificador;

    // Id y casilla de los jugadores del sector que las trampas pueden lastimar.
    std::vector<std::pair<uint32_t, Casilla>> jugadores_lastimables(const Sector& sector) const;
    void avisar_cambios(Sector& sector, const std::vector<int>& cambiadas);
    void aplicar_golpes(Sector& sector, const std::vector<GolpeDeTrampa>& golpes);
    void actualizar_sector(Sector& sector);

public:
    ControladorDeTrampas(
        GestorDeSectores& sectores, std::map<uint32_t, Personaje>& personajes,
        Notificador& notificador);

    void actualizar();
};

#endif
