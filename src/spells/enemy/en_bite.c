// en_bite.c — MORDISCO enemigo (patrón): datos + hooks.
// El jabalí del juego muerde por CONTACTO (combat/melee.c) y no usa patrones;
// este EN_BITE de patrón lo usa solo la clase de TEST (ENEMY_CLS_TESTGHOST).

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "spells/enemy/en_bite.h"

static void en_bite_on_finish(SpellContext *ctx)    // fin natural: el mordisco alcanza al jugador
{
    SPR_setAnim(spr_enemy[ctx->enemyId], ANIM_IDLE);
    hit_player(1);
}

void en_bite_init(void)
{
    spell_defs[SPELL_EN_BITE] = (SpellDef){
        .id = SPELL_EN_BITE,
        .notes = { NOTE_MI, NOTE_SOL, NOTE_DO }, .noteCount = 3,
        .isPalindrome = false,
        .counterable = false,
        .baseDuration = SCREEN_FPS,
        .rechargeInit = SCREEN_FPS * 2,
        .onFinish = en_bite_on_finish,
    };
}
