#include <genesis.h>
#include "globals.h"

#include "texts.h"


const char **game_texts[] =
{
    // Act 1, Sequence 1 with 2 messages
    (const char *[])
    {
        "Es tarde, Linus|Y uno no debe llegar tarde|a su cumpleanos", "It's late, Linus|And you shouldn' be late|at your birthday",
        "He tenido el sueno|mas extano, madre", "I have had the strangest|dream, mother",
        "Un cisne venia a|mi cuarto, y...", "A swan came to my room|and...",
        "Luego me lo cuentas|Xander nos espera", "You can tell me later|Xander is wating for us",
        "Por fin|estas despierto, Linus", "At last,|you're awake Linus",
        "Perdoname, maestro|Un extrano sueno me ha|mantenido despierto", "Forgive me, master|A strange dream has|kept me awake",
        "Ciertamente eres el|hijo de tu padre.|Aiden tenia grandes suenos,", "You are certainly your|father's son.|Aiden had big dreams,",
        "y estamos aqui para hablar|sobre uno que|nunca llego a cumplir", "and we are here to talk|about one that he|never achieved",
        "He leido sus historias|mil veces|De cual hablamos?", "I've read his stories a thousand times|Which one is this?",
        "Una que no encontraras en|un libro. La de la isla|del gremio de los Tejedores", "One you won't find in a book|The one about Weavers|guild island",
        "La leyenda de los Tejedores|siempre fue mi favorita", "Weavers legend was always|my favourite",
        "Para el no era una leyenda|Los pastores la cantaban|como cierta","That was no legend for him|Shepards sang it as a fact",
        "Tu padre queria encontrarla", "Your father wanted to find it",
        "Nuestro destino es|documentar hechos|No perseguirlos", "Our destiny is to|document facts|Not to chase them",
        "Linus tiene diecisiete anos|Esa era mi edad cuando|viaje por el mundo", "Linus is seventeen|That was my age when|I traveled the world",
        "Y la edad de su padre cuando|llego aqui", "And his father's age when|he came to us",
        "Un ano antes de que|le acogieramos|como uno de los nuestros", "A year before we|took him as one of ours",
        "Pero...", "But...",
        "Madre, necesito visitar|esa isla", "Mother, I need to visit|that island",
        "Si Xander lo quiere,|asi será|Pero no iras solo", "If Xander wants it that way,|it will be so|But you'll not go alone",
        NULL
    },
    // Act 1, Sequence 2...
    (const char *[])
    {
        "Texto en español A1S2 M1", "English text A1S2 M1",
        "Texto en español A1S2 M2", "English text A1S2 M2",
        "Texto en español A1S2 M3", "English text A1S2 M3", NULL
    }
    // ... Add more text arrays for each act and sequence combination
};



// Get a text in a given language
const char* getText(int actSeq, int message, int language)
{
    return game_texts[actSeq][message * 2 + language];
}
