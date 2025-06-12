#include "globals.h"

void talk(u8 nface, bool isinleft, char *text, u16 max_seconds)    // Display dialog with optional face portrait and timed text
{
    u16 faceposx,buttonposx;
    u16 textposx_line1=0, textposx_line2=0, textposx_line3=0;
    u16 joy_state;
    u16 num_ticks, max_ticks;

    char text_line1[40]={0},text_line2[40]={0},text_line3[40]={0};

    // If the active character is walking, stop it
    if (obj_character[active_character].state==STATE_WALKING) {
        obj_character[active_character].state=STATE_IDLE;
        update_character_animations();
    }

    // Initialize the needed face
    if (nface!=FACE_none) init_face(nface);

    // Divide the text in up to three lines
    split_text(text, text_line1, text_line2, text_line3);

    // Calculate centered positions using reusable function
    textposx_line1 = calculate_text_position(text_line1, isinleft, nface != FACE_none);
    textposx_line2 = calculate_text_position(text_line2, isinleft, nface != FACE_none);
    textposx_line3 = calculate_text_position(text_line3, isinleft, nface != FACE_none);

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
            SPR_setHFlip (spr_face[nface], true);
            SPR_setVisibility (spr_face_right, VISIBLE);
        }
        SPR_setPosition (spr_face[nface], faceposx, 160); // Show face
        SPR_setVisibility (spr_face[nface], VISIBLE);
    }
    else { // No face
        buttonposx=296;
        textposx_line1 = calculate_text_position(text_line1, false, false); // Center text
        textposx_line2 = calculate_text_position(text_line2, false, false); // Center text
        textposx_line3 = calculate_text_position(text_line3, false, false); // Center text
    }

    // Show text and button
    if (strlen(text_line2)<1) { // One line -> Center vertically
        strcpy(text_line2,text_line1);
        textposx_line2=textposx_line1;
        strcpy(text_line1,"");
    }
    print_line(text_line1, textposx_line1, 23, true);
    print_line(text_line2, textposx_line2, 24, true);
    print_line(text_line3, textposx_line3, 25, true);
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

void talk_dialog(const DialogItem *dialog)    // Display a predefined dialog item with face and text
{
    reset_character_animations();
    dprintf(2, "Leyendo texto: %s", (char *)dialog->text[game_language]);
    talk(dialog->face, dialog->side, (char *)dialog->text[game_language], dialog->max_seconds);
}

void talk_cluster(const DialogItem *start)    // Display several dialog lines
{
    dprintf(2, "Iniciando cluster");
    const DialogItem *it = start;
    while (it->text[0] != NULL) {
        talk_dialog(it);
        ++it;
    }
    dprintf(2, "NULL encontrado. Fin del cluster");
}

void split_text(char *text, char *line1, char *line2, char *line3)    // Break text into three lines using | as separator
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

void print_line(char *text, u16 x, u16 y, bool wait_for_frame)    // Display text line with character-by-character animation
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
        if (wait_for_frame) {
            joy_state = JOY_readJoypad(JOY_ALL);
            if ((joy_state & BUTTON_A) == 0) next_frame(false); // If button A is being pressed, skip frame update
        }
        i++;
    }

    if (wait_for_frame) {
        joy_state = JOY_readJoypad(JOY_ALL);
        while ((joy_state & BUTTON_A) != 0)
        {
            joy_state = JOY_readJoypad(JOY_ALL);
            next_frame(false); // If button A is being pressed, wait until release
        }
    }

    // Free the encoded text if it was allocated
    if (encoded_text != NULL) {
        free(encoded_text);
    }
}

u8 choice(u8 nface, bool isinleft, char **options, u8 num_options, u16 max_seconds)    // Display dialog with multiple choice options
{
    u16 faceposx, buttonposx;
    u16 textposx[MAX_CHOICES] = {0};
    u8 choice_lenght[MAX_CHOICES];
    char *encoded_text = NULL;
    u16 joy_state = JOY_readJoypad(JOY_ALL), prev_joy_state = 0;
    u8 current_option = 0;
    u16 num_ticks = 0;
    u16 max_ticks = max_seconds * SCREEN_FPS;
    
    // Load magic animation sprite
    Sprite* spr_magic_anim = SPR_addSprite(&int_magin_anim_sprite, 0, 0, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_setVisibility(spr_magic_anim, HIDDEN);
    
    // If the active character is walking, stop it
    if (obj_character[active_character].state==STATE_WALKING) {
        obj_character[active_character].state=STATE_IDLE;
        update_character_animations();
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

    // Show initial options and calculate positions
    u8 start_y = (num_options == 2) ? 24 : 23; // Start one line lower for 2 options
    
    for(u8 i = 0; i < num_options && i < MAX_CHOICES; i++) {
        
        // Calculate lenght (it's important do encode Spanish characters to the standard font before)
        if (game_language==LANG_ENGLISH) choice_lenght[i]=strlen(options[i]);
        else {
            encoded_text = encode_spanish_text(options[i]);
            choice_lenght[i] = strlen(encoded_text);
        }

        // Center text (calculate position based on plain text)
        if(nface != FACE_none) {
            textposx[i] = 8 + ((33 - choice_lenght[i]) >> 1);
            if(!isinleft) textposx[i] -= 8;
        }
        else {
            textposx[i] = calculate_text_position(options[i], false, false);
        }

        // Show plain text first
        print_line(options[i], textposx[i], start_y + i, false);
    }

    // Add selection markers to initial option
    VDP_drawTextBG(WINDOW, "}", textposx[current_option] - 2, start_y + current_option);
    VDP_drawTextBG(WINDOW, "{", textposx[current_option] + choice_lenght[current_option] + 1, start_y + current_option);

    // Show magic animation on initial option
    u16 magic_x = (textposx[current_option] * 8) + ((choice_lenght[current_option] * 8) / 2) - 80;
    u16 magic_y = (start_y + current_option) * 8;
    SPR_setPosition(spr_magic_anim, magic_x, magic_y);
    SPR_setVisibility(spr_magic_anim, VISIBLE);

    // Show button A
    SPR_setVisibility(spr_int_button_A, VISIBLE);
    SPR_setPosition(spr_int_button_A, buttonposx, 208);
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
            VDP_drawTextBG(WINDOW, " ", textposx[prev_option] + choice_lenght[prev_option] + 1, start_y + prev_option);

            // Add selection markers to new option
            VDP_drawTextBG(WINDOW, "}", textposx[current_option] - 2, start_y + current_option);
            VDP_drawTextBG(WINDOW, "{", textposx[current_option] + choice_lenght[current_option] + 1, start_y + current_option);
            
            // Update magic animation position
            // Center horizontally: text position + (text length * 8) / 2 - (160 / 2)
            u16 magic_x = (textposx[current_option] * 8) + ((choice_lenght[current_option] * 8) / 2) - 80;
            // Vertically align with text: start_y + current_option lines down (8 pixels per line)
            u16 magic_y = (start_y + current_option) * 8;
            SPR_setPosition(spr_magic_anim, magic_x, magic_y);
            SPR_setVisibility(spr_magic_anim, VISIBLE);
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
    SPR_releaseSprite(spr_magic_anim);
    VDP_clearTextLineBG(WINDOW, 23);
    VDP_clearTextLineBG(WINDOW, 24);
    VDP_clearTextLineBG(WINDOW, 25);

    // Release the face from memory
    if(nface != FACE_none) release_face(nface);

    next_frame(false);

    dprintf(2,"Choice: %d\n", current_option);
    return current_option;
}

u8 choice_dialog(const ChoiceItem *item)    // Display a predefined choice dialog item
{
    reset_character_animations();
    u8 result = choice(item->face, item->side, (char **)item->options[game_language], item->num_options, item->max_seconds);
    return result;
}
