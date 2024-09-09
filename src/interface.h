#ifndef _INTERFACE_H_
#define _INTERFACE_H_

// Define a type to store visibility state (for hidding them all in pause screen)
typedef struct {
    Sprite* sprite;
    SpriteVisibility visibility;
} SpriteState;

// Interface sprites
Sprite *spr_face_left; // Left face BG
Sprite *spr_face_right; // Right face BG
Sprite *spr_int_button_A; // Button with an A
Map *map_int_rod; // Rod
Sprite *spr_int_rod_1,*spr_int_rod_2,*spr_int_rod_3,*spr_int_rod_4,*spr_int_rod_5,*spr_int_rod_6; // Rod (notes)
Sprite *spr_int_enemy_rod_1,*spr_int_enemy_rod_2,*spr_int_enemy_rod_3,*spr_int_enemy_rod_4,*spr_int_enemy_rod_5,*spr_int_enemy_rod_6; // Rod (enemy notes)
Sprite *spr_int_pentagram_1,*spr_int_pentagram_2,*spr_int_pentagram_3,*spr_int_pentagram_4,*spr_int_pentagram_5,*spr_int_pentagram_6; // Pentagram (notes)
Sprite *spr_int_life_counter; // Life counter

// Pause screen sprites
Sprite *spr_pause_icon[5]; // Icon list
Sprite *spr_pattern_list_note[4]; // The 4 notes in right of the pattern list

// Others
bool interface_active; // Do we have to show the interface?

void show_interface(bool visible); // Show or hide the bottom interface
void show_note(u8 nnote, bool visible); // Show or hide notes
void hide_rod_icons(void); // Hide icons in the rod
void hide_pentagram_icons(void); // Hide icons in the pentagram
void hide_pattern_icons(void); // Hide all pattern icons
void show_pattern_icon(u16 npattern, bool show, bool priority); // Show or hide the icon of a pattern spell
void restoreSpritesVisibility(SpriteState* states, u16 count); // Function to restore the visibility of sprites
SpriteState* hideAllSprites(u16* count); // Function to hide all sprites and save their state
void pause_screen(void); // Pause / State screen
void show_pause_pattern_list(bool show, u8 active_pattern); // Show or hide pattern list (Pause screen)
void show_note_in_pause_pattern_list(u8 npattern, u8 nnote, bool show); // Show one of the notes of a pattern in the pattern list (Pause screen)
void show_icon_in_pause_list(u16 npattern, u8 nicon, u16 x, bool show, bool priority); // Show one of the notes of a pattern in the pattern list (Pause screen)

#endif