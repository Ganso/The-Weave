#ifndef _ENEMY_SPELLS_H_
#define _ENEMY_SPELLS_H_

// Registro de los hechizos de ENEMIGO. Cada hechizo vive en su propio fichero
// (spells/enemy/<nombre>.c con su <nombre>_init); esta función los registra
// todos en spell_defs[]. La llama init_spells() (spell.c).

void init_enemy_spell_defs(void);

#endif
