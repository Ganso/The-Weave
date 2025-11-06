#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#define DEFAULT_TALK_TIME 10   // Default maximum time in conversations (10 seconds)
#define DEFAULT_CHOICE_TIME 30   // Default maximum time for choices (30 seconds)

// Voice types
#define VOICE_WOMAN 0
#define VOICE_MAN 1
#define VOICE_DEEP 2
#define VOICE_TYPEWRITER 3

#define MAX_VOICE 4 // Total number of voice types
#define MAX_DIALOG_SOUNDS 26 // Maximum number of dialog sounds per voice

// Voice sample arrays (numeric indices)
extern const u8 *voice_sample[MAX_VOICE][MAX_DIALOG_SOUNDS]; // Character voice samples
extern u32 voice_sample_size[MAX_VOICE][MAX_DIALOG_SOUNDS]; // Size of each character voice sample
extern u8 voice_numsamples[MAX_VOICE]; // Number of samples per voice

// Function prototypes
void talk(u8 nface, bool isinleft, char *text, u16 max_seconds, bool sound_on); // Make a character talk
void talk_dialog(const DialogItem *dialog, bool sound_on); // Talk a dialog line
void split_text(char *input_text, char *line1, char *line2, char *line3); // Split a text in up to three lines
void print_line(char *text, u16 x, u16 y, bool wait_for_fram, u8 nface, bool sound_on); // Print a line of text, character by character
u8 choice(u8 nface, bool isinleft, char **options, u8 num_options, u16 max_seconds); // Show a choice dialog directly
u8 choice_dialog(const ChoiceItem *choice); // Show a choice dialog from a ChoiceItem
void talk_cluster(const DialogItem *start, bool sound_on); // Talk multiple lines
void init_voice_samples(void); // Initialize character voice samples

#endif
