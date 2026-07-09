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
// Lógica pura (sin pintar): PASS si canUse coincide con lo esperado.
static bool check_pass(const SmokeCase *c)
{
    u8 old_zone = spell_zone;
    spell_zone = c->zone;
    bool got = spell_can_use(c->spellId, c->reversed);
    spell_zone = old_zone;
    return got == c->expected;
}

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
// Lógica pura (sin pintar el resultado): monta el nivel, lanza el hechizo,
// cuenta frames hasta que el slot se libera y devuelve PASS/FAIL (baseDuration±2).
static bool cast_run(const SmokeCase *c, u16 *out_frames)
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

    if (out_frames) *out_frames = frames;
    return pass;
}

static void run_cast(const SmokeCase *c)
{
    u16 frames = 0;
    bool pass = cast_run(c, &frames);
    u16 expected = spell_defs[c->spellId].baseDuration;

    char buf[24];
    VDP_clearPlane(BG_A, true);
    VDP_drawText(c->name, 4, 8);
    sprintf(buf, "%u / %u frames", frames, expected);
    result_line("duracion:", buf, 10);
    result_line("resultado:", pass ? "PASS" : "FAIL", 12);
    VDP_drawText("A = volver al menu", 4, 20);
}

// --- AUTO: corre todos los CHECK (invariantes de canUse) y deja resultados ---
// Solo los CHECK: instantáneos, sin render ni sprites, deterministas → seguros de
// encadenar y sin input. Los CAST se excluyen a propósito (miden duración + efecto
// visual "a ojo" y encadenar new_level/end_level reinicia el hardware con fades
// asíncronos en vuelo, corrompiendo el sprite engine); se siguen ejecutando uno a
// uno desde el menú. Pensado para validación desatendida (RetroArch NCI no envia
// input; ver docs/retroarch-mcp.md): la ROM llega sola a esta pantalla.
static void run_auto(void)
{
    u16 ok = 0, n = 0;
    const char *fails[SMOKE_CASE_COUNT];
    u16 nfail = 0;

    for (u16 i = 0; i < SMOKE_CASE_COUNT; i++)
    {
        const SmokeCase *c = &smoke_cases[i];
        if (c->kind != SMOKE_CHECK) continue;   // solo invariantes de canUse
        n++;
        bool pass = check_pass(c);
        if (pass) ok++;
        else if (nfail < SMOKE_CASE_COUNT) fails[nfail++] = c->name;
        dprintf(1, "AUTO %s: %s", c->name, pass ? "PASS" : "FAIL");
    }

    char buf[32];
    VDP_clearPlane(BG_A, true);
    VDP_clearPlane(BG_B, true);
    VDP_drawText("SMOKE AUTO RESULTS", 4, 2);
    VDP_drawText("(CHECK invariantes canUse)", 4, 3);
    sprintf(buf, "CHECKS  %u OK  %u FAIL", ok, (u16)(n - ok)); VDP_drawText(buf, 4, 5);
    VDP_drawText((nfail == 0) ? "RESULT: ALL PASS" : "RESULT: FAIL", 4, 7);

    for (u16 i = 0; i < nfail && i < 10; i++) VDP_drawText(fails[i], 6, 9 + i);

    VDP_drawText("A = menu", 4, 22);
    dprintf(1, "AUTO DONE: %u/%u OK", ok, n);
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
        case SMOKE_AUTO:  run_auto();   break;
        default: break;
    }
}

#endif // HACK_SMOKE_BUILD
