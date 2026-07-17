#ifndef _ENEMY_BITE_H_
#define _ENEMY_BITE_H_

// MORDISCO ENEMIGO — no counterable; al terminar golpea al jugador. En el
// JUEGO el jabalí muerde por contacto (combat/melee.c); esta versión de patrón
// solo la usa la clase de TEST.

void en_bite_init(void); // Registra EN_BITE en spell_defs[] (lo llama init_enemy_spell_defs)

#endif
