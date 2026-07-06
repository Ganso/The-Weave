#include <genesis.h>
#include "core/config.h"
#include "narrative/texts.h"
#include "narrative/encode.h"
#include "narrative/dialogs.h"
#include "actors/characters.h"

// MAX TEXT LENGHT: "123456789012345678901234567890" (30 characters)

// Global variable definitions
u8 game_language=LANG_ENGLISH;

const ChoiceItem act1_choice1[] = {
    {FACE_linus, SIDE_RIGHT, DEFAULT_CHOICE_TIME, 3,
        {{ "¿Los Tejedores?", "Era mi leyenda favorita", "¿Qué pasó con ellos?"},
        { "The Weavers?", "It was my favourite legend", "What happened to them?"}}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_CHOICE_TIME, 2,
        {{ "Tengo que ir a la isla", "¿Vendrías conmigo?"},
        { "I have to go to the island", "Would you come with me?"}}},
    {0, false, DEFAULT_TALK_TIME, 0, {{NULL}}} // Terminator
};

const ChoiceItem *choices[] = {
    act1_choice1,   // 0
};

 // Code Spanish text in the game font charset
// SPANISH CHARSET
// ñ --> ^
// á --> #
// é --> $
// í --> %
// ó --> *
// ú --> /
// < --> ¿
// > --> ¡

// Calculate visible length ignoring color escape codes
u16 visible_length(const char *text)
{
    u16 len = 0;
    for (u16 i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '@' && (text[i + 1] == '[' || text[i + 1] == ']'))
        {
            i++; // Skip escape sequence
            continue;
        }
        len++;
    }
    return len;
}

u16 calculate_text_position(const char *text, bool is_face_left, bool has_face)
{
    u16 base_pos = 8; // Default left padding when face is present
    u16 max_width = has_face ? 33 : 40; // Available characters based on face presence

    char *encoded_text = NULL;
    const char *ptr = text;
    if (game_language == LANG_SPANISH)
    {
        encoded_text = encode_spanish_text(text);
        if (encoded_text != NULL)
            ptr = encoded_text;
    }

    u16 text_length = visible_length(ptr);
    u16 centered_pos = (max_width - text_length) >> 1; // (width - len)/2 using bit shift

    if (has_face && !is_face_left)
        base_pos -= 8; // Adjust for right-positioned face
    else if (!has_face)
        base_pos = 0; // No face, full width centering

    return base_pos + centered_pos;
}
