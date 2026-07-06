// scene_vm.c — intérprete de cutscenes. Ver el bloque de arquitectura en scene_vm.h.

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/scene_vm.h"
#include "scenes/scene_hooks.h"
#include "narrative/texts.h"
#include "narrative/texts_data.h"
#include "narrative/choices_data.h"
#include "narrative/dialogs.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "world/background.h"
#include "combat/combat.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "interface/interface.h"

u8 last_choice; // resultado del último CHOICE

// Espera N décimas de segundo con frames no-interactivos (equivale a wait_seconds)
static void wait_tenths(u16 tenths)
{
    u16 frames = (u16)((tenths * SCREEN_FPS) / 10);
    while (frames--) next_frame(false);
}

static void set_scene_flag(u8 flag, bool on)
{
    switch (flag)
    {
        case SCENE_FLAG_MOVEMENT:  movement_active = on; break;
        case SCENE_FLAG_SCROLL:    player_scroll_active = on; break;
        case SCENE_FLAG_INTERFACE:
            if (on) interface_active = true;    // off NO toca interface_active (ver scene_vm.h)
            show_or_hide_interface(on);
            break;
        case SCENE_FLAG_SPELLS:    player_patterns_enabled = on; break;
        default: break;
    }
}

void scene_run(const SceneScript *s)    // Ejecuta una escena completa
{
    u16 pc = 0;

    dprintf(2,"Scene start: %s (%d steps)", s->name, s->stepCount);

    while (pc < s->stepCount)
    {
        const SceneStep *st = &s->steps[pc];    // ROM: solo lectura

        switch (st->op)
        {
            case SCENE_OP_CALL:
                scene_hook_table[st->a]();
                break;

            case SCENE_OP_SAY:
                talk_dialog(&dialogs[st->a][st->b], st->c);
                break;

            case SCENE_OP_SAY_CLUSTER:
                talk_cluster(&dialogs[st->a][st->b], st->c);
                break;

            case SCENE_OP_SAY_RESPONSE: // respuesta a un choice: id = base + last_choice
                talk_dialog(&dialogs[st->a][st->b + last_choice], st->c);
                break;

            case SCENE_OP_CHOICE:
                last_choice = choice_dialog(&choices[st->a][st->b]);
                dprintf(2,"Choice %d/%d -> %d", st->a, st->b, last_choice);
                break;

            case SCENE_OP_BRANCH:
                if (last_choice == st->a) { pc = st->b; continue; }
                break;

            case SCENE_OP_GOTO:
                pc = st->a; continue;

            case SCENE_OP_MOVE:
                move_character(st->a, st->b, st->c);
                break;

            case SCENE_OP_MOVE_INSTANT:
                move_character_instant(st->a, st->b, st->c);
                break;

            case SCENE_OP_SHOW:
                show_character(st->a, st->b);
                break;

            case SCENE_OP_LOOK:
                look_left(st->a, st->b);
                break;

            case SCENE_OP_WAIT:
                wait_tenths(st->a);
                break;

            case SCENE_OP_WAIT_SCROLL: // deja jugar hasta alcanzar el offset de scroll
                while (FASTFIX32_TO_INT(offset_BGA) < st->a) next_frame(true);
                break;

            case SCENE_OP_SET:
                set_scene_flag(st->a, st->b);
                break;

            case SCENE_OP_COMBAT: // combate interactivo completo (el FSM manda)
                combat_init();
                while (combat_state != COMBAT_NO) next_frame(true);
                break;

            case SCENE_OP_CAST: // lanzamiento scripted (puzzles / demostraciones)
                spell_narrative_cast(st->a, st->b);
                break;

            case SCENE_OP_WAIT_SPELL:
                while (spell_slot_active(SPELL_SLOT_PLAYER)) next_frame(true);
                break;

            case SCENE_OP_ZONE:
                spell_zone = st->a;
                break;

            case SCENE_OP_FADE_OUT:
                PAL_fadeOutAll(st->a, false);
                break;

            case SCENE_OP_NEXT_SCENE:
                end_level();                    // libera recursos del nivel
                current_act = st->a;
                current_scene = st->b;
                dprintf(2,"Scene end: %s -> act %d scene %d", s->name, st->a, st->b);
                return;

            case SCENE_OP_HARD_RESET:
                SYS_hardReset();
                return; // no se alcanza

            case SCENE_OP_END:
                dprintf(2,"Scene end: %s", s->name);
                return;

            default:
                dprintf(1,"Scene %s: unknown op %d at step %d", s->name, st->op, pc);
                break;
        }
        pc++;
    }
}
