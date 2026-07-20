// enemies.h — clases e instancias de enemigo (HP, IA de follow, hechizos)
#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include <genesis.h>
#include "actors/entity.h"

// Game constants
#define ENEMY_ENTITY_ID_BASE 100

// Enemies
#define MAX_ENEMIES 5
#define ENEMY_NONE 254
#define MAX_SPELLS_PER_ENEMY 2   // huecos de hechizo por clase de enemigo

// Enemy classes (B21: removed unimplemented 3-head-monkey class; re-add classes here when their art exists)
#define MAX_ENEMY_CLASSES 10
#define ENEMY_CLS_WEAVERGHOST    0
#define ENEMY_CLS_TESTGHOST      1   // SOLO para act1_test: dos hechizos (thunder + bite); no usar en el juego
#define ENEMY_CLS_BOAR           2   // Jabalí: enemigo físico del acto 1 (mordisco, no counterable)

// Enemy modes
typedef enum {
    ENEMY_MODE_IDLE,        /* doing nothing */
    ENEMY_MODE_PLAY_NOTE,   /* playing the 4 pre-spell notes */
    ENEMY_MODE_CASTING,     /* spell effect is currently running */
    ENEMY_MODE_COOLDOWN     /* post-spell recovery */
} EnemyMode;

// El ROL decide qué subsistema de combate dirige al enemigo (docs/combat.md)
typedef enum {
    ENEMY_ROLE_CONTACT,   // persigue y ataca de cerca (combat/contact.c)
    ENEMY_ROLE_RANGED     // canta patrones desde lejos (combat_state + spells)
} EnemyRole;

// Perfil del ataque de contacto (datos por clase): el del jabalí es el
// mordisco; otra clase tendrá otro alcance/ritmo/daño con la misma FSM
typedef struct {
    u16 range_x;       // alcance del ataque en X (entre centros de pies)
    u16 range_y;       // alcance en Y
    u16 attack_time;   // duración del ciclo de la anim de ataque (frames)
    u16 hit_at;        // frame del ciclo en el que golpea de verdad
    u8  damage;        // daño al jugador por impacto
} ContactProfile;

// Enemy classes
typedef struct
{
    u16 max_hitpoints;
    u8  role;                        // EnemyRole: contacto o a distancia
    const ContactProfile *contact;   // perfil de ataque de contacto (NULL si ranged)
    u8 spell[MAX_SPELLS_PER_ENEMY];  // SPELL_* que puede lanzar (SPELL_NONE = hueco; solo ranged)
} Enemy_Class;
extern Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES]; // Enemy class object

// Enemies
typedef struct
{
    Enemy_Class class;
    u16 class_id;
    Entity obj_character;
    u16 hitpoints;
    EnemyMode mode; // What's the enemy doing?
    u16 modeTimer; // Timer for the current mode (in frames)
} Enemy;


extern Enemy obj_enemy[MAX_ENEMIES]; // Enemy object
extern Sprite *spr_enemy[MAX_ENEMIES]; // Enemy sprites
extern Sprite *spr_enemy_shadow[MAX_ENEMIES]; // Enemy shadows sprites


/* --- Init / cleanup --- */
void init_enemy_classes(void); // Initialize enemy classes
void init_enemy(u16 numenemy, u16 class_id); // Initialize an enemy
void release_enemy(u16 nenemy); // Release an enemy

/* --- Runtime updates --- */
void update_enemy_shadow(u16 nenemy); // Update enemy shadow sprite
void update_enemy(u16 nenemy); // Update enemy state
void show_enemy(u16 nenemy, bool show); // Show or hide an enemy
void anim_enemy(u16 nenemy, u8 newanimation); // Change enemy animation
void look_enemy_left(u16 nenemy, bool direction_right); // Turn enemy left or right
void move_enemy(u16 nenemy, fastfix32 newx, fastfix32 newy); // Move enemy smoothly
void move_enemy_instant(u16 nenemy, fastfix32 x, fastfix32 y); // Instantly move enemy
void update_enemy_animations(void); // Update enemy animations based on their current state

#endif
