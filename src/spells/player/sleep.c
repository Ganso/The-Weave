// sleep.c — DORMIR: hechizo solo-guion (la nana del acto 1).

#include <genesis.h>
#include "core/core.h"
#include "spells/spells.h"
#include "spells/player/sleep.h"

void sleep_init(void)
{
    spell_defs[SPELL_SLEEP] = (SpellDef){
        .id = SPELL_SLEEP,
        .notes = { NOTE_FA, NOTE_MI, NOTE_DO, NOTE_LA }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 5 / 4,
        .enabled = false,
        .canUse = spell_scripted_only_can_use,
    };
}
