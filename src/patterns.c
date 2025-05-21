// patterns.c  – static pattern system

#include "globals.h"


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
    return p && p->enabled && (!p->canUse || p->canUse());
}

// Devuelve true si el patrón del enemigo está habilitado y sin cooldown
static inline bool enemyPatternReady(u8 slot, u8 pslot)
{
    EnemyPattern* p = getEnemyPattern(slot, pslot);
    return p && p->enabled && (p->rechargeFrames == 0);
}


// ---------------------------------------------------------------------
// Player-side: Initialisation
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// initPlayerPatterns  –  set up the 4 default spells (thunder, hide,
//                        open, sleep) according to the new system.
// ---------------------------------------------------------------------
void initPlayerPatterns(void)
{
    kprintf("Initializing player patterns");

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
        .enabled  = true,
        .notes    = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }, // 1-2-3-4
        .baseDuration = 60,
        .canUse   = playerThuderCanUse,
        .launch   = playerThunderLaunch,
        .update   = playerThunderUpdate,
        .icon     = NULL // Will load on demand
   };

    // 2) Hide
    playerPatterns[PATTERN_HIDE] = (PlayerPattern){
        .id       = PATTERN_HIDE,
        .enabled  = true,
        .notes    = { NOTE_FA, NOTE_SI, NOTE_SOL, NOTE_DO }, // 2-5-3-6
        .baseDuration = 90,
        .canUse   = playerHideCanUse,
        .launch   = playerHideLaunch,
        .update   = playerHideUpdate,
        .icon     = NULL // Will load on demand
    };

    // 3) Open
    playerPatterns[PATTERN_OPEN] = (PlayerPattern){
        .id       = PATTERN_OPEN,
        .enabled  = true,
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
        .enabled  = true,
        .notes    = { NOTE_FA, NOTE_MI, NOTE_DO, NOTE_LA }, // 2-1-6-4
        .baseDuration = 75,
        .canUse   = playerSleepCanUse,
        .launch   = playerSleepLaunch,
        .update   = playerSleepUpdate,
        .icon     = NULL // Will load on demand
    };

    kprintf("Player patterns ready: E=%d H=%d O=%d S=%d",
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
    playPlayerPatternSound(patternId);       // sound layer only

    // PENDING - showPatternUnlockIcon(patternId);

    p->enabled = true;
}

// ---------------------------------------------------------------------
// Player-side: launch / update
// ---------------------------------------------------------------------
void launchPlayerPattern(u16 patternId)
{
    PlayerPattern* p = getPlayerPattern(patternId);
    if (!p || !p->enabled || (p->canUse && !p->canUse()))
        return;

    combatContext.activePattern = patternId;
    combatContext.effectTimer   = 0;
    combat_state                = COMBAT_STATE_PLAYER_EFFECT;

    if (p->launch) p->launch();
}

bool updatePlayerPattern(void)
{
    if (combat_state != COMBAT_STATE_PLAYER_EFFECT)
        return true;                       // nothing to do

    PlayerPattern* p = getPlayerPattern(combatContext.activePattern);
    if (!p) return true;                   // safety

    // If there's no update callback, we finish immediately
    bool finished = (p->update)
                  ? p->update()
                  : true;

    if (finished) {
        combat_state = COMBAT_STATE_ENEMY_PLAYING;  // or next phase
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------
// Player-side: note input
// ---------------------------------------------------------------------

// Player presses a note (called from input layer)
void patternPlayerAddNote(u8 noteCode)
{
    if (noteCode < NOTE_MI || noteCode > NOTE_DO) return;

    // reset and increment context counters
    combatContext.noteTimer   = 0;
    combatContext.playerNotes = (combatContext.playerNotes < 4)
                              ? combatContext.playerNotes + 1
                              : 4;

    // play SFX
    playPlayerNote(noteCode);

    // PENDING - display HUD icon for player's note
}


// ---------------------------------------------------------------------
// Enemy-side: Initialisation
// ---------------------------------------------------------------------
/*  Rellena la tabla de un enemigo concreto.
 *  Llama a esta función justo al entrar en combate
 *  para cada enemigo activo.                                       */
void initEnemyPatterns(u8 enemyId)
{
    /* --- THUNDER / ELECTRIC  (slot 0) --------------------------- */
    enemyPatterns[enemyId][0] = (EnemyPattern){
        .id             = PATTERN_EN_THUNDER,
        .noteCount      = 0,
        .baseDuration   = SCREEN_FPS, 
        .rechargeFrames = SCREEN_FPS*3,
        .enabled        = TRUE,
        .launch         = enemyThunderLaunch,
        .update         = enemyThunderUpdate,
        .counterable    = TRUE,
        .onCounter      = enemyThunderOnCounter
    };

    /* --- BITE  (slot 1) ----------------------------------------- */
    enemyPatterns[enemyId][1] = (EnemyPattern){
        .id             = PATTERN_EN_BITE,
        .noteCount      = 0,
        .baseDuration   = SCREEN_FPS,
        .rechargeFrames = SCREEN_FPS*2,
        .enabled        = TRUE,
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
    combat_state                = COMBAT_STATE_ENEMY_EFFECT;

    if (pat->launch) pat->launch(enemySlot);

    // start cooldown
    pat->rechargeFrames = pat->baseDuration;   // or any value you prefer
}

bool updateEnemyPattern(u8 enemySlot)
{
    if (combat_state != COMBAT_STATE_ENEMY_EFFECT ||
        combatContext.activeEnemy != enemySlot)
        return true;

    // Locate the pattern inside the enemy slot
    for (u8 pslot = 0; pslot < MAX_PATTERN_ENEMY; ++pslot)
    {
        EnemyPattern* pat = &enemyPatterns[enemySlot][pslot];
        if (pat->id == combatContext.activePattern)
        {
            // Tick cooldown if >0
            if (pat->rechargeFrames) --pat->rechargeFrames;

            bool finished = (pat->update)
                          ? pat->update(enemySlot)
                          : true;

            if (finished) {
                combat_state = COMBAT_STATE_PLAYER_PLAYING;
                return true;
            }
            return false;
        }
    }
    return true; // pattern not found → treat as finished
}

// ---------------------------------------------------------------------
// Enemy-side: note input
// ---------------------------------------------------------------------
void patternEnemyAddNote(u8 enemySlot, u8 noteCode)
{
    if (noteCode < NOTE_MI || noteCode > NOTE_DO) return;

    combatContext.noteTimer  = 0;
    combatContext.enemyNotes = (combatContext.enemyNotes < 4)
                             ? combatContext.enemyNotes + 1
                             : 4;

    playEnemyNote(noteCode);

    // PENDING - display HUD icon for enemy's note
}

// ---------------------------------------------------------------------
// Validate 4-note sequence (returns PATTERN_PLAYER_NONE if invalid)
// ---------------------------------------------------------------------
u16 validatePattern(const u8 notes[4], bool* reversed)
{
    for (u8 i = 0; i < PATTERN_PLAYER_COUNT; ++i)
    {
        PlayerPattern* p = &playerPatterns[i];

        // direct order
        if (notes[0]==p->notes[0] && notes[1]==p->notes[1] &&
            notes[2]==p->notes[2] && notes[3]==p->notes[3])
        {
            if (reversed) *reversed = false;
            return p->id;
        }

        // reversed order
        if (notes[0]==p->notes[3] && notes[1]==p->notes[2] &&
            notes[2]==p->notes[1] && notes[3]==p->notes[0])
        {
            if (reversed) *reversed = true;
            return p->id;
        }
    }
    if (reversed) *reversed = false;
    return PATTERN_PLAYER_NONE;
}