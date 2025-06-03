#include "globals.h"

Enemy obj_enemy[MAX_ENEMIES];                    // Array of enemy instances with their properties
Sprite *spr_enemy[MAX_ENEMIES];                  // Array of enemy sprites
Sprite *spr_enemy_face[MAX_ENEMIES];             // Array of enemy face sprites for dialogs
Sprite *spr_enemy_shadow[MAX_ENEMIES];           // Array of enemy shadow sprites
Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES];  // Array of enemy class definitions

void update_enemy_shadow(u16 nenemy)    // Update shadow sprite position based on enemy position
{
    if (obj_enemy[nenemy].obj_character.drops_shadow && spr_enemy_shadow[nenemy] != NULL) {
        // Position shadow at the bottom of enemy's collision box
        s16 shadow_x = obj_enemy[nenemy].obj_character.x;
        s16 shadow_y = obj_enemy[nenemy].obj_character.y + obj_enemy[nenemy].obj_character.collision_y_offset - 4;
        
        // Flip shadow if enemy is looking to the left
        SPR_setHFlip(spr_enemy_shadow[nenemy], obj_enemy[nenemy].obj_character.flipH);

        // Set shadow position
        SPR_setPosition(spr_enemy_shadow[nenemy], shadow_x, shadow_y);
    }
}

void init_enemy_classes(void)    // Setup enemy class definitions with HP, patterns, and behavior
{
    obj_enemy_class[ENEMY_CLS_WEAVERGHOST]=(Enemy_Class) { 2, false, 0, {true, false}}; // 2 HP, can use electric pattern, don't follow
    obj_enemy_class[ENEMY_CLS_3HEADMONKEY]=(Enemy_Class) {3, true, 3, {false, true}}; // 3 HP, can use bite pattern, follows at speed 3
}


void init_enemy(u16 numenemy, u16 class)    // Create new enemy instance of given class with sprites and collision
{
    u16 i;
    u8 npal = PAL3;
    u8 x_size, y_size;
    u8 collision_x_offset=0,collision_y_offset=0,collision_width=0,collision_height=0;
    bool drops_shadow=true;
    const SpriteDefinition *nsprite = NULL;
    const SpriteDefinition *nsprite_face = NULL;
    const SpriteDefinition *nsprite_shadow = NULL;

    obj_enemy[numenemy].class_id=class;
    obj_enemy[numenemy].class=obj_enemy_class[class];
    obj_enemy[numenemy].hitpoints=obj_enemy_class[class].max_hitpoints;

    // Set specific attributes based on enemy class
    switch (class)
    {
    case ENEMY_CLS_WEAVERGHOST:
        nsprite = &weaver_ghost_sprite;
        nsprite_face = &weaver_ghost_sprite_face;
        nsprite_shadow = NULL;
        drops_shadow=false;
        break;
    case ENEMY_CLS_3HEADMONKEY:
        nsprite = &three_head_monkey_sprite;
        nsprite_face = &three_head_monkey_sprite_face;
        nsprite_shadow = &three_head_monkey_sprite_shadow;
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
        true, nsprite, nsprite_shadow, 0, 0, x_size, y_size, npal, false, false, 
        ANIM_IDLE, false, collision_x_offset, collision_y_offset, 
        collision_width, collision_height, STATE_IDLE, 
        obj_enemy_class[class].follows_character, obj_enemy_class[class].follow_speed,
        drops_shadow
    };

    // Add enemy sprite if not already present
    if (spr_enemy[numenemy]==NULL) spr_enemy[numenemy] = SPR_addSpriteSafe(nsprite, obj_enemy[numenemy].obj_character.x, obj_enemy[numenemy].obj_character.y, 
                                       TILE_ATTR(npal, obj_enemy[numenemy].obj_character.priority, false, obj_enemy[numenemy].obj_character.flipH));
    
    // Add enemy face sprite if not already present
    if (spr_enemy_face[numenemy]==NULL) spr_enemy_face[numenemy] = SPR_addSpriteSafe(nsprite_face, 198, 178, TILE_ATTR(npal, false, false, false));

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
    SPR_setVisibility(spr_enemy_face[numenemy], HIDDEN);
    
    initEnemyPatterns(numenemy); // Initialize enemy patterns

    for (i = 0; i < MAX_PATTERN_ENEMY; i++)
        obj_enemy[numenemy].last_pattern_time[i] = 0;
}

void release_enemy(u16 nenemy)    // Free enemy resources and reset related combat state
{

    /* If this enemy was the active one in combat, reset the state */
    if (combatContext.activeEnemy == nenemy) {
        combatContext.activeEnemy = ENEMY_NONE;
        combat_state       = COMBAT_STATE_IDLE;
        combatContext.effectTimer = 0;
    }

    /* Hide any note indicators (through interface) */
    for (u8 note = 0; note < 6; note++) {
        // ******************** show_enemy_note(note + 1, false, false);
    }

    /* Deactivate entity */
    obj_enemy[nenemy].obj_character.active = false;

    /* Release sprites */
    if (spr_enemy[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy[nenemy]);
        spr_enemy[nenemy] = NULL;
    }
    if (spr_enemy_face[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy_face[nenemy]);
        spr_enemy_face[nenemy] = NULL;
    }
    if (spr_enemy_shadow[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy_shadow[nenemy]);
        spr_enemy_shadow[nenemy] = NULL;
    }
}

void update_enemy(u16 nenemy)    // Update enemy sprite properties from current state
{
    SPR_setPosition(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.x, obj_enemy[nenemy].obj_character.y);
    SPR_setPriority(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.priority);
    SPR_setVisibility(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.visible ? VISIBLE : HIDDEN);
    SPR_setHFlip(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.flipH);
    SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    update_enemy_shadow(nenemy);
    SPR_update();
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

void move_enemy(u16 nenemy, s16 newx, s16 newy)    // Move enemy with walking animation and direction update
{
    show_enemy(nenemy, true);
    anim_enemy(nenemy, ANIM_WALK);

    // Determine direction to face based on movement
    s16 dx = newx - obj_enemy[nenemy].obj_character.x;
    if (dx < 0) {
        look_enemy_left(nenemy, true); // Face left
    } else if (dx > 0) {
        look_enemy_left(nenemy, false); // Face right
    }

    move_entity(&obj_enemy[nenemy].obj_character, spr_enemy[nenemy], newx, newy);
    update_enemy_shadow(nenemy);

    anim_enemy(nenemy, ANIM_IDLE);
}

void move_enemy_instant(u16 nenemy, s16 x, s16 y)    // Set enemy position immediately without animation
{
    y-=obj_enemy[nenemy].obj_character.y_size; // Adjust y position relative to the bottom line

    SPR_setPosition(spr_enemy[nenemy], x, y);
    obj_enemy[nenemy].obj_character.x = x;
    obj_enemy[nenemy].obj_character.y = y;
    update_enemy_shadow(nenemy);
    next_frame(false);
}

void approach_enemies(void)    // Update enemy positions to follow player during combat
{
    u16 nenemy;
    s16 newx, newy;
    s16 dx, dy;
    u16 collision_result;
    bool has_moved;

    if (combat_state == COMBAT_STATE_IDLE && combatContext.activePattern != PATTERN_HIDE) { // Only move enemies during combat when the player is not hidden
        for (nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            has_moved=false;
            if (obj_enemy[nenemy].obj_character.follows_character == true) { // Check if this enemy type follows characters
                if (frame_counter%obj_enemy[nenemy].obj_character.follow_speed==0) { // Move at enemy's specific speed
                    // Calculate direction towards active character
                    dx = obj_character[active_character].x - obj_enemy[nenemy].obj_character.x;
                    dy = (obj_character[active_character].y + obj_character[active_character].y_size) - 
                        (obj_enemy[nenemy].obj_character.y + obj_enemy[nenemy].obj_character.y_size);

                    // Move by 1 pixel in the calculated direction
                    newx = obj_enemy[nenemy].obj_character.x + (dx != 0 ? (dx > 0 ? 1 : -1) : 0);
                    newy = obj_enemy[nenemy].obj_character.y + (dy != 0 ? (dy > 0 ? 1 : -1) : 0);

                    // Check for collision at new position
                    collision_result = detect_enemy_char_collision(nenemy, newx, newy);

                    // Move the enemy if there's no collision and it's not currently attacking
                    if (collision_result == CHR_NONE && combatContext.activeEnemy==ENEMY_NONE) {
                        obj_enemy[nenemy].obj_character.x = newx;
                        obj_enemy[nenemy].obj_character.y = newy;
                        obj_enemy[nenemy].obj_character.animation=ANIM_WALK;
                        obj_enemy[nenemy].obj_character.flipH=(dx<0);
                        update_enemy(nenemy);
                        has_moved=true;
                    }
                if (has_moved==false && combatContext.activeEnemy!=nenemy) anim_enemy(nenemy,ANIM_IDLE); // Set to idle if not moved and not attacking
                }
            }
        }
    }
}

// Update enemy animations based on their current state
void update_enemy_animations(void)
{
    for (u16 e = 0; e < MAX_ENEMIES; ++e)
    {
        if (!obj_enemy[e].obj_character.active)          // empty slot
            continue;

        Entity *en = &obj_enemy[e].obj_character;        // shorthand

        switch (en->state)
        {
            /* ---------------------------------------------------------- */
            case STATE_HIT:
                dprintf(2, "Enemy %d: HURT state", e);

                // Start — or keep — the HURT animation
                if (en->animation != ANIM_HURT) {
                    anim_enemy(e, ANIM_HURT);
                    break;
                }

                // When the animation finishes, go back to idle
                if (SPR_isAnimationDone(spr_enemy[e]))
                {
                    dprintf(2, "Enemy %d: HURT animation finished", e);
                    en->state     = STATE_IDLE;
                    anim_enemy(e, ANIM_IDLE);
                }
                break;

            /* ---------------------------------------------------------- */
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
