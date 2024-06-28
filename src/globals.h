#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Genesis
#include <genesis.h>
#include "resources.h"

// Debug
#include "KDebug.h"
#include "tools.h"
#include "timer.h"

// Animations
#define ANIM_IDLE       0
#define ANIM_WALK       1
#define ANIM_ACTION     2
#define ANIM_MAGIC      3

// Characters
#define MAX_CHR       2
#define CHR_linus     0
#define CHR_clio      1

// Faces
#define MAX_FACE      2
#define FACE_linus    0
#define FACE_clio     1

// Other In Game Parameters
#define MAX_TALK_TIME   10000   // Default maximum time in conversations (10 seconds)

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
} Entity;

// Characters
Entity obj_character[MAX_CHR];
Sprite *spr_chr[MAX_CHR];


// Faces backgrounds
Sprite *spr_face_left; // Left face BG
Sprite *spr_face_right; // Right face BG


// Faces
Entity obj_face[MAX_FACE];
Sprite *spr_face[MAX_FACE];


// Game files
#include "init.h"
#include "act_1.h"


// Global functions
void wait_seconds(int); // Wait for N seconds
void initialize_character(u8); // Initialize a character
void initialize_face(u8); // Initialize a face

// In game functions
void show_character(u8, bool); // Show or hide a character
void look_right(u8, bool); // Make a character look to the left (or right)
void talk(u8, bool, char *, char *, char *, u16); // Make a character talk

#endif
