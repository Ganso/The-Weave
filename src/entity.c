#include "globals.h"

bool movement_active;    // Whether entity movement is currently allowed

void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy)    // Move entity to new position with smooth animation and shadow updates
{
    u16 nchar=CHR_NONE;
    u16 nenemy = ENEMY_NONE;

    newy-=entity->y_size; // Now all calculations are relative to the bottom line, not the upper one
    
    s16 x = entity->x;
    s16 y = entity->y;
    s16 dx = newx - x;
    s16 dy = newy - y;
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
        entity->x = x;
        entity->y = y;
        next_frame(false);

        if (x == newx && y == newy) break;

        e2 = err;
        if (e2 > -abs(dx)) { err -= abs(dy); x += sx; }
        if (e2 < abs(dy)) { err += abs(dx); y += sy; }
    }
    movement_active=old_movement_active;
}
