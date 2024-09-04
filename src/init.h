#ifndef _INIT_H_
#define _INIT_H_

void initialize(void); // Initialize engine
void new_level(const TileSet *tile_bg, const MapDefinition *map_bg, const TileSet *tile_front, const MapDefinition *map_front, Palette new_pal, u16 new_background_width, u8 new_scroll_mode, u8 new_scroll_speed); // Initialize level 

#endif
