#include <genesis.h>
#include "globals.h"


// Move an entity
void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy)
{
    s16 x = entity->x;
    s16 y = entity->y;
    s16 dx = newx - x;
    s16 dy = newy - y;
    s16 sx = dx > 0 ? 1 : -1;
    s16 sy = dy > 0 ? 1 : -1;
    s16 err = (abs(dx) > abs(dy) ? abs(dx) : -abs(dy)) / 2;
    s16 e2;

    for(;;)
    {
        SPR_setPosition(sprite, x, y);
        next_frame();

        if (x == newx && y == newy) break;

        e2 = err;
        if (e2 > -abs(dx)) { err -= abs(dy); x += sx; }
        if (e2 < abs(dy)) { err += abs(dx); y += sy; }
    }

    entity->x = x;
    entity->y = y;
}