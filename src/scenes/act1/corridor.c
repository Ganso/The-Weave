// act1/corridor.c — hooks del pasillo (lógica; la secuencia está en corridor.scene)

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "narrative/narrative.h"
#include "res_all.h"
#include "scenes/act1/corridor.h"

void act1_corridor_items(void)    // Bucle de libros/puertas/mapas + condición de salida + salida
{
    bool item_interacted[2] = {false, false}; // Books read

    while (true) {
        switch (last_interacted_item)
        {
        case 0: // Guild history book
            if (item_interacted[0] == false) talk_cluster(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_BOOK_HISTORY], false);
            else talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_BOOK_AGAIN], false); // relectura (guión 2.2)
            item_interacted[0] = true;
            last_interacted_item = ITEM_NONE;
            break;
        case 1: // Myths and legends
            if (item_interacted[1] == false) talk_cluster(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_MYTH_COLLECTION], false);
            else talk_dialog(&dialogs[ACT1_CORRIDOR][A1_CORRIDOR_MYTH_AGAIN], false); // relectura (guión 2.2)
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
