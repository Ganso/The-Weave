#include <genesis.h>
#include "globals.h"

u8 num_colls;    // Counter to prevent infinite collision loops

u16 char_distance(u16 char1, s16 x1, u8 y1, u16 char2)    // Calculate Manhattan distance between character collision boxes
{
    // Calculate char1's collision box boundaries
    s16 char1_left = x1 + obj_character[char1].collision_x_offset;
    s16 char1_right = char1_left + obj_character[char1].collision_width;
    s16 char1_top = y1 + obj_character[char1].collision_y_offset;
    s16 char1_bottom = char1_top + obj_character[char1].collision_height;

    // Calculate char2's collision box boundaries
    s16 char2_left = obj_character[char2].x + obj_character[char2].collision_x_offset;
    s16 char2_right = char2_left + obj_character[char2].collision_width;
    s16 char2_top = obj_character[char2].y + obj_character[char2].collision_y_offset;
    s16 char2_bottom = char2_top + obj_character[char2].collision_height;

    // Find closest x point on char2's box to char1's box
    s16 closest_x;
    if (char1_right < char2_left) closest_x = char2_left;
    else if (char1_left > char2_right) closest_x = char2_right;
    else closest_x = (char1_left + char1_right) / 2; // If boxes overlap in x, use midpoint

    // Find closest y point on char2's box to char1's box
    s16 closest_y;
    if (char1_bottom < char2_top) closest_y = char2_top;
    else if (char1_top > char2_bottom) closest_y = char2_bottom;
    else closest_y = (char1_top + char1_bottom) / 2; // If boxes overlap in y, use midpoint

    // If boxes overlap completely, return 0
    if (char1_left < char2_right && char1_right > char2_left &&
        char1_top < char2_bottom && char1_bottom > char2_top) {
        return 0;
    }

    // Calculate Manhattan distance to closest point
    return abs((char1_left + char1_right) / 2 - closest_x) + 
           abs((char1_top + char1_bottom) / 2 - closest_y);
}

u16 item_distance(u16 nitem, u16 x, u8 y)    // Calculate Manhattan distance from point to item collision box
{
    // Calculate item's collision box boundaries
    s16 item_left = obj_item[nitem].entity.x + obj_item[nitem].entity.collision_x_offset;
    s16 item_right = item_left + obj_item[nitem].entity.collision_width;
    s16 item_top = obj_item[nitem].entity.y + obj_item[nitem].entity.collision_y_offset;
    s16 item_bottom = item_top + obj_item[nitem].entity.collision_height;
    
    // Find closest x point on box
    s16 closest_x;
    if (x < item_left) closest_x = item_left;
    else if (x > item_right) closest_x = item_right;
    else closest_x = x;

    // Find closest y point on box
    s16 closest_y;
    if (y < item_top) closest_y = item_top;
    else if (y > item_bottom) closest_y = item_bottom;
    else closest_y = y;

    // If point is inside box, return 0
    if (x >= item_left && x <= item_right && y >= item_top && y <= item_bottom) {
        return 0;
    }

    // Calculate Manhattan distance to closest point
    return abs(x - closest_x) + abs(y - closest_y);
}

u16 detect_char_char_collision(u16 nchar, u16 x, u8 y)    // Check for collisions between character and other characters
{
    u16 other_char;

    if (obj_character[nchar].active == true) {
        // Compute character collision box
        u16 char_col_x1, char_col_x2;
        if (obj_character[nchar].flipH) {
            char_col_x1 = x + obj_character[nchar].x_size - obj_character[nchar].collision_x_offset - obj_character[nchar].collision_width;
            char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
        } else {
            char_col_x1 = x + obj_character[nchar].collision_x_offset;
            char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
        }
        u8 char_col_y1 = y + obj_character[nchar].collision_y_offset;
        u8 char_col_y2 = char_col_y1 + obj_character[nchar].collision_height;

        // Check collision with each active character (except itself)
        for (other_char = 0; other_char < MAX_CHR; other_char++) {
            if (other_char != nchar && obj_character[other_char].active == true) {
                // Compute other character collision box
                u16 other_col_x1, other_col_x2;
                if (obj_character[other_char].flipH) {
                    other_col_x1 = obj_character[other_char].x + obj_character[other_char].x_size - obj_character[other_char].collision_x_offset - obj_character[other_char].collision_width;
                    other_col_x2 = other_col_x1 + obj_character[other_char].collision_width;
                } else {
                    other_col_x1 = obj_character[other_char].x + obj_character[other_char].collision_x_offset;
                    other_col_x2 = other_col_x1 + obj_character[other_char].collision_width;
                }
                u8 other_col_y1 = obj_character[other_char].y + obj_character[other_char].collision_y_offset;
                u8 other_col_y2 = other_col_y1 + obj_character[other_char].collision_height;

                // Check if collision boxes overlap
                if (char_col_x1 < other_col_x2 &&
                    char_col_x2 > other_col_x1 &&
                    char_col_y1 < other_col_y2 &&
                    char_col_y2 > other_col_y1) {
                    if (num_colls < MAX_COLLISIONS) { // Prevent character from being trapped
                        num_colls++;
                        return other_char;
                    } else {
                        num_colls = 0;
                        return CHR_NONE;
                    }
                }
            }
        }
    }
    
    // No collision detected
    return CHR_NONE;
}

u16 detect_char_item_collision(u16 nchar, u16 x, u8 y)    // Check for collisions between character and items
{
    u16 nitem;
    s16 char_left, char_right, char_top, char_bottom;
    s16 item_left, item_right, item_top, item_bottom;

    // Calculate character's bounding box
    char_left = x + obj_character[nchar].collision_x_offset;
    char_right = char_left + obj_character[nchar].collision_width;
    char_top = y + obj_character[nchar].collision_y_offset;
    char_bottom = char_top + obj_character[nchar].collision_height;
    //kprintf("CAJA PERSONAJE: (%d,%d)-(%d,%d)",char_left,char_top,char_right,char_bottom);

    for (nitem = 0; nitem < MAX_ITEMS; nitem++)
    {
        if (obj_item[nitem].entity.active && obj_item[nitem].entity.visible)
        {
            //kprintf("Detectando colisión con %d", nitem);
            // Calculate item's bounding box
            item_left = obj_item[nitem].entity.x + obj_item[nitem].entity.collision_x_offset;
            item_right = item_left + obj_item[nitem].entity.collision_width;
            item_top = obj_item[nitem].entity.y + obj_item[nitem].entity.collision_y_offset;
            item_bottom = item_top + obj_item[nitem].entity.collision_height;
            //kprintf("DATOS OBJETO: (x,y)=(%d,%d), offset(x,y)=(%d,%d), tam(x,y)=(%d,%d)",obj_item[nitem].entity.x,obj_item[nitem].entity.y,obj_item[nitem].entity.collision_x_offset,obj_item[nitem].entity.collision_y_offset,obj_item[nitem].entity.collision_width,obj_item[nitem].entity.collision_height);
            //kprintf("CAJA OBJETO: (%d,%d)-(%d,%d)",item_left,item_top,item_right,item_bottom);

            // Check for collision
            if (char_left < item_right && char_right > item_left &&
                char_top < item_bottom && char_bottom > item_top)
            {
                //kprintf(" *** COLISION ***");
                return nitem; // Return the index of the collided item
            }
        }
    }

    return ITEM_NONE; // No collision detected
}

u16 detect_char_enemy_collision(u16 nchar, u16 x, u8 y)    // Check for collisions between character and enemies
{
    u16 nenemy;

    if (obj_character[nchar].active == true) {
        // Compute character collision box
        u16 char_col_x1, char_col_x2;
        if (obj_character[nchar].flipH) {
            char_col_x1 = x + obj_character[nchar].x_size - obj_character[nchar].collision_x_offset - obj_character[nchar].collision_width;
            char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
        } else {
            char_col_x1 = x + obj_character[nchar].collision_x_offset;
            char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
        }
        u8 char_col_y1 = y + obj_character[nchar].collision_y_offset;
        u8 char_col_y2 = char_col_y1 + obj_character[nchar].collision_height;

        // Check collision with each active enemy
        for (nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active == true) {
                // Compute enemy collision box
                u16 enemy_col_x1, enemy_col_x2;
                if (obj_enemy[nenemy].obj_character.flipH) {
                    enemy_col_x1 = obj_enemy[nenemy].obj_character.x + obj_enemy[nenemy].obj_character.x_size - obj_enemy[nenemy].obj_character.collision_x_offset - obj_enemy[nenemy].obj_character.collision_width;
                    enemy_col_x2 = enemy_col_x1 + obj_enemy[nenemy].obj_character.collision_width;
                } else {
                    enemy_col_x1 = obj_enemy[nenemy].obj_character.x + obj_enemy[nenemy].obj_character.collision_x_offset;
                    enemy_col_x2 = enemy_col_x1 + obj_enemy[nenemy].obj_character.collision_width;
                }
                u8 enemy_col_y1 = obj_enemy[nenemy].obj_character.y + obj_enemy[nenemy].obj_character.collision_y_offset;
                u8 enemy_col_y2 = enemy_col_y1 + obj_enemy[nenemy].obj_character.collision_height;

                // Check if collision boxes overlap
                if (char_col_x1 < enemy_col_x2 &&
                    char_col_x2 > enemy_col_x1 &&
                    char_col_y1 < enemy_col_y2 &&
                    char_col_y2 > enemy_col_y1) {
                    if (num_colls<MAX_COLLISIONS) { // Prevent player from being trapped by limiting consecutive collisions
                        num_colls++;
                        return nenemy;
                    } else {
                        num_colls=0;
                        return ENEMY_NONE;
                    }
                }
            }
        }
    }
    
    // No collision detected
    return ENEMY_NONE;
}

u16 detect_enemy_char_collision(u16 nenemy, u16 x, u8 y)    // Check for collisions between enemy and characters
{
    u16 nchar;

    if (obj_enemy[nenemy].obj_character.active == true) {
        // Compute enemy collision box
        u16 enemy_col_x1, enemy_col_x2;
        if (obj_enemy[nenemy].obj_character.flipH) {
            enemy_col_x1 = x + obj_enemy[nenemy].obj_character.x_size - obj_enemy[nenemy].obj_character.collision_x_offset - obj_enemy[nenemy].obj_character.collision_width;
            enemy_col_x2 = enemy_col_x1 + obj_enemy[nenemy].obj_character.collision_width;
        } else {
            enemy_col_x1 = x + obj_enemy[nenemy].obj_character.collision_x_offset;
            enemy_col_x2 = enemy_col_x1 + obj_enemy[nenemy].obj_character.collision_width;
        }
        u8 enemy_col_y1 = y + obj_enemy[nenemy].obj_character.collision_y_offset;
        u8 enemy_col_y2 = enemy_col_y1 + obj_enemy[nenemy].obj_character.collision_height;

        // Check collision with each active character
        for (nchar = 0; nchar < MAX_CHR; nchar++) {
            if (obj_character[nchar].active == true) {
                // Compute character collision box
                u16 char_col_x1, char_col_x2;
                if (obj_character[nchar].flipH) {
                    char_col_x1 = obj_character[nchar].x + obj_character[nchar].x_size - obj_character[nchar].collision_x_offset - obj_character[nchar].collision_width;
                    char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
                } else {
                    char_col_x1 = obj_character[nchar].x + obj_character[nchar].collision_x_offset;
                    char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
                }
                u8 char_col_y1 = obj_character[nchar].y + obj_character[nchar].collision_y_offset;
                u8 char_col_y2 = char_col_y1 + obj_character[nchar].collision_height;

                // Check if collision boxes overlap
                if (enemy_col_x1 < char_col_x2 &&
                    enemy_col_x2 > char_col_x1 &&
                    enemy_col_y1 < char_col_y2 &&
                    enemy_col_y2 > char_col_y1) {
                    return nchar;
                }
            }
        }
    }
    
    // No collision detected
    return CHR_NONE;
}
