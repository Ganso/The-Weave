#include "globals.h"

// Launch callback
void player_hide_launch(void)
{
    bool cancelled_thunder = false;

    // If an enemy thunder is active, cancel it and show a hint
    if (combat_state == COMBAT_STATE_ENEMY_EFFECT &&
        combatContext.activePattern == PATTERN_EN_THUNDER)
    {
        cancelled_thunder = true;
        show_or_hide_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][A1D3_GOOD_IDEA_HIDE]);
        show_or_hide_interface(true);
        enemy_thunder_cancel(combatContext.activeEnemy);
    }
    else if (combatContext.activeEnemy != ENEMY_NONE)
    {
        cancel_enemy_pattern(combatContext.activeEnemy);
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
