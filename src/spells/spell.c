// spell.c — motor de hechizos: slots, ciclo de vida, counter, fases.
// Ver el bloque de arquitectura en spell.h.

#include <genesis.h>
#include "core/config.h"
#include "core/hack.h"
#include "core/frame.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "spells/player_spells.h"
#include "spells/enemy_spells.h"
#include "spells/fire.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "combat/combat.h"
#include "interface/interface.h"
#include "audio/sound.h"
#include "narrative/dialogs.h"
#include "narrative/texts_data.h"

SpellDef spell_defs[SPELL_COUNT];
u8 spell_zone = ZONE_NONE;

// Un hechizo en ejecución
typedef struct {
    const SpellDef *def;    // NULL = slot libre
    SpellContext ctx;
    SpellSlotPhase phase;
} ActiveSpell;
static ActiveSpell slots[SPELL_SLOT_COUNT];

// Estado runtime del lado enemigo
static u16 enemy_recharge[MAX_ENEMIES][MAX_SPELLS_PER_ENEMY]; // frames de recarga por enemigo/hueco
static u8  enemy_note_index;   // siguiente nota a tocar por el enemigo
static u16 enemy_note_timer;   // frames desde la última nota enemiga

// Helpers internos definidos más abajo
static void enemy_recharge_reset(u8 enemyId, u8 spellId);
static void enemy_state_cleanup(u8 enemyId);

// ---------------------------------------------------------------------
// Consultas
// ---------------------------------------------------------------------
bool spell_slot_active(SpellSlot s)          { return slots[s].def != NULL; }
u8   spell_active_id(SpellSlot s)            { return slots[s].def ? slots[s].ctx.spellId : SPELL_NONE; }
SpellSlotPhase spell_slot_phase(SpellSlot s) { return slots[s].def ? slots[s].phase : SLOT_FREE; }
SpellContext* spell_ctx(SpellSlot s)         { return slots[s].def ? &slots[s].ctx : NULL; }

u8 spell_enemy_caster(void)    // Enemigo que está lanzando, o ENEMY_NONE
{
    return slots[SPELL_SLOT_ENEMY].def ? slots[SPELL_SLOT_ENEMY].ctx.enemyId : ENEMY_NONE;
}

// ---------------------------------------------------------------------
// Inicialización y reset
// ---------------------------------------------------------------------
static void slot_free(SpellSlot s)
{
    slots[s].def = NULL;
    slots[s].phase = SLOT_FREE;
}

void spell_engine_reset(void)    // Libera slots y resetea el input (new_level / combat_init)
{
    slot_free(SPELL_SLOT_PLAYER);
    slot_free(SPELL_SLOT_ENEMY);
    enemy_note_index = 0;
    enemy_note_timer = 0;
    notes_input_reset();
}

void init_spells(void)    // Rellena la tabla (runtime: baseDuration escala con SCREEN_FPS)
{
    memset(spell_defs, 0, sizeof(spell_defs));
    init_player_spells();   // thunder, hide, open, sleep
    init_enemy_spell_defs();// en_thunder, en_bite
    fire_init();            // ejemplo end-to-end

    if (HACK_ALL_SPELLS) { // Dev hack (core/hack.h): todos los hechizos + vara desde el principio
        for (u8 i = 0; i < SPELL_PLAYER_COUNT; i++) spell_defs[i].enabled = true;
        player_has_rod = true;
        player_patterns_enabled = true;
    }

    spell_engine_reset();
}

// ---------------------------------------------------------------------
// Validación de notas (solo hechizos de jugador, 4 notas)
// ---------------------------------------------------------------------
u8 spell_validate(const u8 *notes, bool *reversed)
{
    for (u8 i = 0; i < SPELL_PLAYER_COUNT; ++i)
    {
        const SpellDef *d = &spell_defs[i];

        // Orden directo
        if (notes[0]==d->notes[0] && notes[1]==d->notes[1] &&
            notes[2]==d->notes[2] && notes[3]==d->notes[3])
        {
            dprintf(2,"Spell %d recognised (direct order)", d->id);
            if (reversed) *reversed = false;
            return d->id;
        }

        // Orden invertido (los palíndromos no tienen forma invertida)
        if (notes[0]==d->notes[3] && notes[1]==d->notes[2] &&
            notes[2]==d->notes[1] && notes[3]==d->notes[0])
        {
            if (reversed) *reversed = !d->isPalindrome;
            dprintf(2,"Spell %d recognised (reversed=%d)", d->id, !d->isPalindrome);
            return d->id;
        }
    }
    if (reversed) *reversed = false;
    return SPELL_NONE;
}

// ---------------------------------------------------------------------
// Lanzamiento del jugador
// ---------------------------------------------------------------------
static SpellContext make_player_ctx(u8 spellId, bool reversed)
{
    return (SpellContext){
        .spellId = spellId, .origin = SPELL_ORIGIN_PLAYER, .reversed = reversed,
        .frameCounter = 0, .enemyId = spell_enemy_caster(), .zoneId = spell_zone
    };
}

// Comprueba canUse de un hechizo del jugador con un contexto temporal
bool spell_can_use(u8 spellId, bool reversed)
{
    const SpellDef *d = &spell_defs[spellId];
    if (!d->canUse) return true;
    SpellContext ctx = make_player_ctx(spellId, reversed);
    return d->canUse(&ctx);
}

void spell_player_cast(u8 spellId, bool reversed)    // Lanza (o contrarresta) un hechizo validado y usable
{
    const SpellDef *d = &spell_defs[spellId];

    // Un invertido dentro de la ventana de counter contrarresta en vez de lanzar
    if (reversed && spell_try_counter()) return;

    dprintf(2,"Launching player spell %d (reversed=%d)", spellId, reversed);

    slots[SPELL_SLOT_PLAYER].def = d;
    slots[SPELL_SLOT_PLAYER].ctx = make_player_ctx(spellId, reversed);
    slots[SPELL_SLOT_PLAYER].phase = SLOT_EFFECT;

    if (d->onLaunch) d->onLaunch(&slots[SPELL_SLOT_PLAYER].ctx);

    // El motor fija los estados DESPUÉS de onLaunch: así un hook puede cancelar
    // al enemigo (hide corta thunder) y el efecto del jugador queda igualmente activo
    combat_state = COMBAT_STATE_PLAYER_EFFECT;
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
}

// ---------------------------------------------------------------------
// Rechazo con feedback ("no puedo usar ese hechizo ahora")
// ---------------------------------------------------------------------
void spell_reject(u8 spellId, bool reversed)
{
    const SpellDef *d = &spell_defs[spellId];
    bool resume_enemy = spell_slot_active(SPELL_SLOT_ENEMY);

    dprintf(2, "Spell %d not usable right now", spellId);
    set_idle();
    show_or_hide_interface(false);
    hide_enemy_notes();

    if (d->onRejected) {
        SpellContext ctx = make_player_ctx(spellId, reversed);
        d->onRejected(&ctx);
    } else {
        talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN], false); // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"
    }

    show_enemy_notes();
    show_or_hide_interface(true);

    if (resume_enemy) {
        // El thunder enemigo se cancela (comportamiento heredado: el flash no se retoma);
        // cualquier otro hechizo enemigo continúa donde estaba
        if (spell_active_id(SPELL_SLOT_ENEMY) == SPELL_EN_THUNDER)
            spell_cancel(SPELL_SLOT_ENEMY);
        else
            combat_state = (spell_slot_phase(SPELL_SLOT_ENEMY) == SLOT_NOTES)
                         ? COMBAT_STATE_ENEMY_PLAYING : COMBAT_STATE_ENEMY_EFFECT;
    }
}

// ---------------------------------------------------------------------
// Counter
// ---------------------------------------------------------------------
bool spell_try_counter(void)
{
    ActiveSpell *en = &slots[SPELL_SLOT_ENEMY];

    if (!en->def || en->phase != SLOT_EFFECT ||
        combat_state != COMBAT_STATE_ENEMY_EFFECT) return false;
    if (!en->def->counterable || !en->def->onCounter) return false;

    dprintf(2,"Countering enemy spell %d", en->ctx.spellId);
    en->def->onCounter(&en->ctx);     // p.ej. en_thunder: restaurar paleta + hit_enemy

    // Cleanup del lado enemigo (recarga + notas + estado) y del input del jugador
    enemy_recharge_reset(en->ctx.enemyId, en->ctx.spellId);
    enemy_notes_clear();
    enemy_state_cleanup(en->ctx.enemyId);
    slot_free(SPELL_SLOT_ENEMY);

    reset_note_queue();
    notes_set_lock(0);
    set_idle();
    return true;
}

// ---------------------------------------------------------------------
// Cancelación (sin counter: hide corta thunder, rechazos)
// ---------------------------------------------------------------------
void spell_cancel(SpellSlot s)
{
    ActiveSpell *a = &slots[s];
    if (!a->def) return;

    dprintf(2,"Cancelling spell %d in slot %d", a->ctx.spellId, s);
    if (a->def->onCancel) a->def->onCancel(&a->ctx);

    if (s == SPELL_SLOT_ENEMY) {
        enemy_recharge_reset(a->ctx.enemyId, a->ctx.spellId);
        enemy_notes_clear();
        enemy_state_cleanup(a->ctx.enemyId);
        slot_free(s);
        combat_state = COMBAT_STATE_IDLE;
    } else {
        slot_free(s);
        reset_note_queue();
        set_idle();
    }
}

// ---------------------------------------------------------------------
// Fases declarativas
// ---------------------------------------------------------------------
static void run_phases(ActiveSpell *a)
{
    for (u8 i = 0; i < a->def->phaseCount; ++i)
    {
        const SpellPhase *ph = &a->def->phases[i];
        if (a->ctx.frameCounter < ph->startFrame || a->ctx.frameCounter > ph->endFrame)
            continue;

        switch (ph->kind)
        {
            case PHASE_VISUAL_FLASH:
                PAL_setColor(ph->p1, ph->p2);
                break;
            case PHASE_LOGIC_DAMAGE:
                if (a->ctx.frameCounter == ph->startFrame) { // acción puntual: una sola vez
                    if (ph->p1 == PHASE_TARGET_PLAYER) hit_player(ph->p2);
                    else if (a->ctx.enemyId != ENEMY_NONE) hit_enemy(a->ctx.enemyId, ph->p2);
                }
                break;
            default:
                break;
        }
    }
}

// ---------------------------------------------------------------------
// Update por frame (lo llama update_combat SIEMPRE, en cualquier estado)
// ---------------------------------------------------------------------
static void update_enemy_slot(void);
static void update_player_slot(void);

void spell_update(void)
{
    notes_lock_tick();      // lock global de input
    update_enemy_slot();
    update_player_slot();
}

static void update_enemy_slot(void)
{
    ActiveSpell *en = &slots[SPELL_SLOT_ENEMY];
    if (!en->def) return;

    if (en->phase == SLOT_NOTES)
    {
        if (combat_state != COMBAT_STATE_ENEMY_PLAYING) return; // suspendido (el jugador lanzó algo)

        // Prioridad del jugador: si está metiendo notas, el enemigo espera
        u8 pn = notes_player_count();
        if (pn > 0 && pn < 4) return;

        if (++enemy_note_timer >= (u16)ENEMY_FRAMES_PER_NOTE)
        {
            enemy_note_timer = 0;
            if (enemy_note_index < en->def->noteCount)
            {
                enemy_notes_add(en->ctx.enemyId, en->def->notes[enemy_note_index++]);
            }
            else // Notas completadas: arranca el efecto
            {
                dprintf(2,"Enemy %d finished notes. Launching spell %d.", en->ctx.enemyId, en->ctx.spellId);
                enemy_notes_clear();
                en->phase = SLOT_EFFECT;
                en->ctx.frameCounter = 0;
                combat_state = COMBAT_STATE_ENEMY_EFFECT;
                if (en->def->onLaunch) en->def->onLaunch(&en->ctx);
            }
        }
        return;
    }

    // SLOT_EFFECT
    if (combat_state != COMBAT_STATE_ENEMY_EFFECT) return; // suspendido

    // Prioridad del jugador: el timer del efecto se congela mientras teclea notas
    u8 pn = notes_player_count();
    bool frozen = (pn > 0 && pn < 4);

    if (!frozen) en->ctx.frameCounter++;
    run_phases(en);
    bool done = en->def->onUpdate ? en->def->onUpdate(&en->ctx) : false;
    if (done || en->ctx.frameCounter >= en->def->baseDuration)
    {
        dprintf(2,"Enemy spell %d finished", en->ctx.spellId);
        if (en->def->onFinish) en->def->onFinish(&en->ctx);
        enemy_recharge_reset(en->ctx.enemyId, en->ctx.spellId);
        enemy_notes_clear();
        enemy_state_cleanup(en->ctx.enemyId);
        slot_free(SPELL_SLOT_ENEMY);
        combat_state = COMBAT_STATE_IDLE;
    }
}

static void update_player_slot(void)
{
    ActiveSpell *pl = &slots[SPELL_SLOT_PLAYER];
    if (!pl->def) return;
    if (combat_state != COMBAT_STATE_PLAYER_EFFECT) return;

    pl->ctx.frameCounter++;
    run_phases(pl);
    bool done = pl->def->onUpdate ? pl->def->onUpdate(&pl->ctx) : false;
    if (done || pl->ctx.frameCounter >= pl->def->baseDuration)
    {
        dprintf(2,"Player spell %d finished", pl->ctx.spellId);
        if (pl->def->onFinish) pl->def->onFinish(&pl->ctx);
        slot_free(SPELL_SLOT_PLAYER);
        obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;

        // Retomar al enemigo suspendido, si lo hay; si no, a reposo.
        // (El sistema antiguo dejaba combat_state en ENEMY_PLAYING incluso sin
        // enemigo — estado zombi que se auto-curaba en combat_init; aquí va a idle.)
        if (spell_slot_active(SPELL_SLOT_ENEMY))
            combat_state = (spell_slot_phase(SPELL_SLOT_ENEMY) == SLOT_NOTES)
                         ? COMBAT_STATE_ENEMY_PLAYING : COMBAT_STATE_ENEMY_EFFECT;
        else
            set_idle();
    }
}

// ---------------------------------------------------------------------
// Lado enemigo: recargas y lanzamiento
// ---------------------------------------------------------------------
static void enemy_recharge_reset(u8 enemyId, u8 spellId)    // Recarga = baseDuration tras usar/cancelar (heredado)
{
    if (enemyId >= MAX_ENEMIES) return;
    const Enemy_Class *cls = &obj_enemy[enemyId].class;
    for (u8 s = 0; s < MAX_SPELLS_PER_ENEMY; ++s)
        if (cls->spell[s] == spellId)
            enemy_recharge[enemyId][s] = spell_defs[spellId].baseDuration;
}

void init_enemy_spells(u8 enemyId)    // Recargas iniciales al entrar en combate
{
    const Enemy_Class *cls = &obj_enemy[enemyId].class;
    for (u8 s = 0; s < MAX_SPELLS_PER_ENEMY; ++s)
    {
        u8 id = cls->spell[s];
        enemy_recharge[enemyId][s] = (id != SPELL_NONE) ? spell_defs[id].rechargeInit : 0;
    }
}

void spell_notify_enemy_released(u8 enemyId)    // El enemigo ha muerto/desaparecido
{
    if (slots[SPELL_SLOT_ENEMY].def && slots[SPELL_SLOT_ENEMY].ctx.enemyId == enemyId)
    {
        enemy_notes_clear();
        slot_free(SPELL_SLOT_ENEMY);
        combat_state = COMBAT_STATE_IDLE;
    }
}

bool spell_enemy_try_launch(void)    // Tick de recargas + lanza el primer hechizo enemigo listo
{
    // Sin lanzamientos mientras el jugador o algún enemigo está en HURT
    if (obj_character[active_character].state == STATE_HIT) return false;
    for (u8 e = 0; e < MAX_ENEMIES; ++e)
        if (obj_enemy[e].obj_character.active &&
            obj_enemy[e].obj_character.state == STATE_HIT)
            return false;

    // Tick de recargas
    for (u8 e = 0; e < MAX_ENEMIES; ++e)
        if (obj_enemy[e].obj_character.active)
            for (u8 s = 0; s < MAX_SPELLS_PER_ENEMY; ++s)
                if (enemy_recharge[e][s]) --enemy_recharge[e][s];

    // Primer hechizo listo
    for (u8 e = 0; e < MAX_ENEMIES; ++e)
    {
        if (!obj_enemy[e].obj_character.active) continue;
        const Enemy_Class *cls = &obj_enemy[e].class;
        for (u8 s = 0; s < MAX_SPELLS_PER_ENEMY; ++s)
        {
            u8 id = cls->spell[s];
            if (id == SPELL_NONE || enemy_recharge[e][s] != 0) continue;

            dprintf(2, "Enemy %d launching spell %d", e, id);
            enemy_notes_clear();
            slots[SPELL_SLOT_ENEMY].def = &spell_defs[id];
            slots[SPELL_SLOT_ENEMY].ctx = (SpellContext){
                .spellId = id, .origin = SPELL_ORIGIN_ENEMY, .reversed = false,
                .frameCounter = 0, .enemyId = e, .zoneId = spell_zone
            };
            slots[SPELL_SLOT_ENEMY].phase = SLOT_NOTES;
            enemy_note_timer = 0;

            // Primera nota inmediata + animación de "tocando"
            SPR_setAnim(spr_enemy[e], ANIM_ACTION);
            enemy_notes_add(e, spell_defs[id].notes[0]);
            enemy_note_index = 1;

            combat_state = COMBAT_STATE_ENEMY_PLAYING;
            return true;
        }
    }
    return false;
}

// Restaura el estado visual del enemigo al acabar/cancelar su hechizo
static void enemy_state_cleanup(u8 enemyId)
{
    enemy_note_index = 0;
    enemy_note_timer = 0;

    if (enemyId >= MAX_ENEMIES || !obj_enemy[enemyId].obj_character.active) return;
    if (obj_enemy[enemyId].obj_character.state == STATE_HIT) return; // en hurt/muriendo: no tocar

    obj_enemy[enemyId].obj_character.state = STATE_IDLE;
    SPR_setAnim(spr_enemy[enemyId], ANIM_IDLE);
}

// ---------------------------------------------------------------------
// Desbloqueo de hechizos del jugador
// ---------------------------------------------------------------------
void spell_enable(u8 spellId)    // Desbloquea sin feedback
{
    if (spellId < SPELL_PLAYER_COUNT) spell_defs[spellId].enabled = true;
}

void activate_spell(u16 spellId)    // Desbloquea con jingle + notas en el HUD (cutscenes)
{
    if (spellId >= SPELL_PLAYER_COUNT) return;
    SpellDef *d = &spell_defs[spellId];
    if (d->enabled) return;              // ya desbloqueado

    play_spell_jingle(spellId);
    show_pattern_icon(spellId, true, true);
    for (u8 i = 0; i < MAX_SPELL_NOTES; i++) {
        show_note(d->notes[i], true);
        play_player_note(d->notes[i]);
        wait_seconds(1);
        show_note(d->notes[i], false);
    }
    show_pattern_icon(spellId, false, false);

    d->enabled = true;
    dprintf(2, "Spell %d activated", spellId);
}
