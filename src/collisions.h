#ifndef _COLLISIONS_H_
#define _COLLISIONS_H_

#define MAX_INTERACTIVE_DISTANCE 20 // Distance in pixels with items you can interact

#define MAX_COLLISIONS 30 // If we collide with an enemy more than X times, move the character anyway (CHANGE IT!)
u8 num_colls;             // Number of collisions that already happened

// Distances
u16 char_distance(u16 char1, s16 x1, u8 y1, u16 char2); // Calculate distance between two characters at given coordinates
u16 item_distance(u16 nitem, u16 x, u8 y); // Calculate distance between coordinates and an item's collision box center

// Collisions
u16 detect_char_char_collision(u16 nchar, u16 x, u8 y); // Detect collisions between a character and all other characters at given coordinates
u16 detect_char_item_collision(u16 nchar, u16 x, u8 y); // Detect collisions between a character and every item, given some new coordinates
u16 detect_char_enemy_collision(u16 nchar, u16 x, u8 y); // Detect collisons between a character in every enemy, given some new coordinates
u16 detect_enemy_char_collision(u16 nenemy, u16 x, u8 y); // Detect collisions between an enemy and every character, given some new coordinates


#endif