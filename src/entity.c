#include "globals.h"

bool movement_active;    // Whether entity movement is currently allowed

void move_entity(Entity *entity, Sprite *sprite, fastfix32 newx, fastfix32 newy)    // Move entity to new position with smooth animation and shadow updates
{
    u16 nchar=CHR_NONE;
    u16 nenemy = ENEMY_NONE;

    s16 target_x = FASTFIX32_TO_INT(newx);
    s16 target_y = FASTFIX32_TO_INT(newy) - entity->y_size; // Now all calculations are relative to the bottom line, not the upper one

    s16 x = FASTFIX32_TO_INT(entity->x);
    s16 y = FASTFIX32_TO_INT(entity->y);
    s16 dx = target_x - x;
    s16 dy = target_y - y;
    s16 sx = dx > 0 ? 1 : -1;
    s16 sy = dy > 0 ? 1 : -1;
    s16 err = (abs(dx) > abs(dy) ? abs(dx) : -abs(dy)) / 2;
    s16 e2;
    bool old_movement_active=movement_active;


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

    movement_active=false; // Player can't move while an entity is moving
    for(;;)
    {
        SPR_setPosition(sprite, x, y);
        if (nchar != CHR_NONE) update_character_shadow(nchar);
        if (nenemy != ENEMY_NONE) update_enemy_shadow(nenemy);
        entity->x = FASTFIX32_FROM_INT(x);
        entity->y = FASTFIX32_FROM_INT(y);
        next_frame(false);

        if (x == target_x && y == target_y) break;

        e2 = err;
        if (e2 > -abs(dx)) { err -= abs(dy); x += sx; }
        if (e2 < abs(dy)) { err += abs(dx); y += sy; }
    }
    movement_active=old_movement_active;
}
