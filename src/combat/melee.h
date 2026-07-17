/*
 * src/combat/melee.h — Combate de CONTACTO (la manada)
 * ------------------------------------------------------
 * Director del combate contra enemigos de contacto (jabalíes): persiguen al
 * jugador, muerden de cerca y huyen cuando el arma del jugador los alcanza.
 * El arma se elige por configuración: el golpe físico (botón A) o el patrón
 * de TRUENO cantado (para escenas donde Linus ya tiene el bastón).
 *
 * Cómo funciona por dentro, con dibujos: docs/combat.md
 *
 * Uso: spawnear los enemigos (init_enemy + move_enemy_instant + show_enemy)
 * y llamar a melee_combat_run(), que BLOQUEA hasta la victoria (todos huyen)
 * o la derrota (player_defeated: la escena decide el reintento con el op
 * if_defeated del DSL).
 */
#ifndef _MELEE_H_
#define _MELEE_H_

#include <genesis.h>

typedef struct {
    u8   hits_to_win;          // golpes (o truenos) que ahuyentan a la manada
    u16  companion;            // CHR_* que espera quieto durante el combate (CHR_NONE si no hay)
    bool reposition_companion; // true: si no está detrás del jugador, se recoloca andando
    bool weapon_is_thunder;    // true: el TRUENO ahuyenta (habilitar spells en la escena);
                               // false: el golpe físico con A (deshabilitar spells)
} MeleeConfig;

void melee_combat_run(const MeleeConfig *cfg);

#endif
