#ifndef _INIT_H_
#define _INIT_H_

void initialize(bool first_time); // Initialize engine
void new_level(const TileSet *tile_bg, const MapDefinition *map_bg, const TileSet *tile_front, const MapDefinition *map_front, Palette new_pal, u16 new_background_width, u8 new_scroll_mode, u8 new_scroll_speed); // Initialize level 
void end_level(void); // Free all resources used by the level

#endif
