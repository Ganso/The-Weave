#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include "globals.h"

/**
 * @brief Make a character talk with text
 * @param nface Face ID to show
 * @param isinleft Whether face is on left side
 * @param text Text to display
 * @param max_seconds How long to show dialog (0 for DEFAULT_TALK_TIME)
 */
void talk(u8 nface, bool isinleft, char *text, u16 max_seconds);

/**
 * @brief Display a dialog item
 * @param dialog Dialog item to display
 */
void talk_dialog(const DialogItem *dialog);

/**
 * @brief Split text into up to three lines
 * @param input_text Text to split
 * @param line1 First line output
 * @param line2 Second line output
 * @param line3 Third line output
 */
void split_text(char *input_text, char *line1, char *line2, char *line3);

/**
 * @brief Print text line character by character
 * @param text Text to print
 * @param x X position
 * @param y Y position
 */
void print_line(char *text, u16 x, u16 y);

/**
 * @brief Show a choice dialog
 * @param nface Face ID to show
 * @param isinleft Whether face is on left side
 * @param options Array of choice text options
 * @param num_options Number of options
 * @param max_seconds How long to show choices (0 for DEFAULT_CHOICE_TIME)
 * @return Selected option index
 */
u8 choice(u8 nface, bool isinleft, char **options, u8 num_options, u16 max_seconds);

/**
 * @brief Display a choice item
 * @param choice Choice item to display
 * @return Selected option index
 */
u8 choice_dialog(const ChoiceItem *choice);

#endif // _DIALOGS_H_
