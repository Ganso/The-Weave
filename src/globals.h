#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Sega Genesis
#include <genesis.h>

// Debug
#include "KDebug.h"
#include "tools.h"
#include "timer.h"

// Resources
#include "../res/resources.h"
#include "../res/res_backgrounds.h"
#include "../res/res_characters.h"
#include "../res/res_faces.h"

// Game files
#include "texts.h"
#include "init.h"
#include "act_1.h"
#include "dialogs.h"
#include "texts.h"
#include "characters.h"
#include "controler.h"

// Animations
#define ANIM_IDLE       0
#define ANIM_WALK       1
#define ANIM_ACTION     2
#define ANIM_MAGIC      3

// Characters
#define MAX_CHR       3
#define CHR_linus     0
#define CHR_clio      1
#define CHR_xander    2

// Faces
#define MAX_FACE      3
#define FACE_linus    0
#define FACE_clio     1
#define FACE_xander   2
#define FACE_none     250
#define SIDE_left     true
#define SIDE_right    false
#define SIDE_none     true

// Languages
#define NUM_LANGUAGES 2
enum Languages {
    LANG_SPANISH,
    LANG_ENGLISH
};
u8 game_language;

// Other In Game Parameters
#define MAX_TALK_TIME   600   // Default maximum time in conversations (600 ticks, 10 seconds)

u16 tile_ind; // Tiles index

// Game entity definition
typedef struct
{
    const SpriteDefinition   *sd;
    s16                      x;
    s16                      y;
    u16                      palette;
    u8                       priority;
    u8                       flipH;
    u8                       animation;
    bool                     visible;
} Entity;

// Characters
Entity obj_character[MAX_CHR];
Sprite *spr_chr[MAX_CHR];
u8 active_character;

// Interface sprites
Sprite *spr_face_left; // Left face BG
Sprite *spr_face_right; // Right face BG
Sprite *spr_button_A; // Button with an A

// Faces
Entity obj_face[MAX_FACE];
Sprite *spr_face[MAX_FACE];

// Backgrounds
Map *background_BGA;
Map *background_BGB;
u32 offset_BGA;
u32 offset_BGB;

// Background scroll modes
u8 background_scroll_mode;
u8 scroll_speed;
#define BG_SCRL_AUTO_RIGHT   0  // auto mode (updated every frame)
#define BG_SCRL_AUTO_LEFT    1
#define BG_SCRL_USER_RIGHT   10 // user mode (updated on character walk)
#define BG_SCRL_USER_LEFT    11


// Screen limits
u16 x_limit_min; // Minimum x position
u16 x_limit_max; // Maximum x position
u16 y_limit_min; // Minimum y position
u16 y_limit_max; // Maximum y position

// Global functions
void wait_seconds(int sec); // Wait for N seconds
void initialize_character(u8 nchar); // Initialize a character
void initialize_face(u8 nface); // Initialize a face
void update_bg(void); // Update background
void next_frame(void); // Wait for next frame
void new_level(TileSet tile_bg, MapDefinition map_bg, TileSet tile_front, MapDefinition map_front, Palette new_pal, u8 new_scroll_mode, u8 new_scroll_speed); // Initialice level 
void set_limits(u16 x1, u16 y1, u16 x2, u16 y2); // Set background limits

#endif
