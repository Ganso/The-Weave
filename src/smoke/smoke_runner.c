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
    // Nivel de pruebas: bosque + Linus visible (los efectos usan paleta y sprite).
    // Cada hechizo se prueba en SU contexto real: el TRUENO en el bosque OSCURO
    // (forest_dark), donde su destello cambia toda la paleta; el resto en el
    // bosque de día. Anchura 1440 + BG_SCRL_USER_RIGHT (trampa del scroll §7).
    Palette level_pal = (c->spellId == SPELL_THUNDER) ? forest_dark_pal : forest_pal;
    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map, level_pal, 1440, BG_SCRL_USER_RIGHT, 3);
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

// --- SIM: teclear un patrón inyectando botones por el embudo de input --------
// Es el único caso que ejercita core/input.h desde DENTRO de la ROM: en vez de
// llamar a spell_narrative_cast (como hacen los CAST), TECLEA las cuatro notas
// como lo haría un jugador, y comprueba que la cola las recoge una a una y que
// el motor reconoce el hechizo al completar la cuarta. Valida de paso la
// detección de patrones, que hasta ahora no tenía test.
//
// Se teclea LIGHT (SOL LA SI DO = botones C X Y Z): cuatro notas distintas en
// cuatro botones distintos, y ninguna es A (que además interactúa con items).
#define SIM_PRESS_FRAMES    4    // frames con el botón pulsado
#define SIM_RELEASE_FRAMES  16   // frames sueltos: por encima de MIN_TIME_BETWEEN_NOTES (SCREEN_FPS/4)

// Una pulsación humana: presionar, MEDIR, soltar y esperar. La medida se toma
// con el botón aún pulsado porque la nota se registra en el primer frame del
// flanco; al soltar, la cola puede haberse vaciado ya (la 4ª nota la vacía).
static void sim_tap(u16 button, u8 *out_queued, u8 *out_spell)
{
    input_setOverride(button);
    for (u8 f = 0; f < SIM_PRESS_FRAMES; f++) next_frame(true);

    *out_queued = notes_player_count();
    *out_spell  = spell_active_id(SPELL_SLOT_PLAYER);

    input_setOverride(0);   // soltar el botón SIN levantar el override: lo que
                            // esperan los llamadores es el FLANCO, no el estado
    for (u8 f = 0; f < SIM_RELEASE_FRAMES; f++) next_frame(true);
}

// PASS si las 3 primeras notas se encolan (1, 2, 3) y la 4ª lanza LIGHT.
// out_reached deja cuántas notas llegaron a registrarse, para el informe.
static bool sim_notes_run(u16 *out_reached)
{
    static const u16 buttons[MAX_SPELL_NOTES] = { BUTTON_C, BUTTON_X, BUTTON_Y, BUTTON_Z };

    // player_has_rod ANTES de init_character (trampa de la vara: AGENTS.md §7).
    player_has_rod = true;
    player_patterns_enabled = true;
    player_note_limit = NOTE_DO;   // LIGHT termina en DO, la nota más alta

    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map,
              forest_pal, 1440, BG_SCRL_USER_RIGHT, 3);
    init_character(CHR_linus);
    move_character_instant(CHR_linus, 140, 154);
    show_character(CHR_linus, true);

    movement_active = true;        // sin esto next_frame no llama a joy_check
    spell_engine_reset();
    spell_enable(SPELL_LIGHT);

    // La primera nota también pasa por el debounce (note_timer arranca a 0).
    for (u8 f = 0; f < SIM_RELEASE_FRAMES; f++) next_frame(true);

    u16 reached = 0;
    bool ok = true;

    for (u8 i = 0; i < MAX_SPELL_NOTES; i++)
    {
        u8 queued = 0, spellId = SPELL_NONE;
        sim_tap(buttons[i], &queued, &spellId);

        if (i < MAX_SPELL_NOTES - 1)
        {
            if (queued != (u8)(i + 1)) { ok = false; break; }   // no llegó el botón
            reached = i + 1;
        }
        else
        {
            reached = MAX_SPELL_NOTES;
            if (spellId != SPELL_LIGHT) ok = false;             // patrón no reconocido
        }
        dprintf(2, "SIM nota %u: cola=%u spell=%u", i + 1, queued, spellId);
    }

    input_clearOverride();         // devolver el mando al jugador
    movement_active = false;

    // Dejar que el efecto termine antes de desmontar (acotado, como los CAST).
    u16 guard = spell_defs[SPELL_LIGHT].baseDuration * 2;
    while (spell_slot_active(SPELL_SLOT_PLAYER) && guard--) next_frame(true);

    end_level();

    if (out_reached) *out_reached = reached;
    return ok;
}

static void run_sim(const SmokeCase *c)
{
    u16 reached = 0;
    bool pass = sim_notes_run(&reached);

    char buf[24];
    PAL_setColor(15, 0x0EEE);            // end_level dejó las paletas en negro
    VDP_setTextPalette(PAL_BACKGROUND);
    VDP_clearPlane(BG_A, true);
    VDP_drawText(c->name, 4, 8);
    sprintf(buf, "%u / %u notas", reached, (u16)MAX_SPELL_NOTES);
    result_line("tecleadas:", buf, 10);
    result_line("resultado:", pass ? "PASS" : "FAIL", 12);
    VDP_drawText("A = volver al menu", 4, 20);
}

// --- AUTO: recorrido scripted por las mecánicas + invariantes de canUse -------
// Corre sin input: pensado para validación desatendida vía RetroArch NCI/MCP
// (ver docs/retroarch-mcp.md). Tres globales en WRAM dejan que el host (el MCP)
// sincronice capturas y pruebe write_ram:
//   smoke_phase   fase actual del recorrido (SmokePhase); 0xFFFF = resultados.
//   smoke_scratch libre para probar retroarch_write_ram / read_ram (la ROM no lo usa).
//   smoke_gate    puerta de sincronización: la ROM se congela en PH_WAIT_GATE hasta
//                  que el host escribe un valor != 0 (garantiza que no se pierdan
//                  frames) o expira el timeout (~10 s: una run a mano arranca sola).
// volatile + escritas explícitas para que --gc-sections no las descarte.
//   smoke_result  veredicto legible por RAM: máscara de FALLOS (0 = ALL PASS),
//                  válida solo cuando smoke_phase == PH_DONE. Sin esto el
//                  resultado solo existía PINTADO en pantalla, así que una run
//                  headless (sin capturas) no podía saber si pasó.
volatile u16 smoke_phase   = 0;
volatile u16 smoke_scratch = 0;
volatile u16 smoke_gate    = 0;
volatile u16 smoke_result  = 0xFFFF;   // 0xFFFF = aún sin veredicto

// Bits de smoke_result (encendido = ESE bloque falló)
#define SMOKE_FAIL_CHECKS   0x0001
#define SMOKE_FAIL_LIGHT    0x0002
#define SMOKE_FAIL_THUNDER  0x0004
#define SMOKE_FAIL_COMBAT   0x0008
#define SMOKE_FAIL_SIM      0x0010

typedef enum {
    PH_BOOT = 0, PH_IDLE = 1, PH_WALK = 2, PH_CAST_LIGHT = 3,
    PH_CAST_THUNDER = 4, PH_COMBAT = 5, PH_SIM_NOTES = 6,
    PH_WAIT_GATE = 0xFFFE, PH_DONE = 0xFFFF
} SmokePhase;

static void hold(u16 frames)   // deja correr N frames (anima sprites, combate, scroll)
{
    for (u16 f = 0; f < frames; f++) next_frame(true);
}

static void run_auto(void)
{
    dprintf(3, "AUTO: entra run_auto");
    smoke_scratch = 0;
    smoke_gate    = 0;
    smoke_phase   = PH_WAIT_GATE;   // señal al host: "estoy listo, esperando gate"
    dprintf(3, "AUTO: en PH_WAIT_GATE, esperando gate");

    // Esperar a que el host dé luz verde inyectando un valor != 0 en smoke_gate.
    // Así el host sabe que el emulador está listo y no se pierden trazas/frames.
    // Timeout de fallback (~10 s): una run A MANO (sin host que abra el gate)
    // arranca sola; el driver lo abre en milisegundos y nunca llega aquí.
    u16 gate_wait = SCREEN_FPS * 10;
    while (smoke_gate == 0 && gate_wait--)
    {
        next_frame(false);
    }
    dprintf(3, "AUTO: gate abierto (o timeout), sigue");

    smoke_phase = PH_BOOT;

    // --- 1. Invariantes de canUse (instantáneas, sin render) ---
    dprintf(3, "AUTO: fase 1 - checks canUse");
    u16 ok = 0, n = 0;
    const char *fails[SMOKE_CASE_COUNT];
    u16 nfail = 0;
    for (u16 i = 0; i < SMOKE_CASE_COUNT; i++)
    {
        const SmokeCase *c = &smoke_cases[i];
        if (c->kind != SMOKE_CHECK) continue;
        n++;
        bool pass = check_pass(c);
        if (pass) ok++;
        else if (nfail < SMOKE_CASE_COUNT) fails[nfail++] = c->name;
        dprintf(1, "AUTO %s: %s", c->name, pass ? "PASS" : "FAIL");
    }
    dprintf(3, "AUTO: checks %u/%u OK", ok, n);

    // --- 2. Recorrido por el nivel: movimiento del personaje ---
    dprintf(3, "AUTO: fase 2 - new_level + linus");
    // player_has_rod ANTES de init_character (trampa de la vara: AGENTS.md §7).
    // Anchura 1440 + BG_SCRL_USER_RIGHT (trampa del scroll: AGENTS.md §7).
    player_has_rod = true;
    player_patterns_enabled = true;

    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map,
              forest_pal, 1440, BG_SCRL_USER_RIGHT, 3);
    dprintf(3, "AUTO: new_level ok");
    init_character(CHR_linus);
    dprintf(3, "AUTO: init_character ok");
    move_character_instant(CHR_linus, 150, 154);
    show_character(CHR_linus, true);
    movement_active = false;
    dprintf(3, "AUTO: linus visible, a hold");

    smoke_phase = PH_IDLE;                              // reposo (hold largo: da margen al host
    anim_character(CHR_linus, ANIM_IDLE);               // para engancharse y probar pause/frameadvance,
    hold(150);                                          // frame_counter avanza aquí)

    smoke_phase = PH_WALK;                              // movimiento: recorre el nivel a der. e izq.
    move_character(CHR_linus, 240, 154);   hold(20);
    move_character(CHR_linus,  40, 154);   hold(20);
    move_character_instant(CHR_linus, 150, 154);
    anim_character(CHR_linus, ANIM_IDLE);  hold(40);

    // --- 3. Castear hechizos (PH_CAST_LIGHT y PH_CAST_THUNDER) -------------------
    // Mismo criterio que los casos CAST del menú: PASS si el slot se libera en
    // baseDuration±2. El bucle está acotado (x2) para que la suite siempre llegue
    // a la pantalla de resultados aunque un hechizo no termine.
    spell_enable(SPELL_LIGHT);
    spell_enable(SPELL_THUNDER);

    smoke_phase = PH_CAST_LIGHT;
    u16 light_expected = spell_defs[SPELL_LIGHT].baseDuration;
    u16 light_frames = 0;
    spell_narrative_cast(SPELL_LIGHT, false);
    while (spell_slot_active(SPELL_SLOT_PLAYER) && light_frames < light_expected * 2)
    {
        next_frame(true);
        light_frames++;
    }
    bool light_ok = !spell_slot_active(SPELL_SLOT_PLAYER) &&
                    light_frames + 2 >= light_expected && light_frames <= light_expected + 2;
    hold(30);

    smoke_phase = PH_CAST_THUNDER;
    // El TRUENO es el hechizo del bosque OSCURO: cambiar a forest_dark para ver
    // su destello real (toda la paleta de fondo alternando con la clara)
    PAL_setPalette(PAL_BACKGROUND, forest_dark_pal.data, DMA);
    u16 thunder_expected = spell_defs[SPELL_THUNDER].baseDuration;
    u16 thunder_frames = 0;
    spell_narrative_cast(SPELL_THUNDER, false);
    while (spell_slot_active(SPELL_SLOT_PLAYER) && thunder_frames < thunder_expected * 2)
    {
        next_frame(true);
        thunder_frames++;
    }
    bool thunder_ok = !spell_slot_active(SPELL_SLOT_PLAYER) &&
                      thunder_frames + 2 >= thunder_expected && thunder_frames <= thunder_expected + 2;
    hold(30);

    // --- 4. Combate (PH_COMBAT): WeaverGhost ataca, counter con trueno invertido ---
    // Cada espera lleva una guarda generosa: si la IA no progresa, el combate se da
    // por FAIL y la suite sigue hasta los resultados en vez de colgarse.
    smoke_phase = PH_COMBAT;
    PAL_setPalette(PAL_ENEMIES, weaver_ghost_sprite.palette->data, DMA);
    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    obj_enemy[0].hitpoints = 1; // 1 HP para que muera de un solo counter
    move_enemy_instant(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(154));
    show_enemy(0, true);

    combat_start();   // sin config previa: encuentro por defecto (solo patrones)

    u16 guard = SCREEN_FPS * 30;   // techo total del combate (30 s)

    // Esperar a que el enemigo inicie su ataque (entra en COMBAT_STATE_ENEMY_PLAYING)
    while (combat_state == COMBAT_STATE_IDLE && guard) { next_frame(true); guard--; }

    // Esperar a que termine de tocar las notas (entra en COMBAT_STATE_ENEMY_EFFECT)
    while (combat_state == COMBAT_STATE_ENEMY_PLAYING && guard) { next_frame(true); guard--; }

    // El rayo enemigo está activo: contrarrestar con trueno invertido
    if (combat_state == COMBAT_STATE_ENEMY_EFFECT)
    {
        spell_player_cast(SPELL_THUNDER, true);
    }

    // Esperar a que el enemigo sea liberado y el combate termine
    while (combat_state != COMBAT_NO && guard) { next_frame(true); guard--; }

    bool combat_ok = (combat_state == COMBAT_NO) && !obj_enemy[0].obj_character.active;
    combat_end();

    dprintf(3, "AUTO: pre end_level");
    end_level();
    dprintf(3, "AUTO: post end_level, resultados");

    // --- 5. Input simulado (PH_SIM_NOTES): teclear un patrón por el embudo ----
    // Va al final y monta/desmonta su propio nivel, para no interferir con el
    // recorrido de arriba.
    smoke_phase = PH_SIM_NOTES;
    u16 sim_notes = 0;
    bool sim_ok = sim_notes_run(&sim_notes);
    dprintf(1, "AUTO SIM patron notas: %s (%u notas)", sim_ok ? "PASS" : "FAIL", sim_notes);

    // --- 6. Pantalla de resultados (end_level dejó las paletas en negro) ---
    PAL_setColor(15, 0x0EEE);   // texto en blanco
    VDP_setTextPalette(PAL_BACKGROUND);
    VDP_clearPlane(BG_A, true);
    VDP_clearPlane(BG_B, true);

    bool all_pass = (nfail == 0) && light_ok && thunder_ok && combat_ok && sim_ok;

    // Veredicto legible por RAM ANTES de pintar nada: una run headless (sin
    // capturas) lee esto en vez de mirar la pantalla.
    smoke_result = (nfail ? SMOKE_FAIL_CHECKS : 0)
                 | (light_ok   ? 0 : SMOKE_FAIL_LIGHT)
                 | (thunder_ok ? 0 : SMOKE_FAIL_THUNDER)
                 | (combat_ok  ? 0 : SMOKE_FAIL_COMBAT)
                 | (sim_ok     ? 0 : SMOKE_FAIL_SIM);

    char buf[40];
    VDP_drawText("SMOKE AUTO - WALKTHROUGH", 4, 2);
    sprintf(buf, "CHECKS  %u OK  %u FAIL", ok, (u16)(n - ok)); VDP_drawText(buf, 4, 4);
    VDP_drawText("WALK    OK", 4, 5);
    sprintf(buf, "CAST    LIGHT %s (%u)", light_ok ? "OK" : "FAIL", light_frames);
    VDP_drawText(buf, 4, 6);
    sprintf(buf, "CAST    THUNDER %s (%u)", thunder_ok ? "OK" : "FAIL", thunder_frames);
    VDP_drawText(buf, 4, 7);
    sprintf(buf, "COMBAT  %s", combat_ok ? "OK" : "FAIL");
    VDP_drawText(buf, 4, 8);
    sprintf(buf, "SIM     NOTAS %s (%u/4)", sim_ok ? "OK" : "FAIL", sim_notes);
    VDP_drawText(buf, 4, 9);
    VDP_drawText(all_pass ? "RESULT: ALL PASS" : "RESULT: FAIL", 4, 11);
    for (u16 i = 0; i < nfail && i < 8; i++) VDP_drawText(fails[i], 6, 13 + i);
    VDP_drawText("A = menu", 4, 22);

    dprintf(1, "AUTO DONE: checks %u/%u OK, light=%u thunder=%u combat=%u sim=%u",
            ok, n, light_ok, thunder_ok, combat_ok, sim_ok);
    smoke_phase = PH_DONE;      // señal al host: resultados en pantalla
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
        case SMOKE_SIM:   run_sim(c);   break;
        case SMOKE_SCENE: run_scene(c); break;
        case SMOKE_AUTO:  run_auto();   break;
        default: break;
    }
}

#endif // HACK_SMOKE_BUILD
