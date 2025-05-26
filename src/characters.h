#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

// Characters
#define MAX_CHR       4
#define CHR_linus     0
#define CHR_clio      1
#define CHR_xander    2
#define CHR_swan      3
#define CHR_NONE      254

// Faces
#define MAX_FACE      4
#define FACE_linus    0
#define FACE_clio     1
#define FACE_xander   2
#define FACE_swan     3
#define FACE_none     250

// Sides
#define SIDE_left     true
#define SIDE_right    false
#define SIDE_none     true

// Distances for following characters
#define MAX_FOLLOW_DISTANCE 40
#define MIN_FOLLOW_DISTANCE 20

// Characters
extern Entity obj_character[MAX_CHR];
extern Sprite *spr_chr[MAX_CHR];
extern Sprite *spr_chr_shadow[MAX_CHR];
extern u16 active_character; // Which character is the active one
extern bool movement_active; // Can you move ?

// Faces
extern Entity obj_face[MAX_FACE];
extern Sprite *spr_face[MAX_FACE];

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
void update_character_shadow(u16 nchar); // Update shadow position for a character
void follow_active_character(u16 nchar, bool follow, u8 follow_speed); // Follow (or unfollow active character)
void approach_characters(void); // Move characters with STATE_FOLLOWING towards the active character
void reset_character_animations(void); // Reset all character animations to idle
void update_character_animations(void); //Update the character's animation based on its current state

#endif
