#include <genesis.h>
#include "globals.h"

#include "characters.h"

// Update a character based on every parameter
void update_character(u8 nchar)
{
    SPR_setPosition(spr_chr[nchar],obj_character[nchar].x,obj_character[nchar].y);
    SPR_setPriority(spr_chr[nchar],obj_character[nchar].priority);
    SPR_setVisibility(spr_chr[nchar],obj_character[nchar].visible?VISIBLE:HIDDEN);
    SPR_setHFlip(spr_chr[nchar],obj_character[nchar].flipH);
    SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
    SPR_update();
}

// Show or hide a character
void show_character(u8 nchar, bool show)
{
    obj_character[nchar].visible=show;
    SPR_setVisibility(spr_chr[nchar],show?VISIBLE:HIDDEN);
    SPR_update();
}

// Change a character's animation
void anim_character(u8 nchar, u8 newanimation)
{
    obj_character[nchar].animation=newanimation;
    SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
}

// Make a character look to the left (or right)
void look_left(u8 nchar, bool direction_right)
{
    obj_character[nchar].flipH=direction_right;
    SPR_setHFlip (spr_chr[nchar], direction_right);
    SPR_update();
}

// Move a character to a new position
void move_character(u8 nchar, s16 newx, s16 newy)
{
    // Bresenham algorithm
    show_character(nchar, true);

    s16 x = obj_character[nchar].x;
    s16 y = obj_character[nchar].y;
    s16 dx = newx - x;
    s16 dy = newy - y;
    s16 sx = dx > 0 ? 1 : -1;
    s16 sy = dy > 0 ? 1 : -1;
    s16 err = (abs(dx) > abs(dy) ? abs(dx) : -abs(dy)) / 2;
    s16 e2;

    for(;;)
    {
        SPR_setPosition(spr_chr[nchar], x, y);
        next_frame();

        if (x == newx && y == newy) break;

        e2 = err;
        if (e2 > -abs(dx)) { err -= abs(dy); x += sx; }
        if (e2 < abs(dy)) { err += abs(dx); y += sy; }
    }

    obj_character[nchar].x = x;
    obj_character[nchar].y = y;
}

// Move a character to a new position (instantly)
void move_character_instant(u8 nchar,s16 x,s16 y)
{
    SPR_setPosition(spr_chr[nchar], x, y);
    obj_character[nchar].x = x;
    obj_character[nchar].y = y;
    update_bg();
}