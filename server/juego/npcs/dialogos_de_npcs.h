#ifndef DIALOGOS_DE_NPCS_H
#define DIALOGOS_DE_NPCS_H

#include <map>
#include <string>
#include <vector>

#include "rutinas_de_npcs.h"

// Las tres frases que un NPC dice durante un dia: una por cada momento de la jornada.
// Se sortean una sola vez al empezar el dia, asi que no cambian hasta el dia siguiente.
struct FrasesDelDia {
    std::string antes_de_trabajar;
    std::string trabajando;
    std::string despues_de_trabajar;

    // La frase que corresponde a ese momento. Vacia si el NPC no tiene frases.
    const std::string& para(MomentoDelDia momento) const;
};

// Las frases de todos los NPC, escritas en dialogos_de_npcs.cpp.
//
// Cada NPC tiene tres listas: lo que puede decir antes de ir a trabajar, mientras trabaja y
// despues de trabajar.
class DialogosDeNpcs {
private:
    struct FrasesPosibles {
        std::vector<std::string> antes_de_trabajar;
        std::vector<std::string> trabajando;
        std::vector<std::string> despues_de_trabajar;
    };

    std::map<std::string, FrasesPosibles> frases_por_npc;

    std::string sortear(const std::vector<std::string>& opciones) const;

public:
    // Carga las frases de todos los NPC.
    DialogosDeNpcs();

    // Elige al azar una frase de cada lista del NPC. Si el NPC no existe, las tres quedan vacias.
    FrasesDelDia sortear_frases_del_dia(const std::string& id_npc) const;
};

#endif
