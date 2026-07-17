// heal.c — CURACIÓN: hechizo solo-guion (lo canta Clio en el acto 1).

#include <genesis.h>
#include "core/core.h"
#include "spells/spells.h"
#include "spells/player/heal.h"

void heal_init(void)
{
    spell_defs[SPELL_HEAL] = (SpellDef){
        .id = SPELL_HEAL,
        .notes = { NOTE_LA, NOTE_SI, NOTE_DO, NOTE_SI }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 5 / 4,
        .enabled = false,
        .canUse = spell_scripted_only_can_use,
    };
}
