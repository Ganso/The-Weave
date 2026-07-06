// act1/corridor.c — hooks del pasillo (lógica; la secuencia está en corridor.scene)

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/act1/corridor.h"
#include "narrative/texts.h"
#include "narrative/texts_data.h"
#include "narrative/dialogs.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/items.h"
#include "world/background.h"
#include "res_backgrounds.h"
#include "res_items.h"

void act1_corridor_setup(void)    // Nivel del pasillo + libros/lámparas/puertas/mapas
{
    new_level(NULL, NULL, &historians_corridor_front_tile, &historians_corridor_front_map, historians_corridor_pal, 800, BG_SCRL_USER_LEFT, 0);
    set_limits(0,140,275,168);

    init_item(0, &item_corridor_bookpedestal_sprite, PAL0, 600, 95, COLLISION_DEFAULT, COLLISION_DEFAULT, 8, 58, CALCULATE_DEPTH); // Guild history book
    init_item(1, &item_corridor_bookpedestal_sprite, PAL0, 200, 95, COLLISION_DEFAULT, COLLISION_DEFAULT, 8, 58, CALCULATE_DEPTH); // Myths and legends
    init_item(2, &item_corridor_lamp_sprite, PAL0, 32, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(3, &item_corridor_lamp_sprite, PAL0, 176, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(4, &item_corridor_lamp_sprite, PAL0, 336, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(5, &item_corridor_lamp_sprite, PAL0, 464, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(6, &item_corridor_lamp_sprite, PAL0, 656, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(7, &item_corridor_door_bottom_sprite, PAL0, 96, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(8, &item_corridor_door_bottom_sprite, PAL0, 400, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(9, &item_corridor_door_bottom_sprite, PAL0, 720, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(10, &item_corridor_map_bottom_sprite, PAL0, 240, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(11, &item_corridor_map_bottom_sprite, PAL0, 544, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);

    init_character(CHR_linus);
}

void act1_corridor_entry(void)    // Entrada de Linus por la derecha
{
    move_character_instant(CHR_linus, 340, 154);
    move_character(CHR_linus, 270, 154);
}

void act1_corridor_items(void)    // Bucle de libros/puertas/mapas + condición de salida + salida
{
    bool item_interacted[2] = {false, false}; // Books read

    while (true) {
        switch (last_interacted_item)
        {
        case 0: // Guild history book
            if (item_interacted[0] == false) talk_cluster(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_BOOK_HISTORY], false);
            else talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_BOOK_HISTORY], false);
            item_interacted[0] = true;
            last_interacted_item = ITEM_NONE;
            break;
        case 1: // Myths and legends
            if (item_interacted[1] == false) talk_cluster(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_MYTH_COLLECTION], false);
            else talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_MYTH_COLLECTION], false);
            item_interacted[1] = true;
            last_interacted_item = ITEM_NONE;
            break;
        case 7: // Left door
            talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_DOOR1_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 8: // Middle door
            talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_DOOR2_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 9: // Right door
            talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_DOOR3_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 10: // Map
        case 11: // Map
            talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_MAP_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        default:
            break;
        }

        // El jugador intenta salir por la izquierda
        if (FASTFIX32_TO_INT(offset_BGA) <= 1 && FASTFIX32_TO_INT(obj_character[active_character].x) <= 1) {
            if (item_interacted[0] == false || item_interacted[1] == false) { // Sin leer ambos libros
                talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_REVISIT_MEMORIES], false);
                move_character(active_character,
                    20,
                    FASTFIX32_TO_INT(obj_character[active_character].y) +
                    obj_character[active_character].y_size); // Go backwards
            }
            else break; // Leídos --> salir
        }

        next_frame(true);
    }

    // Salida por la izquierda
    move_character(active_character,
        -30,
        FASTFIX32_TO_INT(obj_character[active_character].y) +
        obj_character[active_character].y_size);
}
