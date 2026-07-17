/*
 * src/spells/spell.h — Sistema de hechizos
 * ------------------------------------------
 * Un hechizo es una SpellDef: datos (notas, duración) + fases declarativas
 * opcionales + hooks opcionales. El motor (spell.c) gestiona el ciclo de vida
 * en DOS slots simultáneos — jugador y enemigo — imprescindibles para el
 * counter (el hechizo enemigo sigue vivo mientras el jugador lanza el
 * invertido). Los hooks nunca gestionan timers ni cleanup: eso es del motor.
 *
 * Archivos:
 *   spell.c          — motor: validate, cast, update, try_counter, cancel, slots
 *   notes.c          — input de notas del jugador (cola, debounce, timeout) y
 *                      HUD de notas del enemigo
 *   player_spells.c  — defs + hooks de thunder/hide/open/sleep
 *   enemy_spells.c   — defs + hooks de en_thunder/en_bite
 *   fire.c           — ejemplo end-to-end: zona + counter + fases declarativas
 *
 * Data flow en un frame de combate:
 *   controller → spell_note_input(nota) → [4 notas] spell_validate →
 *     spell_player_cast → (counter | launch slot PLAYER)
 *   update_combat → spell_update() → cadencia de notas del enemigo /
 *     fases + onUpdate de cada slot → fin natural → onFinish → slot libre
 *
 * combat_state (combat.c) sigue siendo el director de orquesta: el motor lo
 * consulta para saber qué slot avanza y lo actualiza en las transiciones.
 *
 * Ver docs/spells.md (Fase 6) y AGENTS.md para la guía de autoría.
 */
#ifndef _SPELL_H_
#define _SPELL_H_

#include <genesis.h>
#include "spells/constants_spells.h"
#include "actors/enemies.h"     // MAX_ENEMIES, MAX_SPELLS_PER_ENEMY

// Quién lanza el hechizo
typedef enum {
    SPELL_ORIGIN_PLAYER    = 0,
    SPELL_ORIGIN_ENEMY     = 1,
    SPELL_ORIGIN_NARRATIVE = 2    // lanzado por script de escena (Fase 5)
} SpellOrigin;

// Slot de ejecución: dos hechizos pueden estar vivos a la vez (counter)
typedef enum {
    SPELL_SLOT_PLAYER = 0,
    SPELL_SLOT_ENEMY  = 1,
    SPELL_SLOT_COUNT  = 2
} SpellSlot;

// Fase de vida de un slot
typedef enum {
    SLOT_FREE   = 0,
    SLOT_NOTES  = 1,   // el enemigo está tocando sus notas (pre-efecto)
    SLOT_EFFECT = 2    // el efecto está corriendo (frameCounter avanza)
} SpellSlotPhase;

// Tipos de fase declarativa
typedef enum {
    PHASE_VISUAL_FLASH = 0,   // fija un color de paleta: p1=PAL_ENTRY, p2=color VDP
    PHASE_LOGIC_DAMAGE = 1    // aplica daño (dispara cuando frameCounter==startFrame): p1=PhaseTarget, p2=cantidad
} PhaseKind;

typedef enum {
    PHASE_TARGET_ENEMY_ACTIVE = 0,   // el enemigo del slot ENEMY (o el ctx->enemyId)
    PHASE_TARGET_PLAYER       = 1
} PhaseTarget;

// Una fase declarativa. start/end se rellenan en runtime con SCREEN_FPS (D11).
// Activa cada frame con frameCounter en [startFrame,endFrame]; para acciones
// puntuales (daño) usar startFrame==endFrame.
typedef struct {
    u16 startFrame;
    u16 endFrame;
    u8  kind;      // PhaseKind
    u16 p1, p2;    // parámetros según kind (u16: cabe un color VDP)
} SpellPhase;

// Contexto de un hechizo activo. SOLO el motor escribe frameCounter.
typedef struct {
    u8   spellId;
    u8   origin;        // SpellOrigin
    bool reversed;      // lanzado invertido
    u16  frameCounter;  // frames desde el inicio del efecto
    u8   enemyId;       // ORIGIN_ENEMY: quién lanza. ORIGIN_PLAYER: enemigo activo o ENEMY_NONE
    u8   zoneId;        // zona narrativa en el momento del lanzamiento
} SpellContext;

// Definición de un hechizo. Es lo que rellena el autor (ver player_spells.c).
typedef struct {
    u16  id;
    u8   notes[MAX_SPELL_NOTES];
    u8   noteCount;          // 1..4 (en_bite usa 3)
    bool isPalindrome;       // notas == invertidas → no existe forma invertida
    bool counterable;        // el jugador puede contrarrestarlo (enemigos)
    u16  baseDuration;       // frames de efecto; rellenado en runtime (SCREEN_FPS)
    u16  rechargeInit;       // frames de recarga inicial al entrar en combate (enemigos)
    bool enabled;            // jugador: desbloqueado (activate_spell)

    /* Hooks opcionales (NULL si no hacen falta). El motor llama:
       canUse     antes de lanzar; false → onRejected (o diálogo genérico)
       onRejected feedback al jugador tras el rechazo (hints con diálogo)
       onLaunch   al empezar el efecto (enemigos: al acabar sus notas)
       onUpdate   cada frame de efecto; true = terminar ya (además del auto-fin
                  por baseDuration, que aplica siempre)
       onCounter  al ser contrarrestado (daño al lanzador, etc.)
       onCancel   al cancelarse sin counter (hide corta thunder, rechazos)
       onFinish   SOLO al terminar de forma natural */
    bool (*canUse)    (const SpellContext *ctx);
    void (*onRejected)(SpellContext *ctx);
    void (*onLaunch)  (SpellContext *ctx);
    bool (*onUpdate)  (SpellContext *ctx);
    void (*onCounter) (SpellContext *ctx);
    void (*onCancel)  (SpellContext *ctx);
    void (*onFinish)  (SpellContext *ctx);

    /* Fases declarativas opcionales */
    const SpellPhase *phases;
    u8 phaseCount;

    Sprite* icon;            // icono HUD/pausa (se carga bajo demanda)
} SpellDef;

// Tabla única de hechizos, indexada por SPELL_*. Se rellena en init_spells()
// (runtime, para escalar duraciones con SCREEN_FPS — que es variable PAL/NTSC).
extern SpellDef spell_defs[SPELL_COUNT];

// Zona narrativa actual (ZONE_*; la fija la escena; ZONE_NONE por defecto)
extern u8 spell_zone;

// canUse reutilizable: hechizo SOLO guionizado (el jugador nunca puede tocarlo
// libremente; se lanza desde cutscenes con `cast`). Lo usan OPEN, SLEEP, HEAL.
bool spell_scripted_only_can_use(const SpellContext *ctx);

// --- Ciclo de vida ---
void init_spells(void);            // Rellena spell_defs y resetea el motor (una vez, en initialize)
void spell_engine_reset(void);     // Libera slots y resetea input (new_level/end_level)
u8   spell_validate(const u8 *notes, bool *reversed); // 4 notas → SPELL_* del jugador o SPELL_NONE
bool spell_can_use(u8 spellId, bool reversed);        // Evalúa canUse con un contexto temporal
void spell_player_cast(u8 spellId, bool reversed);    // Lanza (o contrarresta) un hechizo del jugador
void spell_narrative_cast(u8 spellId, bool reversed); // Cast scripted desde una escena (origin NARRATIVE, sin canUse)
void spell_reject(u8 spellId, bool reversed);         // Feedback de "no puedo usarlo ahora" + resume del enemigo
void spell_update(void);           // Avanza ambos slots; llamar cada frame desde update_combat
bool spell_try_counter(void);      // Intenta contrarrestar el hechizo del slot ENEMY
void spell_cancel(SpellSlot s);    // Cancela el slot (onCancel, sin onFinish)

// --- Consultas (para hooks, FSM de combate, HUD, escenas) ---
bool spell_slot_active(SpellSlot s);
u8   spell_active_id(SpellSlot s);         // SPELL_NONE si libre
SpellSlotPhase spell_slot_phase(SpellSlot s);
u8   spell_enemy_caster(void);             // enemyId del slot ENEMY, o ENEMY_NONE
SpellContext* spell_ctx(SpellSlot s);      // contexto del slot (NULL si libre)

// --- Desbloqueo (jugador) ---
void spell_enable(u8 spellId);     // Desbloquea sin feedback
void activate_spell(u16 spellId);  // Desbloquea con jingle + notas en el HUD (cutscenes)

// --- Lado enemigo (lo llama el FSM de combate) ---
void init_enemy_spells(u8 enemyId);        // Resetea recargas del enemigo (combat_init)
bool spell_enemy_try_launch(void);         // Tick de recargas + lanza el primer hechizo listo
void spell_notify_enemy_released(u8 enemyId); // Un enemigo ha sido liberado (release_enemy)

#endif
