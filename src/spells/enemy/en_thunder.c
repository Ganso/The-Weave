// en_thunder.c — TRUENO enemigo: datos + hooks.

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "spells/enemy/en_thunder.h"

#define COLOR_INITIAL_SKY     RGB24_TO_VDPCOLOR(0x1585c2) // color inicial del cielo
#define COLOR_ENEMY_FLASH     RGB24_TO_VDPCOLOR(0x4444a3) // color de flash del thunder enemigo

static bool en_thunder_flash_on; // true = el CRAM está en color de flash

static void en_thunder_on_launch(SpellContext *ctx)
{
    dprintf(2, "Enemy %d: Thunder spell launched", ctx->enemyId);
    en_thunder_flash_on = false;
    SPR_setAnim(spr_enemy[ctx->enemyId], ANIM_ACTION);
}

static bool en_thunder_on_update(SpellContext *ctx)
{
    if (!en_thunder_flash_on) {                    // primer frame del flash
        PAL_setColor(PAL0_COL4, COLOR_ENEMY_FLASH);
        play_spell_jingle(SPELL_EN_THUNDER);
        en_thunder_flash_on = true;
    } else if ((frame_counter & 1) == 0) {         // alternar cada 2 frames
        u16 col = PAL_getColor(PAL0_COL4);
        PAL_setColor(PAL0_COL4, (col == COLOR_ENEMY_FLASH) ? COLOR_INITIAL_SKY
                                                           : COLOR_ENEMY_FLASH);
    }

    // El fin natural lo detecta el motor por baseDuration; los efectos del final van en onFinish
    (void)ctx;
    return false;
}

static void en_thunder_on_finish(SpellContext *ctx)    // fin natural: el rayo alcanza al jugador
{
    PAL_setColor(PAL0_COL4, COLOR_INITIAL_SKY);    // restaurar cielo
    en_thunder_flash_on = false;
    SPR_setAnim(spr_enemy[ctx->enemyId], ANIM_IDLE);
    hit_player(1);
}

static void en_thunder_on_counter(SpellContext *ctx)    // contrarrestado: el rayo vuelve al lanzador
{
    dprintf(2, "Enemy %d: Thunder countered", ctx->enemyId);
    PAL_setColor(PAL0_COL4, COLOR_INITIAL_SKY);
    en_thunder_flash_on = false;
    hit_enemy(ctx->enemyId, 1);
}

static void en_thunder_on_cancel(SpellContext *ctx)    // cancelado (hide, rechazo): solo limpiar
{
    dprintf(2, "Enemy %d: Thunder cancelled", ctx->enemyId);
    (void)ctx;
    PAL_setColor(PAL0_COL4, COLOR_INITIAL_SKY);
    en_thunder_flash_on = false;
}

void en_thunder_init(void)
{
    spell_defs[SPELL_EN_THUNDER] = (SpellDef){
        .id = SPELL_EN_THUNDER,
        .notes = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_SOL }, .noteCount = 4,
        .isPalindrome = false,
        .counterable = true,
        .baseDuration = SCREEN_FPS,              // 1 segundo de efecto
        .rechargeInit = SCREEN_FPS * 3,          // 3 s antes del primer lanzamiento
        .onLaunch  = en_thunder_on_launch,
        .onUpdate  = en_thunder_on_update,
        .onFinish  = en_thunder_on_finish,
        .onCounter = en_thunder_on_counter,
        .onCancel  = en_thunder_on_cancel,
    };
}
