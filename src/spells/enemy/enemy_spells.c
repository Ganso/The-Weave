// enemy_spells.c — registrador de los hechizos de enemigo.
// Cada hechizo se define en su propio fichero (spells/enemy/<nombre>.c);
// aquí solo se llaman sus *_init() en orden.

#include <genesis.h>
#include "spells/enemy/enemy_spells.h"
#include "spells/enemy/en_thunder.h"
#include "spells/enemy/en_bite.h"

void init_enemy_spell_defs(void)
{
    en_thunder_init();   // TRUENO enemigo (counterable)
    en_bite_init();      // MORDISCO (patrón; solo la clase de test)
}
