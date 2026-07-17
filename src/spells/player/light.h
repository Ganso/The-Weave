#ifndef _PLAYER_LIGHT_H_
#define _PLAYER_LIGHT_H_

// LUZ — hechizo de test/demostración: el primero definido casi íntegramente por
// FASES declarativas (dos flashes de paleta encadenados), y el primero casteable
// por el jugador tanto DIRECTO como INVERTIDO (base del puzzle 2 de act1_test).
// No está desbloqueado en el juego; lo habilita el setup de la escena de test.

void light_init(void); // Registra LUZ en spell_defs[] (lo llama init_spells)

#endif
