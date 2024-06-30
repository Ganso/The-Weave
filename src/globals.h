#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Genesis
#include <genesis.h>
#include "resources.h"

// Debug
#include "KDebug.h"
#include "tools.h"
#include "timer.h"

// Game files
#include "texts.h"
#include "init.h"
#include "act_1.h"

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

// Global functions
void wait_seconds(int); // Wait for N seconds
void initialize_character(u8); // Initialize a character
void initialize_face(u8); // Initialize a face

// In game functions
void update_character(u8); // Update a character based on every parameter
void show_character(u8, bool); // Show or hide a character
void anim_character(u8 nchar, u8 newanimation); // Change a character's animation
void look_left(u8, bool); // Make a character look to the left (or right)
void talk(u8, bool, char *, u16); // Make a character talk
void talk_dialog(u8, bool, u16, u16, u16); // Make a character talk a dialog line
void move_character(u8,s16,s16); // Move a character to a new position
void update_bg(void); // Update background
void next_frame(void); // Wait for next frame

#endif
