#include <genesis.h>
#include "globals.h"

#define SIDE_LEFT true
#define SIDE_RIGHT false

// MAX TEXT LENGHT: "123456789012345678901234567890" (30 characters)

// Global variable definitions
u8 game_language=LANG_ENGLISH;

const DialogItem system_dialog[] = { // System messages
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 0
        {"No puedo usar ese patrón|ahora mismo", 
         "I can't use that pattern|right now"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 1
        {"The Weave|Demo técnica|Enero de 2025", 
         "The Weave|Tech demo|January 2025"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 2
        {"Los gráficos, mecánicas o sonidos|no son definitivos, ni|representan el resultado final", 
         "Graphics, mechanics or sounds|aren't final, nor they|represent the final result"}},
    {0, false, DEFAULT_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem act1_dialog1[] = { // Historians corridor
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 0
        {"Creo que he dormido demasiado|Debo llegar rápido al salón", 
         "I think I've overslept|I should go to the hall quickly"}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 1
        {"Aunque siendo el día que es|este pasillo me trae|demasiados recuerdos", 
         "Although in a day like this|this hallway brings back|too many memories"}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 2
        {"Antes de irme quiero|repasar algunos recuerdos|Se lo debo a papá", 
         "Before I leave I want to|revisit some memories|I owe it to dad"}},        
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 3
        {"Este tomo narra la historia|de nuestro gremio|desde la Gran Separación", 
        "This volume narrates the history|of our guild|since the Great Split"}},        
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 4
        {"El último capítulo|termina con el fallecimiento|de mi padre", 
        "The last chapter|ends with the passing|of my father"}},       
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 5
        {"Madre dice que seré yo|el que deba escribir|el siguiente", 
        "Mother says it will be me|who has to write|the next one"}},       
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 6
        {"Una colección de|mitos y leyendas|de los distintos gremios", 
        "A collection of|myths and legends|from the different guilds"}},        
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 7
        {"Gracias a mi padre|tenemos documentadas|las que cantaban los Pastores", 
        "Thanks to my father|we have documented|those the Shepherds sang"}},     
    {0, false, DEFAULT_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem act1_dialog2[] = { // Histoiran hall
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 0
        {"Gremio de los historiadores|Año 8121", 
         "Historians guild|Year 8121"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 1
        {"Lunes|Primera hora de la mañana", 
         "Monday|Early morning"}},
    {FACE_clio, SIDE_LEFT, DEFAULT_TALK_TIME, // 2
        {"Es tarde, Linus|Y uno no debe llegar tarde|a su cumpleaños", 
         "It's late, Linus|And you shouldn't be late|at your birthday"}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 3
        {"He tenido el sueño|más extraño, Madre", 
         "I have had the strangest|dream, Mother"}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 4
        {"Un cisne venía a|mi cuarto y...", 
         "A swan came to my room|and..."}},
    {FACE_clio, SIDE_LEFT, DEFAULT_TALK_TIME, // 5
        {"Luego me lo cuentas|Xander nos espera", 
         "You can tell me later|Xander is waiting for us"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 6
        {"Por fin|estás despierto, Linus", 
         "At last,|you're awake Linus"}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 7
        {"Perdóname, maestro|Un extraño sueño me ha|mantenido despierto", 
         "Forgive me, master|A strange dream has|kept me awake"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 8
        {"Ciertamente eres el|hijo de tu padre|Aiden tenía grandes sueños", 
         "You are certainly your|father's son|Aiden had big dreams"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 9
        {"Y estamos aquí para hablar|sobre uno que|nunca llegó a cumplir", 
         "And we are here to talk|about one that he|never achieved"}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_TALK_TIME, // 10
        {"He leído sus historias|mil veces|¿De cuál hablamos?", 
         "I've read his stories|a thousand times|Which one is this?"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 11
        {"Una que no encontrarás en|un libro. La de la isla|del gremio de los Tejedores", 
         "One you won't find in a book|The one about Weavers|guild island"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 12
        {"Según la leyenda|Fueron un gremio|capaz de tejer hechizos",
         "According to the legend|They were a guild|able to weave spells"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 13
        {"Para él no era una leyenda|Los Pastores la cantaban|como cierta",
         "That was no legend for him|Shepherds sang it as a fact"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 14
        {"Desaparecieron sin dejar rastro|Aunque muchos creen|que era solo un cuento",
         "They vanished without a trace|Although many believe|it was just a tale"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 15
        {"Tu padre quería encontrarla", 
         "Your father wanted to find it"}},
    {FACE_clio, SIDE_RIGHT, DEFAULT_TALK_TIME, // 16
        {"Nuestro destino es|documentar hechos,|no perseguirlos", 
         "Our destiny is to|document facts,|not to chase them"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 17
        {"Linus tiene diecisiete años|Esa era mi edad cuando|viajé por el mundo", 
         "Linus is seventeen|That was my age when|I traveled the world"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 18
        {"Y la edad de su padre cuando|llegó aquí", 
         "And his father's age when|he came to us"}},
    {FACE_xander, SIDE_LEFT, DEFAULT_TALK_TIME, // 19
        {"Un año antes de que|le acogiéramos|como uno de los nuestros", 
         "A year before we|took him as one of ours"}},
    {FACE_linus, SIDE_RIGHT, 100, // 20
        {"Madre...", 
         "Mother..."}},
    {FACE_clio, SIDE_LEFT, DEFAULT_TALK_TIME, // 21
        {"Si Xander lo quiere, así será|Pero no irás solo", 
         "If Xander wants it that way,|so it will be|But you'll not go alone"}},
    {FACE_clio, SIDE_LEFT, DEFAULT_TALK_TIME, // 22
        {"Nunca me ha gustado viajar|Pero no dejaré que vayas solo", 
         "I've never liked to travel|But I won't let you go alone"}},
    {0, false, DEFAULT_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem act1_dialog3[] = { // Combat zone
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 0
        {"Algún tiempo después", 
         "Some time later"}},
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 1
        {"Se aproximan enemigos|Tenemos que estar atentos|Quédate cerca, madre",
         "Enemies are approaching|We have to stay alert|Stay close, mother"}},
     {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 2
         {"Eso ha dolido",
         "That hurts"}},
     {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 3
         {"Quizá deba pensar|al revés",
         "I should maybe|think backwards"}},
     {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 4
         {"Puedo probar a esconderme|o tratar de invocar|al trueno",
         "I could try to hide|or attempt to summon|the thunder"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 5
        {"Gracias por probar la demo técnica|Sígueme por X para estar al día|@GeeseBumpsGames", 
         "Thtanks for testing our tech demo|Follow me in X for updates|@GeeseBumpsGames"}},
     {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 6
         {"Si reproduzco al revés|las notas, podré|contraatacar este hechizo",
         "If I play the notes backwards|I could be able to|counter the spell"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 7
        {"Apaga tu consola|y haz algo constructivo|como jugar un poco al frontón", 
         "Turn off your console|and do something constructive|like play a little racquetball"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 8
        {"o preparar la cena,|o organizar tu cajón de calcetines|alfabéticamente.", 
         "or cook dinner,|or organize your sock drawer|alphabetically."}},
     {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 9
         {"No puedo lanzar hechizos|si estoy escondido",
         "I can't launch spells|while hiding"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 10
        {"¡Esto es todo!|(por ahora)", 
         "That's all!|(by now)"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 11
        {"NOTA: Ni esta escena ni estos|gráficos estarán en el|juego cuando esté terminado", 
         "NOTE: Neither that scene nor those|graphics will be present|in the game when it's finished"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 12
        {"Se ha decidido incluirla|en esta demo técnica|como prueba de ciertas mecánicas", 
         "It's been decided to include it|in this technical demo|as a test of certain mechanics"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 13
        {"Pulsa START para ver|tu inventario de hechizos", 
         "Press START to view|your spell inventory"}},
    {0, false, DEFAULT_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem act1_dialog4[] = { // Linus bedroom
    {FACE_swan, SIDE_LEFT, DEFAULT_TALK_TIME, // 0
        {"Han pasado ya cien años",
        "A hundred years have passed"}},
    {FACE_swan, SIDE_LEFT, DEFAULT_TALK_TIME, // 1
        {"No podremos pararles|por mucho más tiempo",
        "We won't be able to|stop them for much longer"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 2
        {"A la mañana siguiente...",
        "The next morning..."}},
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 3
        {"He dormido regular esta noche|Tuve horribles pesadillas",
        "I've not slept well last night|I had terrible nightmares"}},
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 4
        {"No ha podido ser real|La ventana está cerrada",
        "It couldn't be real|The windows is closed"}},
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 5
        {"No tengo tiempo de sentarme|Madre me espera",
        "I don't have time to sit down|Mother wating for me"}},
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 6
        {"Esta es la nana que me|cantaban cada noche",
        "That's the lullaby they used|to sing to me every night"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 7
        {"Has aprendido|tu primer patrón",
        "You have learned|your first pattern"}},
    {FACE_none, SIDE_LEFT, DEFAULT_TALK_TIME, // 8
        {"Entra en el menú de|pausa para verlo",
        "Enter the pause menu|to check it out"}},
    {FACE_clio, SIDE_RIGHT, DEFAULT_TALK_TIME, // 9
        {"Linus, ¿dónde estás?",
        "Linus, where are you?"}},
    {FACE_linus, SIDE_LEFT, DEFAULT_TALK_TIME, // 10
        {"Se me ha hecho demasiado tarde|tengo que ir al salón",
        "It's gotten too late|I need to go the hall"}},
    {0, false, DEFAULT_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem *dialogs[] = {
    system_dialog,  // 0
    act1_dialog1,   // 1
    act1_dialog2,   // 2
    act1_dialog3,   // 3
    act1_dialog4,   // 4
};

const ChoiceItem act1_choice1[] = {
    {FACE_linus, SIDE_RIGHT, DEFAULT_CHOICE_TIME, 3,
        {{ "¿Los tejedores?", "Era mi leyenda favorita", "¿Qué pasó con ellos?"},
        { "The Weavers?", "It was my favourite legend", "What happened to them?"}}},
    {FACE_linus, SIDE_RIGHT, DEFAULT_CHOICE_TIME, 2,
        {{ "Tengo que ir a la isla", "¿Vendrías conmigo?"},
        { "I have to go to the island", "Would you come with me?"}}},
    {0, false, DEFAULT_TALK_TIME, 0, {{NULL}}} // Terminator
};

const ChoiceItem *choices[] = {
    act1_choice1,   // 0
};

 // Code Spanish text in the game font charset
// SPANISH CHARSET
// ñ --> ^
// á --> #
// é --> $
// í --> %
// ó --> *
// ú --> /
// < --> ¿
// > --> ¡

char* encode_spanish_text(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    size_t len = strlen(input);
    char* result = (char*)malloc(len + 1); // +1 for the null terminator
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    size_t i = 0;
    size_t j = 0;
    while (i < len) {
        if ((unsigned char)input[i] == 0xC3) {
            // This is a two-byte UTF-8 sequence
            if (i + 1 < len) {
                switch ((unsigned char)input[i + 1]) {
                    case 0xB1: // ñ
                    case 0x91: // Ñ
                        result[j] = '^';
                        break;
                    case 0xA1: // á
                    case 0x81: // Á
                        result[j] = '#';
                        break;
                    case 0xA9: // é
                    case 0x89: // É
                        result[j] = '$';
                        break;
                    case 0xAD: // í
                    case 0x8D: // Í
                        result[j] = '%';
                        break;
                    case 0xB3: // ó
                    case 0x93: // Ó
                        result[j] = '*';
                        break;
                    case 0xBA: // ú
                    case 0x9A: // Ú
                        result[j] = '/';
                        break;
                    default:
                        // If it's not a special character, copy both bytes
                        result[j] = input[i];
                        j++;
                        result[j] = input[i + 1];
                }
                i += 2; // Skip the next byte as we've already processed it
            } else {
                // Unexpected end of string, just copy the byte
                result[j] = input[i];
                i++;
            }
        } else if ((unsigned char)input[i] == 0xC2) {
            // This is a two-byte UTF-8 sequence for ¿ and ¡
            if (i + 1 < len) {
                switch ((unsigned char)input[i + 1]) {
                    case 0xBF: // ¿
                        result[j] = '<';
                        break;
                    case 0xA1: // ¡
                        result[j] = '>';
                        break;
                    default:
                        // If it's not a special character, copy both bytes
                        result[j] = input[i];
                        j++;
                        result[j] = input[i + 1];
                }
                i += 2; // Skip the next byte as we've already processed it
            } else {
                // Unexpected end of string, just copy the byte
                result[j] = input[i];
                i++;
            }
        } else {
            // Regular ASCII character, just copy it
            result[j] = input[i];
            i++;
        }
        j++;
    }
    result[j] = '\0'; // Ensure null-termination

    return result;
}
