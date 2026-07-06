#include <genesis.h>
#include "core/config.h"
#include "core/hack.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/geesebumps.h"
#include "scenes/intro.h"
#include "act_1.h"
#include "narrative/texts.h"


int main(bool hard)    // Main game loop - handles initialization and scene transitions
{
    dprintf(1,"The Weave - %s",GAMEVERSION);
    dprintf(1,"https://geese-bumps.itch.io/\n\n");

#if !HACK_START_SCENE
    geesebumps_logo(); // GeeseBumps Logo
    theweave_intro();  // Game intro
#endif
#if HACK_FORCE_LANGUAGE == 1
    game_language = LANG_SPANISH;  // Dev hack (core/hack.h)
#elif HACK_FORCE_LANGUAGE == 2
    game_language = LANG_ENGLISH;  // Dev hack (core/hack.h)
#endif

    initialize(true);

    current_act=1;
#if HACK_START_SCENE
    current_scene=HACK_START_SCENE; // Dev toggle (core/hack.h): saltar directamente a una escena
#else
    current_scene=1;
#endif

    dprintf(2,"Loading Act %d, Scene %d\n", current_act, current_scene);
    while (true) { // MAIN LOOP
        switch (current_act)
        {
        case 1: // FIRST ACT
            switch (current_scene)
            {
            case 1:
                act_1_scene_1(); // ACT 1 - scene 1
                break;
            case 2:
                act_1_scene_2(); // ACT 1 - scene 2
                break;
            case 3:
                act_1_scene_3(); // ACT 1 - scene 3
                break;
            case 5:
                act_1_scene_5(); // ACT 1 - scene 5
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}
