#ifndef _INTERFACE_H_
#define _INTERFACE_H_

void update_bg(void); // Update background
void set_limits(u16 x1, u16 y1, u16 x2, u16 y2); // Set background limits
void show_interface(bool visible); // Show or hide the bottom interface
void pause_screen(void); // Pause / State screen
void show_pattern_list(bool show, u8 active_pattern); // Show or hide pattern list
void show_note_in_pattern_list(u8 npattern, u8 nnote, bool show); // Show or hide pattern list
void show_note(u8 nnote, bool visible); // Show or hide notespentsprite
void hide_rod_icons(void); // Hide icons in the rod
void show_pattern_icon(u16 npattern, u16 x, bool show, bool priority); // Show or hide the icon of a pattern spell

#endif