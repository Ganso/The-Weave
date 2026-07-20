/*
 * src/combat/combat.h — El director de combate
 * ----------------------------------------------
 * UN combate = enemigos presentes + una configuración de encuentro. El
 * director reparte cada enemigo activo a su subsistema según el ROL de su
 * clase (actors/enemies.h): CONTACTO → contact.c (persiguen y atacan de
 * cerca); A DISTANCIA → el motor de patrones (combat_state + spells/spell.c).
 * Ambos roles pueden convivir en el mismo encuentro.
 *
 * Como en los hechizos (SpellDef), la personalidad del encuentro se declara:
 * datos + hooks opcionales en CombatConfig. El hook de escena spawnea a los
 * enemigos y deja la config con combat_configure(); el op `combat` del DSL
 * ejecuta combat_run(). Sin config previa, el encuentro por defecto es "solo
 * patrones, se gana matando a todos".
 *
 * Receta en escena (siempre la misma):  call <hook_spawn> → combat →
 * if_defeated goto <reintento>. Detalle completo en docs/combat.md.
 *
 * combat_state NO es el estado del encuentro: es el FSM de turnos del motor
 * de PATRONES (spell.c/notes.c lo usan también en los cast libres fuera de
 * combate). set_idle() lo deja en IDLE solo si el encuentro tiene enemigos a
 * distancia vivos; en cualquier otro caso vuelve a COMBAT_NO.
 */
#ifndef _COMBAT_H_
#define _COMBAT_H_

#include <genesis.h>

// FSM de turnos del motor de PATRONES (no del encuentro; ver cabecera)
typedef enum
{
    COMBAT_NO,                          /* sin patrones en juego */
    COMBAT_STATE_IDLE,                 /* combate de patrones activo, todos esperando */
    /* --- turno del jugador (también en casts libres fuera de combate) ----- */
    COMBAT_STATE_PLAYER_PLAYING,       /* el jugador está tocando sus notas */
    COMBAT_STATE_PLAYER_EFFECT,        /* el efecto de su hechizo está corriendo */
    /* --- turno del enemigo ------------------------------------------------ */
    COMBAT_STATE_ENEMY_PLAYING,        /* el enemigo está tocando sus notas */
    COMBAT_STATE_ENEMY_EFFECT          /* el efecto de su hechizo está corriendo */
} CombatState;
extern CombatState combat_state;

// Vida del jugador durante los combates. Se reinicia a player_max_hitpoints
// al empezar cada encuentro; cámbiala antes si un combate concreto necesita
// otra cifra. Al llegar a 0 se marca player_defeated y el encuentro termina
// (el op if_defeated del DSL permite el reintento).
extern u16 player_max_hitpoints;  // vida inicial de cada combate (5 por defecto)
extern u16 player_hitpoints;      // vida restante del combate en curso
extern bool player_defeated;      // el último combate acabó con la vida a 0

// Timings
#define ENEMY_HURT_DURATION   SCREEN_FPS    // Duración de la animación de daño (enemigos)
#define PLAYER_HURT_DURATION  SCREEN_FPS/2  // Duración de la animación de daño (jugador)

// ---------------------------------------------------------------------------
// Configuración del encuentro (la deja el hook; la consume combat_start)
// ---------------------------------------------------------------------------
typedef struct {
    // --- Capacidades del jugador en este encuentro ---
    bool weapon_strike;          // el golpe físico con A está disponible
    // (cantar lo decide la escena con `set spells on/off`, como siempre)

    // --- Regla estándar de manada (0 = no aplica) ---
    u8   hits_to_win;            // impactos que ahuyentan a TODOS los de contacto

    // --- Acompañante ---
    u16  companion;              // CHR_* que espera quieto (CHR_NONE si no hay)
    bool reposition_companion;   // recolocarlo detrás del jugador al empezar

    /* Hooks opcionales (NULL si no hacen falta), como en SpellDef:
       onStart  al arrancar el encuentro (tras vida/acompañante/scroll)
       onTick   cada frame: reglas propias del encuentro (p.ej. la prefab
                combat_rule_thunder_scares de weapons.h)
       isWon    condición de victoria propia; NULL = "no quedan enemigos
                activos" (contacto huido / distancia muerta). Un combate
                futuro puede ganarse por puzzle, diálogo interno, etc.
       onEnd    al cerrar (victoria o derrota), antes de la limpieza común */
    void (*onStart)(void);
    void (*onTick) (void);
    bool (*isWon)  (void);
    void (*onEnd)  (void);
} CombatConfig;

// OJO: pon SIEMPRE .companion (CHR_NONE si no hay): el 0 implícito de C es
// CHR_linus. El director ignora un companion == personaje activo por si acaso.
void combat_configure(const CombatConfig *cfg); // el hook la deja lista (se consume al arrancar)
void combat_config_clear(void);                 // descarta la config pendiente (end_level)

// ---------------------------------------------------------------------------
// Ciclo de vida del encuentro
// ---------------------------------------------------------------------------
// Vía bloqueante (la usa el op `combat` de la VM y cualquier hook que lo
// necesite) y vía por pasos (la usa la instrumentación de la smoke ROM).
void combat_run(void);     // bloqueante: start + { next_frame + tick } + end
void combat_start(void);   // consume la config pendiente; vida, acompañante, scroll
void combat_tick(void);    // un frame: armas + reglas + contacto + ¿fin?
bool combat_running(void); // el encuentro sigue vivo (ni victoria ni derrota)
void combat_end(void);     // limpieza garantizada (única para victoria y derrota)

bool combat_ranged_present(void); // encuentro activo con enemigos a distancia vivos

// ---------------------------------------------------------------------------
// Daño (común a ambos roles) y bomba por-frame del motor de patrones
// ---------------------------------------------------------------------------
void hit_enemy(u8 enemyId, u8 damage); // resta HP; a 0 arranca la muerte diferida
void hit_player(u8 damage);            // resta vida; a 0 marca player_defeated
void update_combat(void); // cada frame (la llama next_frame): motor de patrones
void set_idle(void);      // repone combat_state según haya patrones en juego o no

#include "combat/contact.h"  // FSM de los enemigos de contacto
#include "combat/weapons.h"  // armas del jugador y reglas prefab

#endif
