#include "globals.h"


int main(bool hard)    // Main game loop - handles initialization and scene transitions
{
    dprintf(2,"The Weave - %s",GAMEVERSION);
    dprintf(2,"https://geese-bumps.itch.io/\n\n");

    geesebumps_logo(); // GeeseBumps Logo
    theweave_intro();  // Game intro

    initialize(true);

    current_act=1;
    current_scene=2;

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
