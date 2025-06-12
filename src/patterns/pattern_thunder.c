#include "globals.h"

#define PAL_ENTRY(pal, col)   (((pal) << 4) | (col)) 
#define PAL0_COL4             PAL_ENTRY(0, 4)
#define COLOR_WHITE_VDP  RGB24_TO_VDPCOLOR(0xFFFFFF)

static u16 savedColor; // original colour in CRAM before the flash

// Launch callback
void playerThunderLaunch(void)
{
    // If the player is trying to attack a Ghost, give him a clue and return false
    if (combatContext.activeEnemy != ENEMY_NONE &&
        obj_enemy[combatContext.activeEnemy].class_id == ENEMY_CLS_WEAVERGHOST)
    {
        show_or_hide_interface(false); // Hide interface
        talk_dialog(&dialogs[ACT1_DIALOG3][A1D3_THINK_BACKWARDS]); // (ES) "Quizá deba pensar|al revés" - (EN) "I should maybe|think backwards"
        show_or_hide_interface(true);  // Show interface again
        cancelPlayerPattern(); // Cancel the pattern
        return;
    }

    // Save current CRAM entry so we can restore it later            */
    savedColor = PAL_getColor(PAL0_COL4); 
    dprintf(2, "Thunder: initial CRAM colour 0-4 = 0x%04X\n", savedColor);

    // First frame: set to white so the flash is immediate
    PAL_setColor(PAL0_COL4, COLOR_WHITE_VDP);
    combatContext.effectTimer = 0;

    // Sound effect
    playPlayerPatternSound(PATTERN_THUNDER); // play thunder sound
}

// Update callback
bool playerThunderUpdate(void)
{
    const u16 duration = playerPatterns[PATTERN_THUNDER].baseDuration;

    // Toggle each frame between white and the original color
    const bool evenFrame  = (combatContext.effectTimer & 1) == 0;
    const u16  flashColor = evenFrame ? COLOR_WHITE_VDP : savedColor;
    PAL_setColor(PAL0_COL4, flashColor);

    // Advance timer and test end-of-effect                          
    if (combatContext.effectTimer >= duration)
    {
        // Restore CRAM entry exactly as it was before the spell     
        PAL_setColor(PAL0_COL4, savedColor); 
        dprintf(2, "Thunder done: restoring color");
        return true; // pattern finished
    }
    return false;
}

// Check if the pattern can be used
bool playerThunderCanUse(void)
{
    dprintf(2, "Checking if Thunder can be used. Combat state: %d, active enemy: %d, pattern reversed: %d",
            combat_state, combatContext.activeEnemy, combatContext.patternReversed);
 
    // If reversed, allowed only to counter enemy thunder
    if (combatContext.patternReversed)
        return (combat_state == COMBAT_STATE_ENEMY_EFFECT) &&
               (combatContext.activePattern == PATTERN_EN_THUNDER);

    // Normal order: reject if the current enemy is a ghost
    if (combatContext.activeEnemy != ENEMY_NONE &&
        obj_enemy[combatContext.activeEnemy].class_id == ENEMY_CLS_WEAVERGHOST)
    {
        return false;   // hint will be shown by launchPlayerPattern
    }
    return true;
}
