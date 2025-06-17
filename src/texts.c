#include "globals.h"

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

    if (encoded_text != NULL)
        free(encoded_text);

    return base_pos + centered_pos;
}

char* encode_spanish_text(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    size_t len = strlen(input);
    char* result = (char*)malloc(len + 1); // +1 for the null terminator
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    size_t i = 0;
    size_t j = 0;
    while (i < len) {
        if ((unsigned char)input[i] == 0xC3) {
            // This is a two-byte UTF-8 sequence
            if (i + 1 < len) {
                switch ((unsigned char)input[i + 1]) {
                    case 0xB1: // ñ
                    case 0x91: // Ñ
                        result[j] = '^';
                        break;
                    case 0xA1: // á
                    case 0x81: // Á
                        result[j] = '#';
                        break;
                    case 0xA9: // é
                    case 0x89: // É
                        result[j] = '$';
                        break;
                    case 0xAD: // í
                    case 0x8D: // Í
                        result[j] = '%';
                        break;
                    case 0xB3: // ó
                    case 0x93: // Ó
                        result[j] = '*';
                        break;
                    case 0xBA: // ú
                    case 0x9A: // Ú
                        result[j] = '/';
                        break;
                    default:
                        // If it's not a special character, copy both bytes
                        result[j] = input[i];
                        j++;
                        result[j] = input[i + 1];
                }
                i += 2; // Skip the next byte as we've already processed it
            } else {
                // Unexpected end of string, just copy the byte
                result[j] = input[i];
                i++;
            }
        } else if ((unsigned char)input[i] == 0xC2) {
            // This is a two-byte UTF-8 sequence for ¿ and ¡
            if (i + 1 < len) {
                switch ((unsigned char)input[i + 1]) {
                    case 0xBF: // ¿
                        result[j] = '<';
                        break;
                    case 0xA1: // ¡
                        result[j] = '>';
                        break;
                    default:
                        // If it's not a special character, copy both bytes
                        result[j] = input[i];
                        j++;
                        result[j] = input[i + 1];
                }
                i += 2; // Skip the next byte as we've already processed it
            } else {
                // Unexpected end of string, just copy the byte
                result[j] = input[i];
                i++;
            }
        } else {
            // Regular ASCII character, just copy it
            result[j] = input[i];
            i++;
        }
        j++;
    }
    result[j] = '\0'; // Ensure null-termination

    return result;
}
