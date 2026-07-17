// hide.c — ESCONDER del jugador: datos + hooks.

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "narrative/narrative.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "spells/player/hide.h"

static bool hide_can_use(const SpellContext *ctx)
{
    if (ctx->reversed) return false;

    // No se puede si ya se está escondiendo
    if (combat_state == COMBAT_STATE_PLAYER_EFFECT &&
        spell_active_id(SPELL_SLOT_PLAYER) == SPELL_HIDE)
        return false;

    return true;
}

static void hide_on_launch(SpellContext *ctx)
{
    (void)ctx;
    dprintf(2,"Hide spell launched");

    // Si el thunder enemigo estaba activo, cancelarlo con una pista
    if (spell_active_id(SPELL_SLOT_ENEMY) == SPELL_EN_THUNDER)
    {
        show_or_hide_interface(false);
        talk_dialog(&dialogs[ACT1_FOREST][A1_FOREST_GOOD_IDEA_HIDE], false); // (ES) "Buena idea. Así no me verán" - (EN) "Good idea. They won't see me"
        show_or_hide_interface(true);
        spell_cancel(SPELL_SLOT_ENEMY);
    }
    else if (spell_slot_active(SPELL_SLOT_ENEMY))
    {
        dprintf(2,"Some enemy spell cancelled by hide");
        spell_cancel(SPELL_SLOT_ENEMY);
    }
    // (el motor restablece PLAYER_EFFECT después de este hook)

    SPR_setVisibility(spr_chr[active_character], HIDDEN); // empezar invisible
    play_spell_jingle(SPELL_HIDE);
}

static bool hide_on_update(SpellContext *ctx)
{
    // Parpadeo de visibilidad cada frame
    bool visible = (ctx->frameCounter & 1) != 0;
    SPR_setVisibility(spr_chr[active_character], visible ? VISIBLE : HIDDEN);

    if (ctx->frameCounter >= spell_defs[SPELL_HIDE].baseDuration)
    {
        SPR_setVisibility(spr_chr[active_character], VISIBLE);
        return true;
    }
    return false;
}

void hide_init(void)
{
    spell_defs[SPELL_HIDE] = (SpellDef){
        .id = SPELL_HIDE,
        .notes = { NOTE_FA, NOTE_SOL, NOTE_SOL, NOTE_FA }, .noteCount = 4,
        .isPalindrome = true,                    // FA SOL SOL FA: sin forma invertida
        .counterable = false,
        .baseDuration = SCREEN_FPS * 4,
        .enabled = false,
        .canUse = hide_can_use,
        .onLaunch = hide_on_launch, .onUpdate = hide_on_update,
    };
}
