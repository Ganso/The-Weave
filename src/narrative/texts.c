// texts.c — idioma activo y utilidades de posicionado de texto
#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "narrative/narrative.h"

// MAX TEXT LENGHT: "123456789012345678901234567890" (30 characters)

// Global variable definitions
u8 game_language=LANG_ENGLISH;

// (Los choices viven ahora en data/choices.csv → narrative/choices_data.c, generado)

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
