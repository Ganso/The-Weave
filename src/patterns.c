#include <genesis.h>
#include "globals.h"

// Show or hide notes
void show_note(u8 nnote, bool visible)
{
    Sprite *pentsprite;
    Sprite *rodsprite;
    u8 *notesong;

    switch (nnote) 
    {
    case NOTE_MI:
        pentsprite=spr_int_pentagram_1;
        rodsprite=spr_int_rod_1;
        notesong=(u8*)snd_note_mi;
        break;
    case NOTE_FA:
        pentsprite=spr_int_pentagram_2;
        rodsprite=spr_int_rod_2;
        notesong=(u8*)snd_note_fa;
        break;
    case NOTE_SOL:
        pentsprite=spr_int_pentagram_3;
        rodsprite=spr_int_rod_3;
        notesong=(u8*)snd_note_sol;
        break;
    case NOTE_LA:
        pentsprite=spr_int_pentagram_4;
        rodsprite=spr_int_rod_4;
        notesong=(u8*)snd_note_la;
        break;
    case NOTE_SI:
        pentsprite=spr_int_pentagram_5;
        rodsprite=spr_int_rod_5;
        notesong=(u8*)snd_note_si;
        break;
    default:
        pentsprite=spr_int_pentagram_6;
        rodsprite=spr_int_rod_6;
        notesong=(u8*)snd_note_do;
        break;
    }

    if (visible == true) {
        SPR_setVisibility(pentsprite, VISIBLE);
        SPR_setVisibility(rodsprite, VISIBLE);
        XGM_setLoopNumber(0);
        XGM_startPlay(notesong);
    }
    else {
        SPR_setVisibility(pentsprite, HIDDEN);
        //SPR_setVisibility(rodsprite, HIDDEN);
    }
}

// Play a note
void play_note(u8 nnote)
{
    if (note_playing==NOTE_NONE) {
        if (note_playing_time==0) { // Note starting
            show_note(nnote, true);
            note_playing=nnote;
            note_playing_time++;
            played_notes[num_played_notes]=nnote;
            num_played_notes++;
        }
    }
}

// Check if a note is being played
void check_note(void)
{
    if (note_playing_time!=0) { // A note is being played
        if (note_playing_time==MAX_NOTE_PLAYING_TIME) { // Finished
            show_note(note_playing, false); // Hide the note
            note_playing=NOTE_NONE;
            time_since_last_note=1; // A pattern is possible. Start counting ticks to cancel it
            note_playing_time=0;
            if (num_played_notes==4) {
                check_pattern(); // Pattern finished
            }
        }
        else note_playing_time++; // Keep playing
    }
    else if (time_since_last_note!=0) {
        time_since_last_note++;
        if (time_since_last_note==MAX_PATTERN_WAIT_TIME) { // Done waiting. The pattern is cancelled
            hide_rod_icons();
            time_since_last_note=0;
            num_played_notes=0;
        }
    }
}

// Check the finished pattern
void check_pattern(void)
{
    u8 npattern,nnote; // Loop indexes
    u8 matches,matched_pattern;

    matched_pattern=254;
    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        matches=0;
        for (nnote=0;nnote<4;nnote++) {
            if (played_notes[nnote]==obj_pattern[npattern].notes[nnote]) {
                matches++;
            }
        }
        if (matches==4) matched_pattern=npattern;
    }
    hide_rod_icons(); // Hide the rod pattern icons

    // Pattern effects
    if (matched_pattern!=254) { // We have a match!
        SPR_setVisibility(spr_int_rod, HIDDEN); // Hide the rod itself
        show_pattern_icon(matched_pattern, true); // Show appropiate icon
        if (matched_pattern==PTRN_ELECTIC) { // THUNDER !!!
            u8 i;
            play_pattern_sound(PTRN_ELECTIC);
            for (i=0;i<100;i++) {
                VDP_setHilightShadow(true);
                SYS_doVBlankProcess();
                VDP_setHilightShadow(false);
                SYS_doVBlankProcess();
            }
        }
        SPR_setVisibility(spr_int_rod, VISIBLE); // Show the rod again
        show_pattern_icon(matched_pattern, false); // Hide the icon
    }

    num_played_notes=0;
    time_since_last_note=0;
    next_frame();
}


// Hide icons in the rod
void hide_rod_icons(void)
{
    SPR_setVisibility(spr_int_rod_1,HIDDEN);
    SPR_setVisibility(spr_int_rod_2,HIDDEN);
    SPR_setVisibility(spr_int_rod_3,HIDDEN);
    SPR_setVisibility(spr_int_rod_4,HIDDEN);
    SPR_setVisibility(spr_int_rod_5,HIDDEN);
    SPR_setVisibility(spr_int_rod_6,HIDDEN);
    SPR_update();
}

// Play the sound of a pattern spell
void play_pattern_sound(u16 npattern)
{
    // Only thunder by now
    XGM2_playPCM(snd_pattern_thunder,sizeof(snd_pattern_thunder),SOUND_PCM_CH_AUTO);
}

// Show the icon of a pattern spell
void show_pattern_icon(u16 npattern, bool show)
{
    u8 npal = PAL2;
    const SpriteDefinition *nsprite = NULL;

    if (show==TRUE) {
        nsprite = &int_pattern_thunder; // Ya daremos soporte a los demás
        KDebug_Alert("MOSTRANDO ICONO");
        obj_pattern[npattern].sd = SPR_addSpriteSafe(nsprite, 96, 182, TILE_ATTR(npal, true, false, false)); // Priority TRUE
        SPR_setAlwaysOnTop(obj_pattern[npattern].sd);
    }
    else {
        SPR_releaseSprite(obj_pattern[npattern].sd);
        obj_pattern[npattern].sd=NULL;
    }
    next_frame();
}