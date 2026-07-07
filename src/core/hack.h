// hack.h
// Compile-time toggles for development/testing (estilo RedPlanet).
// Este es EL ÚNICO archivo que hay que tocar para debugear el juego.
// Set any macro to TRUE (or a value) to enable the corresponding hack.
// When FALSE / 0 (default), the game behaves normally.

#ifndef _HACK_H_
#define _HACK_H_

// --- Game hacks (set to TRUE / value to enable) ---

// Nombre de escena para saltar logo/intro y arrancar directamente en ella
// ("" = flujo normal). Ej: "act1_forest" = directo al combate del bosque.
// Nombres = data/scenes/<acto>/<escena>.scene → "<acto>_<escena>"
#define HACK_START_SCENE        ""

// 0 = normal (menú de la intro) · 1 = forzar español · 2 = forzar inglés
#define HACK_FORCE_LANGUAGE     0

// Vara y los 4 hechizos del jugador activados desde el principio
#define HACK_ALL_SPELLS         FALSE

// El jugador no recibe daño ni entra en estado HURT
#define HACK_PLAYER_INVULNERABLE FALSE

// Los enemigos mueren al primer golpe (counter mata de una)
#define HACK_ENEMIES_ONE_HP     FALSE

// --- Debug switches (for development, not meant for release) ---

// 0: no debug, 1: easter eggs & errors, 2: debug messages, 3: verbose debug messages
// Los mensajes salen por KDebug (visibles en la consola de BlastEm)
#define DEBUG_LEVEL             3

// Los diálogos avanzan solos casi al instante (para recorrer escenas rápido)
#define HACK_FAST_DIALOGS       FALSE

// Silenciar música / efectos (para escuchar solo lo que se está depurando)
#define HACK_MUTE_MUSIC         FALSE
#define HACK_MUTE_SFX           FALSE

#endif // _HACK_H_
