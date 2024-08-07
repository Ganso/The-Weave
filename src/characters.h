#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

// Characters
Entity obj_character[MAX_CHR];
Sprite *spr_chr[MAX_CHR];
u8 active_character; // Which character is the active one
bool movement_active; // Can you move ?

// Faces
Entity obj_face[MAX_FACE];
Sprite *spr_face[MAX_FACE];

void init_character(u8 nchar); // Initialize a character
void release_character(u8 nchar); // Release a character from memory
void init_face(u8 nface); // Initialize a face
void release_face(u8 nface); // Release a face from memory
void update_character(u8); // Update a character based on every parameter
void show_character(u8, bool); // Show or hide a character
void anim_character(u8 nchar, u8 newanimation); // Change a character's animation
void look_left(u8, bool); // Make a character look to the left (or right)
void move_character(u8,s16,s16); // Move a character to a new position
void move_character_instant(u8,s16,s16); // Move a character to a new position (instantly)
void update_sprites_depth(void); // Update characters and enemies depth

#endif