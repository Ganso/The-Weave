#ifndef _DIALOGS_H_
#define _DIALOGS_H_

void talk(u8, bool, char *, u16); // Make a character talk
void talk_dialog(u8, bool, u16, u16, u16); // Make a character talk a dialog line
void split_text(char *, char *, char *, char *); // Split a text in up to three lines

#endif