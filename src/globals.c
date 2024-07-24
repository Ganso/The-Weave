#include <genesis.h>
#include "globals.h"

// Wait for N seconds
void wait_seconds(int sec)
{
    u16 num_ticks=0;
    u16 max_ticks=sec*60; // Ticks (at 60fps)
    while (num_ticks<max_ticks)
    {
        next_frame();
        num_ticks++;
    }

}

// Initialize a character
void initialize_character(u8 nchar)
{
    u8 npal = PAL1;
    const SpriteDefinition *nsprite = NULL;

    if (obj_character[nchar].sd == NULL) {
        switch (nchar)
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
        case CHR_badbobbin:
            nsprite = &badbobbin_sprite;
            npal = PAL3;
            break;
        default:
            return; 
        }
        // * Sprite definition, x, y, palette, priority, flipH, animation, visible
        obj_character[nchar] = (Entity) { nsprite, 0, 0, npal, false, false, ANIM_IDLE, false };
    } else {
        nsprite = obj_character[nchar].sd;
        npal = obj_character[nchar].palette;
    }

    spr_chr[nchar] = SPR_addSpriteSafe(nsprite, obj_character[nchar].x, obj_character[nchar].y, 
                                       TILE_ATTR(npal, obj_character[nchar].priority, false, obj_character[nchar].flipH));
    
    if (spr_chr[nchar] != NULL) {
        SPR_setVisibility(spr_chr[nchar], HIDDEN);
    }
}

// Release a character from memory (Just the sprite, keep the Entity)
void release_character(u8 nchar)
{
    if (spr_chr[nchar] != NULL)
    {
        SPR_releaseSprite(spr_chr[nchar]);
        spr_chr[nchar] = NULL;
    }
}

// Initialize a face
void initialize_face(u8 nface)
{
    u8 npal = PAL1;
    const SpriteDefinition *nsprite = NULL;

    if (obj_face[nface].sd == NULL) { // Object never initialized
        switch (nface)
        {
        case CHR_linus:
            nsprite = &linus_face_sprite;        
            break;
        case CHR_clio:
            nsprite = &clio_face_sprite;
            break;
        case CHR_xander:
            nsprite = &xander_face_sprite;
            break;
        default:
            return; // Salir si la cara no es válida
        }
        obj_face[nface] = (Entity) { nsprite, 0, 160, npal, false, false, ANIM_IDLE, false };
    } else {
        nsprite = obj_face[nface].sd;
    }

    spr_face[nface] = SPR_addSpriteSafe(nsprite, obj_face[nface].x, obj_face[nface].y, 
                                        TILE_ATTR(obj_face[nface].palette, obj_face[nface].priority, false, obj_face[nface].flipH));
    
    if (spr_face[nface] != NULL) {
        SPR_setVisibility(spr_face[nface], HIDDEN);
        SPR_setDepth(spr_face[nface], SPR_MIN_DEPTH); // Faces are above any other sprite
    }
}

// Release a face from memory (Just the sprite, keep the Entity)
void release_face(u8 nface)
{
    if (spr_face[nface] != NULL)
    {
        SPR_releaseSprite(spr_face[nface]);
        spr_face[nface] = NULL;
    }
}

// Wait for next frame and do each-frame actions
void next_frame(void)
{
    if (note_playing_time!=0) { // A note is being played
        if (note_playing_time==MAX_NOTE_PLAYING_TIME) { // Finished
            show_note(note_playing, false);
            note_playing=NOTE_NONE;
            note_playing_time=0;
        }
        else note_playing_time++; // Keep playing
    }
    update_bg();
    SPR_update();
    SYS_doVBlankProcess();
}
