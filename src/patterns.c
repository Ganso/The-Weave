// patterns.c  – static pattern system

#include "globals.h"

bool player_has_rod;          /* can physically use patterns?      */
bool player_patterns_enabled; /* not silenced by a cut-scene, etc. */
u8  current_note;         /* NOTE_MI … NOTE_DO or NOTE_NONE      */
u8  noteQueue[4] = {NOTE_NONE,NOTE_NONE,NOTE_NONE,NOTE_NONE}; // Queue of notes played by the player (up to 4)
static Sprite* spr_enemy_rod[6] = { NULL };   // Enemy rod sprites (1-6: MI-DO)
static bool    enemy_note_active[6] = { false }; // Active enemy notes (1-6: MI-DO)

// --------------------------------------------------------------------
// Static player-pattern table
// --------------------------------------------------------------------

PlayerPattern playerPatterns[MAX_PLAYER_PATTERNS];
EnemyPattern enemyPatterns[MAX_ENEMIES][MAX_PATTERN_ENEMY];

// Remember which enemy pattern was active when the player launched a spell
static u16 last_enemy_pattern = PATTERN_ENEMY_NONE;

u16 get_last_enemy_pattern(void)
{
    return last_enemy_pattern;
}

// ---------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------
static inline PlayerPattern* get_player_pattern(u16 id)
{
    return (id < PATTERN_PLAYER_COUNT) ? &playerPatterns[id] : NULL;
}

static inline EnemyPattern* get_enemy_pattern(u8 slot, u8 pslot)
{
    return (slot < MAX_ENEMIES && pslot < MAX_PATTERN_ENEMY)
           ? &enemyPatterns[slot][pslot]
           : NULL;
}

static inline bool player_pattern_enabled(u16 id)
{
    PlayerPattern* p = get_player_pattern(id);
    dprintf(2,"Checking player pattern %d: enabled=%d", id, p ? p->enabled : 0);
    return p && p->enabled;
}

// Devuelve true si el patrón del enemigo está habilitado y sin cooldown
static inline bool enemy_pattern_ready(u8 slot, u8 pslot)
{
    EnemyPattern* p = get_enemy_pattern(slot, pslot);
    return p && p->enabled && (p->rechargeFrames == 0);
}

// Return true if the 4 notes in the array form a palindrome (so it can be reversed)
static bool is_palindrome(const u8 n[4])
{ return (n[0]==n[3]) && (n[1]==n[2]); }


// ---------------------------------------------------------------------
// Player-side: Initialisation
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// init_player_patterns  –  set up the 4 default spells (thunder, hide,
//                        open, sleep) according to the new system.
// ---------------------------------------------------------------------
void init_player_patterns(void)
{
    dprintf(2,"Initializing player patterns");

    // 0) Limpia todos los slots
    for (u8 i = 0; i < MAX_PLAYER_PATTERNS; ++i)
    {
        playerPatterns[i] = (PlayerPattern){
            .id       = PATTERN_PLAYER_NONE,
            .enabled  = false,
            .notes    = { NOTE_NONE, NOTE_NONE, NOTE_NONE, NOTE_NONE },
            .baseDuration = 0,
            .canUse   = NULL,
            .launch   = NULL,
            .update   = NULL,
            .icon     = NULL
        };
    }

    // 1) Electric
    playerPatterns[PATTERN_THUNDER] = (PlayerPattern){
        .id       = PATTERN_THUNDER,
        .enabled  = false,
        .notes    = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }, // 1-2-3-4
        .baseDuration = SCREEN_FPS * 4, // 4 seconds
        // .baseDuration = 120, // 2 seconds (for testing)
        .canUse   = player_thunder_can_use,
        .launch   = player_thunder_launch,
        .update   = player_thunder_update,
        .icon     = NULL // Will load on demand
   };

    // 2) Hide
    playerPatterns[PATTERN_HIDE] = (PlayerPattern){
        .id       = PATTERN_HIDE,
        .enabled  = false,
        .notes    = { NOTE_FA, NOTE_SI, NOTE_SOL, NOTE_DO }, // 2-5-3-6
        .baseDuration = SCREEN_FPS * 4, // 4 seconds
        .canUse   = player_hide_can_use,
        .launch   = player_hide_launch,
        .update   = player_hide_update,
        .icon     = NULL // Will load on demand
    };

    // 3) Open
    playerPatterns[PATTERN_OPEN] = (PlayerPattern){
        .id       = PATTERN_OPEN,
        .enabled  = false,
        .notes    = { NOTE_FA, NOTE_SOL, NOTE_SOL, NOTE_FA }, // 2-3-3-2
        .baseDuration = 45,
        .canUse   = player_open_can_use,
        .launch   = player_open_launch,
        .update   = player_open_update,
        .icon     = NULL // Will load on demand
    };

    // 4) Sleep
    playerPatterns[PATTERN_SLEEP] = (PlayerPattern){
        .id       = PATTERN_SLEEP,
        .enabled  = false,
        .notes    = { NOTE_FA, NOTE_MI, NOTE_DO, NOTE_LA }, // 2-1-6-4
        .baseDuration = 75,
        .canUse   = player_sleep_can_use,
        .launch   = player_sleep_launch,
        .update   = player_sleep_update,
        .icon     = NULL // Will load on demand
    };

    dprintf(2,"Player patterns ready: E=%d H=%d O=%d S=%d",
            playerPatterns[PATTERN_THUNDER].enabled,
            playerPatterns[PATTERN_HIDE].enabled,
            playerPatterns[PATTERN_OPEN].enabled,
            playerPatterns[PATTERN_SLEEP].enabled);
}


// ---------------------------------------------------------------------
// Player-side: Spell activation (unlock new pattern)
// ---------------------------------------------------------------------
void activate_spell(u16 patternId)
{
    PlayerPattern* p = get_player_pattern(patternId);
    if (!p || p->enabled) return;            // already unlocked

    // simple feedback: play jingle & flash icon
    play_player_pattern_sound(patternId);
    show_pattern_icon(patternId, true, true);
    for (u8 i=0; i<4; i++) {
        show_note(p->notes[i], true);
        play_player_note(p->notes[i]);
        wait_seconds(1);
        show_note(p->notes[i], false);
    }
    show_pattern_icon(patternId, false, false);
    
    p->enabled = true;

    dprintf(2, "Pattern %d activated", patternId);
}

// ---------------------------------------------------------------------
// Player-side: launch / update
// ---------------------------------------------------------------------
void launch_player_pattern(u16 patternId)
{
    PlayerPattern* p = get_player_pattern(patternId);
    if (!p || !p->enabled)
        return;

    // Spell recognised but not usable right now → abort cleanly
    if (p->canUse && !p->canUse())
    {
        dprintf(2,"Pattern %d not usable right now", patternId);
        show_or_hide_interface(false); // hide interface
        talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN]); // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"
        if (interface_active==true) show_or_hide_interface(true); // show interface
        combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
        set_idle(); // reset combat state
        return;
    }

    if (try_counter_spell()) { reset_note_queue(); return; }

    // Remember current enemy pattern, if any
    if (combat_state == COMBAT_STATE_ENEMY_PLAYING ||
        combat_state == COMBAT_STATE_ENEMY_EFFECT)
        last_enemy_pattern = combatContext.activePattern;
    else
        last_enemy_pattern = PATTERN_ENEMY_NONE;

    // We are launching a pattern: Set combat and player context
    combatContext.activePattern = patternId;
    combatContext.effectTimer  = 0;
    combat_state = COMBAT_STATE_PLAYER_EFFECT;
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
    dprintf(2,"Launching player pattern %d", patternId);

    if (p->launch) p->launch();
}

bool update_player_pattern(void)
{
    if (combat_state != COMBAT_STATE_PLAYER_EFFECT)
        return true;                       // nothing to do

    PlayerPattern* p = get_player_pattern(combatContext.activePattern);
    if (!p) return true;                   // safety

    combatContext.effectTimer++;

    // If there's no update callback, we finish immediately
    bool finished = (p->update)
                  ? p->update()
                  : true;

    if (finished) {
        // Pattern finished, reset state
        combat_state = COMBAT_STATE_ENEMY_PLAYING;
        obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------
// Player-side: note input
// ---------------------------------------------------------------------

// Player presses a note (called from input layer)
bool pattern_player_add_note(u8 noteCode)
{
    // --- Guards ------------------------------------------------------
    if (noteCode < NOTE_MI || noteCode > NOTE_DO)           return false;

    if (combatContext.patternLockTimer) {                   // global lock
        dprintf(2,"Reject %d: pattern lock (%u frames left)",
                noteCode, combatContext.patternLockTimer);
        return false;
    }

    if (combatContext.noteTimer < MIN_TIME_BETWEEN_NOTES) { // debounce
        dprintf(2,"Reject %d: debounce (timer=%u < %u)",
                noteCode, combatContext.noteTimer, MIN_TIME_BETWEEN_NOTES);
        return false;
    }

    if (combatContext.playerNotes >= 4) {                   // should never hit
        dprintf(2,"Reject %d: queue full", noteCode);
        return false;
    }

    // --- Accept note -------------------------------------------------
    combatContext.noteTimer = 0;
    u8 slot = combatContext.playerNotes;
    noteQueue[slot] = noteCode;
    combatContext.playerNotes = slot + 1;

    show_note(noteCode, true);
    play_player_note(noteCode);

    dprintf(2,"NOTE OK %d -> slot %d (playerNotes=%u)",
            noteCode, slot, combatContext.playerNotes);

    // --- Validate when 4 notes reached -------------------------------
    if (combatContext.playerNotes == 4)
    {
        bool rev;
        u16 id = validate_pattern(noteQueue, &rev);

        combatContext.patternReversed = rev;
        PlayerPattern* pat = get_player_pattern(id);

        if (id != PATTERN_PLAYER_NONE && pat && pat->enabled &&
            (!pat->canUse || pat->canUse()))
        {
            dprintf(2,"Pattern %d recognised (reversed=%d)", id, rev);
            reset_note_queue();
            combatContext.patternReversed = rev;
            launch_player_pattern(id);                    // success
            combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
            return true; // valid pattern
        }
        else                                            // fail
        {
            dprintf(2,"Pattern invalid: %d (reversed=%d)", id, rev);
            reset_note_queue();
            play_player_pattern_sound(PATTERN_PLAYER_NONE); // play invalid sound
            combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
            obj_character[active_character].state = STATE_IDLE; // reset player state
            if (id != PATTERN_PLAYER_NONE)
            {
                // Pattern was valid but unusable right now
                dprintf(2, "Pattern %d not usable right now", id);
                bool resume_enemy = (combatContext.activeEnemy != ENEMY_NONE);
                set_idle();                    // reset combat state
                show_or_hide_interface(false); // hide interface

                // Default dialog is SYSTEM_DIALOG[0]
                DialogItem* dialog = (DialogItem*) &dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN];  // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"

                // If there's a Ghost and we're trying to launch a direct thunder spell, show specific message
                for (u8 i = 0; i < MAX_ENEMIES; i++)
                {
                    if (obj_enemy[i].class_id == ENEMY_CLS_WEAVERGHOST && id == PATTERN_THUNDER && !rev)
                    {
                        dialog = (DialogItem*) &dialogs[ACT1_DIALOG3][A1D3_THINK_BACKWARDS];
                        break; // No need to check other enemies
                    }
                }

                talk_dialog(dialog);           // Show dialog message
                show_or_hide_interface(true);  // show interface again
                if (resume_enemy)
                    combat_state = COMBAT_STATE_ENEMY_EFFECT; // resume enemy
            }
            return false; // invalid pattern
        }
    }

    // --- Sprite state ------------------------------------------------
    if (obj_character[active_character].state != STATE_PATTERN_EFFECT) {
        dprintf(2,"Player state: %d --> playing note", obj_character[active_character].state);
        obj_character[active_character].state = STATE_PLAYING_NOTE;
    }

    if (combat_state == COMBAT_STATE_IDLE) {
        dprintf(2,"Combat state: idle --> player playing");
        combat_state = COMBAT_STATE_PLAYER_PLAYING;
    }

    return true;
}


// Reset the note queue and player notes count
void reset_note_queue(void)
{
    dprintf(2,"Resetting note queue");
    for (u8 i = 0; i < 4; ++i)
    {
        if (noteQueue[i] != NOTE_NONE)
            show_note(noteQueue[i], false);         // hide icon
        noteQueue[i] = NOTE_NONE;
    }
    combatContext.playerNotes = 0;
}

// ---------------------------------------------------------------------
// Player-side: Cancel an active pattern
// ---------------------------------------------------------------------

// Cancel the current player pattern (e.g. if the player wants to stop playing)
void cancel_player_pattern(void)
{
    dprintf(2,"Cancelling player pattern");
    combatContext.activePattern = PATTERN_PLAYER_NONE;
    combatContext.playerNotes = 0;
    combatContext.patternReversed  = false;
    combatContext.patternLockTimer = 0;   // allow immediate re-input
    reset_note_queue();
    set_idle(); // reset combat state
}

// ---------------------------------------------------------------------
// Enemy-side: Initialisation
// ---------------------------------------------------------------------

// Initialize enemy patterns for a specific enemy slot
void init_enemy_patterns(u8 enemyId)
{
    /* --- THUNDER / ELECTRIC  (slot 0) --------------------------- */
    enemyPatterns[enemyId][0] = (EnemyPattern){
        .id             = PATTERN_EN_THUNDER,
        .notes         = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }, // 1-2-3-4
        .noteCount      = 4,
        .baseDuration   = SCREEN_FPS, 
        .rechargeFrames = SCREEN_FPS*3,
        .enabled        = (obj_enemy[enemyId].class.has_pattern[PATTERN_EN_THUNDER]),
        .launch         = enemy_thunder_launch,
        .update         = enemy_thunder_update,
        .counterable    = TRUE,
        .onCounter      = enemy_thunder_on_counter
    };

    /* --- BITE  (slot 1) ----------------------------------------- */
    enemyPatterns[enemyId][1] = (EnemyPattern){
        .id             = PATTERN_EN_BITE,
        .notes         = { NOTE_MI, NOTE_SOL, NOTE_DO }, // 1-3-6
        .noteCount      = 3,
        .baseDuration   = SCREEN_FPS,
        .rechargeFrames = SCREEN_FPS*2,
        .enabled        = (obj_enemy[enemyId].class.has_pattern[PATTERN_EN_BITE]),
        .launch         = enemy_bite_launch,
        .update         = enemy_bite_update,
        .counterable    = FALSE,
        .onCounter      = NULL
    };
}

// -----------------------------------------------------------------------
// Enemy-side: Add a note to the enemy pattern
// ------------------------------------------------------------------------
void pattern_enemy_add_note(u8 enemySlot, u8 noteCode)
{
    if (noteCode < NOTE_MI || noteCode > NOTE_DO) return;

    u8 idx = noteCode - NOTE_MI;       // 0-based 0‥5
    const SpriteDefinition* def = NULL;
    const u8*               sfx = NULL;
    u16 x = 24 + 32 * idx;             // Same spacing you used
    switch (noteCode)
    {
        case NOTE_MI: def = &int_enemy_rod_1_sprite; sfx = snd_enemy_note_mi;  break;
        case NOTE_FA: def = &int_enemy_rod_2_sprite; sfx = snd_enemy_note_fa;  break;
        case NOTE_SOL:def = &int_enemy_rod_3_sprite; sfx = snd_enemy_note_sol; break;
        case NOTE_LA: def = &int_enemy_rod_4_sprite; sfx = snd_enemy_note_la;  break;
        case NOTE_SI: def = &int_enemy_rod_5_sprite; sfx = snd_enemy_note_si;  break;
        default:      def = &int_enemy_rod_6_sprite; sfx = snd_enemy_note_do;  break;
    }

    if (!spr_enemy_rod[idx]) { // create once
        dprintf(2,"Adding sprite for enemy note %d at %d", noteCode, x);
        spr_enemy_rod[idx] =
            SPR_addSpriteSafe(def, x, 184, TILE_ATTR(PAL2, false, false, false));
        if (!spr_enemy_rod[idx]) return;        // VRAM full
    } else {
        dprintf(2,"Revealing sprite for enemy note %d at %d", noteCode, x);
        SPR_setVisibility(spr_enemy_rod[idx], VISIBLE);
    }
    enemy_note_active[idx] = true;
    play_music(sfx);
    dprintf(2, "Enemy %u playing note %u", enemySlot, noteCode);
}

// -------------------------------------------------------------------------
// Enemy-side: Remove every enemy-note sprite (called when the pattern ends)
// --------------------------------------------------------------------------
void pattern_enemy_clear_notes(void)
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

// ---------------------------------------------------------------------
// Enemy-side: launch / update
// ---------------------------------------------------------------------
void launch_enemy_pattern(u8 enemySlot, u16 patternSlot)
{
    EnemyPattern* pat = get_enemy_pattern(enemySlot, patternSlot);
    if (!pat || !pat->enabled) return;

    // clear any HUD leftovers from a previous pattern
    pattern_enemy_clear_notes();

    // Set combat context
    combatContext.activePattern = pat->id;
    combatContext.effectTimer   = 0;
    combatContext.activeEnemy   = enemySlot;
    combatContext.enemyNoteIndex = 0;
    combatContext.enemyNoteTimer = 0;
    combat_state                = COMBAT_STATE_ENEMY_PLAYING;

    // "playing" animation
    SPR_setAnim(spr_enemy[enemySlot], ANIM_ACTION);

    // Play first note
    if (pat->noteCount > 0) {
        pattern_enemy_add_note(enemySlot, pat->notes[0]); // Sound & HUD
        dprintf(2,"Enemy %d playing note %d", enemySlot, pat->notes[0]);
        combatContext.enemyNoteIndex = 1; // First note is already played
        combatContext.enemyNoteTimer = 0; // Reset timer for next note
    }

    // Pattern specific launch callback
    if (pat->launch) {
        dprintf(2,"Launching enemy pattern %d for enemy %d", pat->id, enemySlot);
        pat->launch(enemySlot);
    }
}

// ---------------------------------------------------------------------
// Update enemy pattern (called every frame)
// ---------------------------------------------------------------------
bool update_enemy_pattern(u8 enemySlot)
{
    EnemyPattern *pat = &enemyPatterns[enemySlot][0]; // Active pattern in slot 0

    if (!pat->enabled) return true;

    switch (combat_state)
    {
        case COMBAT_STATE_ENEMY_PLAYING:
            // Wait if player is playing notes (give player priority)
            if (combatContext.playerNotes > 0 && combatContext.playerNotes < 4)
                return false;

            // Increment timer for enemy note
            combatContext.enemyNoteTimer++;

            // Check if it is time to play the next note
            if (combatContext.enemyNoteTimer >= ENEMY_FRAMES_PER_NOTE)
            {
                combatContext.enemyNoteTimer = 0;

                // Play next note if not finished yet
                if (combatContext.enemyNoteIndex < pat->noteCount)
                {
                    u8 note = pat->notes[combatContext.enemyNoteIndex++];
                    dprintf(2,"Enemy %d playing note %d", enemySlot, note);
                    pattern_enemy_add_note(combatContext.activeEnemy, note); // Sound & HUD (implement HUD separately if needed)
                }
                else // All notes have been played
                {
                    dprintf(2,"Enemy %d finished playing notes. Launching pattern.", enemySlot);

                    // hide HUD before the effect
                    pattern_enemy_clear_notes();

                    // Switch state to effect (actual spell execution)
                    combat_state = COMBAT_STATE_ENEMY_EFFECT;
                    combatContext.effectTimer = 0;
                    combatContext.enemyNoteIndex = 0;
                    
                    // Call pattern launch callback (to start visual effects, animations, etc.)
                    if (pat->launch)
                        pat->launch(enemySlot);
                }
            }
            return false; // Pattern still playing notes

        case COMBAT_STATE_ENEMY_EFFECT:
            // Update ongoing pattern effect (visual/audio effect, damage)
            if (pat->update && pat->update(enemySlot))
            {
                // Pattern effect ended
                pat->rechargeFrames = pat->baseDuration; // Reset cooldown
                cancel_enemy_pattern(enemySlot);          // Restore combat state
                return true;
            }
            return false; // Pattern effect still running

        default:
            return true; // Idle or other states, nothing to update
    }
}

// ---------------------------------------------------------------------
// Validate 4-note sequence (returns PATTERN_PLAYER_NONE if invalid)
// ---------------------------------------------------------------------
u16 validate_pattern(const u8 notes[4], bool* reversed)
{
    for (u8 i = 0; i < PATTERN_PLAYER_COUNT; ++i)
    {
        PlayerPattern* p = &playerPatterns[i];
        dprintf(2,"Validating pattern %d. Queue: %d %d %d %d - Pattern: %d %d %d %d", i, notes[0], notes[1], notes[2], notes[3], p->notes[0], p->notes[1], p->notes[2], p->notes[3]);

        // direct order
        if (notes[0]==p->notes[0] && notes[1]==p->notes[1] &&
            notes[2]==p->notes[2] && notes[3]==p->notes[3])
        {
            dprintf(2,"Pattern %d recognised (direct order)", p->id);
            if (reversed) *reversed = false;
            return p->id;
        }

        // reversed order
        if (notes[0]==p->notes[3] && notes[1]==p->notes[2] &&
            notes[2]==p->notes[1] && notes[3]==p->notes[0])
        {
            if (reversed && !is_palindrome(p->notes)) {
                *reversed = true;
                dprintf(2,"Pattern %d recognised (reversed order)", p->id);
            }
            return p->id;
        }
    }
    if (reversed) *reversed = false;
    return PATTERN_PLAYER_NONE;
}

// ---------------------------------------------------------------------
// Cancel an enemy pattern (e.g. if the player counters it)
// ---------------------------------------------------------------------
void cancel_enemy_pattern(u8 enemyId)
{
    dprintf(2,"Finishing enemy pattern for enemy %d", enemyId);

    // Clear HUD
    pattern_enemy_clear_notes();
    
    // Reset combat context
    combatContext.activePattern = PATTERN_PLAYER_NONE;
    combatContext.activeEnemy = ENEMY_NONE;
    combatContext.enemyNoteIndex = 0;
    combatContext.enemyNoteTimer = 0;
    combatContext.effectTimer = 0;
    last_enemy_pattern = PATTERN_ENEMY_NONE;
    combat_state = COMBAT_STATE_IDLE;

    // Reset enemy state, if the enemy stil exists
    if (enemyId >= MAX_ENEMIES || !obj_enemy[enemyId].obj_character.active) {
        dprintf(2,"Enemy %d does not exist or is inactive", enemyId);
        return;
    }
    // Change to idle state except if the enemy is already hit
    if (obj_enemy[enemyId].obj_character.state == STATE_HIT) {
        dprintf(2,"Enemy %d is hit, keeping hit state", enemyId);
    } else {
        dprintf(2,"Enemy %d is idle, resetting state to idle", enemyId);
        obj_enemy[enemyId].obj_character.state = STATE_IDLE;
        SPR_setAnim(spr_enemy[enemyId], ANIM_IDLE);
    }
}
