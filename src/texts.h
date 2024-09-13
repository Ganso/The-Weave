#ifndef _TXT_H_
#define _TXT_H_

// Languages
#define NUM_LANGUAGES 2
enum Languages {
    LANG_SPANISH,
    LANG_ENGLISH
};
u8 game_language;

// Acts and dialogs
#define SYSTEM_DIALOG   0
#define ACT1_DIALOG1    1
#define ACT1_DIALOG2    2
#define ACT1_DIALOG3    3

typedef struct {
    u8 face;
    bool side;
    u16 max_seconds;
    const char *text[NUM_LANGUAGES];
} DialogItem;

// Game texts
extern const DialogItem *dialogs[];

// Functions
char* encode_spanish_text(const char* input); // Code Spanish text in the game font charset

#endif