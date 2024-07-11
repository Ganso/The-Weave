#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

void update_character(u8); // Update a character based on every parameter
void show_character(u8, bool); // Show or hide a character
void anim_character(u8 nchar, u8 newanimation); // Change a character's animation
void look_left(u8, bool); // Make a character look to the left (or right)
void move_character(u8,s16,s16); // Move a character to a new position
void move_character_instant(u8,s16,s16); // Move a character to a new position (instantly)

#endif