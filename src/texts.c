#include "globals.h"

#define SIDE_LEFT true
#define SIDE_RIGHT false

// MAX TEXT LENGHT: "123456789012345678901234567890" (30 characters)

// Global variable definitions
u8 game_language=LANG_ENGLISH;

// Dialog data generated from CSV
#include "texts_generated.c"

const DialogItem *dialogs[] = {
    system_dialog,  // 0
    act1_dialog1,   // 1
    act1_dialog2,   // 2
    act1_dialog3,   // 3
    act1_dialog4,   // 4
};

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

u16 calculate_text_position(const char *text, bool is_face_left, bool has_face) {
    u16 base_pos = 8; // Default left padding when face is present
    u16 max_width = has_face ? 33 : 40; // Available characters based on face presence
    
    u16 text_length = strlen(text);
    u16 centered_pos = (max_width - text_length) >> 1; // (width - len)/2 using bit shift
    
    if (has_face && !is_face_left) {
        base_pos -= 8; // Adjust for right-positioned face
    } else if (!has_face) {
        base_pos = 0; // No face, full width centering
    }
    
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
