// player_spells.c — registrador de los hechizos del jugador.
// Cada hechizo se define en su propio fichero (spells/player/<nombre>.c);
// aquí solo se llaman sus *_init() en orden. Para AÑADIR un hechizo: crea
// spells/player/<nombre>.{c,h} con su <nombre>_init(), su SPELL_* en
// constants_spells.h, y una línea aquí.

#include <genesis.h>
#include "spells/player/player_spells.h"
#include "spells/player/thunder.h"
#include "spells/player/hide.h"
#include "spells/player/open.h"
#include "spells/player/sleep.h"
#include "spells/player/heal.h"
#include "spells/player/fire.h"
#include "spells/player/light.h"

void init_player_spells(void)
{
    thunder_init();   // TRUENO (combate; su invertido es el counter)
    hide_init();      // ESCONDER (palíndromo)
    open_init();      // ABRIR (solo guion)
    sleep_init();     // DORMIR (solo guion; la nana)
    heal_init();      // CURACIÓN (solo guion; la canta Clio)
    fire_init();      // FUEGO (ejemplo end-to-end: zona + fases)
    light_init();     // LUZ (showcase de fases; test)
}
