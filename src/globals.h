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
#include "../res/res_enemies.h"
#include "../res/res_interface.h"
#include "../res/res_sound.h"

// Game files
#include "texts.h"
#include "init.h"
#include "act_1.h"
#include "dialogs.h"
#include "texts.h"
#include "characters.h"
#include "controller.h"
#include "interface.h"
#include "patterns.h"

// Animations
#define ANIM_IDLE       0
#define ANIM_WALK       1
#define ANIM_ACTION     2
#define ANIM_MAGIC      3

// Characters
#define MAX_CHR       4
#define CHR_linus     0
#define CHR_clio      1
#define CHR_xander    2
#define CHR_badbobbin 3

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
u8 active_character; // Which character is the active one
bool movement_active; // Can you move ?

// Faces
Entity obj_face[MAX_FACE];
Sprite *spr_face[MAX_FACE];

// Backgrounds
Map *background_BGA;
Map *background_BGB;
u32 offset_BGA;
u32 offset_BGB;

// Background scroll modes
u8 background_scroll_mode; // Scroll modes (BG_SCRL_*)
u8 scroll_speed; // Scroll speed (each mode uses it in a way)
bool player_scroll_active; // Can you scroll ?
#define BG_SCRL_AUTO_RIGHT   0  // auto mode (updated every frame)
#define BG_SCRL_AUTO_LEFT    1  // no use
#define BG_SCRL_USER_RIGHT   10 // user mode (updated on character walk)
#define BG_SCRL_USER_LEFT    11 // no use

// Screen limits
u16 x_limit_min; // Minimum x position
u16 x_limit_max; // Maximum x position
u16 y_limit_min; // Minimum y position
u16 y_limit_max; // Maximum y position

// Global functions
void wait_seconds(int sec); // Wait for N seconds
void initialize_character(u8 nchar); // Initialize a character
void release_character(u8 nchar); // Release a character from memory
void initialize_face(u8 nface); // Initialize a face
void release_face(u8 nface); // Release a face from memory
void next_frame(void); // Wait for next frame and do each-frame actions

#endif
