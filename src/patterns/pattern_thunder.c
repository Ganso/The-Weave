#include "globals.h"

#define PAL_ENTRY(pal, col)   (((pal) << 4) | (col)) 
#define PAL0_COL4             PAL_ENTRY(0, 4)
#define COLOR_WHITE_VDP  RGB24_TO_VDPCOLOR(0xFFFFFF)

static u16 savedColor; // original colour in CRAM before the flash

// Launch callback
void playerThunderLaunch(void)
{
    // Save current CRAM entry so we can restore it later            */
    savedColor = PAL_getColor(PAL0_COL4); 
    dprintf(2, "Thunder: initial CRAM colour 0-4 = 0x%04X\n", savedColor);

    // First frame: set to white so the flash is immediate
    PAL_setColor(PAL0_COL4, COLOR_WHITE_VDP);
    combatContext.effectTimer = 0;

    // Sound effect
    play_sample(snd_pattern_thunder, sizeof(snd_pattern_thunder)); // play thunder sound
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

bool playerThunderCanUse(void)
{
    return true;           // always available
}