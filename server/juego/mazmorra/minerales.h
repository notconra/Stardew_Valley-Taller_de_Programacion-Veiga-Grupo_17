#ifndef MINERALES_H
#define MINERALES_H

#include <vector>

#include "../../configuracion/configuracion_servidor_datos.h"
#include "../objetos_del_mundo/objetos_del_mundo.h"

// Los minerales del juego, expresados como objetos del mundo. No hay una clase por mineral: cada
// uno es una fila de datos. Lo que se puede ajustar desde la configuración (golpes, frecuencia,
// nivel mínimo, cantidad, probabilidad de no dejar nada) viaja en ConfiguracionMineralServidor,
// lo que no (con qué se pica, qué ítem deja, cómo se ve) es una regla fija de acá.
//
// Si la configuración no trae minerales se usan los de por_defecto(). Si trae alguno, esos
// reemplazan a los de defecto. Las funciones lanzan std::runtime_error si hay una clave
// desconocida o repetida.
class Minerales {
public:
    static std::vector<ConfiguracionMineralServidor> por_defecto();

    // Siempre define los cinco minerales (los no configurados, con sus valores por defecto), así
    // el catálogo de objetos está completo aunque un mineral no aparezca en los niveles.
    static std::vector<DefinicionDeObjeto> definiciones(
        const std::vector<ConfiguracionMineralServidor>& configurados);

    // Frecuencia relativa de cada mineral en un nivel. Los pesos no tienen que sumar 100: la
    // probabilidad de cada tipo es su peso sobre el total. Un mineral antes de su "desde_nivel"
    // tiene peso cero y nunca se sortea.
    static std::vector<PesoDeObjeto> pesos(
        int numero_de_nivel, const std::vector<ConfiguracionMineralServidor>& configurados);
};

#endif
