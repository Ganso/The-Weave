#include <genesis.h>
#include "globals.h"

#define SIDE_LEFT true
#define SIDE_RIGHT false

const DialogItem act1_dialog1[] = {
    {FACE_none, SIDE_LEFT, MAX_TALK_TIME, 
        {"Gremio de los historiadores|A^o 8121", 
         "Historians guild|Year 8121"}},
    {FACE_none, SIDE_LEFT, MAX_TALK_TIME, 
        {"Lunes|Primera hora de la ma^ana", 
         "Monday|Early morning"}},
    {0, false, MAX_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem act1_dialog2[] = {
    {FACE_clio, SIDE_LEFT, MAX_TALK_TIME, 
        {"Es tarde, Linus|Y uno no debe llegar tarde|a su cumplea^os", 
         "It's late, Linus|And you shouldn't be late|at your birthday"}},
    {FACE_linus, SIDE_RIGHT, MAX_TALK_TIME, 
        {"He tenido el sue^o|mas exta^o, madre", 
         "I have had the strangest|dream, mother"}},
    {FACE_linus, SIDE_RIGHT, MAX_TALK_TIME, 
        {"Un cisne venia a|mi cuarto, y...", 
         "A swan came to my room|and..."}},
    {FACE_clio, SIDE_LEFT, MAX_TALK_TIME, 
        {"Luego me lo cuentas|Xander nos espera", 
         "You can tell me later|Xander is waiting for us"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Por fin|estas despierto, Linus", 
         "At last,|you're awake Linus"}},
    {FACE_linus, SIDE_RIGHT, MAX_TALK_TIME, 
        {"Perdoname, maestro|Un extrano sue^o me ha|mantenido despierto", 
         "Forgive me, master|A strange dream has|kept me awake"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Ciertamente eres el|hijo de tu padre.|Aiden tenia grandes sue^os,", 
         "You are certainly your|father's son.|Aiden had big dreams,"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"y estamos aqui para hablar|sobre uno que|nunca llego a cumplir", 
         "and we are here to talk|about one that he|never achieved"}},
    {FACE_linus, SIDE_RIGHT, MAX_TALK_TIME, 
        {"He leido sus historias|mil veces|<De cual hablamos?", 
         "I've read his stories|a thousand times|Which one is this?"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Una que no encontraras en|un libro. La de la isla|del gremio de los Tejedores", 
         "One you won't find in a book|The one about Weavers|guild island"}},
    {FACE_linus, SIDE_RIGHT, MAX_TALK_TIME, 
        {"La leyenda de los Tejedores|siempre fue mi favorita", 
         "Weavers legend was always|my favourite"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Para el no era una leyenda|Los pastores la cantaban|como cierta",
         "That was no legend for him|Shepherds sang it as a fact"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Tu padre queria encontrarla", 
         "Your father wanted to find it"}},
    {FACE_clio, SIDE_RIGHT, MAX_TALK_TIME, 
        {"Nuestro destino es|documentar hechos|No perseguirlos", 
         "Our destiny is to|document facts|Not to chase them"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Linus tiene diecisiete a^os|Esa era mi edad cuando|viaje por el mundo", 
         "Linus is seventeen|That was my age when|I traveled the world"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Y la edad de su padre cuando|llego aqui", 
         "And his father's age when|he came to us"}},
    {FACE_xander, SIDE_LEFT, MAX_TALK_TIME, 
        {"Un a^o antes de que|le acogieramos|como uno de los nuestros", 
         "A year before we|took him as one of ours"}},
    {FACE_linus, SIDE_RIGHT, MAX_TALK_TIME, 
        {"Pero...", 
         "But..."}},
    {FACE_linus, SIDE_RIGHT, 100, 
        {"Madre, necesito visitar|esa isla", 
         "Mother, I need to visit|that island"}},
    {FACE_clio, SIDE_LEFT, MAX_TALK_TIME, 
        {"Si Xander lo quiere,|asi sera|Pero no iras solo", 
         "If Xander wants it that way,|so it will be|But you'll not go alone"}},
    {0, false, MAX_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem act1_dialog3[] = {
    {FACE_none, SIDE_LEFT, MAX_TALK_TIME, 
        {"Algun tiempo despues", 
         "Some time later"}},
    {FACE_linus, SIDE_LEFT, MAX_TALK_TIME, 
        {"Contraataca a los magos|Escondete de los monos", 
         "Counterattack wizards|Hide from monkeys"}},
    {0, false, MAX_TALK_TIME, {NULL, NULL}} // Terminator
};

const DialogItem *dialogs[] = {
    act1_dialog1,
    act1_dialog2,
    act1_dialog3,
};