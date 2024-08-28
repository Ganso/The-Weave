#include <genesis.h>
#include "globals.h"

void init_item(u16 nitem)
{
    u8 npal = PAL1;
    u8 x_size = 48;
    u8 y_size = 64;
    u8 collision_x_offset=4;
    u8 collision_y_offset=63;
    u8 collision_width=36;
    u8 collision_height=2;
    const SpriteDefinition *nsprite = NULL;

    if (obj_character[nitem].sd == NULL) {
        switch (nitem)
        {
        case CHR_linus:
            nsprite = &linus_sprite;        
            break;
        case CHR_clio:
            nsprite = &clio_sprite;
            break;
        case CHR_xander:
            nsprite = &xander_sprite;
            break;
        default:
            return; 
        }
        obj_item[nitem].entity = (Entity) { true, nsprite, 0, 0, x_size, y_size, npal, false, false, ANIM_IDLE, false, collision_x_offset, collision_y_offset, collision_width, collision_height };
    } else {
        nsprite = obj_item[nitem].entity.sd;
        npal = obj_item[nitem].entity.palette;
        obj_item[nitem].entity.active=true;
    }

    spr_item[nitem] = SPR_addSpriteSafe(nsprite, obj_character[nitem].x, obj_character[nitem].y, 
                                       TILE_ATTR(npal, obj_character[nitem].priority, false, obj_character[nitem].flipH));
    
    if (spr_item[nitem] != NULL) {
        SPR_setVisibility(spr_item[nitem], HIDDEN);
    }
}

void release_item(u16 nitem) {
    obj_item[nitem].entity.active = false;
    if (spr_item[nitem] != NULL)
    {
        SPR_releaseSprite(spr_item[nitem]);
        spr_item[nitem] = NULL;
    }
}