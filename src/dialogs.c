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

    // If the active character is walking, stop it
    if (obj_character[active_character].state==STATE_WALKING) {
        obj_character[active_character].state=STATE_IDLE;
        update_character_animation();
    }

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

// Shows a choice dialog directly
u8 choice(u8 nface, bool isinleft, char **options, u8 num_options, u16 max_seconds)
{
    u16 faceposx, buttonposx;
    u16 textposx[MAX_CHOICES] = {0};
    u16 joy_state = JOY_readJoypad(JOY_ALL), prev_joy_state = 0;
    u8 current_option = 0;
    u16 num_ticks = 0;
    u16 max_ticks = max_seconds * SCREEN_FPS;
    
    // If the active character is walking, stop it
    if (obj_character[active_character].state==STATE_WALKING) {
        obj_character[active_character].state=STATE_IDLE;
        update_character_animation();
    }

    // Initialize the needed face
    if (nface!=FACE_none) init_face(nface);

    // Check if face is in left or right (or none)
    if (nface!=FACE_none) {
        if (isinleft) {
            faceposx=0;
            buttonposx=296;
            SPR_setHFlip(spr_face[nface], false);
            SPR_setVisibility(spr_face_left, VISIBLE);
        }
        else {
            faceposx=256;
            buttonposx=232;
            SPR_setHFlip(spr_face[nface], true);
            SPR_setVisibility(spr_face_right, VISIBLE);
        }
        SPR_setPosition(spr_face[nface], faceposx, 160);
        SPR_setVisibility(spr_face[nface], VISIBLE);
    }
    else {
        buttonposx=296;
    }

    // Show button A
    SPR_setVisibility(spr_int_button_A, VISIBLE);
    SPR_setPosition(spr_int_button_A, buttonposx, 208);

    // Show initial options and calculate positions
    u8 start_y = (num_options == 2) ? 24 : 23; // Start one line lower for 2 options
    
    for(u8 i = 0; i < num_options && i < MAX_CHOICES; i++) {
        // Center text (calculate position based on plain text)
        if(nface != FACE_none) {
            textposx[i] = 8 + ((33 - strlen(options[i])) >> 1);
            if(!isinleft) textposx[i] -= 8;
        }
        else {
            textposx[i] = ((40 - strlen(options[i])) >> 1);
        }

        // Show plain text first
        print_line(options[i], textposx[i], start_y + i);
    }

    // Add selection markers to initial option
    VDP_drawTextBG(WINDOW, "}", textposx[current_option] - 2, start_y + current_option);
    VDP_drawTextBG(WINDOW, "{", textposx[current_option] + strlen(options[current_option]) + 1, start_y + current_option);

    next_frame(false);

    // Main loop
    while(num_ticks < max_ticks) {
        // Handle input
        prev_joy_state = joy_state;
        joy_state = JOY_readJoypad(JOY_ALL);

        // Only process new button presses
        u16 new_press = joy_state & ~prev_joy_state;

        bool need_redraw = false;

        if(new_press & BUTTON_UP) {
            if(current_option > 0) {
                current_option--;
                need_redraw = true;
            }
        }
        else if(new_press & BUTTON_DOWN) {
            if(current_option < num_options - 1) {
                current_option++;
                need_redraw = true;
            }
        }
        else if(new_press & BUTTON_A) {
            break;
        }

        // Only update selection markers if needed
        if(need_redraw) {
            u8 prev_option = current_option + (new_press & BUTTON_UP ? 1 : -1);
            
            // Remove selection markers from previous option
            VDP_drawTextBG(WINDOW, " ", textposx[prev_option] - 2, start_y + prev_option);
            VDP_drawTextBG(WINDOW, " ", textposx[prev_option] + strlen(options[prev_option]) + 1, start_y + prev_option);

            // Add selection markers to new option
            VDP_drawTextBG(WINDOW, "}", textposx[current_option] - 2, start_y + current_option);
            VDP_drawTextBG(WINDOW, "{", textposx[current_option] + strlen(options[current_option]) + 1, start_y + current_option);
        }

        next_frame(false);
        num_ticks++;
    }

    // Wait for button A release
    joy_state = JOY_readJoypad(JOY_ALL);
    while(joy_state & BUTTON_A) {
        joy_state = JOY_readJoypad(JOY_ALL);
        next_frame(false);
    }

    // Hide everything
    SPR_setVisibility(spr_face_left, HIDDEN);
    SPR_setVisibility(spr_face_right, HIDDEN);
    if(nface != FACE_none) SPR_setVisibility(spr_face[nface], HIDDEN);
    SPR_setVisibility(spr_int_button_A, HIDDEN);
    VDP_clearTextLineBG(WINDOW, 23);
    VDP_clearTextLineBG(WINDOW, 24);
    VDP_clearTextLineBG(WINDOW, 25);

    // Release the face from memory
    if(nface != FACE_none) release_face(nface);

    next_frame(false);

    kprintf("Choice: %d\n", current_option);
    return current_option;
}

// Shows a choice dialog from a ChoiceItem
u8 choice_dialog(const ChoiceItem *item)
{
    return choice(item->face, item->side, (char **)item->options[game_language], item->num_options, item->max_seconds);
}
