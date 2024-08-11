#ifndef _INTERFACE_H_
#define _INTERFACE_H_

// Interface sprites
Sprite *spr_face_left; // Left face BG
Sprite *spr_face_right; // Right face BG
Sprite *spr_int_button_A; // Button with an A
Map *map_int_rod; // Rod
Sprite *spr_int_rod_1,*spr_int_rod_2,*spr_int_rod_3,*spr_int_rod_4,*spr_int_rod_5,*spr_int_rod_6; // Rod (notes)
Sprite *spr_int_enemy_rod_1,*spr_int_enemy_rod_2,*spr_int_enemy_rod_3,*spr_int_enemy_rod_4,*spr_int_enemy_rod_5,*spr_int_enemy_rod_6; // Rod (enemy notes)
Sprite *spr_int_pentagram_1,*spr_int_pentagram_2,*spr_int_pentagram_3,*spr_int_pentagram_4,*spr_int_pentagram_5,*spr_int_pentagram_6; // Pentagram (notes)

bool interface_active; // Do we have to show the interface?

void show_interface(bool visible); // Show or hide the bottom interface
void pause_screen(void); // Pause / State screen
void show_pattern_list(bool show, u8 active_pattern); // Show or hide pattern list
void show_note_in_pattern_list(u8 npattern, u8 nnote, bool show); // Show or hide pattern list
void show_note(u8 nnote, bool visible); // Show or hide notespentsprite
void hide_rod_icons(void); // Hide icons in the rod
void hide_pentagram_icons(void); // Hide icons in the pentagram
void show_pattern_icon(u16 npattern, u16 x, bool show, bool priority); // Show or hide the icon of a pattern spell

#endif