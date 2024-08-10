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
#define ACT1_DIALOG1 0
#define ACT1_DIALOG2 1
#define ACT1_DIALOG3 2

// Game texts
extern const char **dialog[];

#endif