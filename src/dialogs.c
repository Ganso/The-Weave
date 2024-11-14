#include <genesis.h>
#include "globals.h"

// Displays a face, in the left (or not), and a text string, during a maximum of maxtime milisecons
void talk(u8 nface, bool isinleft, char *text, u16 max_seconds)
{
    u16 faceposx,buttonposx;
    u16 textposx_line1=0, textposx_line2=0, textposx_line3=0;
    u16 joy_state;
    u16 num_ticks, max_ticks;

    char text_line1[40]={0},text_line2[40]={0},text_line3[40]={0};

    // Initialize the needed face
    if (nface!=FACE_none) init_face(nface);

    // Divide the text in up to three lines
    split_text(text, text_line1, text_line2, text_line3);

    // Center the text in each line
    textposx_line1=8 + ((33 - strlen(text_line1)) >> 1);
    textposx_line2=8 + ((33 - strlen(text_line2)) >> 1);
    textposx_line3=8 + ((33 - strlen(text_line3)) >> 1);

    // Check if face is in left or right (or none)
    if (nface!=FACE_none) {
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
        SPR_setPosition (spr_face[nface], faceposx, 160); // Show face
        SPR_setVisibility (spr_face[nface], VISIBLE);
    }
    else { // No face
        buttonposx=296;
        textposx_line1=((40 - strlen(text_line1)) >> 1); // Center text
        textposx_line2=((40 - strlen(text_line2)) >> 1); // Center text
        textposx_line3=((40 - strlen(text_line3)) >> 1); // Center text
    }

    // Show text and button
    if (strlen(text_line2)<1) { // One line -> Center vertically
        strcpy(text_line2,text_line1);
        textposx_line2=textposx_line1;
        strcpy(text_line1,"");
    }
    print_line(text_line1, textposx_line1, 23);
    print_line(text_line2, textposx_line2, 24);
    print_line(text_line3, textposx_line3, 25);
    SPR_setVisibility (spr_int_button_A, VISIBLE);
    SPR_setPosition (spr_int_button_A, buttonposx, 208);
    next_frame(false);

    // Wait for time or button A
    num_ticks=0;
    max_ticks=max_seconds*SCREEN_FPS;
    joy_state=JOY_readJoypad (JOY_ALL);
    while (num_ticks<max_ticks && ((joy_state & BUTTON_A)==0))
    {
        next_frame(false);
        joy_state=JOY_readJoypad (JOY_ALL);
        num_ticks++;
    }

    // Is button A is still pressed, wait until release
    joy_state=JOY_readJoypad (JOY_ALL);
    while (joy_state & BUTTON_A)
    {
        joy_state=JOY_readJoypad (JOY_ALL);
        next_frame(false);
    }

    // Hide everything
    SPR_setVisibility (spr_face_left, HIDDEN);
    SPR_setVisibility (spr_face_right, HIDDEN);
    if (nface!=FACE_none) SPR_setVisibility (spr_face[nface], HIDDEN);
    SPR_setVisibility (spr_int_button_A, HIDDEN);
    VDP_clearTextLineBG(WINDOW,23);
    VDP_clearTextLineBG(WINDOW,24);
    VDP_clearTextLineBG(WINDOW,25);

    // Release the face from memory
    if (nface!=FACE_none) release_face(nface);

    next_frame(false);
}

// Talk a dialog line
void talk_dialog(const DialogItem *dialog)
{
    talk(dialog->face, dialog->side, (char *)dialog->text[game_language], dialog->max_seconds); 
    kprintf("Idioma: %d",game_language);
}

// Split a text in up to three lines
void split_text(char *text, char *line1, char *line2, char *line3)
{
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

void print_line(char *text, u16 x, u16 y)
{
    int i = 0;
    u16 joy_state;
    char temp[2] = {0, 0};  // Temporary one character storage
    char *encoded_text = NULL;

    // Code Spanish text
    if (game_language == LANG_SPANISH) {
        encoded_text = encode_spanish_text(text);
        if (encoded_text != NULL) {
            text = encoded_text;
        }
    }

    // Print the text, character by character
    while (text[i] != '\0') {
        temp[0] = text[i];
        VDP_drawTextBG(WINDOW, temp, x + i, y);
        joy_state = JOY_readJoypad(JOY_ALL);
        if ((joy_state & BUTTON_A) == 0) next_frame(false); // If button A is being pressed, skip frame update
        i++;
    }

    joy_state = JOY_readJoypad(JOY_ALL);
    while ((joy_state & BUTTON_A) != 0)
    {
        joy_state = JOY_readJoypad(JOY_ALL);
        next_frame(false); // If button A is being pressed, wait until release
    }

    // Free the encoded text if it was allocated
    if (encoded_text != NULL) {
        free(encoded_text);
    }
}