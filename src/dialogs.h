#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#define DEFAULT_TALK_TIME 10   // Default maximum time in conversations (10 seconds)
#define DEFAULT_CHOICE_TIME 30   // Default maximum time for choices (30 seconds)

// Voices
#define MAX_VOICE          4
#define VOICE_typewriter   0
#define VOICE_woman        1
#define VOICE_man          2
#define VOICE_deep         3

// Dialog sounds
#define MAX_DIALOG_SOUNDS 8  // Maximum number of dialog sounds per character

extern const u8 *voice_sample[MAX_VOICE][MAX_DIALOG_SOUNDS]; // Character voice samples
extern u32 voice_sample_size[MAX_VOICE][MAX_DIALOG_SOUNDS]; // Size of each character voice sample
extern u8 voice_numsamples[MAX_VOICE]; // Number of samples per voice

void talk(u8 nface, bool isinleft, char *text, u16 max_seconds); // Make a character talk
void talk_dialog(const DialogItem *dialog); // Talk a dialog line
void split_text(char *input_text, char *line1, char *line2, char *line3); // Split a text in up to three lines
void print_line(char *text, u16 x, u16 y, bool wait_for_fram, u8 nface); // Print a line of text, character by character
u8 choice(u8 nface, bool isinleft, char **options, u8 num_options, u16 max_seconds); // Show a choice dialog directly
u8 choice_dialog(const ChoiceItem *choice); // Show a choice dialog from a ChoiceItem
void talk_cluster(const DialogItem *start); // Talk multiple lines
void init_voice_samples(void); // Initialize character voice samples

#endif
