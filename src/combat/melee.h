/*
 * src/combat/melee.h — Combate físico (sin hechizos)
 * ----------------------------------------------------
 * Combate cuerpo a cuerpo del acto 1 (Linus sin vara): los enemigos activos
 * persiguen al jugador y muerden de cerca; el jugador golpea con el botón A.
 * Cada golpe conectado ahuyenta al enemigo (corre a su punto de entrada y
 * vuelve). Al acumular hits_to_win golpes, todos huyen por la derecha y el
 * combate termina.
 *
 * No usa el FSM de combate por hechizos: combat_state queda en COMBAT_NO y
 * ni las notas ni el motor de hechizos intervienen (desactiva `spells` antes).
 *
 * melee_combat_run() es BLOQUEANTE (estilo cutscene, B5): toma el control de
 * todos los enemigos ya spawneados (init_enemy + move_enemy_instant + show
 * antes de llamar) y devuelve cuando todos se han ido.
 */
#ifndef _MELEE_H_
#define _MELEE_H_

#include <genesis.h>

// companion: personaje que se queda quieto detrás del jugador mirando a la
// derecha (se recoloca andando si no estaba detrás; al acabar vuelve a seguir).
// CHR_NONE si no hay.
void melee_combat_run(u8 hits_to_win, u16 companion);

// Variante del regreso (guión 6.1): el golpe físico NO cuenta; solo el patrón
// de TRUENO cantado entero ahuyenta a los jabalíes (habilitar spells antes).
void melee_combat_run_thunder(u8 casts_to_win, u16 companion);

#endif
