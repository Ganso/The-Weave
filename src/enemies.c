#include <genesis.h>
#include "globals.h"

// Initialize enemy pattern
void init_enemy_patterns(u8 num_enemy_pattern)
{
    switch (num_enemy_pattern)
    {
    case PTRN_EN_ELECTIC:
        obj_Pattern_Enemy[num_enemy_pattern]=(Pattern_Enemy) {4, {1,2,3,4}};
        break;
    case PTRN_EN_BITE:
        obj_Pattern_Enemy[num_enemy_pattern]=(Pattern_Enemy) {2, {2,3,NULL,NULL}};
        break;
    default:
        break;
    }
}