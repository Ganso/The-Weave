#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#define DEFAULT_TALK_TIME 10   // Default maximum time in conversations (10 seconds)
#define DEFAULT_CHOICE_TIME 30   // Default maximum time for choices (30 seconds)

void talk(u8 nface, bool isinleft, char *text, u16 max_seconds); // Make a character talk
void talk_dialog(const DialogItem *dialog); // Talk a dialog line
void split_text(char *input_text, char *line1, char *line2, char *line3); // Split a text in up to three lines
void print_line(char *text, u16 x, u16 y, bool wait_for_frame); // Print a line of text, character by character
u8 choice(u8 nface, bool isinleft, char **options, u8 num_options, u16 max_seconds); // Show a choice dialog directly
u8 choice_dialog(const ChoiceItem *choice); // Show a choice dialog from a ChoiceItem
void talk_cluster(const DialogCluster *cluster); // Talk multiple lines

#endif
