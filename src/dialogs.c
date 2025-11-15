#include "globals.h"

const u8 *voice_sample[MAX_VOICE][MAX_DIALOG_SOUNDS]; // Character voice samples
u32 voice_sample_size[MAX_VOICE][MAX_DIALOG_SOUNDS]; // Size of each character voice sample
u8 voice_numsamples[MAX_VOICE]; // Number of samples per voice

// HELPERS

static inline u8 letter_index_from_char(char c) {
    if (c >= 'A' && c <= 'Z') return (u8)(c - 'A');
    if (c >= 'a' && c <= 'z') return (u8)(c - 'a');
    return 255; // not a letter
}

// FUNCTIONS

void talk(u8 nface, bool isinleft, char *text, u16 max_seconds, bool sound_on)    // Display dialog with optional face portrait and timed text
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
    dprintf(3,"Split text in lines: \"%s\" / \"%s\" / \"%s\"\n",text_line1,text_line2,text_line3);

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
    dprintf(3,"Calling print_line for each line with params: (%d,23), (%d,24), (%d,25)\n",textposx_line1,textposx_line2,textposx_line3);
    print_line(text_line1, textposx_line1, 23, true, nface, sound_on);
    print_line(text_line2, textposx_line2, 24, true, nface, sound_on);
    print_line(text_line3, textposx_line3, 25, true, nface, sound_on);
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

void talk_dialog(const DialogItem *dialog, bool sound_on)    // Display a predefined dialog item with face and text
{
    reset_character_animations();
    dprintf(2, "Reading text: %s", (char *)dialog->text[game_language]);
    talk(dialog->face, dialog->side, (char *)dialog->text[game_language], dialog->max_seconds, sound_on);
}

void talk_cluster(const DialogItem *start, bool sound_on)    // Display several dialog lines
{
    dprintf(2, "Starting cluster");
    const DialogItem *it = start;
    while (it->text[0] != NULL) {
        talk_dialog(it, sound_on);
        ++it;
    }
    dprintf(2, "NULL found. End of cluster");
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


void print_line(char *text, u16 x, u16 y, bool wait_for_frame, u8 nface, bool sound_on) // Display text line with character-by-character animation
{
    int i = 0, pos = 0;
    u16 joy_state;
    char temp[2] = {0, 0}; // Temporary one character storage
    char *encoded_text = NULL;
    Sprite *spr_fadein = NULL;
    u8 voice_talking;

    // Fade-in sprite
    spr_fadein = SPR_addSprite(&int_fadein_sprite, 0, 0, TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_setVisibility(spr_fadein, HIDDEN);
    SPR_setAnimationLoop(spr_fadein, false);

    // Encode Spanish text if needed
    if (game_language == LANG_SPANISH) {
        encoded_text = encode_spanish_text(text);
        if (encoded_text != NULL) text = encoded_text;
    }

    VDP_setTextPalette(PAL2);

    // Print the text, character by character, handling palette escape codes
    while (text[i] != '\0') {

        // Color start @[  -> switch to PAL1
        if (text[i] == '@' && text[i + 1] == '[') {
            VDP_setTextPalette(PAL1);
            i += 2;
            continue;
        }
        // Color end @] -> back to PAL2
        if (text[i] == '@' && text[i + 1] == ']') {
            VDP_setTextPalette(PAL2);
            i += 2;
            continue;
        }

        temp[0] = text[i];

        // Position and show fade-in sprite over the character position
        SPR_setPosition(spr_fadein, (x + pos) * 8, y * 8);
        SPR_setVisibility(spr_fadein, VISIBLE);
        SPR_setAnimAndFrame(spr_fadein, 0, 0); // Reset animation

        // Draw the character behind the fade-in sprite
        VDP_drawTextBG(WINDOW, temp, x + pos, y);

        // Decide voice from face
        if (sound_on && text[i] != ' ') {
            switch (nface) {
                case FACE_clio:   voice_talking = VOICE_WOMAN; break;
                case FACE_linus:  voice_talking = VOICE_MAN;   break;
                case FACE_xander: voice_talking = VOICE_DEEP;  break;
                case FACE_swan:   voice_talking = VOICE_DEEP;  break;
                default:          voice_talking = VOICE_TYPEWRITER; break;
            }

            if (voice_talking == VOICE_TYPEWRITER) {
                // Random typewriter tick for any non-space character
                u8 idx = (u8)(random() % voice_numsamples[VOICE_TYPEWRITER]);
                XGM2_playPCM(voice_sample[VOICE_TYPEWRITER][idx],
                             voice_sample_size[VOICE_TYPEWRITER][idx],
                             SOUND_PCM_CH3);
            } else {
                // Map A..Z / a..z to 0..25 and play exact phoneme
                u8 idx = 255;
                char c = text[i];
                if (c >= 'A' && c <= 'Z') idx = (u8)(c - 'A');
                else if (c >= 'a' && c <= 'z') idx = (u8)(c - 'a');

                if (idx < 26) {
                    play_sample(voice_sample[voice_talking][idx], voice_sample_size[voice_talking][idx]);
                }
                // Non-letter: no sound
            }
        }

        // Optionally wait for fade-in animation (skippable with A)
        bool animation_done = false;
        if (wait_for_frame) {
            while (!animation_done) {
                joy_state = JOY_readJoypad(JOY_ALL);
                if ((joy_state & BUTTON_A) == 0) {
                    next_frame(false);
                    animation_done = SPR_isAnimationDone(spr_fadein);
                } else {
                    animation_done = true; // Skip on A
                }
            }
        }

        pos++;
        i++;
    }

    VDP_setTextPalette(PAL2); // Ensure palette reset

    // If waiting, also wait for A release to avoid accidental skips on next text
    if (wait_for_frame) {
        joy_state = JOY_readJoypad(JOY_ALL);
        while ((joy_state & BUTTON_A) != 0) {
            joy_state = JOY_readJoypad(JOY_ALL);
            next_frame(false);
        }
    }

    if (encoded_text != NULL) {
        free(encoded_text);
    }

    // Hide and release fade-in sprite
    SPR_setVisibility(spr_fadein, HIDDEN);
    SPR_releaseSprite(spr_fadein);
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
    
    dprintf(2, "Displaying choice with %d options\n", num_options);

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
        
        // Calculate length ignoring color codes (encode Spanish characters first)
        const char *len_text = options[i];
        if (game_language == LANG_SPANISH) {
            encoded_text = encode_spanish_text(options[i]);
            if (encoded_text != NULL) len_text = encoded_text;
        }
        choice_lenght[i] = visible_length(len_text);
        if (encoded_text != NULL) {
            free(encoded_text);
            encoded_text = NULL;
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
        print_line(options[i], textposx[i], start_y + i, false, nface, false);
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
    dprintf(2, "Displaying choice dialog with %d options\n", item->num_options);
    u8 result = choice(item->face, item->side, (char **)item->options[game_language], item->num_options, item->max_seconds);
    return result;
}

void init_voice_samples(void)
{
    // WOMAN A-Z
    voice_sample[VOICE_WOMAN][0]  = snd_dialog_woman_A;
    voice_sample[VOICE_WOMAN][1]  = snd_dialog_woman_B;
    voice_sample[VOICE_WOMAN][2]  = snd_dialog_woman_C;
    voice_sample[VOICE_WOMAN][3]  = snd_dialog_woman_D;
    voice_sample[VOICE_WOMAN][4]  = snd_dialog_woman_E;
    voice_sample[VOICE_WOMAN][5]  = snd_dialog_woman_F;
    voice_sample[VOICE_WOMAN][6]  = snd_dialog_woman_G;
    voice_sample[VOICE_WOMAN][7]  = snd_dialog_woman_H;
    voice_sample[VOICE_WOMAN][8]  = snd_dialog_woman_I;
    voice_sample[VOICE_WOMAN][9]  = snd_dialog_woman_J;
    voice_sample[VOICE_WOMAN][10] = snd_dialog_woman_K;
    voice_sample[VOICE_WOMAN][11] = snd_dialog_woman_L;
    voice_sample[VOICE_WOMAN][12] = snd_dialog_woman_M;
    voice_sample[VOICE_WOMAN][13] = snd_dialog_woman_N;
    voice_sample[VOICE_WOMAN][14] = snd_dialog_woman_O;
    voice_sample[VOICE_WOMAN][15] = snd_dialog_woman_P;
    voice_sample[VOICE_WOMAN][16] = snd_dialog_woman_Q;
    voice_sample[VOICE_WOMAN][17] = snd_dialog_woman_R;
    voice_sample[VOICE_WOMAN][18] = snd_dialog_woman_S;
    voice_sample[VOICE_WOMAN][19] = snd_dialog_woman_T;
    voice_sample[VOICE_WOMAN][20] = snd_dialog_woman_U;
    voice_sample[VOICE_WOMAN][21] = snd_dialog_woman_V;
    voice_sample[VOICE_WOMAN][22] = snd_dialog_woman_W;
    voice_sample[VOICE_WOMAN][23] = snd_dialog_woman_X;
    voice_sample[VOICE_WOMAN][24] = snd_dialog_woman_Y;
    voice_sample[VOICE_WOMAN][25] = snd_dialog_woman_Z;

    // WOMAN sizes
    voice_sample_size[VOICE_WOMAN][0]  = sizeof(snd_dialog_woman_A);
    voice_sample_size[VOICE_WOMAN][1]  = sizeof(snd_dialog_woman_B);
    voice_sample_size[VOICE_WOMAN][2]  = sizeof(snd_dialog_woman_C);
    voice_sample_size[VOICE_WOMAN][3]  = sizeof(snd_dialog_woman_D);
    voice_sample_size[VOICE_WOMAN][4]  = sizeof(snd_dialog_woman_E);
    voice_sample_size[VOICE_WOMAN][5]  = sizeof(snd_dialog_woman_F);
    voice_sample_size[VOICE_WOMAN][6]  = sizeof(snd_dialog_woman_G);
    voice_sample_size[VOICE_WOMAN][7]  = sizeof(snd_dialog_woman_H);
    voice_sample_size[VOICE_WOMAN][8]  = sizeof(snd_dialog_woman_I);
    voice_sample_size[VOICE_WOMAN][9]  = sizeof(snd_dialog_woman_J);
    voice_sample_size[VOICE_WOMAN][10] = sizeof(snd_dialog_woman_K);
    voice_sample_size[VOICE_WOMAN][11] = sizeof(snd_dialog_woman_L);
    voice_sample_size[VOICE_WOMAN][12] = sizeof(snd_dialog_woman_M);
    voice_sample_size[VOICE_WOMAN][13] = sizeof(snd_dialog_woman_N);
    voice_sample_size[VOICE_WOMAN][14] = sizeof(snd_dialog_woman_O);
    voice_sample_size[VOICE_WOMAN][15] = sizeof(snd_dialog_woman_P);
    voice_sample_size[VOICE_WOMAN][16] = sizeof(snd_dialog_woman_Q);
    voice_sample_size[VOICE_WOMAN][17] = sizeof(snd_dialog_woman_R);
    voice_sample_size[VOICE_WOMAN][18] = sizeof(snd_dialog_woman_S);
    voice_sample_size[VOICE_WOMAN][19] = sizeof(snd_dialog_woman_T);
    voice_sample_size[VOICE_WOMAN][20] = sizeof(snd_dialog_woman_U);
    voice_sample_size[VOICE_WOMAN][21] = sizeof(snd_dialog_woman_V);
    voice_sample_size[VOICE_WOMAN][22] = sizeof(snd_dialog_woman_W);
    voice_sample_size[VOICE_WOMAN][23] = sizeof(snd_dialog_woman_X);
    voice_sample_size[VOICE_WOMAN][24] = sizeof(snd_dialog_woman_Y);
    voice_sample_size[VOICE_WOMAN][25] = sizeof(snd_dialog_woman_Z);

    // MAN A-Z
    voice_sample[VOICE_MAN][0]  = snd_dialog_man_A;
    voice_sample[VOICE_MAN][1]  = snd_dialog_man_B;
    voice_sample[VOICE_MAN][2]  = snd_dialog_man_C;
    voice_sample[VOICE_MAN][3]  = snd_dialog_man_D;
    voice_sample[VOICE_MAN][4]  = snd_dialog_man_E;
    voice_sample[VOICE_MAN][5]  = snd_dialog_man_F;
    voice_sample[VOICE_MAN][6]  = snd_dialog_man_G;
    voice_sample[VOICE_MAN][7]  = snd_dialog_man_H;
    voice_sample[VOICE_MAN][8]  = snd_dialog_man_I;
    voice_sample[VOICE_MAN][9]  = snd_dialog_man_J;
    voice_sample[VOICE_MAN][10] = snd_dialog_man_K;
    voice_sample[VOICE_MAN][11] = snd_dialog_man_L;
    voice_sample[VOICE_MAN][12] = snd_dialog_man_M;
    voice_sample[VOICE_MAN][13] = snd_dialog_man_N;
    voice_sample[VOICE_MAN][14] = snd_dialog_man_O;
    voice_sample[VOICE_MAN][15] = snd_dialog_man_P;
    voice_sample[VOICE_MAN][16] = snd_dialog_man_Q;
    voice_sample[VOICE_MAN][17] = snd_dialog_man_R;
    voice_sample[VOICE_MAN][18] = snd_dialog_man_S;
    voice_sample[VOICE_MAN][19] = snd_dialog_man_T;
    voice_sample[VOICE_MAN][20] = snd_dialog_man_U;
    voice_sample[VOICE_MAN][21] = snd_dialog_man_V;
    voice_sample[VOICE_MAN][22] = snd_dialog_man_W;
    voice_sample[VOICE_MAN][23] = snd_dialog_man_X;
    voice_sample[VOICE_MAN][24] = snd_dialog_man_Y;
    voice_sample[VOICE_MAN][25] = snd_dialog_man_Z;

    // MAN sizes
    voice_sample_size[VOICE_MAN][0]  = sizeof(snd_dialog_man_A);
    voice_sample_size[VOICE_MAN][1]  = sizeof(snd_dialog_man_B);
    voice_sample_size[VOICE_MAN][2]  = sizeof(snd_dialog_man_C);
    voice_sample_size[VOICE_MAN][3]  = sizeof(snd_dialog_man_D);
    voice_sample_size[VOICE_MAN][4]  = sizeof(snd_dialog_man_E);
    voice_sample_size[VOICE_MAN][5]  = sizeof(snd_dialog_man_F);
    voice_sample_size[VOICE_MAN][6]  = sizeof(snd_dialog_man_G);
    voice_sample_size[VOICE_MAN][7]  = sizeof(snd_dialog_man_H);
    voice_sample_size[VOICE_MAN][8]  = sizeof(snd_dialog_man_I);
    voice_sample_size[VOICE_MAN][9]  = sizeof(snd_dialog_man_J);
    voice_sample_size[VOICE_MAN][10] = sizeof(snd_dialog_man_K);
    voice_sample_size[VOICE_MAN][11] = sizeof(snd_dialog_man_L);
    voice_sample_size[VOICE_MAN][12] = sizeof(snd_dialog_man_M);
    voice_sample_size[VOICE_MAN][13] = sizeof(snd_dialog_man_N);
    voice_sample_size[VOICE_MAN][14] = sizeof(snd_dialog_man_O);
    voice_sample_size[VOICE_MAN][15] = sizeof(snd_dialog_man_P);
    voice_sample_size[VOICE_MAN][16] = sizeof(snd_dialog_man_Q);
    voice_sample_size[VOICE_MAN][17] = sizeof(snd_dialog_man_R);
    voice_sample_size[VOICE_MAN][18] = sizeof(snd_dialog_man_S);
    voice_sample_size[VOICE_MAN][19] = sizeof(snd_dialog_man_T);
    voice_sample_size[VOICE_MAN][20] = sizeof(snd_dialog_man_U);
    voice_sample_size[VOICE_MAN][21] = sizeof(snd_dialog_man_V);
    voice_sample_size[VOICE_MAN][22] = sizeof(snd_dialog_man_W);
    voice_sample_size[VOICE_MAN][23] = sizeof(snd_dialog_man_X);
    voice_sample_size[VOICE_MAN][24] = sizeof(snd_dialog_man_Y);
    voice_sample_size[VOICE_MAN][25] = sizeof(snd_dialog_man_Z);

    // DEEP A-Z
    voice_sample[VOICE_DEEP][0]  = snd_dialog_deep_A;
    voice_sample[VOICE_DEEP][1]  = snd_dialog_deep_B;
    voice_sample[VOICE_DEEP][2]  = snd_dialog_deep_C;
    voice_sample[VOICE_DEEP][3]  = snd_dialog_deep_D;
    voice_sample[VOICE_DEEP][4]  = snd_dialog_deep_E;
    voice_sample[VOICE_DEEP][5]  = snd_dialog_deep_F;
    voice_sample[VOICE_DEEP][6]  = snd_dialog_deep_G;
    voice_sample[VOICE_DEEP][7]  = snd_dialog_deep_H;
    voice_sample[VOICE_DEEP][8]  = snd_dialog_deep_I;
    voice_sample[VOICE_DEEP][9]  = snd_dialog_deep_J;
    voice_sample[VOICE_DEEP][10] = snd_dialog_deep_K;
    voice_sample[VOICE_DEEP][11] = snd_dialog_deep_L;
    voice_sample[VOICE_DEEP][12] = snd_dialog_deep_M;
    voice_sample[VOICE_DEEP][13] = snd_dialog_deep_N;
    voice_sample[VOICE_DEEP][14] = snd_dialog_deep_O;
    voice_sample[VOICE_DEEP][15] = snd_dialog_deep_P;
    voice_sample[VOICE_DEEP][16] = snd_dialog_deep_Q;
    voice_sample[VOICE_DEEP][17] = snd_dialog_deep_R;
    voice_sample[VOICE_DEEP][18] = snd_dialog_deep_S;
    voice_sample[VOICE_DEEP][19] = snd_dialog_deep_T;
    voice_sample[VOICE_DEEP][20] = snd_dialog_deep_U;
    voice_sample[VOICE_DEEP][21] = snd_dialog_deep_V;
    voice_sample[VOICE_DEEP][22] = snd_dialog_deep_W;
    voice_sample[VOICE_DEEP][23] = snd_dialog_deep_X;
    voice_sample[VOICE_DEEP][24] = snd_dialog_deep_Y;
    voice_sample[VOICE_DEEP][25] = snd_dialog_deep_Z;

    // DEEP sizes
    voice_sample_size[VOICE_DEEP][0]  = sizeof(snd_dialog_deep_A);
    voice_sample_size[VOICE_DEEP][1]  = sizeof(snd_dialog_deep_B);
    voice_sample_size[VOICE_DEEP][2]  = sizeof(snd_dialog_deep_C);
    voice_sample_size[VOICE_DEEP][3]  = sizeof(snd_dialog_deep_D);
    voice_sample_size[VOICE_DEEP][4]  = sizeof(snd_dialog_deep_E);
    voice_sample_size[VOICE_DEEP][5]  = sizeof(snd_dialog_deep_F);
    voice_sample_size[VOICE_DEEP][6]  = sizeof(snd_dialog_deep_G);
    voice_sample_size[VOICE_DEEP][7]  = sizeof(snd_dialog_deep_H);
    voice_sample_size[VOICE_DEEP][8]  = sizeof(snd_dialog_deep_I);
    voice_sample_size[VOICE_DEEP][9]  = sizeof(snd_dialog_deep_J);
    voice_sample_size[VOICE_DEEP][10] = sizeof(snd_dialog_deep_K);
    voice_sample_size[VOICE_DEEP][11] = sizeof(snd_dialog_deep_L);
    voice_sample_size[VOICE_DEEP][12] = sizeof(snd_dialog_deep_M);
    voice_sample_size[VOICE_DEEP][13] = sizeof(snd_dialog_deep_N);
    voice_sample_size[VOICE_DEEP][14] = sizeof(snd_dialog_deep_O);
    voice_sample_size[VOICE_DEEP][15] = sizeof(snd_dialog_deep_P);
    voice_sample_size[VOICE_DEEP][16] = sizeof(snd_dialog_deep_Q);
    voice_sample_size[VOICE_DEEP][17] = sizeof(snd_dialog_deep_R);
    voice_sample_size[VOICE_DEEP][18] = sizeof(snd_dialog_deep_S);
    voice_sample_size[VOICE_DEEP][19] = sizeof(snd_dialog_deep_T);
    voice_sample_size[VOICE_DEEP][20] = sizeof(snd_dialog_deep_U);
    voice_sample_size[VOICE_DEEP][21] = sizeof(snd_dialog_deep_V);
    voice_sample_size[VOICE_DEEP][22] = sizeof(snd_dialog_deep_W);
    voice_sample_size[VOICE_DEEP][23] = sizeof(snd_dialog_deep_X);
    voice_sample_size[VOICE_DEEP][24] = sizeof(snd_dialog_deep_Y);
    voice_sample_size[VOICE_DEEP][25] = sizeof(snd_dialog_deep_Z);

    // TYPEWRITER (se mantiene igual, 8 sonidos)
    voice_sample[VOICE_TYPEWRITER][0] = snd_dialog_typewriter1;
    voice_sample[VOICE_TYPEWRITER][1] = snd_dialog_typewriter2;
    voice_sample[VOICE_TYPEWRITER][2] = snd_dialog_typewriter3;
    voice_sample[VOICE_TYPEWRITER][3] = snd_dialog_typewriter4;
    voice_sample[VOICE_TYPEWRITER][4] = snd_dialog_typewriter5;
    voice_sample[VOICE_TYPEWRITER][5] = snd_dialog_typewriter6;
    voice_sample[VOICE_TYPEWRITER][6] = snd_dialog_typewriter7;
    voice_sample[VOICE_TYPEWRITER][7] = snd_dialog_typewriter8;

    voice_sample_size[VOICE_TYPEWRITER][0] = sizeof(snd_dialog_typewriter1);
    voice_sample_size[VOICE_TYPEWRITER][1] = sizeof(snd_dialog_typewriter2);
    voice_sample_size[VOICE_TYPEWRITER][2] = sizeof(snd_dialog_typewriter3);
    voice_sample_size[VOICE_TYPEWRITER][3] = sizeof(snd_dialog_typewriter4);
    voice_sample_size[VOICE_TYPEWRITER][4] = sizeof(snd_dialog_typewriter5);
    voice_sample_size[VOICE_TYPEWRITER][5] = sizeof(snd_dialog_typewriter6);
    voice_sample_size[VOICE_TYPEWRITER][6] = sizeof(snd_dialog_typewriter7);
    voice_sample_size[VOICE_TYPEWRITER][7] = sizeof(snd_dialog_typewriter8);

    // Número de muestras por voz
    voice_numsamples[VOICE_WOMAN]      = 26;
    voice_numsamples[VOICE_MAN]        = 26;
    voice_numsamples[VOICE_DEEP]       = 26;
    voice_numsamples[VOICE_TYPEWRITER] = 8;
}
