// patterns.c  – static pattern system

#include "globals.h"

bool player_has_rod;          /* can physically use patterns?      */
bool player_patterns_enabled; /* not silenced by a cut-scene, etc. */
u8  current_note;         /* NOTE_MI … NOTE_DO or NOTE_NONE      */
u8  noteQueue[4] = {NOTE_NONE,NOTE_NONE,NOTE_NONE,NOTE_NONE}; // Queue of notes played by the player (up to 4)

// --------------------------------------------------------------------
// Static player-pattern table
// --------------------------------------------------------------------

PlayerPattern playerPatterns[MAX_PLAYER_PATTERNS];
EnemyPattern enemyPatterns[MAX_ENEMIES][MAX_PATTERN_ENEMY];

// ---------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------
static inline PlayerPattern* getPlayerPattern(u16 id)
{
    return (id < PATTERN_PLAYER_COUNT) ? &playerPatterns[id] : NULL;
}

static inline EnemyPattern* getEnemyPattern(u8 slot, u8 pslot)
{
    return (slot < MAX_ENEMIES && pslot < MAX_PATTERN_ENEMY)
           ? &enemyPatterns[slot][pslot]
           : NULL;
}

static inline bool playerPatternEnabled(u16 id)
{
    PlayerPattern* p = getPlayerPattern(id);
    dprintf(2,"Checking player pattern %d: enabled=%d", id, p ? p->enabled : 0);
    return p && p->enabled;
}

// Devuelve true si el patrón del enemigo está habilitado y sin cooldown
static inline bool enemyPatternReady(u8 slot, u8 pslot)
{
    EnemyPattern* p = getEnemyPattern(slot, pslot);
    return p && p->enabled && (p->rechargeFrames == 0);
}

// Return true if the 4 notes in the array form a palindrome (so it can be reversed)
static bool isPalindrome(const u8 n[4])
{ return (n[0]==n[3]) && (n[1]==n[2]); }


// ---------------------------------------------------------------------
// Player-side: Initialisation
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// initPlayerPatterns  –  set up the 4 default spells (thunder, hide,
//                        open, sleep) according to the new system.
// ---------------------------------------------------------------------
void initPlayerPatterns(void)
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
        .canUse   = playerThunderCanUse,
        .launch   = playerThunderLaunch,
        .update   = playerThunderUpdate,
        .icon     = NULL // Will load on demand
   };

    // 2) Hide
    playerPatterns[PATTERN_HIDE] = (PlayerPattern){
        .id       = PATTERN_HIDE,
        .enabled  = false,
        .notes    = { NOTE_FA, NOTE_SI, NOTE_SOL, NOTE_DO }, // 2-5-3-6
        .baseDuration = SCREEN_FPS * 4, // 4 seconds
        .canUse   = playerHideCanUse,
        .launch   = playerHideLaunch,
        .update   = playerHideUpdate,
        .icon     = NULL // Will load on demand
    };

    // 3) Open
    playerPatterns[PATTERN_OPEN] = (PlayerPattern){
        .id       = PATTERN_OPEN,
        .enabled  = false,
        .notes    = { NOTE_FA, NOTE_SOL, NOTE_SOL, NOTE_FA }, // 2-3-3-2
        .baseDuration = 45,
        .canUse   = playerOpenCanUse,
        .launch   = playerOpenLaunch,
        .update   = playerOpenUpdate,
        .icon     = NULL // Will load on demand
    };

    // 4) Sleep
    playerPatterns[PATTERN_SLEEP] = (PlayerPattern){
        .id       = PATTERN_SLEEP,
        .enabled  = false,
        .notes    = { NOTE_FA, NOTE_MI, NOTE_DO, NOTE_LA }, // 2-1-6-4
        .baseDuration = 75,
        .canUse   = playerSleepCanUse,
        .launch   = playerSleepLaunch,
        .update   = playerSleepUpdate,
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
    PlayerPattern* p = getPlayerPattern(patternId);
    if (!p || p->enabled) return;            // already unlocked

    // simple feedback: play jingle & flash icon
    playPlayerPatternSound(patternId);
    show_pattern_icon(patternId, true, true);
    for (u8 i=0; i<4; i++) {
        show_note(p->notes[i], true);
        playPlayerNote(p->notes[i]);
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
void launchPlayerPattern(u16 patternId)
{
    PlayerPattern* p = getPlayerPattern(patternId);
    if (!p || !p->enabled)
        return;

    // Spell recognised but not usable right now → abort cleanly
    if (p->canUse && !p->canUse())
    {
        dprintf(2,"Pattern %d not usable right now", patternId);
        show_or_hide_interface(false); // hide interface
        talk_dialog(&dialogs[SYSTEM_DIALOG][0]); // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"
        if (interface_active==true) show_or_hide_interface(true); // show interface
        combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
        setIdle(); // reset combat state
        return;
    }

    if (tryCounterSpell()) { reset_note_queue(); return; }

    // We are launching a pattern: Set combat and player context
    combatContext.activePattern = patternId;
    combatContext.effectTimer  = 0;
    combat_state = COMBAT_STATE_PLAYER_EFFECT;
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
    dprintf(2,"Launching player pattern %d", patternId);

    if (p->launch) p->launch();
}

bool updatePlayerPattern(void)
{
    if (combat_state != COMBAT_STATE_PLAYER_EFFECT)
        return true;                       // nothing to do

    PlayerPattern* p = getPlayerPattern(combatContext.activePattern);
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
bool patternPlayerAddNote(u8 noteCode)
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
    playPlayerNote(noteCode);

    dprintf(2,"NOTE OK %d -> slot %d (playerNotes=%u)",
            noteCode, slot, combatContext.playerNotes);

    // --- Validate when 4 notes reached -------------------------------
    if (combatContext.playerNotes == 4)
    {
        bool rev;
        u16 id = validatePattern(noteQueue, &rev);

        combatContext.patternReversed = rev;
        PlayerPattern* pat = getPlayerPattern(id);

        if (id != PATTERN_PLAYER_NONE && pat && pat->enabled &&
            (!pat->canUse || pat->canUse()))
        {
            dprintf(2,"Pattern %d recognised (reversed=%d)", id, rev);
            reset_note_queue();
            combatContext.patternReversed = rev;
            launchPlayerPattern(id);                    // success
            combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
            return true; // valid pattern
        }
        else                                            // fail
        {
            dprintf(2,"Pattern invalid: %d (reversed=%d)", id, rev);
            reset_note_queue();
            playPlayerPatternSound(PATTERN_PLAYER_NONE); // play invalid sound
            combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
            obj_character[active_character].state = STATE_IDLE; // reset player state
            if (id != PATTERN_PLAYER_NONE) {
                // If the pattern was valid but not usable right now
                dprintf(2,"Pattern %d not usable right now", id);
                setIdle(); // reset combat state´
                show_or_hide_interface(false); // hide interface
                talk_dialog(&dialogs[SYSTEM_DIALOG][0]); // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"
                show_or_hide_interface(true); // show interface again
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
void cancelPlayerPattern(void)
{
    dprintf(2,"Cancelling player pattern");
    combatContext.activePattern = PATTERN_PLAYER_NONE;
    combatContext.playerNotes = 0;
    combatContext.patternReversed  = false;
    combatContext.patternLockTimer = 0;   // allow immediate re-input
    reset_note_queue();
    setIdle(); // reset combat state
}

// ---------------------------------------------------------------------
// Enemy-side: Initialisation
// ---------------------------------------------------------------------

// Initialize enemy patterns for a specific enemy slot
void initEnemyPatterns(u8 enemyId)
{
    /* --- THUNDER / ELECTRIC  (slot 0) --------------------------- */
    enemyPatterns[enemyId][0] = (EnemyPattern){
        .id             = PATTERN_EN_THUNDER,
        .notes         = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }, // 1-2-3-4
        .noteCount      = 4,
        .baseDuration   = SCREEN_FPS, 
        .rechargeFrames = SCREEN_FPS*3,
        .enabled        = (obj_enemy[enemyId].class.has_pattern[PATTERN_EN_THUNDER]),
        .launch         = enemyThunderLaunch,
        .update         = enemyThunderUpdate,
        .counterable    = TRUE,
        .onCounter      = enemyThunderOnCounter
    };

    /* --- BITE  (slot 1) ----------------------------------------- */
    enemyPatterns[enemyId][1] = (EnemyPattern){
        .id             = PATTERN_EN_BITE,
        .notes         = { NOTE_MI, NOTE_SOL, NOTE_DO }, // 1-3-6
        .noteCount      = 3,
        .baseDuration   = SCREEN_FPS,
        .rechargeFrames = SCREEN_FPS*2,
        .enabled        = (obj_enemy[enemyId].class.has_pattern[PATTERN_EN_BITE]),
        .launch         = enemyBiteLaunch,
        .update         = enemyBiteUpdate,
        .counterable    = FALSE,
        .onCounter      = NULL
    };
}


// ---------------------------------------------------------------------
// Enemy-side: launch / update
// ---------------------------------------------------------------------
void launchEnemyPattern(u8 enemySlot, u16 patternSlot)
{
    EnemyPattern* pat = getEnemyPattern(enemySlot, patternSlot);
    if (!pat || !pat->enabled) return;

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
        playEnemyNote(pat->notes[0]); // Sound & HUD
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
bool updateEnemyPattern(u8 enemySlot)
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
                    dprintf(2,"Enemy %d playing note %d", enemySlot, combatContext.enemyNoteIndex);
                    u8 note = pat->notes[combatContext.enemyNoteIndex++];
                    playEnemyNote(note); // Sound & HUD (implement HUD separately if needed)
                }
                else // All notes have been played
                {
                    dprintf(2,"Enemy %d finished playing notes. Launching pattern.", enemySlot);

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
                pat->rechargeFrames = pat->baseDuration; // Reset cooldown to baseDuration
                combat_state = COMBAT_STATE_IDLE;

                // Restore enemy to idle animation
                SPR_setAnim(spr_enemy[enemySlot], ANIM_IDLE);

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
u16 validatePattern(const u8 notes[4], bool* reversed)
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
            if (reversed && !isPalindrome(p->notes)) {
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
void cancelEnemyPattern(u8 enemyId)
{
    dprintf(2,"Finishing enemy pattern for enemy %d", enemyId);
    combatContext.activePattern = PATTERN_PLAYER_NONE;
    combatContext.activeEnemy = ENEMY_NONE;
    combatContext.enemyNoteIndex = 0;
    combatContext.enemyNoteTimer = 0;
    combatContext.effectTimer = 0;

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
