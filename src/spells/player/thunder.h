#ifndef _PLAYER_THUNDER_H_
#define _PLAYER_THUNDER_H_

// TRUENO — hechizo de combate del jugador. Su forma invertida contrarresta el
// trueno enemigo (counter). Efecto de rayo: en el bosque (paleta forest_dark)
// el fondo PARPADEA con la paleta clara (forest.pal) y el cielo con blanco.

void thunder_init(void); // Registra TRUENO en spell_defs[] (lo llama init_player_spells)

#endif
