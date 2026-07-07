// main.c — bucle principal: intro + bucle de escenas por nombre (scene_vm)
#include <genesis.h>
#include "core/config.h"
#include "core/hack.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/geesebumps.h"
#include "scenes/intro.h"
#include "scenes/scene_vm.h"
#include "scenes/scene_data.h"
#include "narrative/texts.h"


#ifndef HACK_SMOKE_BUILD // en la smoke ROM el main vive en src/smoke/smoke_main.c

int main(bool hard)    // Main game loop - handles initialization and scene transitions
{
    dprintf(1,"The Weave - %s",GAMEVERSION);
    dprintf(1,"https://geese-bumps.itch.io/\n\n");

    if (HACK_START_SCENE[0] == 0) { // Dev toggle (core/hack.h): "" = flujo normal
        geesebumps_logo(); // GeeseBumps Logo
        theweave_intro();  // Game intro
    }
#if HACK_FORCE_LANGUAGE == 1
    game_language = LANG_SPANISH;  // Dev hack (core/hack.h)
#elif HACK_FORCE_LANGUAGE == 2
    game_language = LANG_ENGLISH;  // Dev hack (core/hack.h)
#endif

    initialize(true);

    current_scene_id = SCENE_ACT1_BEDROOM; // primera escena del juego
    if (HACK_START_SCENE[0]) {             // Dev toggle: arrancar en otra escena, por nombre
        s16 id = scene_id_by_name(HACK_START_SCENE);
        if (id >= 0) current_scene_id = id;
        else dprintf(1,"HACK_START_SCENE: escena '%s' no existe", HACK_START_SCENE);
    }

    dprintf(2,"Starting at scene %s\n", scenes[current_scene_id].name);
    while (true) { // MAIN LOOP: cada escena deja current_scene_id apuntando a la siguiente
        scene_run(&scenes[current_scene_id]);
    }
}

#endif // HACK_SMOKE_BUILD
