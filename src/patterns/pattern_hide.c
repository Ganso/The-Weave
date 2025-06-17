#include "globals.h"

// Launch callback
void player_hide_launch(void)
{
    bool cancelled_thunder = false;

    dprintf(2,"Hide spell launched");

    // If an enemy thunder was active, cancel it and show a hint
    if (get_last_enemy_pattern() == PATTERN_EN_THUNDER)
    {
        dprintf(2,"Thunder spell cancelled by hide");
        cancelled_thunder = true;
        show_or_hide_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][A1D3_GOOD_IDEA_HIDE]);
        show_or_hide_interface(true);
        enemy_thunder_cancel(combatContext.activeEnemy);
        combat_state = COMBAT_STATE_PLAYER_EFFECT; // Resume hide effect
        obj_character[active_character].state = STATE_PATTERN_EFFECT;
    }
    else if (combatContext.activeEnemy != ENEMY_NONE)
    {
        dprintf(2,"Some spell cancelled by hide");
        cancel_enemy_pattern(combatContext.activeEnemy);
        combat_state = COMBAT_STATE_PLAYER_EFFECT; // Resume hide effect
        obj_character[active_character].state = STATE_PATTERN_EFFECT;
    }

    // Restore active pattern to HIDE in case it was cleared
    combatContext.activePattern = PATTERN_HIDE;
    combatContext.activeEnemy   = ENEMY_NONE;

    // Start invisible
    SPR_setVisibility(spr_chr[active_character], HIDDEN);

    play_player_pattern_sound(PATTERN_HIDE);
}

// Update callback
bool player_hide_update(void)
{
    const u16 duration = playerPatterns[PATTERN_HIDE].baseDuration;

    // Toggle visibility every frame
    bool visible = (combatContext.effectTimer & 1) != 0;
    SPR_setVisibility(spr_chr[active_character], visible ? VISIBLE : HIDDEN);

    if (combatContext.effectTimer >= duration)
    {
        SPR_setVisibility(spr_chr[active_character], VISIBLE);
        return true;        // finished
    }
    return false;           // keep running
}

// Check if the pattern can be used
bool player_hide_can_use(void)
{
    if (combatContext.patternReversed) return false; // Not allowed reversed

    // Cannot use if already hiding
    if (combat_state == COMBAT_STATE_PLAYER_EFFECT &&
        combatContext.activePattern == PATTERN_HIDE)
        return false;

    return true;
}
