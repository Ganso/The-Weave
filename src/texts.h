#ifndef _TXT_H_
#define _TXT_H_

// Languages
#define NUM_LANGUAGES 2
#define MAX_CHOICES 4

// Sides
#define SIDE_LEFT true
#define SIDE_RIGHT false

enum Languages {
    LANG_SPANISH,
    LANG_ENGLISH
};
extern u8 game_language;


// Choices
#define ACT1_CHOICE1    0

typedef struct {
    u8 face;
    bool side;
    u16 max_seconds;
    const char *text[NUM_LANGUAGES];
} DialogItem;

typedef struct {
    const DialogItem *dialog; // Array containing the lines
} DialogCluster;

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
extern const DialogCluster dialog_clusters[];

// Functions
char* encode_spanish_text(const char* input); // Code Spanish text in the game font charset
u16 calculate_text_position(const char *text, bool is_face_left, bool has_face); // Calculate centered text position

#endif
