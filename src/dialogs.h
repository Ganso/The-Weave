#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#define MAX_TALK_TIME 600   // Default maximum time in conversations (600 ticks, 10 seconds)

void talk(u8 nface, bool isinleft, char *text, u16 max_ticks); // Make a character talk
void talk_dialog(const DialogItem *dialog); // Talk a dialog line
void split_text(char *input_text, char *line1, char *line2, char *line3); // Split a text in up to three lines
void print_line(char *text, u16 x, u16 y); // Print a line of text, character by character

#endif