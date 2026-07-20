// enemies.c — enemigos: init, IA de movimiento, animaciones y liberación
#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "res_all.h"

Enemy obj_enemy[MAX_ENEMIES];                    // Array of enemy instances with their properties
Sprite *spr_enemy[MAX_ENEMIES];                  // Array of enemy sprites
Sprite *spr_enemy_shadow[MAX_ENEMIES];           // Array of enemy shadow sprites
Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES];  // Array of enemy class definitions

void update_enemy_shadow(u16 nenemy)    // Update shadow sprite position based on enemy position
{
    if (obj_enemy[nenemy].obj_character.drops_shadow && spr_enemy_shadow[nenemy] != NULL) {
        // Position shadow at the bottom of enemy's collision box
        s16 shadow_x = FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.x);
        s16 shadow_y = FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.y) + obj_enemy[nenemy].obj_character.collision_y_offset - 4;
        
        // Flip shadow if enemy is looking to the left
        SPR_setHFlip(spr_enemy_shadow[nenemy], obj_enemy[nenemy].obj_character.flipH);

        // Set shadow position
        SPR_setPosition(spr_enemy_shadow[nenemy], shadow_x, shadow_y);
    }
}

// Perfil de contacto del jabalí: el mordisco (valores afinados en playtest)
static const ContactProfile boar_bite = {
    .range_x = 34, .range_y = 8,
    .attack_time = 48,   // ciclo de ANIM_ACTION (6 frames x timer 8)
    .hit_at = 24,        // muerde a mitad del ciclo
    .damage = 1,
};

// Doctrina de combate (docs/combat.md): cada enemigo es de CONTACTO (persigue
// y ataca de cerca, con su ContactProfile) o A DISTANCIA (canta patrones, con
// sus spells); el jugador puede o no cantar según la escena (flag spells).
void init_enemy_classes(void)    // Setup enemy class definitions
{
    obj_enemy_class[ENEMY_CLS_WEAVERGHOST]=(Enemy_Class) { 2, ENEMY_ROLE_RANGED, NULL, {SPELL_EN_THUNDER, SPELL_NONE}};
    obj_enemy_class[ENEMY_CLS_TESTGHOST]=(Enemy_Class) { 2, ENEMY_ROLE_RANGED, NULL, {SPELL_EN_THUNDER, SPELL_EN_BITE}}; // SOLO TEST: multi-hechizo
    obj_enemy_class[ENEMY_CLS_BOAR]=(Enemy_Class) { 2, ENEMY_ROLE_CONTACT, &boar_bite, {SPELL_NONE, SPELL_NONE}};
}


void init_enemy(u16 numenemy, u16 class)    // Create new enemy instance of given class with sprites and collision
{
    u16 i;
    u8 npal = PAL3;
    u8 x_size, y_size;
    u8 collision_x_offset=0,collision_y_offset=0,collision_width=0,collision_height=0;
    bool drops_shadow=true;
    const SpriteDefinition *nsprite = NULL;
    const SpriteDefinition *nsprite_shadow = NULL;

    obj_enemy[numenemy].class_id=class;
    obj_enemy[numenemy].class=obj_enemy_class[class];
    obj_enemy[numenemy].hitpoints=obj_enemy_class[class].max_hitpoints;

    // Set specific attributes based on enemy class
    switch (class)
    {
    case ENEMY_CLS_WEAVERGHOST:
    case ENEMY_CLS_TESTGHOST:   // la clase de test reutiliza el sprite del ghost
        nsprite = &weaver_ghost_sprite;
        nsprite_shadow = NULL;
        drops_shadow=false;
        break;
    case ENEMY_CLS_BOAR:
        nsprite = &boar_sprite;
        nsprite_shadow = &boar_shadow_sprite;
        drops_shadow=true;
        break;
    default:
        return;
    }
    
    // Get width and height from the sprite definition
    x_size = nsprite->w; 
    y_size = nsprite->h;
    // Set default collision box if not defined previously
    if (collision_width==0) collision_width=x_size/2; // Half width size
    if (collision_x_offset==0) collision_x_offset=x_size/4; // Centered in X
    if (collision_height==0) collision_height=2; // Two lines height
    if (collision_y_offset==0) collision_y_offset=y_size-1; // At the feet

    // Initialize enemy character with sprite, position, and collision attributes
    obj_enemy[numenemy].obj_character = (Entity) {
        true, nsprite, nsprite_shadow,
        FASTFIX32_FROM_INT(0), FASTFIX32_FROM_INT(0),
        FASTFIX32_FROM_INT(0),           // speed: la locomoción la dirige el combate
        x_size, y_size, npal, false, false,
        ANIM_IDLE, false, collision_x_offset, collision_y_offset,
        collision_width, collision_height, STATE_IDLE,
        false,                           // follows_character: solo lo usan los personajes
        drops_shadow, 0
    };

    // Add enemy sprite if not already present
    if (spr_enemy[numenemy]==NULL) spr_enemy[numenemy] = SPR_addSpriteSafe(nsprite, FASTFIX32_TO_INT(obj_enemy[numenemy].obj_character.x), FASTFIX32_TO_INT(obj_enemy[numenemy].obj_character.y),
                                       TILE_ATTR(npal, obj_enemy[numenemy].obj_character.priority, false, obj_enemy[numenemy].obj_character.flipH));
    
    // Initialize shadow if enemy drops one
    if (obj_enemy[numenemy].obj_character.drops_shadow) {
        if (spr_enemy_shadow[numenemy] == NULL) {
            spr_enemy_shadow[numenemy] = SPR_addSpriteSafe(nsprite_shadow, 0, 0, TILE_ATTR(npal, TRUE, FALSE, FALSE));
        }
        if (spr_enemy_shadow[numenemy] != NULL) {
            SPR_setVisibility(spr_enemy_shadow[numenemy], HIDDEN);
            SPR_setDepth(spr_enemy_shadow[numenemy], SPR_MAX_DEPTH); // Shadow always at back
            update_enemy_shadow(numenemy);
        }
    }

    // Initially hide enemy sprites
    SPR_setVisibility(spr_enemy[numenemy], HIDDEN);
    
    init_enemy_spells(numenemy); // Recargas iniciales de sus hechizos
    (void)i;
}

void release_enemy(u16 nenemy)    // Free enemy resources and reset related combat state
{

    /* Si este enemigo estaba lanzando, el motor libera su slot y resetea el estado */
    spell_notify_enemy_released(nenemy);

    /* Hide any note indicators (through interface) */
    enemy_notes_clear();

    /* Deactivate entity */
    obj_enemy[nenemy].obj_character.active = false;

    /* Release sprites */
    if (spr_enemy[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy[nenemy]);
        spr_enemy[nenemy] = NULL;
    }
    if (spr_enemy_shadow[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy_shadow[nenemy]);
        spr_enemy_shadow[nenemy] = NULL;
    }
}

void update_enemy(u16 nenemy)    // Update enemy sprite properties from current state
{
    SPR_setPosition(spr_enemy[nenemy], FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.x), FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.y));
    SPR_setPriority(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.priority);
    SPR_setVisibility(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.visible ? VISIBLE : HIDDEN);
    SPR_setHFlip(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.flipH);
    SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    update_enemy_shadow(nenemy);
    // OJO: aquí NO va SPR_update(). Esta función se llama por enemigo y por frame
    // (combate de contacto); el SPR_update() global de next_frame ya recoge
    // los cambios. Un SPR_update() por enemigo hundía el framerate: con 5
    // jabalíes el framerate caía de 50 a ~25 FPS (medido en RetroArch).
}

void show_enemy(u16 nenemy, bool show)    // Toggle visibility of enemy and its shadow
{
    obj_enemy[nenemy].obj_character.visible = show;
    SPR_setVisibility(spr_enemy[nenemy], show ? VISIBLE : HIDDEN);
    
    // Update shadow visibility if it exists
    if (obj_enemy[nenemy].obj_character.drops_shadow && spr_enemy_shadow[nenemy] != NULL) {
        SPR_setVisibility(spr_enemy_shadow[nenemy], show ? VISIBLE : HIDDEN);
    }
    
    SPR_update();
}

void anim_enemy(u16 nenemy, u8 newanimation)    // Set enemy animation if different from current
{
    dprintf(2,"Enemy %d animation %d\n", nenemy, newanimation);
    if (obj_enemy[nenemy].obj_character.animation != newanimation) {
        obj_enemy[nenemy].obj_character.animation = newanimation;
        SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    }
}

void look_enemy_left(u16 nenemy, bool direction_right)    // Set enemy sprite horizontal flip
{
    obj_enemy[nenemy].obj_character.flipH = direction_right;
    SPR_setHFlip(spr_enemy[nenemy], direction_right);
    update_enemy_shadow(nenemy);
    SPR_update();
}

void move_enemy(u16 nenemy, fastfix32 newx, fastfix32 newy)    // Move enemy with walking animation and direction update
{
    show_enemy(nenemy, true);
    anim_enemy(nenemy, ANIM_WALK);

    // Determine direction to face based on movement
    s16 dx = FASTFIX32_TO_INT(newx) - FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.x);
    if (dx < 0) {
        look_enemy_left(nenemy, true); // Face left
    } else if (dx > 0) {
        look_enemy_left(nenemy, false); // Face right
    }

    move_entity(&obj_enemy[nenemy].obj_character, spr_enemy[nenemy], newx, newy);
    update_enemy_shadow(nenemy);

    anim_enemy(nenemy, ANIM_IDLE);
}

void move_enemy_instant(u16 nenemy, fastfix32 x, fastfix32 y)    // Set enemy position immediately without animation
{
    s16 xi = FASTFIX32_TO_INT(x);
    s16 yi = FASTFIX32_TO_INT(y) - obj_enemy[nenemy].obj_character.y_size; // Adjust y position relative to the bottom line

    SPR_setPosition(spr_enemy[nenemy], xi, yi);
    obj_enemy[nenemy].obj_character.x = x;
    obj_enemy[nenemy].obj_character.y = FASTFIX32_FROM_INT(yi);
    update_enemy_shadow(nenemy);
    next_frame(false);
}


// Update enemy animations based on their current state
void update_enemy_animations(void)
{
    for (u16 e = 0; e < MAX_ENEMIES; ++e)
    {
        if (!obj_enemy[e].obj_character.active)          // empty slot
            continue;

        Entity *en = &obj_enemy[e].obj_character;        // shorthand

        // B4: dying enemy (0 HP) — let the death animation play for a fixed time, then release.
        // Replaces the blocking while() that lived in hit_enemy. Deterministic timer instead of
        // SPR_isAnimationDone: queried on the same frame the animation changes (before SPR_update
        // processes it) that function reads stale state and could release the sprite too early.
        if (obj_enemy[e].hitpoints == 0) {
            if (obj_enemy[e].modeTimer > 0)
                --obj_enemy[e].modeTimer;   // death animation still running
            else
                release_enemy(e);           // fixed duration elapsed → free the enemy
            continue;
        }

        switch (en->state)
        {
            case STATE_HIT:
                dprintf(2, "Enemy %d: HURT state", e);

                // Start — or keep — the HURT animation
                if (en->animation != ANIM_HURT) {
                    anim_enemy(e, ANIM_HURT);
                    break;
                }

                if (obj_enemy[e].modeTimer > 0) {
                    --obj_enemy[e].modeTimer; // Decrease the hurt timer
                }
                else {
                    dprintf(2, "Enemy %d: HURT animation finished", e);
                    en->state     = STATE_IDLE;
                    anim_enemy(e, ANIM_IDLE);
                }
                break;

            case STATE_WALKING:
                // Locomoción dirigida desde fuera (combate de contacto, contact.c):
                // el módulo que mueve al enemigo es dueño de su animación.
                break;

            case STATE_PATTERN_EFFECT:
                if (en->animation != ANIM_MAGIC)
                    anim_enemy(e, ANIM_MAGIC);
                break;

            case STATE_PATTERN_EFFECT_FINISH:
                // Ensure we exit the magic pose cleanly
                anim_enemy(e, ANIM_IDLE);
                en->state = STATE_IDLE;
                break;

            /* ---------------------------------------------------------- */
            case STATE_IDLE:
            default:
                // If the current animation has ended, enforce the idle pose
                if (en->animation != ANIM_IDLE &&
                    SPR_isAnimationDone(spr_enemy[e]))
                {
                    anim_enemy(e, ANIM_IDLE);
                }
                break;
        }
    }

    update_life_counter();    // Update life counter sprite if needed
}
