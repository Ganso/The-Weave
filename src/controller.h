#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <genesis.h>

void joy_check(void); // Main function to check and handle joystick input
void handle_movement(u16 joy_value); // Handle character movement based on joystick input
void handle_character_movement(s16 dx, s16 dy); // Move the character if there's no collision and it's within the screen limits. This function also handles background scrolling if the character is at the screen edge.
void update_character_animation(void); //Update the character's animation based on its current state
void handle_action_buttons(u16 joy_value); // Handle action button inputs (A, B, C, X, Y, Z)
void update_action_animation(void); // Update the character's animation when playing a musical note
void handle_pause_button(u16 joy_value); // Handle the pause button input

#endif // _CONTROLLER_H_
