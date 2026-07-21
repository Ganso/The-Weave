// notes.c — input de notas del jugador y HUD de notas del enemigo.
// La cola del jugador valida a las 4 notas y delega en el motor (spell.c).

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "narrative/narrative.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"

bool player_has_rod;          // puede físicamente usar hechizos
u8 player_note_limit = NOTE_DO; // nota más alta disponible (acto 1: hasta LA tras el bastón)
bool player_patterns_enabled; // no silenciado por una cutscene

// Cola del jugador
static u8  note_queue[MAX_SPELL_NOTES];
static u8  player_notes;      // notas en cola (0-4)
static u16 note_timer;        // frames desde la última nota (debounce + timeout)
static u16 pattern_lock;      // lock global anti-mashing

// HUD de notas del enemigo
static Sprite* spr_enemy_rod[6] = { NULL };      // sprites de nota (1-6: MI-DO)
static bool    enemy_note_active[6] = { false };

// ---------------------------------------------------------------------
// Estado del input
// ---------------------------------------------------------------------
u8   notes_player_count(void)      { return player_notes; }
void notes_set_lock(u16 frames)    { pattern_lock = frames; }
bool notes_locked(void)            { return pattern_lock != 0; }
void notes_lock_tick(void)         { if (pattern_lock) --pattern_lock; }

void notes_input_reset(void)    // Reset completo (engine reset)
{
    for (u8 i = 0; i < MAX_SPELL_NOTES; ++i) note_queue[i] = NOTE_NONE;
    player_notes = 0;
    note_timer = 0;
    pattern_lock = 0;
}

void reset_note_queue(void)    // Vacía la cola y oculta los iconos de nota del HUD
{
    dprintf(2,"Resetting note queue");
    for (u8 i = 0; i < MAX_SPELL_NOTES; ++i)
    {
        if (note_queue[i] != NOTE_NONE)
            show_note(note_queue[i], false);
        note_queue[i] = NOTE_NONE;
    }
    player_notes = 0;
}

// ---------------------------------------------------------------------
// Input del jugador
// ---------------------------------------------------------------------
bool spell_note_input(u8 noteCode)    // El jugador pulsa una nota (desde controller)
{
    // --- Guards ------------------------------------------------------
    if (noteCode < NOTE_MI || noteCode > NOTE_DO)           return false;

    if (noteCode > player_note_limit) {                     // nota aún no disponible
        dprintf(2,"Reject %d: por encima del límite de notas (%u)", noteCode, player_note_limit);
        play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
        reset_note_queue();                                 // cancela el patrón en curso
        talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_NOTE_LOCKED], false); // "Aún no soy capaz de tocar esa nota"
        return false;
    }

    if (pattern_lock) {                                     // lock global
        dprintf(2,"Reject %d: pattern lock (%u frames left)", noteCode, pattern_lock);
        return false;
    }

    if (note_timer < (u16)MIN_TIME_BETWEEN_NOTES) {         // debounce
        dprintf(2,"Reject %d: debounce (timer=%u)", noteCode, note_timer);
        return false;
    }

    if (player_notes >= MAX_SPELL_NOTES) {                  // no debería pasar
        dprintf(2,"Reject %d: queue full", noteCode);
        return false;
    }

    // --- Aceptar nota -------------------------------------------------
    note_timer = 0;
    note_queue[player_notes++] = noteCode;

    show_note(noteCode, true);
    play_player_note(noteCode);

    dprintf(2,"NOTE OK %d (playerNotes=%u)", noteCode, player_notes);

    // --- Validar al llegar a 4 notas -----------------------------------
    if (player_notes == MAX_SPELL_NOTES)
    {
        bool rev;
        u8 id = spell_validate(note_queue, &rev);

        if (id != SPELL_NONE && spell_defs[id].enabled && spell_can_use(id, rev))
        {
            dprintf(2,"Spell %d recognised (reversed=%d)", id, rev);
            reset_note_queue();
            spell_player_cast(id, rev);
            pattern_lock = MIN_TIME_BETWEEN_PATTERNS;
            return true;
        }
        else // No reconocido, no desbloqueado o no usable ahora
        {
            dprintf(2,"Spell invalid: %d (reversed=%d)", id, rev);
            reset_note_queue();
            play_spell_jingle(SPELL_NONE);                  // sonido de "inválido"
            pattern_lock = MIN_TIME_BETWEEN_PATTERNS;
            obj_character[active_character].state = STATE_IDLE;

            if (id != SPELL_NONE && spell_defs[id].enabled)
                spell_reject(id, rev);                      // feedback + resume del enemigo
            return false;
        }
    }

    // --- Estado del sprite mientras teclea -----------------------------
    if (obj_character[active_character].state != STATE_PATTERN_EFFECT) {
        obj_character[active_character].state = STATE_PLAYING_NOTE;
    }

    if (combat_state == COMBAT_STATE_IDLE) {
        dprintf(2,"Combat state: idle --> player playing");
        combat_state = COMBAT_STATE_PLAYER_PLAYING;
    }

    return true;
}

void spell_input_update(void)    // Timeout de patrón a medias (cada frame interactivo)
{
    ++note_timer;

    if (player_notes &&
        note_timer > calc_ticks(MAX_PATTERN_WAIT_TIME))
    {
        dprintf(1,"Pattern aborted after %u ticks", note_timer);

        reset_note_queue();
        obj_character[active_character].state = STATE_IDLE;
        play_spell_jingle(SPELL_NONE);

        pattern_lock = MIN_TIME_BETWEEN_PATTERNS;
        set_idle();
    }
}

// ---------------------------------------------------------------------
// HUD de notas del enemigo
// ---------------------------------------------------------------------
void enemy_notes_add(u8 enemySlot, u8 noteCode)    // Muestra sprite + sonido de una nota enemiga
{
    if (noteCode < NOTE_MI || noteCode > NOTE_DO) return;

    u8 idx = noteCode - NOTE_MI;       // 0-based 0‥5
    const SpriteDefinition* def = NULL;
    const u8*               sfx = NULL;
    u16 x = 24 + 32 * idx;
    switch (noteCode)
    {
        case NOTE_MI: def = &int_enemy_rod_1_sprite; sfx = snd_enemy_note_mi;  break;
        case NOTE_FA: def = &int_enemy_rod_2_sprite; sfx = snd_enemy_note_fa;  break;
        case NOTE_SOL:def = &int_enemy_rod_3_sprite; sfx = snd_enemy_note_sol; break;
        case NOTE_LA: def = &int_enemy_rod_4_sprite; sfx = snd_enemy_note_la;  break;
        case NOTE_SI: def = &int_enemy_rod_5_sprite; sfx = snd_enemy_note_si;  break;
        default:      def = &int_enemy_rod_6_sprite; sfx = snd_enemy_note_do;  break;
    }

    if (!spr_enemy_rod[idx]) { // crear una vez
        spr_enemy_rod[idx] =
            SPR_addSpriteSafe(def, x, 184, TILE_ATTR(PAL_INTERFACE, false, false, false));
        if (!spr_enemy_rod[idx]) return;        // VRAM llena
    } else {
        SPR_setVisibility(spr_enemy_rod[idx], VISIBLE);
    }
    enemy_note_active[idx] = true;
    play_music(sfx);
    dprintf(2, "Enemy %u playing note %u", enemySlot, noteCode);
}

void enemy_notes_clear(void)    // Libera todos los sprites de nota enemiga
{
    for (u8 i = 0; i < 6; ++i)
    {
        if (spr_enemy_rod[i])
        {
            SPR_releaseSprite(spr_enemy_rod[i]);
            spr_enemy_rod[i] = NULL;
        }
        enemy_note_active[i] = false;
    }
}

void hide_enemy_notes(void)    // Oculta temporalmente (durante diálogos)
{
    for (u8 i = 0; i < 6; ++i)
        if (spr_enemy_rod[i] && enemy_note_active[i])
            SPR_setVisibility(spr_enemy_rod[i], HIDDEN);
}

void show_enemy_notes(void)    // Vuelve a mostrar las notas activas
{
    for (u8 i = 0; i < 6; ++i)
        if (spr_enemy_rod[i] && enemy_note_active[i])
            SPR_setVisibility(spr_enemy_rod[i], VISIBLE);
}
