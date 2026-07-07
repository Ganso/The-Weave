// scene_vm.c — intérprete de cutscenes. Ver el bloque de arquitectura en scene_vm.h.

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/scene_vm.h"
#include "scenes/scene_hooks.h"
#include "scenes/scene_data.h"
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

u8 last_choice;       // resultado del último CHOICE
u8 current_scene_id;  // SceneId en curso (interno; las transiciones son por nombre)

// Puzzle de secuencia de hechizos activo (uno a la vez; -1 = ninguno)
static s16 active_puzzle = -1;   // índice en puzzle_seqs[] (scene_data, generado)
static u8  puzzle_progress;      // pasos acertados consecutivos

static bool puzzle_solved(s16 idx)
{
    return idx >= 0 && idx == active_puzzle &&
           puzzle_progress >= puzzle_seqs[idx].len;
}

// El motor de hechizos llama aquí cuando un cast (jugador o narrativo) termina
// de forma natural. Acierto → avanza; fallo → reinicia (contando el paso 1 si
// el hechizo fallido coincide con el inicio de la secuencia).
void scene_puzzle_notify(u8 spellId, bool reversed)
{
    if (active_puzzle < 0) return;
    const PuzzleSeq *p = &puzzle_seqs[active_puzzle];
    if (puzzle_progress >= p->len) return;   // ya resuelto

    if (p->spell[puzzle_progress] == spellId &&
        p->reversed[puzzle_progress] == (reversed ? 1 : 0))
    {
        puzzle_progress++;
        dprintf(2,"Puzzle %d: paso %d/%d OK (spell %d)", active_puzzle, puzzle_progress, p->len, spellId);
    }
    else
    {
        puzzle_progress = (p->spell[0] == spellId && p->reversed[0] == (reversed ? 1 : 0)) ? 1 : 0;
        dprintf(2,"Puzzle %d: secuencia rota (spell %d), progreso=%d", active_puzzle, spellId, puzzle_progress);
    }
}

// Espera N décimas de segundo con frames no-interactivos (equivale a wait_seconds)
static void wait_tenths(u16 tenths)
{
    u16 frames = (u16)((tenths * SCREEN_FPS) / 10);
    while (frames--) next_frame(false);
}

// Los diálogos y choices ocultan el HUD de hechizos mientras hablan y lo
// restauran al terminar (solo si estaba activo), para que no se solape con el
// texto. show_or_hide_interface() no toca interface_active, así que el flag
// guardado sigue siendo válido tras ocultar.
static void say_hud(const DialogItem *d, bool sound)
{
    bool hud = interface_active;
    if (hud) show_or_hide_interface(false);
    talk_dialog(d, sound);
    if (hud) show_or_hide_interface(true);
}

static void cluster_hud(const DialogItem *start, bool sound)
{
    bool hud = interface_active;
    if (hud) show_or_hide_interface(false);
    talk_cluster(start, sound);
    if (hud) show_or_hide_interface(true);
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

    active_puzzle = -1;   // los puzzles no sobreviven entre escenas
    puzzle_progress = 0;

    dprintf(2,"Scene start: %s (%d steps)", s->name, s->stepCount);

    while (pc < s->stepCount)
    {
        const SceneStep *st = &s->steps[pc];    // ROM: solo lectura

        switch (st->op)
        {
            case SCENE_OP_CALL:
                // Un hueco NULL en la tabla (hook en el enum pero sin registrar en
                // scene_hooks.c) saltaría a la dirección 0 y colgaría la consola:
                // mejor avisar por KDebug y seguir.
                if (st->a < HOOK_COUNT && scene_hook_table[st->a] != NULL)
                    scene_hook_table[st->a]();
                else
                    dprintf(1,"Scene %s: hook %d SIN REGISTRAR en scene_hook_table", s->name, st->a);
                break;

            case SCENE_OP_SAY:
                say_hud(&dialogs[st->a][st->b], st->c);
                break;

            case SCENE_OP_SAY_CLUSTER:
                cluster_hud(&dialogs[st->a][st->b], st->c);
                break;

            case SCENE_OP_SAY_RESPONSE: // respuesta a un choice: id = base + last_choice
                say_hud(&dialogs[st->a][st->b + last_choice], st->c);
                break;

            case SCENE_OP_CHOICE: {
                bool hud = interface_active;
                if (hud) show_or_hide_interface(false);
                last_choice = choice_dialog(&choices[st->a][st->b]);
                if (hud) show_or_hide_interface(true);
                dprintf(2,"Choice %d/%d -> %d", st->a, st->b, last_choice);
                break;
            }

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
                current_scene_id = st->a;       // SceneId (emitido por nombre en el DSL)
                dprintf(2,"Scene end: %s -> scene id %d", s->name, st->a);
                return;

            case SCENE_OP_PUZZLE_SEQ: // activa (o reinicia) el puzzle del índice a
                active_puzzle = st->a;
                puzzle_progress = 0;
                break;

            case SCENE_OP_WAIT_PUZZLE: // el jugador castea libremente hasta resolverlo
                while (!puzzle_solved(st->a)) next_frame(true);
                break;

            case SCENE_OP_IF_PUZZLE_SOLVED:
                if (puzzle_solved(st->a)) { pc = st->b; continue; }
                break;

            case SCENE_OP_ANIM:
                anim_character(st->a, st->b);
                break;

            case SCENE_OP_WAIT_PRESS: { // pausa de cutscene: esperar pulsación de A (con release previo)
                u16 joy;
                do { next_frame(false); joy = JOY_readJoypad(JOY_1); } while (joy & BUTTON_A);
                do { next_frame(false); joy = JOY_readJoypad(JOY_1); } while (!(joy & BUTTON_A));
                break;
            }

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
