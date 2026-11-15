#include "dialogos_de_npcs.h"

#include <cstddef>
#include <string>
#include <vector>

#include "../azar.h"

const std::string& FrasesDelDia::para(MomentoDelDia momento) const {
    if (momento == MomentoDelDia::ANTES_DE_TRABAJAR) {
        return antes_de_trabajar;
    }
    if (momento == MomentoDelDia::TRABAJANDO) {
        return trabajando;
    }
    return despues_de_trabajar;
}

DialogosDeNpcs::DialogosDeNpcs() {
    FrasesPosibles pierre;
    pierre.antes_de_trabajar = {
        "Buen dia! A las 8 abro mi puesto en el mercado, pasate a comprar un par de semillas.",
        "Madrugar rinde: regar temprano evita que se te pase el turno de riego. ¿Ya regaste tus "
        "cultivos?",
        "Antes de que abra el puesto, fijate que tus plantas tengan agua. Regar a tiempo es la "
        "clave para obtener mejores resultados. Los cultivos de mejor calidad se venden a otro "
        "precio.",
    };
    pierre.trabajando = {
        "Hola! Si buscas semillas, llegaste al lugar indicado.",
        "Un riego tardio resta un poco de calidad y un riego perdido resta el doble. Ojo con los "
        "turnos, hay que saber administrar el tiempo.",
        "Cosecha apenas madure. Dejar un cultivo listo esperando podría bajar la calidad de la "
        "cosecha.",
        "Las cosechas de mejor calidad se pagan mas caro. Vale la pena cuidarlas.",
        "Ya visitaste el centro civico? A lo mejor podes dar una mano y ayudar a restaurarlo.",
    };
    pierre.despues_de_trabajar = {
        "Fue un dia largo. Mañana abrimos a la misma hora.",
        "Se esta haciendo tarde, acordate de dormir en tu cama antes de las 2am o te vas a "
        "desmayar de cansancio.",
        "El descanso es calve, dormirte muy tarde podría perjudicarte al dia siguiente.",
    };
    frases_por_npc["pierre"] = pierre;

    FrasesPosibles clint;
    clint.antes_de_trabajar = {
        "Buen dia! Antes de bajar a la mina, asegurate de llevar la espada y la energia llena.",
        "La mañana es ideal para bajar a la mina: tenes todo el dia por delante.",
        "Picar cansa mas de lo que parece. Mira tu energia antes de bajar.",
    };
    clint.trabajando = {
        "La mina esta llena de minerales:  carbon, lapislazuli, plata y oro. Si me los traes, yo "
        "te los "
        "compro.",
        "Con el pico rompes los minerales. Algunos aguantan varios golpes, no te rindas.",
        "Si te adentras en la mina, quizas encuentres cofres con oro. Vale la pena mirar cada "
        "rincon.",
        "Si bajas a la mina, debes tener cuidado con las trampas. Mira bien por donde pisas.",
        "Mucho cuidado al minar plata, dicen que podrías atraer una manada de orcos. Asegurate de "
        "estar preparado para pelear.",
        "Dicen que los orcos andan en grupo. Si ves uno, seguro hay mas cerca. No pelees rodeado.",
        "Se dice que los vampiros vienen de a dos. Cuando matas a uno, el otro podria enojarse, "
        "tene mucho cuidado.",
        "Escuche que los esqueletos se vuelven a levantar con el tiempo. No te quedes a festejar "
        "si llegas a matar a alguno.",
    };
    clint.despues_de_trabajar = {
        "Ya cerramos, toca descansar.",
        "Un minero cansado comete errores. Quizas no sea buena idea bajar a la mina a esta hora.",
        "Acordate: volve a tu cama antes de las 2 de la madrugada si queres que la energia te "
        "rinda el proximo dia.",
    };
    frases_por_npc["clint"] = clint;

    FrasesPosibles marnie;
    marnie.antes_de_trabajar = {
        "Buen dia! Es el mejor momento para ver si tus gallinas ya pusieron huevos.",
        "Las vacas se despiertan temprano. Fijate si alguna esta lista para ordenar.",
        "Una gallina te deja un huevo por dia en el suelo de la granja. Acordate de pasar a "
        "juntarlo.",
    };
    marnie.trabajando = {
        "Es cierto que en la mina hay cofres escondidos? Si se lo vendes a Clint, podrias "
        "comprarme "
        "animales.",
        "El centro civico esta viejo y descuidado, pero con un poco de trabajo seguramente podamos "
        "restaurarlo.",
    };
    marnie.despues_de_trabajar = {
        "Mis animales duermen de noche. Dejalos descansar, manana vuelven a andar por la granja.",
    };
    frases_por_npc["marnie"] = marnie;

    FrasesPosibles robin;
    robin.antes_de_trabajar = {
        "Buen dia! Ya armaste tu casa? Una cama y un cofre alcanzan para empezar.",
        "Madrugaste! Un buen dia de trabajo empieza con las manos listas.",
        "Tu ultima cama es donde vas a despertar. Elegila bien.",
    };
    robin.trabajando = {
        "Los cofres sirven para guardar lo que no entra en tu barra. No dejes tus tesoros "
        "tirados.",
        "Con clic derecho podes colocar muebles donde quieras. Si la casilla se ve roja, no "
        "entra.",
        "Antes de bajar a la mina, deja lo que no necesites en un cofre. Vas a volver mas "
        "liviano.",
        "Si te arrepentis de un mueble, proba retirarlo con el pico.",
        "Escuche que se obtienen recompensas increibles al ayudar con la restauracion del centro "
        "civico.",
        "Si buscas muebles para tu casa, mi puesto es el lugar indicado.",
        "Lo que tengo a la venta cambia con los dias. Volve a mirar cada tanto.",
    };
    robin.despues_de_trabajar = {
        "Con la tarde libre, quizas sea hora de acomodar tu casa.",
        "Para algo sirve una buena cama: anda a dormir antes de las 2.",
    };
    frases_por_npc["robin"] = robin;
}

std::string DialogosDeNpcs::sortear(const std::vector<std::string>& opciones) const {
    if (opciones.empty()) {
        return "";
    }
    const int ultimo = static_cast<int>(opciones.size()) - 1;
    return opciones[static_cast<std::size_t>(Azar::entero(0, ultimo))];
}

FrasesDelDia DialogosDeNpcs::sortear_frases_del_dia(const std::string& id_npc) const {
    FrasesDelDia frases;
    if (frases_por_npc.count(id_npc) == 0) {
        return frases;
    }
    const FrasesPosibles& posibles = frases_por_npc.at(id_npc);
    frases.antes_de_trabajar = sortear(posibles.antes_de_trabajar);
    frases.trabajando = sortear(posibles.trabajando);
    frases.despues_de_trabajar = sortear(posibles.despues_de_trabajar);
    return frases;
}
