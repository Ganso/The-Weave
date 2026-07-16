#ifndef _NOTES_H_
#define _NOTES_H_

// Input de notas del jugador (cola de 4, debounce, timeout, lock global)
// y HUD de notas del enemigo. La validación y el lanzamiento los hace spell.c.

#include <genesis.h>

// Globals del jugador
extern bool player_has_rod;          // puede físicamente usar hechizos
extern u8 player_note_limit;         // nota más alta disponible (beep de error por encima)
extern bool player_patterns_enabled; // no silenciado por una cutscene

// --- Input del jugador ---
bool spell_note_input(u8 noteCode);  // El jugador pulsa una nota (desde controller)
void spell_input_update(void);       // Timeout de patrón a medias (cada frame interactivo, desde next_frame)
void reset_note_queue(void);         // Vacía la cola y oculta los iconos de nota
u8   notes_player_count(void);       // Notas en cola (0-4); el motor congela al enemigo si está en (0,4)
void notes_set_lock(u16 frames);     // Lock global de input (anti-mashing)
bool notes_locked(void);
void notes_lock_tick(void);          // Decrementa el lock (lo llama spell_update)
void notes_input_reset(void);        // Resetea cola + timers + lock (engine reset)

// --- HUD de notas del enemigo ---
void enemy_notes_add(u8 enemySlot, u8 noteCode); // Muestra sprite + sonido de una nota enemiga
void enemy_notes_clear(void);                    // Libera todos los sprites de nota enemiga
void hide_enemy_notes(void);                     // Oculta temporalmente (diálogos)
void show_enemy_notes(void);                     // Vuelve a mostrar

#endif
