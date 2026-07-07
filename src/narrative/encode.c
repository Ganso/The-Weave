#include <genesis.h>
#include "narrative/encode.h"

char* encode_spanish_text(const char* input) {
    // Static buffer: no malloc/free on the 64KB RAM (B7). The encoded text is never
    // longer than the input, so len+1 bytes are always enough. Each call overwrites
    // the previous result: consume it before encoding another text.
    static char encoded[MAX_ENCODED_TEXT_LEN];

    if (input == NULL) {
        return NULL;
    }

    size_t len = strlen(input);
    if (len >= MAX_ENCODED_TEXT_LEN) len = MAX_ENCODED_TEXT_LEN - 1; // Defensive truncation
    char* result = encoded;

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
