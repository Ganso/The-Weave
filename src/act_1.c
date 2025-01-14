#include <genesis.h>
#include "globals.h"

void act_1_scene_1(void)
{
    u16 paltmp[64];

    // Initialize level
    new_level(&bedroom_bg_tile, &bedroom_bg_map, &bedroom_front_tile, &bedroom_front_map, bedroom_night_pal, 344, BG_SCRL_AUTO_RIGHT, 3);
    set_limits(20,145,278,176);

    // Use swan palette instead of characters palette
    PAL_setPalette(PAL1, swan_sprite.palette->data, DMA);

    // Initialize characters and dialog faces
    init_character(CHR_swan);
    init_character(CHR_linus);

    // Initialize items
    init_item(0, &item_bedroom_bed, PAL0, 31, 139, 93, 0, 23, 0, true); // Bed
    init_item(1, &item_bedroom_chair, PAL0, 236, 110, 26, 0, 43, 0, true); // Chair
    init_item(2, &item_bedroom_windowsill, PAL0, 125, 121, 99, 0, 22, 0, true); // Windowsill
    init_item(3, &item_bedroom_cabinet, PAL0, 270, 79, 51, 0, 82, 0, true); // Cabinet and sheet music
    init_item(4, &item_bedroom_linus_sleeping, PAL0, 30, 112, 0, 0, 0, 0, true); // Linus sleeping

    // Flash to white, show swan, fade back
    wait_seconds(2);
    PAL_getColors(0, paltmp, 64); // backup current palete
    play_sample(snd_effect_magic_appear, sizeof(snd_effect_magic_appear)); // something magically appearing sound effect
    PAL_fadeToAll(geesebumps_pal_white.data, SCREEN_FPS, false); // fade to white
    move_character_instant(CHR_swan,141,110);
    show_character(CHR_swan, true); // show swan
    PAL_fadeToAll(paltmp, SCREEN_FPS, false); // fade to night palete
    wait_seconds(2);

    // Dialog
    talk_dialog(&dialogs[ACT1_DIALOG4][0]); // (ES) "Han pasado ya cien años" - (EN) "A hundred years have passed"
    talk_dialog(&dialogs[ACT1_DIALOG4][1]); // (ES) "No podremos pararles por mucho más tiempo" - (EN) "We won't be able to stop them for much longer"

    // Flash to white, hide swan, fade back
    wait_seconds(2);
    PAL_getColors(0, paltmp, 64); // backup current palete
    play_sample(snd_effect_magic_disappear, sizeof(snd_effect_magic_disappear)); // something magically disappearing sound effect
    PAL_fadeToAll(geesebumps_pal_white.data, SCREEN_FPS, false); // fade to white
    show_character(CHR_swan, false); // hide swan
    PAL_fadeToAll(paltmp, SCREEN_FPS, false); // fade to palete
    wait_seconds(2);
    
    // Daytime
    PAL_fadeTo(0, 15, bedroom_pal.data, SCREEN_FPS, false);

    // Dialog
    talk_dialog(&dialogs[ACT1_DIALOG4][2]); // (ES) "A la mañana siguiente..." - (EN) "The next morning..."

    // Wake linus up
    release_item(4);
    PAL_setPalette(PAL1, linus_sprite.palette->data, DMA);
    move_character_instant(CHR_linus, 35, 175);
    show_character(CHR_linus, true);
    
    // You can move
    player_scroll_active=true;
    movement_active=true;

    bool item_interacted[4]={false,false,false,false};
    bool scene_timeout=0;
    while (scene_timeout<(SCREEN_FPS*3)) // Scene ends 3 seconds after interacting every object
    {
        switch (pending_item_interaction) // Process item interactions
        {
        case 0: // Bed
            talk_dialog(&dialogs[ACT1_DIALOG4][3]); // (ES) "He dormido regular esta noche Tuve horribles pesadillas" - (EN) "I've not slept well last night I had terrible nightmares"
            pending_item_interaction=ITEM_NONE;
            item_interacted[0]=true;
            break;
        case 1: // Chair
            talk_dialog(&dialogs[ACT1_DIALOG4][5]); // (ES) "No tengo tiempo de sentarme Madre me espera" - (EN) "I don't have time to sit down Mother wating for me"
            pending_item_interaction=ITEM_NONE;
            item_interacted[1]=true;
            break;
        case 2: // Windowsill
            talk_dialog(&dialogs[ACT1_DIALOG4][4]); // (ES) "No ha podido ser real La ventana está cerrada" - (EN) "It couldn't be real The windows is closed"
            pending_item_interaction=ITEM_NONE;
            item_interacted[2]=true;
            break;
        case 3: // Cabinet
            talk_dialog(&dialogs[ACT1_DIALOG4][6]); // (ES) "Esta es la nana que me cantaban cada noche" - (EN) "That's the lullaby they used to sing to me every night"
            pending_item_interaction=ITEM_NONE;
            if (obj_pattern[PTRN_SLEEP].active==false) {
                activate_spell(PTRN_SLEEP);
                talk_dialog(&dialogs[ACT1_DIALOG4][7]); // (ES) "Has aprendido tu primer patrón" - (EN) "You have learned your first pattern"
                talk_dialog(&dialogs[ACT1_DIALOG4][8]); // (ES) "Entra en el menú de pausa para verlo" - (EN) "Enter the pause menu to check it out"
            }
            item_interacted[3]=true;
            break;
        default:
            break;
        }
        if (item_interacted[0]==true && item_interacted[1]==true && item_interacted[2]==true && item_interacted[3]==true) scene_timeout++;
        next_frame(true);
    }
    talk_dialog(&dialogs[ACT1_DIALOG4][9]); // (ES) "Linus, ¿dónde estás?" - (EN) "Linus, where are you?"
    talk_dialog(&dialogs[ACT1_DIALOG4][10]); // (ES) "Se me ha hecho demasiado tarde tengo que ir al salón" - (EN) "It's gotten too late I need to go the hall"

    end_level(); // Free resources
    current_scene=2; // Next scene
}

void act_1_scene_2(void)
{
    // Initialize level
    new_level(NULL, NULL, &historians_corridor_front_tile, &historians_corridor_front_map, historians_corridor_pal, 576, BG_SCRL_USER_LEFT, 0);
    set_limits(0,131,275,170);

    // Initialize items
    init_item(0, &item_corridor_bookpedestal_sprite, PAL0, 400, 90, COLLISION_DEFAULT, COLLISION_DEFAULT, 8, 58, false); // Guild history book
    init_item(1, &item_corridor_bookpedestal_sprite, PAL0, 200, 90, COLLISION_DEFAULT, COLLISION_DEFAULT, 8, 58, false); // Myths and legends

    // Initialize characters and dialog faces
    init_character(CHR_linus);

    // Tech demo warning message
    talk_dialog(&dialogs[SYSTEM_DIALOG][1]); // (ES) "The Weave Demo técnica Enero de 2025" - (EN) "The Weave Tech demo January 2025"
    talk_dialog(&dialogs[SYSTEM_DIALOG][2]); // (ES) "Los gráficos, mecánicas o sonidos no son definitivos, ni representan el resultado final" - (EN) "Graphics, mechanics or sounds aren't final, nor they represent the final result"

    // Put character in screen
    move_character_instant(CHR_linus, 340, 154);
    move_character(CHR_linus, 270, 154);

    // Dialog
    talk_dialog(&dialogs[ACT1_DIALOG1][0]); // (ES) "Creo que he dormido demasiado Debo llegar rápido al salón" - (EN) "I think I've overslept I should go to the hall quickly"
    talk_dialog(&dialogs[ACT1_DIALOG1][1]); // (ES) "Aunque siendo el día que es este pasillo me trae demasiados recuerdos" - (EN) "Although in a day like this this hallway brings back too many memories"

    // You can move
    player_scroll_active=true;
    movement_active=true;

    bool item_interacted[2]={false, false};
    while (true) {
        switch (pending_item_interaction) // Process item interactions
        {
        case 0: // Guild history book
            talk_dialog(&dialogs[ACT1_DIALOG1][3]); // (ES) "Este tomo narra la historia de nuestro gremio desde la Gran Separación" - (EN) "This volume narrates the history of our guild since the Great Split"
            talk_dialog(&dialogs[ACT1_DIALOG1][4]); // (ES) "El último capítulo termina con el fallecimiento de mi padre" - (EN) "The last chapter ends with the passing of my father"
            talk_dialog(&dialogs[ACT1_DIALOG1][5]); // (ES) "Madre dice que seré yo el que deba escribir el siguiente" - (EN) "Mother says it will be me who has to write the next one"
            item_interacted[0]=true;
            pending_item_interaction=ITEM_NONE;
            break;
        case 1: // Myths and legends
            talk_dialog(&dialogs[ACT1_DIALOG1][6]); // (ES) "Una colección de mitos y leyendas de los distintos gremios" - (EN) "A collection of myths and legends from the different guilds"
            talk_dialog(&dialogs[ACT1_DIALOG1][7]); // (ES) "Gracias a mi padre tenemos documentadas las que cantaban los Pastores" - (EN) "Thanks to my father we have documented those the Shepherds sang"
            item_interacted[1]=true;
            pending_item_interaction=ITEM_NONE;
            break;
        default:
            break;
        }

        if (offset_BGA<=1 && obj_character[active_character].x<=1) { // Players try to exit screen
            if (item_interacted[0]==false || item_interacted[1]==false) { // We han't read every book
                talk_dialog(&dialogs[ACT1_DIALOG1][2]); // (ES) "Antes de irme quiero repasar algunos recuerdos Se lo debo a papá" - (EN) "Before I leave I want to revisit some memories I owe it to dad"
                move_character(active_character,20,obj_character[active_character].y+obj_character[active_character].y_size); // Go backwards
            }
            else break; // We have read it --> exit
        }

        next_frame(true);
    }

    move_character(active_character,-30,obj_character[active_character].y+obj_character[active_character].y_size);

    end_level(); // Free resources
    current_scene=3; // Next scene
}

void act_1_scene_3(void)
{
    u16 ndialog;
    const DialogItem *current_dialog;

    // Initialize level
    new_level(&historians_bg_tile, &historians_bg_map, &historians_front_tile, &historians_front_map, historians_pal, SCREEN_WIDTH, BG_SCRL_AUTO_RIGHT, 3);
    
    // Initialize characters and dialog faces
    init_character(CHR_linus);
    init_character(CHR_clio);
    init_character(CHR_xander);
    
    // Starting positions
    move_character_instant(CHR_clio,40,174);
    move_character_instant(CHR_linus,360,164);
    look_left(CHR_clio,false);
    look_left(CHR_linus,true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    
    // Dialog
    ndialog=0;
    while ((current_dialog = &dialogs[ACT1_DIALOG2][ndialog])->text[game_language] != NULL) {
        switch(ndialog) {
            case 3:
                // After Clio's first line
                move_character(CHR_linus, 200, 174);
                break;
            case 6:
                // Xander's entrance
                move_character(CHR_clio, 100, 154);
                wait_seconds(1);
                look_left(CHR_clio, true);
                move_character_instant(CHR_xander,-30,174);
                show_character(CHR_xander, true);
                move_character(CHR_xander, 40, 174);
                break;
            case 20:
                // Before Clio's last line
                look_left(CHR_clio, false);
                break;
        }
        talk_dialog(current_dialog);
        ndialog++;
    }

    wait_seconds(2);

    end_level(); // Free resources
    current_scene=5; // Next scene
}

void act_1_scene_5(void)
{
    // Initialize level
    new_level(&weavers_bg_tile, &weavers_bg_map, &weavers_front_tile, &weavers_front_map, weavers_pal, 1000, BG_SCRL_USER_RIGHT, 2);
    set_limits(0,134,275,172);

    // Initialize characters
    init_character(CHR_linus);
    init_character(CHR_clio);
    active_character=CHR_linus;
    move_character_instant(CHR_linus, -30, 154);
    move_character_instant(CHR_clio, -30, 154);
    follow_active_character(CHR_clio, true, 2);

    // Initialize spells
    obj_pattern[PTRN_ELECTRIC].active=true;
    obj_pattern[PTRN_HIDE].active=true;
    obj_pattern[PTRN_OPEN].active=true;
    obj_pattern[PTRN_SLEEP].active=true;

    // Dialog
    move_character(CHR_linus, 30, 154);
    talk_dialog(&dialogs[ACT1_DIALOG3][0]); // (ES) "Algún tiempo después" - (EN) "Some time later"
    talk_dialog(&dialogs[ACT1_DIALOG3][1]); // (ES) "Se aproximan enemigos Tenemos que estar atentos Quédate cerca, madre" - (EN) "Enemies are approaching We have to stay alert Stay close, mother"
    talk_dialog(&dialogs[ACT1_DIALOG3][11]); // (ES) "NOTA: Ni esta escena ni estos gráficos estarán en el juego cuando esté terminado" - (EN) "NOTE: Neither that scene nor those graphics will be present in the game when it's finished"
    talk_dialog(&dialogs[ACT1_DIALOG3][12]); // (ES) "Se ha decidido incluirla en esta demo técnica como prueba de ciertas mecánicas" - (EN) "It's been decided to include it in this technical demo as a test of certain mechanics"
    talk_dialog(&dialogs[ACT1_DIALOG3][13]); // (ES) "Pulsa START para ver tu inventario de hechizos" - (EN) "Press START to view your spell inventory"

    // Show the interface and allow character to move and play patterns
    player_scroll_active=true;
    movement_active=true;
    interface_active=true;
    player_patterns_enabled=true;
    show_or_hide_interface(true);

    while (offset_BGA<180) {
        next_frame(true);
    }

    // COMBAT SCENE
    show_or_hide_interface(false);

    // Initialize enemies
    PAL_setPalette(PAL3, three_head_monkey_sprite.palette->data, DMA); // Enemy palette

    init_enemy(0,ENEMY_CLS_3HEADMONKEY);
    move_enemy_instant(0, -20, 156);
    move_character(CHR_linus, 200, 144);
    move_enemy(0, 20, 156);
    show_or_hide_interface(true);
    start_combat(true);
    while (is_combat_active==true) {
        next_frame(true);
    }

    init_enemy(0,ENEMY_CLS_BADBOBBIN);
    init_enemy(1,ENEMY_CLS_BADBOBBIN);
    move_enemy_instant(0, 350, 176);
    move_enemy_instant(1, -20, 156);
    move_enemy(0, 250, 136);
    move_enemy(1, 20, 156);
    start_combat(true);
    while (is_combat_active==true) {
        next_frame(true);
    }

    show_or_hide_interface(false);
    talk_dialog(&dialogs[ACT1_DIALOG3][10]); // (ES) "¡Esto es todo! (por ahora)" - (EN) "That's all! (by now)"
    talk_dialog(&dialogs[ACT1_DIALOG3][5]); // (ES) "Gracias por probar la demo técnica Sígueme por X para estar al día @GeeseBumpsGames" - (EN) "Thtanks for testing our tech demo Follow me in X for updates @GeeseBumpsGames"
    talk_dialog(&dialogs[ACT1_DIALOG3][7]); // (ES) "Apaga tu consola y haz algo constructivo como jugar un poco al frontón" - (EN) "Turn off your console and do something constructive like play a little racquetball"
    talk_dialog(&dialogs[ACT1_DIALOG3][8]); // (ES) "o preparar la cena, o organizar tu cajón de calcetines alfabéticamente." - (EN) "or cook dinner, or organize your sock drawer alphabetically."
    PAL_fadeOutAll(120,false);

    SYS_hardReset(); // Reset and start again
}
