// smoke_runner.c — ejecuta un caso de smoke: CHECK (canUse), CAST (duración) o SCENE.
#ifdef HACK_SMOKE_BUILD

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "scenes/scenes.h"
#include "res_all.h"
#include "smoke/smoke_runner.h"

static void result_line(const char *label, const char *value, u8 y)
{
    VDP_drawText(label, 4, y);
    VDP_drawText(value, 24, y);
}

// --- CHECK: invariante de canUse -------------------------------------------
static void run_check(const SmokeCase *c)
{
    u8 old_zone = spell_zone;
    spell_zone = c->zone;
    bool got = spell_can_use(c->spellId, c->reversed);
    spell_zone = old_zone;

    VDP_clearPlane(BG_A, true);
    VDP_drawText(c->name, 4, 8);
    result_line("canUse:", got ? "true" : "false", 10);
    result_line("resultado:", (got == c->expected) ? "PASS" : "FAIL", 12);
    VDP_drawText("A = volver al menu", 4, 20);
}

// --- CAST: lanzar en un nivel de pruebas y medir frames ---------------------
static void run_cast(const SmokeCase *c)
{
    // Nivel de pruebas: bosque + Linus visible (los efectos usan paleta y sprite)
    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map, forest_pal, SCREEN_WIDTH, BG_SCRL_AUTO_RIGHT, 3);
    init_character(CHR_linus);
    move_character_instant(CHR_linus, 140, 154);
    show_character(CHR_linus, true);
    movement_active = false;

    u16 expected = spell_defs[c->spellId].baseDuration;
    u16 frames = 0;

    spell_narrative_cast(c->spellId, c->reversed);
    while (spell_slot_active(SPELL_SLOT_PLAYER) && frames < expected * 2)
    {
        next_frame(true);   // update_combat → spell_update avanza el efecto
        frames++;
    }

    bool pass = !spell_slot_active(SPELL_SLOT_PLAYER) &&
                frames + 2 >= expected && frames <= expected + 2;

    end_level();

    char buf[24];
    VDP_clearPlane(BG_A, true);
    VDP_drawText(c->name, 4, 8);
    sprintf(buf, "%u / %u frames", frames, expected);
    result_line("duracion:", buf, 10);
    result_line("resultado:", pass ? "PASS" : "FAIL", 12);
    VDP_drawText("A = volver al menu", 4, 20);
}

// --- SCENE: ejecutar la escena completa -------------------------------------
static void run_scene(const SmokeCase *c)
{
    s16 id = scene_id_by_name(c->sceneName);
    if (id < 0)
    {
        VDP_clearPlane(BG_A, true);
        VDP_drawText("FAIL: escena no existe", 4, 10);
        return;
    }

    scene_run(&scenes[id]);   // interactiva; vuelve al terminar (forest resetea)

    VDP_clearPlane(BG_A, true);
    VDP_clearPlane(BG_B, true);
    VDP_drawText(c->name, 4, 8);
    result_line("resultado:", "TERMINADA", 10);
    VDP_drawText("A = volver al menu", 4, 20);
}

void smoke_run_case(const SmokeCase *c)    // Ejecuta el caso y deja el resultado en pantalla
{
    dprintf(1, "SMOKE: %s", c->name);
    switch (c->kind)
    {
        case SMOKE_CHECK: run_check(c); break;
        case SMOKE_CAST:  run_cast(c);  break;
        case SMOKE_SCENE: run_scene(c); break;
        default: break;
    }
}

#endif // HACK_SMOKE_BUILD
