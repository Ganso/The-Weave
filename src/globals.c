#include <genesis.h>
#include "globals.h"

#include "globals.h"

// Wait for N seconds
void wait_seconds(int sec)
{
    waitMs(sec*1000);
}

// Initialize a character
void initialize_character(u8 nchar)
{
    spr_chr[nchar] = SPR_addSprite ( obj_character[nchar].sd, obj_character[nchar].x, obj_character[nchar].y, TILE_ATTR(obj_character[nchar].palette, obj_character[nchar].priority, false, obj_character[nchar].flipH));
    SPR_setVisibility (spr_chr[nchar], HIDDEN);
}

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

// Initialize a face
void initialize_face(u8 nface)
{
    spr_face[nface] = SPR_addSprite ( obj_face[nface].sd, obj_face[nface].x, obj_face[nface].y, TILE_ATTR(obj_face[nface].palette, obj_face[nface].priority, false, obj_face[nface].flipH));
    SPR_setVisibility (spr_face[nface], HIDDEN);
    SPR_setDepth (spr_face[nface], SPR_MIN_DEPTH); // Faces are above any other sprite
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

// Displays a face, in the left (or not), and a text string, during a maximum of maxtime milisecons
void talk(u8 nface, bool isinleft, char *text_line1, char *text_line2, char *text_line3, u16 maxtime)
{
    u16 faceposx;
    u16 textposx_line1=0, textposx_line2=0, textposx_line3=0;
    u16 joy_state;

    if (maxtime==0) maxtime=MAX_TALK_TIME; // If maximum time is 0, set it to the deafult value

    textposx_line1=8 + ((33 - strlen(text_line1)) >> 1);
    if (text_line2!=NULL) textposx_line2=8 + ((33 - strlen(text_line2)) >> 1);
    if (text_line3!=NULL) textposx_line3=8 + ((33 - strlen(text_line3)) >> 1);

    if (isinleft==FALSE) {
        textposx_line1-=8;
        textposx_line2-=8;
        textposx_line3-=8;
    }

    if (isinleft) {
        faceposx=0;
        SPR_setHFlip (spr_face[nface], false);
        SPR_setVisibility (spr_face_left, VISIBLE);
    }
    else{
        faceposx=256;
        SPR_setHFlip (spr_face[nface], true);
        SPR_setVisibility (spr_face_right, VISIBLE);
    }

    // Show everything
    VDP_clearTextLine(23);
    VDP_clearTextLine(24);
    VDP_clearTextLine(25);
    SPR_setPosition (spr_face[nface], faceposx, 160);
    SPR_setVisibility (spr_face[nface], VISIBLE);
    VDP_drawText(text_line1,textposx_line1,23);
    if (textposx_line2!=NULL) VDP_drawText(text_line2,textposx_line2,24);
    if (textposx_line3!=NULL) VDP_drawText(text_line3,textposx_line3,25);
    SPR_update();
    SYS_doVBlankProcess();

    // Wait for time or button A
    JOY_waitPressTime (JOY_ALL, BUTTON_A, maxtime);

    // Is button A is still pressed, wait until release
    joy_state=JOY_readJoypad (JOY_ALL);
    while (joy_state & BUTTON_A)
    {
        joy_state=JOY_readJoypad (JOY_ALL);
        SYS_doVBlankProcess();
    }

    // Hide everything
    SPR_setVisibility (spr_face_left, HIDDEN);
    SPR_setVisibility (spr_face_right, HIDDEN);
    SPR_setVisibility (spr_face[nface], HIDDEN);
    VDP_clearTextLine(23);
    VDP_clearTextLine(24);
    VDP_clearTextLine(25);
    SPR_update();
    SYS_doVBlankProcess();
}

// Move a character to a new position
void move_character(u8 nchar,s16 newx,s16 newy,u8 frameskip)
{
    u8 nframe=0;

    show_character(nchar, true);
    while (obj_character[nchar].x!=newx || obj_character[nchar].y!=newy)
    {
        if (obj_character[nchar].x<newx) {
                obj_character[nchar].x++;
                look_left(nchar,false);
        }
        if (obj_character[nchar].x>newx) {
                obj_character[nchar].x--;
                look_left(nchar,true);
        }
        if (obj_character[nchar].y<newy) obj_character[nchar].y++;
        if (obj_character[nchar].y>newy) obj_character[nchar].y--;
        SPR_setPosition(spr_chr[nchar],obj_character[nchar].x,obj_character[nchar].y);
        if (nframe%frameskip==0) {
            SPR_update();
            SYS_doVBlankProcess();
        }
        nframe++;
    }
}
