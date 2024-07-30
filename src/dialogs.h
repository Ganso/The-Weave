#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#define MAX_TALK_TIME 600   // Default maximum time in conversations (600 ticks, 10 seconds)

void talk(u8, bool, char *, u16); // Make a character talk
void talk_dialog(u8, bool, u16, u16, u16); // Make a character talk a dialog line
void split_text(char *, char *, char *, char *); // Split a text in up to three lines
const char* getDialog(int, int, int); // Gets a line of dialog
void print_line(char *text, u16 x, u16 y); // Print a line of text, character by character

#endif