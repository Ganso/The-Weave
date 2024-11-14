#include <genesis.h>
#include "globals.h"


int main(bool hard)
{
    geesebumps_logo(); // GeeseBumps Logo
    theweave_intro();  // Game intro

    current_act=1;
    current_scene=1;

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
