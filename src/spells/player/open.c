// open.c — ABRIR: hechizo solo-guion (sin efecto propio; auto-fin por duración).

#include <genesis.h>
#include "core/core.h"
#include "spells/spells.h"
#include "spells/player/open.h"

void open_init(void)
{
    spell_defs[SPELL_OPEN] = (SpellDef){
        .id = SPELL_OPEN,
        .notes = { NOTE_FA, NOTE_SI, NOTE_SOL, NOTE_DO }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 3 / 4,
        .enabled = false,
        .canUse = spell_scripted_only_can_use,
    };
}
