#include "globals.h"

bool movement_active;    // Whether entity movement is currently allowed

void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy)    // Move entity to new position with smooth animation and shadow updates
{
    u16 nchar=CHR_NONE;
    u16 nenemy = ENEMY_NONE;

    newy-=entity->y_size; // Now all calculations are relative to the bottom line, not the upper one

    fix16 fx = entity->fx;
    fix16 fy = entity->fy;
    fix16 target_fx = INT_TO_FIX16(newx);
    fix16 target_fy = INT_TO_FIX16(newy);
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
    while (fx != target_fx || fy != target_fy)
    {
        if (fx < target_fx) {
            fx += entity->velocity;
            if (fx > target_fx) fx = target_fx;
        }
        else if (fx > target_fx) {
            fx -= entity->velocity;
            if (fx < target_fx) fx = target_fx;
        }

        if (fy < target_fy) {
            fy += entity->velocity;
            if (fy > target_fy) fy = target_fy;
        }
        else if (fy > target_fy) {
            fy -= entity->velocity;
            if (fy < target_fy) fy = target_fy;
        }

        entity->x = FIX16_TO_INT(fx);
        entity->y = FIX16_TO_INT(fy);
        SPR_setPosition(sprite, entity->x, entity->y);
        if (nchar != CHR_NONE) update_character_shadow(nchar);
        if (nenemy != ENEMY_NONE) update_enemy_shadow(nenemy);
        next_frame(false);
    }
    entity->fx = fx;
    entity->fy = fy;
    movement_active=old_movement_active;
}
