#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void new_level(TileSet tile_bg, MapDefinition map_bg, TileSet tile_front, MapDefinition map_front, Palette new_pal, u8 new_scroll_mode, u8 new_scroll_speed); // Initialice level 
void update_bg(void); // Update background
void set_limits(u16 x1, u16 y1, u16 x2, u16 y2); // Set background limits
void show_interface(bool visible); // Show or hide the bottom interface

#endif