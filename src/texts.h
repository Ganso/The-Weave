#ifndef _TXT_H_
#define _TXT_H_

// Languages
#define NUM_LANGUAGES 2
#define MAX_CHOICES 4

enum Languages {
    LANG_SPANISH,
    LANG_ENGLISH
};
extern u8 game_language;

// Acts and dialogs
#define SYSTEM_DIALOG   0
#define ACT1_DIALOG1    1
#define ACT1_DIALOG2    2
#define ACT1_DIALOG3    3
#define ACT1_DIALOG4    4

// Choices
#define ACT1_CHOICE1    0

typedef struct {
    u8 face;
    bool side;
    u16 max_seconds;
    const char *text[NUM_LANGUAGES];
} DialogItem;

typedef struct {
    u8 face;
    bool side;
    u16 max_seconds;
    u8 num_options;
    const char *options[NUM_LANGUAGES][MAX_CHOICES];
} ChoiceItem;

// Game texts
extern const DialogItem *dialogs[];
extern const ChoiceItem *choices[];

// Functions
char* encode_spanish_text(const char* input); // Code Spanish text in the game font charset

#endif
