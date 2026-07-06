#ifndef _FRAME_H_
#define _FRAME_H_

// Game loop heartbeat: per-frame update pump, timing helpers and core globals.
// next_frame() is THE way to advance time; blocking helpers (move_entity, dialogs,
// waits) call it internally.

#include <genesis.h>

extern u16 tile_ind;       // Current tile index for VDP tile loading
extern u16 frame_counter;  // Counter incremented each frame for RNG and timing
extern u8 current_act;     // Current game act number
extern u8 current_scene;   // Current scene number within act
extern u8 SCREEN_FPS;      // Screen refresh rate, runtime detected (50 PAL, 60 NTSC)

void next_frame(bool interactive); // Wait for next frame and do each-frame actions, including interactive actions if selected
void wait_seconds(int sec);        // Wait for N seconds
u16 calc_ticks(u16 milliseconds);  // Translate milliseconds to ticks

#endif
