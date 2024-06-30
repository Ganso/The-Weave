#include <genesis.h>
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

// Split a text in up to three lines
void split_text(char *text, char *line1, char *line2, char *line3) {
    u16 len = strlen(text);
    u16 i, lineStart = 0;
    u16 line = 0;

    for (i = 0; i <= len; ++i) {
        if (text[i] == '|' || text[i] == '\0') {
            if (line == 0) {
                strncpy(line1, text + lineStart, i - lineStart);
                line1[i - lineStart] = '\0';
            } else if (line == 1) {
                strncpy(line2, text + lineStart, i - lineStart);
                line2[i - lineStart] = '\0';
            } else if (line == 2) {
                strncpy(line3, text + lineStart, i - lineStart);
                line3[i - lineStart] = '\0';
            }
            lineStart = i + 1;
            line++;
        }
    }
}

// Displays a face, in the left (or not), and a text string, during a maximum of maxtime milisecons
void talk(u8 nface, bool isinleft, char *text, u16 max_ticks)
{
    u16 faceposx,buttonposx;
    u16 textposx_line1=0, textposx_line2=0, textposx_line3=0;
    u16 joy_state;
    u16 num_ticks;

    char text_line1[40]={0},text_line2[40]={0},text_line3[40]={0};

    // Divide the text in up to three lines
    split_text(text, text_line1, text_line2, text_line3);

    // Define maximum ticks to wait for button
    if (max_ticks==0) max_ticks=MAX_TALK_TIME; // If maximum time is 0, set it to the default value

    // Center the text in each line
    textposx_line1=8 + ((33 - strlen(text_line1)) >> 1);
    textposx_line2=8 + ((33 - strlen(text_line2)) >> 1);
    textposx_line3=8 + ((33 - strlen(text_line3)) >> 1);

    // Check if face is in left or right
    if (isinleft) {
        faceposx=0;
        buttonposx=296;
        SPR_setHFlip (spr_face[nface], false);
        SPR_setVisibility (spr_face_left, VISIBLE);
    }
    else{
        faceposx=256;
        buttonposx=232;
        textposx_line1-=8;
        textposx_line2-=8;
        textposx_line3-=8;
        SPR_setHFlip (spr_face[nface], true);
        SPR_setVisibility (spr_face_right, VISIBLE);
    }

    // Show everything
    SPR_setPosition (spr_face[nface], faceposx, 160);
    SPR_setVisibility (spr_face[nface], VISIBLE);
    if (strlen(text_line2)<1) { // One line -> Center vertically
        strcpy(text_line2,text_line1);
        textposx_line2=textposx_line1;
        strcpy(text_line1,"");
    }
    VDP_drawText(text_line1,textposx_line1,23);
    VDP_drawText(text_line2,textposx_line2,24);
    VDP_drawText(text_line3,textposx_line3,25);
    SPR_setVisibility (spr_button_A, VISIBLE);
    SPR_setPosition (spr_button_A, buttonposx, 208);
    SPR_update();
    SYS_doVBlankProcess();

    // Wait for time or button A
    num_ticks=0;
    joy_state=JOY_readJoypad (JOY_ALL);
    while (num_ticks<max_ticks && ((joy_state & BUTTON_A)==0))
    {
        SPR_update();
        SYS_doVBlankProcess();
        joy_state=JOY_readJoypad (JOY_ALL);
        num_ticks++;
    }

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
    SPR_setVisibility (spr_button_A, HIDDEN);
    VDP_clearTextLine(23);
    VDP_clearTextLine(24);
    VDP_clearTextLine(25);
    SPR_update();
    SYS_doVBlankProcess();
}

// Make a character talk a dialog line
void talk_dialog(u8 nface, bool isinleft, u16 act_seq, u16 nmessage, u16 max_ticks)
{
    talk(nface, isinleft, getDialog(act_seq, nmessage, game_language), max_ticks);
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
        SPR_update();
        SYS_doVBlankProcess();

        if (x == newx && y == newy) break;

        e2 = err;
        if (e2 > -abs(dx)) { err -= abs(dy); x += sx; }
        if (e2 < abs(dy)) { err += abs(dx); y += sy; }
    }
}
