#include "globals.h"
#include "patterns_registry.h"
#include "counter_spell.h"
#include "pattern_types/electric_pattern.h"
#include "pattern_types/hide_pattern.h"
#include "pattern_types/sleep_pattern.h"
#include "pattern_types/open_pattern.h"

StateMachine player_state_machine;        // Global state machine for player character

bool player_has_rod;                      // Has the player the rod?
bool player_patterns_enabled;             // Whether pattern system is currently enabled
u8 note_playing;                          // Currently playing musical note (NOTE_NONE if none)
u16 note_playing_time;                    // Duration of current note in ticks
u16 time_since_last_note;                 // Time since last note in pattern sequence
u16 player_pattern_effect_in_progress;    // Active pattern effect type (PTRN_NONE if none)
bool player_pattern_effect_reversed;      // Whether current pattern is reversed
u16 player_pattern_effect_time;           // Duration of current pattern effect
u8 played_notes[4];                       // Sequence of notes played in current pattern
u8 num_played_notes;                      // Number of notes played in current pattern
Pattern obj_pattern[MAX_PATTERNS];        // Available pattern spells and their states

void activate_spell(u16 npattern)    // Unlock new pattern spell with visual/audio feedback
{
    // If pattern is already active, do nothing
    if (obj_pattern[npattern].active) {
        return;
    }
    
    // Show icon and play notes
    show_pattern_icon(PTRN_SLEEP, true, true);
    for (u8 i = 0; i < 4; i++) {
        show_note(obj_pattern[npattern].notes[i], true);
        wait_seconds(1);
        show_note(obj_pattern[npattern].notes[i], false);
    }
    show_pattern_icon(PTRN_SLEEP, false, false);

    // Activate the pattern
    obj_pattern[npattern].active = true;
    
    // Register the pattern with the registry
    Pattern* pattern = get_pattern(npattern, OWNER_PLAYER);
    if (pattern) {
        pattern->active = true;
    }
}

void play_note(u8 nnote)    // Handle new musical note input and update character state
{
    // Prevenir notas demasiado rápidas, pero permitir la primera nota
    // o si ha pasado suficiente tiempo desde la última nota
    if (note_playing_time < MIN_TIME_BETWEEN_NOTES && note_playing != 0) {
        kprintf("Rejecting note: too soon after previous note");
        return;
    }

    // Asegurarse de que la nota sea válida (1-6)
    if (nnote < 1 || nnote > 6) {
        kprintf("Invalid note: %d", nnote);
        return;
    }

    // Verificar si podemos reproducir una nota ahora
    if (!player_state_machine.pattern_system.is_note_playing) {
        kprintf("Playing note: %d, current count: %d", nnote, num_played_notes);
        
        // Añadir la nota al patrón actual
        if (num_played_notes < 4) {
            played_notes[num_played_notes] = nnote;
            num_played_notes++;
            
            // Sincronizar con la máquina de estados
            for (u8 i = 0; i < 4; i++) {
                player_state_machine.notes[i] = played_notes[i];
            }
            player_state_machine.note_count = num_played_notes;
        }
        
        // Mostrar la nota visualmente
        show_note(nnote, true);
        
        // Mantener la compatibilidad con el sistema actual
        note_playing = nnote;
        note_playing_time = 0;
        obj_character[active_character].state = STATE_PLAYING_NOTE;
        
        // Usar la nueva función de manejo de notas
        StateMachine_HandleNoteInput(&player_state_machine, nnote);
    }
}

void check_active_character_state(void)    // Process character states for pattern system
{
    // Actualizar la máquina de estados
    StateMachine_Update(&player_state_machine, NULL);
    
    // Solo actualizar el estado si no estamos en un estado de movimiento
    if (obj_character[active_character].state != STATE_WALKING) {
        update_character_from_sm_state(&obj_character[active_character],
                                    player_state_machine.current_state);
    }
    
    // Verificar si estamos en combate y hay un ataque enemigo en progreso
    if (is_combat_active && enemy_attacking != ENEMY_NONE && enemy_attack_effect_in_progress) {
        // Si tenemos 4 notas, verificar si es un contraataque
        if (num_played_notes == 4) {
            bool is_reverse_match = false;
            u8 matched_pattern = validate_player_pattern(played_notes, &is_reverse_match);
            
            if (matched_pattern == PTRN_ELECTRIC && is_reverse_match) {
                kprintf("COUNTER-ATTACK DETECTED DURING ENEMY ATTACK!");
                
                // Usar el sistema de contraataque
                execute_counter_spell(&player_state_machine,
                                      &enemy_state_machines[enemy_attacking],
                                      PTRN_ELECTRIC);
                
                // Limpiar el estado del patrón
                reset_pattern_state();
                
                // Actualizar sprites
                SPR_update();
                
                return;
            }
        }
    }
    
    // Mantener la lógica existente temporalmente
    bool is_reverse_match;
    u8 matched_pattern;

    switch (obj_character[active_character].state)
    {
        case STATE_PLAYING_NOTE:
            if (note_playing_time != 0) {
                if (note_playing_time == calc_ticks(MAX_NOTE_PLAYING_TIME)) {
                    kprintf("Note playing time reached max, updating pattern state");
                    update_pattern_state();
                }
                else {
                    note_playing_time++;
                }
            }
            break;

        case STATE_IDLE:
            handle_pattern_timeout();
            break;

        case STATE_PATTERN_CHECK:
            kprintf("Checking pattern. Notes played: %d %d %d %d (count: %d)",
                   played_notes[0], played_notes[1], played_notes[2], played_notes[3],
                   num_played_notes);
            
            if (num_played_notes == 4) {
                matched_pattern = validate_player_pattern(played_notes, &is_reverse_match);
                hide_rod_icons();

                if (matched_pattern != PTRN_NONE) {
                    player_pattern_effect_reversed = is_reverse_match;
                    player_state_machine.pattern_system.effect_reversed = is_reverse_match;
                    player_state_machine.is_reversed = is_reverse_match;
                    
                    // Get the pattern from the registry
                    Pattern* pattern = get_pattern(matched_pattern, OWNER_PLAYER);
                    
                    // Check if pattern can be used
                    bool can_use = true;
                    if (pattern && pattern->can_use) {
                        can_use = pattern->can_use();
                    } else {
                        // Fallback to old system
                        switch (matched_pattern) {
                            case PTRN_ELECTRIC:
                                can_use = can_use_electric_pattern();
                                break;
                            case PTRN_HIDE:
                                can_use = can_use_hide_pattern();
                                break;
                            case PTRN_SLEEP:
                                can_use = can_use_sleep_pattern();
                                break;
                            case PTRN_OPEN:
                                can_use = can_use_open_pattern();
                                break;
                            default:
                                can_use = false;
                                break;
                        }
                    }
                    
                    // Special case for direct thunder during enemy thunder attack
                    if (matched_pattern == PTRN_ELECTRIC && !is_reverse_match &&
                        is_combat_active && enemy_attacking != ENEMY_NONE &&
                        enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
                        
                        kprintf("Direct thunder during enemy thunder - checking if already shown hint");
                        
                        // Verificar si ya hemos tenido éxito con un contraataque
                        if (counter_spell_success) {
                            // Si ya hemos tenido éxito con un contraataque, tratar como un patrón normal
                            kprintf("Counter spell already successful, treating as normal pattern");
                            StateMachine_HandlePatternComplete(&player_state_machine, PTRN_ELECTRIC, false);
                        } else {
                            // Show hint dialog
                            show_or_hide_interface(false);
                            show_or_hide_enemy_combat_interface(false);
                            talk_dialog(&dialogs[ACT1_DIALOG3][6]); // (ES) "Si reproduzco al revés|las notas, podré|contraatacar este hechizo" - (EN) "If I play the notes backwards|I could be able to|counter the spell"
                            show_or_hide_enemy_combat_interface(true);
                            show_or_hide_interface(true);
                            
                            // Reset state without showing the "can't use pattern" dialog
                            play_pattern_sound(PTRN_NONE);
                            obj_character[active_character].state = STATE_IDLE;
                            
                            // Apply damage to player since they failed to counter
                            hit_caracter(active_character);
                            
                            // Directly reset enemy state variables to allow new attacks
                            u16 current_enemy = enemy_attacking;
                            u16 current_pattern = enemy_attack_pattern;
                            
                            // Reset visual effects
                            VDP_setHilightShadow(false);
                            
                            // Reset enemy state
                            if (current_enemy != ENEMY_NONE) {
                                anim_enemy(current_enemy, ANIM_IDLE);
                                obj_enemy[current_enemy].obj_character.state = STATE_IDLE;
                                
                                // Set cooldown to allow attacks after a delay
                                if (current_pattern != PTRN_EN_NONE) {
                                    obj_enemy[current_enemy].last_pattern_time[current_pattern] =
                                        obj_Pattern_Enemy[current_pattern].recharge_time / 2;
                                }
                            }
                        }
                        
                        // Reset global state variables
                        enemy_attack_effect_in_progress = false;
                        enemy_attack_effect_time = 0;
                        enemy_attack_pattern = PTRN_EN_NONE;
                        enemy_attacking = ENEMY_NONE;
                        
                        // Clean up any active notes
                        cleanup_enemy_notes();
                        
                        // Update sprites
                        SPR_update();
                    }
                    // Handle pattern if it can be used
                    else if (can_use) {
                        kprintf("Pattern %d can be used, sending pattern complete message", matched_pattern);
                        StateMachine_HandlePatternComplete(&player_state_machine, matched_pattern, is_reverse_match);
                    }
                    else {
                        kprintf("Pattern %d matched but not usable in current context", matched_pattern);
                        // Pattern matched but not usable in current context
                        show_pattern_icon(matched_pattern, true, true);
                        play_pattern_sound(PTRN_NONE);
                        show_or_hide_interface(false);
                        talk_dialog(&dialogs[SYSTEM_DIALOG][0]); // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"
                        show_or_hide_interface(true);
                        show_pattern_icon(matched_pattern, false, false);
                        obj_character[active_character].state = STATE_IDLE;
                    }
                }
                else {
                    // No pattern matched
                    kprintf("No pattern match");
                    play_pattern_sound(PTRN_NONE);
                    obj_character[active_character].state = STATE_IDLE;
                }
                
                reset_pattern_state();
                next_frame(false);
            } else {
                kprintf("Not enough notes yet (%d/4), waiting...", num_played_notes);
                obj_character[active_character].state = STATE_IDLE;
            }
            break;

        case STATE_PATTERN_EFFECT:
        case STATE_PATTERN_EFFECT_FINISH:
            // These states are now handled by the state machine
            break;

        default:
            break;
    }
}

void play_pattern_sound(u16 npattern)    // Play sound effect for given pattern type
{
    switch (npattern)
    {
    case PTRN_HIDE:
        play_sample(snd_pattern_hide,sizeof(snd_pattern_hide));
        break;
    case PTRN_OPEN:
        play_sample(snd_pattern_open,sizeof(snd_pattern_open));
        break;
    case PTRN_ELECTRIC:
        play_sample(snd_pattern_thunder,sizeof(snd_pattern_thunder));
        break;
    default:
        play_sample(snd_pattern_invalid,sizeof(snd_pattern_invalid));
        break;        
    }
}

void init_patterns(void)    // Setup initial pattern definitions and states
{
    kprintf("Initializing patterns");
    
    // Initialize pattern registry
    pattern_registry_init();
    
    // Inicializar patrones existentes
    obj_pattern[PTRN_ELECTRIC] = (Pattern) {
        .id = PTRN_ELECTRIC,
        .active = true,
        .notes = {1,2,3,4},
        .owner_type = OWNER_PLAYER,
        .can_use = electric_pattern_can_use,
        .launch = electric_pattern_launch,
        .do_effect = electric_pattern_do,
        .finish = electric_pattern_finish,
        .sd = NULL
    };
    
    obj_pattern[PTRN_HIDE] = (Pattern) {
        .id = PTRN_HIDE,
        .active = true,
        .notes = {2,5,3,6},
        .owner_type = OWNER_PLAYER,
        .can_use = hide_pattern_can_use,
        .launch = hide_pattern_launch,
        .do_effect = hide_pattern_do,
        .finish = hide_pattern_finish,
        .sd = NULL
    };
    
    obj_pattern[PTRN_OPEN] = (Pattern) {
        .id = PTRN_OPEN,
        .active = true,
        .notes = {2,3,3,2},
        .owner_type = OWNER_PLAYER,
        .can_use = open_pattern_can_use,
        .launch = open_pattern_launch,
        .do_effect = open_pattern_do,
        .finish = open_pattern_finish,
        .sd = NULL
    };
    
    obj_pattern[PTRN_SLEEP] = (Pattern) {
        .id = PTRN_SLEEP,
        .active = true,
        .notes = {2,1,6,4},
        .owner_type = OWNER_PLAYER,
        .can_use = sleep_pattern_can_use,
        .launch = sleep_pattern_launch,
        .do_effect = sleep_pattern_do,
        .finish = sleep_pattern_finish,
        .sd = NULL
    };

    // Register patterns with the registry
    for (u8 i = 0; i < MAX_PATTERNS; i++) {
        register_pattern(&obj_pattern[i]);
    }

    kprintf("Patterns initialized with active status: %d %d %d %d",
            obj_pattern[PTRN_ELECTRIC].active,
            obj_pattern[PTRN_HIDE].active,
            obj_pattern[PTRN_OPEN].active,
            obj_pattern[PTRN_SLEEP].active);

    // Inicializar máquina de estados
    StateMachine_Init(&player_state_machine, active_character);
    player_state_machine.owner_type = OWNER_PLAYER;
    
    // Configurar patrones disponibles - asignar el puntero al array
    player_state_machine.pattern_system.available_patterns = &obj_pattern[0];
    player_state_machine.pattern_system.pattern_count = MAX_PATTERNS;
    
    kprintf("State machine initialized with entity_id: %d", active_character);
}

// This function is now implemented in patterns_registry.c
// We provide this wrapper for backward compatibility
u8 validate_player_pattern(u8 *notes, bool *is_reverse)    // Check if notes match a player pattern
{
    return validate_pattern_sequence(notes, is_reverse, OWNER_PLAYER);
}

bool can_use_electric_pattern(void)
{
    // Permitir siempre el contraataque (patrón invertido)
    if (is_combat_active && enemy_attacking != ENEMY_NONE &&
        enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
        kprintf("Electric pattern can be used as counter-attack");
        return true;
    }
    // No permitir el patrón eléctrico normal durante un ataque eléctrico enemigo
    else if (is_combat_active && enemy_attacking != ENEMY_NONE &&
             enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
        kprintf("Cannot use normal electric pattern during enemy electric attack");
        return false;
    }
    else if (player_pattern_effect_in_progress == PTRN_HIDE) {
        // Can't use thunder while hidden
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][9]); // (ES) "No puedo lanzar hechizos|si estoy escondido" - (EN) "I can't launch spells|while hiding"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        return false;
    }
    return true;
}

bool can_use_hide_pattern(void)
{
    return true; // Currently no restrictions on hide pattern
}

bool can_use_sleep_pattern(void)
{
    return false; // You can't currently use the spell
}

bool can_use_open_pattern(void)
{
    return false; // You can't currently use the spell
}

void reset_pattern_state(void)    // Clear current pattern sequence state
{
    // Limpiar notas visuales (tanto en la vara como en el pentagrama)
    if (num_played_notes > 0) {
        kprintf("Clearing pattern state");
        
        // Limpiar notas visuales
        for (u8 i = 0; i < 4; i++) {
            if (i < num_played_notes) {
                show_note(played_notes[i], false);
            }
        }
        
        // Limpiar visuales del pentagrama
        hide_pentagram_icons();
        
        // Resetear contadores
        num_played_notes = 0;
        time_since_last_note = 0;
        player_state_machine.pattern_time = 0;
        player_state_machine.note_count = 0;
    }
    
    // Asegurarse de que note_playing_time se incremente para permitir nuevas notas
    if (note_playing_time < MIN_TIME_BETWEEN_NOTES) {
        note_playing_time = MIN_TIME_BETWEEN_NOTES;
    }
}

void handle_pattern_timeout(void)    // Check for pattern sequence timeout
{
    // Incrementar note_playing_time para permitir nuevas notas
    if (note_playing_time < 100) {
        note_playing_time++;
    }
    
    // Verificar si estamos en combate y hay un ataque enemigo en progreso
    if (is_combat_active && enemy_attacking != ENEMY_NONE && enemy_attack_effect_in_progress) {
        // Si tenemos 4 notas, verificar si es un contraataque
        if (num_played_notes == 4) {
            bool is_reverse_match = false;
            u8 matched_pattern = validate_player_pattern(played_notes, &is_reverse_match);
            
            kprintf("Checking for counter-attack in timeout: pattern=%d, is_reverse=%d", matched_pattern, is_reverse_match);
            
            if (matched_pattern == PTRN_ELECTRIC && is_reverse_match) {
                kprintf("COUNTER-ATTACK DETECTED IN handle_pattern_timeout!");
                
                // Usar el sistema de contraataque
                execute_counter_spell(&player_state_machine, 
                                     &enemy_state_machines[enemy_attacking - ENEMY_ENTITY_ID_BASE],
                                     PTRN_ELECTRIC);
                
                // Limpiar el estado del patrón
                reset_pattern_state();
                
                // Actualizar sprites
                SPR_update();
                
                return;
            }
        }
    }
    
    // Manejar timeout para patrones en progreso o a mitad de un patrón
    if (time_since_last_note != 0 || num_played_notes > 0) {
        // Si hay notas en progreso, incrementar el contador de tiempo
        if (time_since_last_note == 0 && num_played_notes > 0) {
            time_since_last_note = 1; // Iniciar el contador si hay notas pero no se ha iniciado
        } else {
            time_since_last_note++;
        }
        
        // Verificar si se ha alcanzado el tiempo máximo de espera
        if (time_since_last_note == calc_ticks(MAX_PATTERN_WAIT_TIME)) {
            kprintf("Pattern timeout detected - clearing %d notes", num_played_notes);
            
            // Limpiar estado primero
            reset_pattern_state();
            
            // Limpiar todas las visuales
            hide_rod_icons();
            hide_pentagram_icons();
            
            // Limpiar todas las notas posibles
            for (u8 i = 1; i <= 6; i++) {
                show_note(i, false);
            }
            
            // Reproducir sonido de patrón inválido
            play_pattern_sound(PTRN_NONE);
            
            // Notificar timeout a la máquina de estados
            StateMachine_HandlePatternTimeout(&player_state_machine);
        }
    }
    // Limpiar visuales incluso si no hay patrón en progreso
    else if (num_played_notes == 0 && note_playing == 0) {
        // Asegurarse de que no haya notas visibles
        hide_rod_icons();
        hide_pentagram_icons();
    }
}

void update_pattern_state(void)    // Process note completion and check for pattern match
{
    show_note(note_playing, false);
    
    // Actualizar estado actual
    note_playing = NOTE_NONE;
    time_since_last_note = 1;
    note_playing_time = 0;
    
    // Sincronizar con la máquina de estados
    player_state_machine.pattern_system.is_note_playing = false;
    player_state_machine.pattern_time = 1;
    
    // Verificar si estamos en combate y hay un ataque enemigo en progreso
    if (is_combat_active && enemy_attacking != ENEMY_NONE && enemy_attack_effect_in_progress) {
        kprintf("In update_pattern_state: Combat active with enemy %d attacking, pattern %d",
                enemy_attacking, enemy_attack_pattern);
        
        // Si tenemos 4 notas, verificar si es un contraataque
        if (num_played_notes == 4) {
            bool is_reverse_match = false;
            u8 matched_pattern = validate_player_pattern(played_notes, &is_reverse_match);
            
            kprintf("Checking for counter-attack: pattern=%d, is_reverse=%d", matched_pattern, is_reverse_match);
            
            if (matched_pattern == PTRN_ELECTRIC && is_reverse_match) {
                kprintf("COUNTER-ATTACK DETECTED IN update_pattern_state!");
                
                // Usar el sistema de contraataque
                execute_counter_spell(&player_state_machine, 
                                     &enemy_state_machines[enemy_attacking - ENEMY_ENTITY_ID_BASE],
                                     PTRN_ELECTRIC);
                
                // Limpiar el estado del patrón
                reset_pattern_state();
                
                // Actualizar sprites
                SPR_update();
                
                return;
            }
        }
    }
    
    if (num_played_notes == 4) {
        // La validación del patrón ahora se maneja en validate_pattern_sequence
        // que enviará MSG_PATTERN_COMPLETE si corresponde
        obj_character[active_character].state = STATE_PATTERN_CHECK;
    } else {
        obj_character[active_character].state = STATE_IDLE;
    }
    update_character_animation();
}

// These functions are now implemented in pattern_types/electric_pattern.c
void launch_electric_pattern(void) { /* Deprecated */ }
void do_electric_pattern_effect(void) { /* Deprecated */ }
void finish_electric_pattern_effect(void) { /* Deprecated */ }

// These functions are now implemented in pattern_types/hide_pattern.c
void launch_hide_pattern(void) { /* Deprecated */ }
void do_hide_pattern_effect(void) { /* Deprecated */ }
void finish_hide_pattern_effect(void) { /* Deprecated */ }

// These functions are now implemented in pattern_types/sleep_pattern.c
void launch_sleep_pattern(void) { /* Deprecated */ }
void do_sleep_pattern_effect(void) { /* Deprecated */ }
void finish_sleep_pattern_effect(void) { /* Deprecated */ }

// These functions are now implemented in pattern_types/open_pattern.c
void launch_open_pattern(void) { /* Deprecated */ }
void do_open_pattern_effect(void) { /* Deprecated */ }
void finish_open_pattern_effect(void) { /* Deprecated */ }
