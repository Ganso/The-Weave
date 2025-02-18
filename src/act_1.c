#include <genesis.h>
#include "globals.h"

void act_1_scene_1(void)    // Bedroom scene with swan's visit and pattern learning
{
    u16 paltmp[64];

    // Initialize level
    new_level(&bedroom_bg_tile, &bedroom_bg_map, &bedroom_front_tile, &bedroom_front_map, bedroom_night_pal, 344, BG_SCRL_AUTO_RIGHT, 3);
    set_limits(20,145,278,176);

    // Use swan palette instead of characters palette
    PAL_setPalette(PAL1, swan_pal.data, DMA);

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
    talk_dialog(&dialogs[ACT1_DIALOG4][1]); // (ES) "No podremos pararles|por mucho más tiempo" - (EN) "We won't be able to|stop them for much longer"

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
    PAL_setPalette(PAL1, characters_pal.data, DMA);
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
            talk_dialog(&dialogs[ACT1_DIALOG4][3]); // (ES) "He dormido regular esta noche|Tuve horribles pesadillas" - (EN) "I've not slept well last night|I had terrible nightmares"
            pending_item_interaction=ITEM_NONE;
            item_interacted[0]=true;
            break;
        case 1: // Chair
            talk_dialog(&dialogs[ACT1_DIALOG4][5]); // (ES) "No tengo tiempo de sentarme|Madre me espera" - (EN) "I don't have time to sit down|Mother wating for me"
            pending_item_interaction=ITEM_NONE;
            item_interacted[1]=true;
            break;
        case 2: // Windowsill
            talk_dialog(&dialogs[ACT1_DIALOG4][4]); // (ES) "No ha podido ser real|La ventana está cerrada" - (EN) "It couldn't be real|The windows is closed"
            pending_item_interaction=ITEM_NONE;
            item_interacted[2]=true;
            break;
        case 3: // Cabinet
            talk_dialog(&dialogs[ACT1_DIALOG4][6]); // (ES) "Esta es la nana que me|cantaban cada noche" - (EN) "That's the lullaby they used|to sing to me every night"
            pending_item_interaction=ITEM_NONE;
            if (obj_pattern[PTRN_SLEEP].active==false) {
                activate_spell(PTRN_SLEEP);
                talk_dialog(&dialogs[ACT1_DIALOG4][7]); // (ES) "Has aprendido|tu primer patrón" - (EN) "You have learned|your first pattern"
                talk_dialog(&dialogs[ACT1_DIALOG4][8]); // (ES) "Entra en el menú de|pausa para verlo" - (EN) "Enter the pause menu|to check it out"
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
    talk_dialog(&dialogs[ACT1_DIALOG4][10]); // (ES) "Se me ha hecho demasiado tarde|tengo que ir al salón" - (EN) "It's gotten too late|I need to go the hall"

    end_level(); // Free resources
    current_scene=2; // Next scene
}

void act_1_scene_2(void)    // Corridor scene with history books and memories
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
    talk_dialog(&dialogs[SYSTEM_DIALOG][1]); // (ES) "The Weave|Demo técnica|Enero de 2025" - (EN) "The Weave|Tech demo|January 2025"
    talk_dialog(&dialogs[SYSTEM_DIALOG][2]); // (ES) "Los gráficos, mecánicas o sonidos|no son definitivos, ni|representan el resultado final" - (EN) "Graphics, mechanics or sounds|aren't final, nor they|represent the final result"

    // Put character in screen
    move_character_instant(CHR_linus, 340, 154);
    move_character(CHR_linus, 270, 154);

    // Dialog
    talk_dialog(&dialogs[ACT1_DIALOG1][0]); // (ES) "Creo que he dormido demasiado|Debo llegar rápido al salón" - (EN) "I think I've overslept|I should go to the hall quickly"
    talk_dialog(&dialogs[ACT1_DIALOG1][1]); // (ES) "Aunque siendo el día que es|este pasillo me trae|demasiados recuerdos" - (EN) "Although in a day like this|this hallway brings back|too many memories"

    // You can move
    player_scroll_active=true;
    movement_active=true;

    bool item_interacted[2]={false, false};
    while (true) {
        switch (pending_item_interaction) // Process item interactions
        {
        case 0: // Guild history book
            talk_dialog(&dialogs[ACT1_DIALOG1][3]); // (ES) "Este tomo narra la historia|de nuestro gremio|desde la Gran Separación" - (EN) "This volume narrates the history|of our guild|since the Great Split"
            talk_dialog(&dialogs[ACT1_DIALOG1][4]); // (ES) "El último capítulo|termina con el fallecimiento|de mi padre" - (EN) "The last chapter|ends with the passing|of my father"
            talk_dialog(&dialogs[ACT1_DIALOG1][5]); // (ES) "Madre dice que seré yo|el que deba escribir|el siguiente" - (EN) "Mother says it will be me|who has to write|the next one"
            item_interacted[0]=true;
            pending_item_interaction=ITEM_NONE;
            break;
        case 1: // Myths and legends
            talk_dialog(&dialogs[ACT1_DIALOG1][6]); // (ES) "Una colección de|mitos y leyendas|de los distintos gremios" - (EN) "A collection of|myths and legends|from the different guilds"
            talk_dialog(&dialogs[ACT1_DIALOG1][7]); // (ES) "Gracias a mi padre|tenemos documentadas|las que cantaban los Pastores" - (EN) "Thanks to my father|we have documented|those the Shepherds sang"
            item_interacted[1]=true;
            pending_item_interaction=ITEM_NONE;
            break;
        default:
            break;
        }

        if (offset_BGA<=1 && obj_character[active_character].x<=1) { // Players try to exit screen
            if (item_interacted[0]==false || item_interacted[1]==false) { // We han't read every book
                talk_dialog(&dialogs[ACT1_DIALOG1][2]); // (ES) "Antes de irme quiero|repasar algunos recuerdos|Se lo debo a papá" - (EN) "Before I leave I want to|revisit some memories|I owe it to dad"
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

void act_1_scene_3(void)    // Hall scene with Clio and Xander discussing Weavers
{
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
    talk_dialog(&dialogs[ACT1_DIALOG2][0]); // (ES) "Gremio de los historiadores|Año 8121" - (EN) "Historians guild|Year 8121"
    talk_dialog(&dialogs[ACT1_DIALOG2][1]); // (ES) "Lunes|Primera hora de la mañana" - (EN) "Monday|Early morning"
    move_character(CHR_linus, 200, 174);
    talk_dialog(&dialogs[ACT1_DIALOG2][2]); // (ES) "Es tarde, Linus|Y uno no debe llegar tarde|a su cumpleaños" - (EN) "It's late, Linus|And you shouldn't be late|at your birthday"
    talk_dialog(&dialogs[ACT1_DIALOG2][3]); // (ES) "He tenido el sueño|más extraño, Madre" - (EN) "I have had the strangest|dream, Mother"
    talk_dialog(&dialogs[ACT1_DIALOG2][4]); // (ES) "Un cisne venía a|mi cuarto y..." - (EN) "A swan came to my room|and..."
    talk_dialog(&dialogs[ACT1_DIALOG2][5]); // (ES) "Luego me lo cuentas|Xander nos espera" - (EN) "You can tell me later|Xander is waiting for us"
    // Xander's entrance
    move_character(CHR_clio, 100, 154);
    wait_seconds(1);
    look_left(CHR_clio, true);
    move_character_instant(CHR_xander,-30,174);
    show_character(CHR_xander, true);
    move_character(CHR_xander, 40, 174);
    talk_dialog(&dialogs[ACT1_DIALOG2][6]); // (ES) "Por fin|estás despierto, Linus" - (EN) "At last,|you're awake Linus"
    talk_dialog(&dialogs[ACT1_DIALOG2][7]); // (ES) "Perdóname, maestro|Un extraño sueño me ha|mantenido despierto" - (EN) "Forgive me, master|A strange dream has|kept me awake"
    talk_dialog(&dialogs[ACT1_DIALOG2][8]); // (ES) "Ciertamente eres el|hijo de tu padre|Aiden tenía grandes sueños" - (EN) "You are certainly your|father's son|Aiden had big dreams"
    talk_dialog(&dialogs[ACT1_DIALOG2][9]); // (ES) "Y estamos aquí para hablar|sobre uno que|nunca llegó a cumplir" - (EN) "And we are here to talk|about one that he|never achieved"
    talk_dialog(&dialogs[ACT1_DIALOG2][10]); // (ES) "He leído sus historias|mil veces|¿De cuál hablamos?" - (EN) "I've read his stories|a thousand times|Which one is this?"
    talk_dialog(&dialogs[ACT1_DIALOG2][11]); // (ES) "Una que no encontrarás en|un libro. La de la isla|del gremio de los Tejedores" - (EN) "One you won't find in a book|The one about Weavers|guild island"
    u8 response = choice_dialog(&choices[ACT1_CHOICE1][0]); // (ES) "¿Los Tejedores?" - (EN) "The Weavers?", (ES) "Era mi leyenda favorita" - (EN) "It was my favourite legend", (ES) "¿Qué pasó con ellos?" - (EN) "What happened to them?"
    kprintf("Response: %d\n",response);
    talk_dialog(&dialogs[ACT1_DIALOG2][12+response]); // Response texts:
    // If response=0: (ES) "Según la leyenda Fueron un gremio capaz de tejer hechizos" - (EN) "According to the legend They were a guild able to weave spells"
    // If response=1: (ES) "Para él no era una leyenda Los Pastores la cantaban como cierta" - (EN) "That was no legend for him Shepherds sang it as a fact"
    // If response=2: (ES) "Desaparecieron sin dejar rastro Aunque muchos creen que era solo un cuento" - (EN) "They vanished without a trace Although many believe it was just a tale"
    talk_dialog(&dialogs[ACT1_DIALOG2][15]); // (ES) "Tu padre quería encontrar|la isla donde vivían" - (EN) "Your father wanted to find|their home island"
    talk_dialog(&dialogs[ACT1_DIALOG2][16]); // (ES) "Nuestro destino es|documentar hechos,|no perseguirlos" - (EN) "Our destiny is to|document facts,|not to chase them"
    talk_dialog(&dialogs[ACT1_DIALOG2][17]); // (ES) "Linus tiene diecisiete años|Esa era mi edad cuando|viajé por el mundo" - (EN) "Linus is seventeen|That was my age when|I traveled the world"
    talk_dialog(&dialogs[ACT1_DIALOG2][18]); // (ES) "Y la edad de su padre cuando|llegó aquí" - (EN) "And his father's age when|he came to us"
    talk_dialog(&dialogs[ACT1_DIALOG2][19]); // (ES) "Un año antes de que|le acogiéramos|como uno de los nuestros" - (EN) "A year before we|took him as one of ours"
    talk_dialog(&dialogs[ACT1_DIALOG2][20]); // (ES) "Madre..." - (EN) "Mother..."
    response = choice_dialog(&choices[ACT1_CHOICE1][1]); // (ES) "Tengo que ir a la isla" - (EN) "I have to go to the island", (ES) "¿Vendrías conmigo?" - (EN) "Would you come with me?"
    look_left(CHR_clio, false);
    talk_dialog(&dialogs[ACT1_DIALOG2][21+response]); // Response texts:
    // If response=0: (ES) "Si Xander lo quiere, así será Pero no irás solo" - (EN) "If Xander wants it that way, so it will be But you'll not go alone"
    // If response=1: (ES) "Nunca me ha gustado viajar Pero no dejaré que vayas solo" - (EN) "I've never liked to travel But I won't let you go alone"
    
    wait_seconds(2);

    end_level(); // Free resources
    current_scene=5; // Next scene
}

void act_1_scene_5(void)    // Combat tutorial scene with pattern demonstrations
{
    // Initialize level
    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map, forest_pal, 1000, BG_SCRL_USER_RIGHT, 3);
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
    talk_dialog(&dialogs[ACT1_DIALOG3][1]); // (ES) "Se aproximan enemigos|Tenemos que estar atentos|Quédate cerca, madre" - (EN) "Enemies are approaching|We have to stay alert|Stay close, mother"
    talk_dialog(&dialogs[ACT1_DIALOG3][11]); // (ES) "NOTA: Ni esta escena ni estos|gráficos estarán en el|juego cuando esté terminado" - (EN) "NOTE: Neither that scene nor those|graphics will be present|in the game when it's finished"
    talk_dialog(&dialogs[ACT1_DIALOG3][12]); // (ES) "Se ha decidido incluirla|en esta demo técnica|como prueba de ciertas mecánicas" - (EN) "It's been decided to include it|in this technical demo|as a test of certain mechanics"
    talk_dialog(&dialogs[ACT1_DIALOG3][13]); // (ES) "Pulsa START para ver|tu inventario de hechizos" - (EN) "Press START to view|your spell inventory"

    // Show the interface and allow character to move and play patterns
    player_scroll_active=true;
    movement_active=true;
    interface_active=true;
    player_patterns_enabled=true;
    show_or_hide_interface(true);

    while (offset_BGA<300) {
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
    talk_dialog(&dialogs[ACT1_DIALOG3][10]); // (ES) "¡Esto es todo!|(por ahora)" - (EN) "That's all!|(by now)"
    talk_dialog(&dialogs[ACT1_DIALOG3][5]); // (ES) "Gracias por probar la demo técnica|Sígueme por X para estar al día|@GeeseBumpsGames" - (EN) "Thtanks for testing our tech demo|Follow me in X for updates|@GeeseBumpsGames"
    talk_dialog(&dialogs[ACT1_DIALOG3][7]); // (ES) "Apaga tu consola|y haz algo constructivo|como jugar un poco al frontón" - (EN) "Turn off your console|and do something constructive|like play a little racquetball"
    talk_dialog(&dialogs[ACT1_DIALOG3][8]); // (ES) "o preparar la cena,|o organizar tu cajón de calcetines|alfabéticamente." - (EN) "or cook dinner,|or organize your sock drawer|alphabetically."
    PAL_fadeOutAll(120,false);

    SYS_hardReset(); // Reset and start again
}
