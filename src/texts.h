#ifndef _TXT_H_
#define _TXT_H_

#include "globals.h"

/**
 * @brief Supported languages enumeration
 */
enum Languages {
    LANG_SPANISH,
    LANG_ENGLISH
};

// Current game language
extern u8 game_language;

/**
 * @brief Dialog item structure
 * Represents a single line of dialog with translations
 */
typedef struct {
    u8 face;                          // Face ID to show
    bool side;                        // Face position (SIDE_left/SIDE_right)
    u16 max_seconds;                  // Display duration
    const char *text[NUM_LANGUAGES];  // Translated text strings
} DialogItem;

/**
 * @brief Choice item structure
 * Represents a dialog choice with multiple options
 */
typedef struct {
    u8 face;                                      // Face ID to show
    bool side;                                    // Face position (SIDE_left/SIDE_right)
    u16 max_seconds;                              // Selection time limit
    u8 num_options;                               // Number of choices
    const char *options[NUM_LANGUAGES][MAX_CHOICES]; // Translated choice options
} ChoiceItem;

// Dialog and choice collections
extern const DialogItem *dialogs[];   // All game dialogs
extern const ChoiceItem *choices[];   // All game choices

/**
 * @brief Convert text to use Spanish characters in game font
 * @param input Text to convert
 * @return Converted text string
 */
char* encode_spanish_text(const char* input);

#endif // _TXT_H_
