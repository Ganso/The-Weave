#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

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
#define CHR_NONE      254

// Faces
#define MAX_FACE      3
#define FACE_linus    0
#define FACE_clio     1
#define FACE_xander   2
#define FACE_none     250
#define SIDE_left     true
#define SIDE_right    false
#define SIDE_none     true

// Characters
Entity obj_character[MAX_CHR];
Sprite *spr_chr[MAX_CHR];
u16 active_character; // Which character is the active one
bool movement_active; // Can you move ?

// Faces
Entity obj_face[MAX_FACE];
Sprite *spr_face[MAX_FACE];

// Collisions
#define MAX_COLLISIONS 30 // If we collide with an enemy more than X times, move the character anyway (CHANGE IT!)
u8 num_colls;             // Number of collisions

void init_character(u16 nchar); // Initialize a character
void release_character(u16 nchar); // Release a character from memory
void init_face(u16 nface); // Initialize a face
void release_face(u16 nface); // Release a face from memory
void update_character(u16 nchar); // Update a character based on every parameter
void show_character(u16 nchar, bool show); // Show or hide a character
void anim_character(u16 nchar, u8 newanimation); // Change a character's animation
void look_left(u16 nchar, bool left); // Make a character look to the left (or right)
void move_character(u16 nchar, s16 x, s16 y); // Move a character to a new position
void move_character_instant(u16 nchar, s16 x, s16 y); // Move a character to a new position (instantly)
void update_sprites_depth(void); // Update characters, items and enemies depth

#endif