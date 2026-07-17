#ifndef _PLAYER_SPELLS_H_
#define _PLAYER_SPELLS_H_

// Registro de los hechizos del JUGADOR. Cada hechizo vive en su propio fichero
// (spells/player/<nombre>.c con su <nombre>_init); esta función los registra
// todos en spell_defs[]. La llama init_spells() (spell.c).

void init_player_spells(void);

#endif
