// player_spells.c — thunder, hide, open y sleep: datos + hooks.
// Cada hechizo es una entrada en spell_defs[] con lo mínimo imperativo en hooks.

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "res_all.h"
#include "narrative/narrative.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "spells/player_spells.h"
#include "spells/enemy_spells.h"

// =====================================================================
// THUNDER — flash blanco; invertido contrarresta el thunder enemigo
// =====================================================================

#define COLOR_WHITE_VDP  RGB24_TO_VDPCOLOR(0xFFFFFF)

static u16 thunder_saved_color; // color original en CRAM antes del flash

static bool thunder_can_use(const SpellContext *ctx)
{
    // Invertido: solo permitido para contrarrestar el thunder enemigo en su ventana
    if (ctx->reversed)
        return (combat_state == COMBAT_STATE_ENEMY_EFFECT) &&
               (spell_active_id(SPELL_SLOT_ENEMY) == SPELL_EN_THUNDER);

    // Directo: rechazado si el enemigo actual es un WeaverGhost (el hint lo da onRejected)
    if (ctx->enemyId != ENEMY_NONE &&
        obj_enemy[ctx->enemyId].class_id == ENEMY_CLS_WEAVERGHOST)
        return false;

    return true;
}

// Hint contextual al rechazar thunder (B18: antes vivía incrustado en el flujo de notas)
static void thunder_on_rejected(SpellContext *ctx)
{
    // ¿Hay un espectro (WeaverGhost) en pantalla? Solo entonces damos pistas.
    bool ghost_present = false;
    for (u8 i = 0; i < MAX_ENEMIES; i++)
        if (obj_enemy[i].obj_character.active &&
            obj_enemy[i].class_id == ENEMY_CLS_WEAVERGHOST) { ghost_present = true; break; }

    if (ctx->reversed)
    {
        // Trueno invertido fuera de su ventana de counter: si hay espectros,
        // la pista de esperar el momento; si no, el hechizo falla y ya está
        // (el sonido de patrón inválido basta como feedback).
        if (ghost_present)
            talk_dialog(&dialogs[ACT1_FOREST][A1_FOREST_REVERSE_HINT], false); // pista: espera su momento
        else
            play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
        return;
    }

    // Trueno directo rechazado (solo pasa contra espectros): pista del counter
    if (ghost_present)
        talk_dialog(&dialogs[ACT1_FOREST][A1_FOREST_REVERSE_NOTES_COUNTER], false); // pista: tócalo al revés
    else
        talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN], false);
}

static void thunder_on_launch(SpellContext *ctx)
{
    (void)ctx;
    thunder_saved_color = PAL_getColor(PAL0_COL4);   // guardar color original
    PAL_setColor(PAL0_COL4, COLOR_WHITE_VDP);        // primer frame: flash inmediato
    play_spell_jingle(SPELL_THUNDER);
}

static bool thunder_on_update(SpellContext *ctx)
{
    // Alterna cada frame entre blanco y el color original
    const bool evenFrame  = (ctx->frameCounter & 1) == 0;
    PAL_setColor(PAL0_COL4, evenFrame ? COLOR_WHITE_VDP : thunder_saved_color);

    if (ctx->frameCounter >= spell_defs[SPELL_THUNDER].baseDuration)
    {
        PAL_setColor(PAL0_COL4, thunder_saved_color); // restaurar exactamente
        dprintf(2, "Thunder done: restoring color");
        return true;
    }
    return false;
}

static void thunder_on_cancel(SpellContext *ctx)    // cancelado a mitad: restaurar el cielo
{
    (void)ctx;
    PAL_setColor(PAL0_COL4, thunder_saved_color);
}

// =====================================================================
// HIDE — el jugador parpadea invisible; corta el hechizo enemigo en curso
// =====================================================================

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

// =====================================================================
// OPEN y SLEEP — aún sin efecto propio (solo scripted); canUse siempre false
// =====================================================================

static bool scripted_only_can_use(const SpellContext *ctx)
{
    (void)ctx;
    return false; // solo se usan desde cutscenes (origin NARRATIVE, Fase 5)
}

// =====================================================================
// Registro en la tabla
// =====================================================================

void init_player_spells(void)    // Registra los hechizos base (llamado desde init_spells)
{
    spell_defs[SPELL_THUNDER] = (SpellDef){
        .id = SPELL_THUNDER,
        .notes = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_SOL }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 4,          // 4 segundos
        .enabled = false,
        .canUse = thunder_can_use, .onRejected = thunder_on_rejected,
        .onLaunch = thunder_on_launch, .onUpdate = thunder_on_update,
        .onCancel = thunder_on_cancel,
    };

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

    spell_defs[SPELL_OPEN] = (SpellDef){
        .id = SPELL_OPEN,
        .notes = { NOTE_FA, NOTE_SI, NOTE_SOL, NOTE_DO }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 3 / 4,      // era 45 frames NTSC
        .enabled = false,
        .canUse = scripted_only_can_use,         // sin hooks de efecto: auto-fin por duración
    };

    spell_defs[SPELL_SLEEP] = (SpellDef){
        .id = SPELL_SLEEP,
        .notes = { NOTE_FA, NOTE_MI, NOTE_DO, NOTE_LA }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 5 / 4,      // era 75 frames NTSC
        .enabled = false,
        .canUse = scripted_only_can_use,
    };

    // CURACIÓN: la canta Clio en el acto 1 (guión 4.3). Linus la anota como
    // Dormir, pero usa la nota más alta (DO), que queda fuera de su límite de
    // notas hasta el final del juego → solo guionizado por partida doble.
    spell_defs[SPELL_HEAL] = (SpellDef){
        .id = SPELL_HEAL,
        .notes = { NOTE_LA, NOTE_SI, NOTE_DO, NOTE_SI }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 5 / 4,
        .enabled = false,
        .canUse = scripted_only_can_use,
    };
}
