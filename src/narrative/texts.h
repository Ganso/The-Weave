// texts.h — tipos de diálogo/choice, idiomas y utilidades de texto
#ifndef _TXT_H_
#define _TXT_H_

#include <genesis.h>

// Languages
#define NUM_LANGUAGES 2
#define MAX_CHOICES 4

// Dialog side (B12: single 3-value vocabulary; replaces two sets of bool defines
// where SIDE_none was accidentally equal to SIDE_left)
#define SIDE_LEFT  0
#define SIDE_RIGHT 1
#define SIDE_NONE  2   // positions like LEFT; reserved for dialogs with no speaker side

enum Languages {
    LANG_SPANISH,
    LANG_ENGLISH
};
extern u8 game_language;

typedef struct {
    u8 face;
    u8 side;          // SIDE_LEFT / SIDE_RIGHT / SIDE_NONE (B12: was bool)
    u16 max_seconds;
    const char *text[NUM_LANGUAGES];
} DialogItem;


typedef struct {
    u8 face;
    u8 side;          // SIDE_LEFT / SIDE_RIGHT / SIDE_NONE (B12: was bool)
    u16 max_seconds;
    u8 num_options;
    const char *options[NUM_LANGUAGES][MAX_CHOICES];
} ChoiceItem;

// Game texts
extern const DialogItem *dialogs[];
// (la tabla choices[] vive en narrative/choices_data.h, generada de data/choices.csv)

// Functions
u16 calculate_text_position(const char *text, bool is_face_left, bool has_face); // Calculate centered text position
u16 visible_length(const char *text); // Length ignoring color codes

#endif
