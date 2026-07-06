#ifndef _ENCODE_H_
#define _ENCODE_H_

// Spanish text encoding: maps UTF-8 Spanish characters to the game font charset.
// ñ→^  á→#  é→$  í→%  ó→*  ú→/  ¿→<  ¡→>

#include <genesis.h>

// Max length of an encoded text line (static buffer in encode_spanish_text)
#define MAX_ENCODED_TEXT_LEN 256

char* encode_spanish_text(const char* input); // Code Spanish text in the game font charset (static buffer, overwritten per call)

#endif
