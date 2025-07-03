#include "globals.h"

bool movement_active;    // Whether entity movement is currently allowed

void move_entity(Entity *entity, Sprite *sprite, fastfix32 newx, fastfix32 newy)    // Move entity to new position with smooth animation and shadow updates
{
    u16 nchar = CHR_NONE;
    u16 nenemy = ENEMY_NONE;

    fastfix32 target_x = newx;
    fastfix32 target_y = newy - FASTFIX32_FROM_INT(entity->y_size); // Coordinates relative to sprite top

    fastfix32 x = entity->x;
    fastfix32 y = entity->y;
    fastfix32 dx = target_x - x;
    fastfix32 dy = target_y - y;
    fastfix32 abs_dx = dx >= 0 ? dx : -dx;
    fastfix32 abs_dy = dy >= 0 ? dy : -dy;
    u32 steps = (abs_dx > abs_dy ? abs_dx : abs_dy) >> 16;
    if (steps == 0) steps = 1;
    fastfix32 step_x = dx / (fastfix32)steps;
    fastfix32 step_y = dy / (fastfix32)steps;
    bool old_movement_active = movement_active;

    dprintf(3,"Moving entity %p to (%d, %d)\n", entity,
            FASTFIX32_TO_INT(target_x), FASTFIX32_TO_INT(target_y));

    // Check if this entity is a character so we can update its shadow
    for (u16 i = 0; i < MAX_CHR; i++) {
        if (&obj_character[i] == entity) {
            nchar = i;
            break;
        }
    }

    // Check if this entity is an enemy so we can update its shadow
    for (u16 i = 0; i < MAX_ENEMIES; i++) {
        if (&obj_enemy[i].obj_character == entity) {
            nenemy = i;
            break;
        }
    }

    movement_active = false; // Player can't move while an entity is moving
    for (u32 i = 0; i < steps; i++)
    {
        x += step_x;
        y += step_y;
        SPR_setPosition(sprite, FASTFIX32_TO_INT(x), FASTFIX32_TO_INT(y));
        if (nchar != CHR_NONE) update_character_shadow(nchar);
        if (nenemy != ENEMY_NONE) update_enemy_shadow(nenemy);
        entity->x = x;
        entity->y = y;
        next_frame(false);
    }

    // Ensure final position matches target exactly
    SPR_setPosition(sprite, FASTFIX32_TO_INT(target_x), FASTFIX32_TO_INT(target_y));
    if (nchar != CHR_NONE) update_character_shadow(nchar);
    if (nenemy != ENEMY_NONE) update_enemy_shadow(nenemy);
    entity->x = target_x;
    entity->y = target_y;
    movement_active = old_movement_active;
}
