#include "globals.h"

// Instancia global de la máquina de estados
StateMachine player_state_machine;

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
}

void play_note(u8 nnote)    // Handle new musical note input and update character state
{
    // Prevenir notas demasiado rápidas
    if (note_playing_time < 5) {
        return;
    }

    // Usar la máquina de estados para procesar la nota
    if (!player_state_machine.pattern_system.is_note_playing) {
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
        
        // Enviar mensaje después de actualizar las notas
        StateMachine_SendMessage(&player_state_machine, MSG_NOTE_PLAYED, nnote);
        show_note(nnote, true);
        
        // Mantener la compatibilidad con el sistema actual
        note_playing = nnote;
        note_playing_time = 0;
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
    
    // Mantener la lógica existente temporalmente
    bool is_reverse_match;
    u8 matched_pattern;

    switch (obj_character[active_character].state)
    {
        case STATE_PLAYING_NOTE:
            if (note_playing_time != 0) {
                if (note_playing_time == calc_ticks(MAX_NOTE_PLAYING_TIME)) {
                    update_pattern_state();
                }
                else note_playing_time++;
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
                matched_pattern = validate_pattern_sequence(played_notes, &is_reverse_match);
                hide_rod_icons();

                if (matched_pattern != PTRN_NONE) {
                player_pattern_effect_reversed = false;
                
                // Handle thunder pattern
                if (matched_pattern == PTRN_ELECTRIC && !is_reverse_match && can_use_electric_pattern()) {
                    kprintf("Thunder spell detected! Configuring callbacks");
                    // Configurar callbacks para el patrón eléctrico
                    player_state_machine.launch_effect = electric_pattern_launch;
                    player_state_machine.do_effect = electric_pattern_do;
                    player_state_machine.finish_effect = electric_pattern_finish;
                    kprintf("Callbacks configured - launch: %d, do: %d, finish: %d",
                           player_state_machine.launch_effect != NULL,
                           player_state_machine.do_effect != NULL,
                           player_state_machine.finish_effect != NULL);
                    // Enviar mensaje para activar el efecto
                    StateMachine_SendMessage(&player_state_machine, MSG_PATTERN_COMPLETE, PTRN_ELECTRIC);
                }
                // Handle hide pattern
                else if (matched_pattern == PTRN_HIDE && !is_reverse_match && can_use_hide_pattern()) {
                    kprintf("Hide spell!");
                    launch_hide_pattern();
                }
                // Handle sleep pattern
                else if (matched_pattern == PTRN_SLEEP && !is_reverse_match && can_use_sleep_pattern()) {
                    kprintf("Sleep spell!");
                    launch_sleep_pattern();
                }
                // Handle open pattern
                else if (matched_pattern == PTRN_OPEN && !is_reverse_match && can_use_open_pattern()) {
                    kprintf("Open spell!");
                    launch_open_pattern();
                }
                // Handle thunder counter (reverse thunder during enemy thunder)
                else if (matched_pattern == PTRN_ELECTRIC && is_reverse_match && 
                         player_pattern_effect_in_progress == PTRN_NONE && 
                         is_combat_active && enemy_attacking != ENEMY_NONE && 
                         enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
                    kprintf("Reverse thunder spell detected");
                    obj_character[active_character].state = STATE_PATTERN_EFFECT;
                    player_pattern_effect_in_progress = PTRN_ELECTRIC;
                    player_pattern_effect_reversed = true;
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
            if (player_pattern_effect_in_progress == PTRN_HIDE) {
                do_hide_pattern_effect();
            }
            else if (player_pattern_effect_in_progress == PTRN_ELECTRIC) {
                do_electric_pattern_effect();
            }
            else if (player_pattern_effect_in_progress == PTRN_SLEEP) {
                do_sleep_pattern_effect();
            }
            break;

        case STATE_PATTERN_EFFECT_FINISH:
            if (player_pattern_effect_in_progress == PTRN_HIDE) {
                finish_hide_pattern_effect();
            }
            else if (player_pattern_effect_in_progress == PTRN_ELECTRIC) {
                finish_electric_pattern_effect();
            }
            else if (player_pattern_effect_in_progress == PTRN_SLEEP) {
                finish_sleep_pattern_effect();
            }
            obj_character[active_character].state = STATE_IDLE;
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
    
    // Inicializar patrones existentes
    obj_pattern[PTRN_ELECTRIC] = (Pattern) {true, {1,2,3,4}, NULL, NULL};
    obj_pattern[PTRN_HIDE] = (Pattern) {true, {2,5,3,6}, NULL, NULL};
    obj_pattern[PTRN_OPEN] = (Pattern) {true, {2,3,3,2}, NULL, NULL};
    obj_pattern[PTRN_SLEEP] = (Pattern) {true, {2,1,6,4}, NULL, NULL};

    kprintf("Patterns initialized with active status: %d %d %d %d",
            obj_pattern[PTRN_ELECTRIC].active,
            obj_pattern[PTRN_HIDE].active,
            obj_pattern[PTRN_OPEN].active,
            obj_pattern[PTRN_SLEEP].active);

    // Inicializar máquina de estados
    StateMachine_Init(&player_state_machine, active_character);
    
    // Configurar patrones disponibles - asignar el puntero al array
    player_state_machine.pattern_system.available_patterns = &obj_pattern[0];
    player_state_machine.pattern_system.pattern_count = MAX_PATTERNS;
    
    kprintf("State machine initialized with entity_id: %d", active_character);
}

bool validate_pattern_sequence(u8 *notes, bool *is_reverse)    // Check if played notes match any known pattern
{
    u8 npattern, nnote;
    u8 matches, reverse_matches;
    
    for (npattern = 0; npattern < MAX_PATTERNS; npattern++) {
        matches = 0;
        reverse_matches = 0;
        // Check both forward and reverse pattern matches
        for (nnote = 0; nnote < 4; nnote++) {
            if (notes[nnote] == obj_pattern[npattern].notes[nnote]) {
                matches++;
            }
            if (notes[nnote] == obj_pattern[npattern].notes[3-nnote]) {
                reverse_matches++;
            }
        }
        // Pattern found if all notes match and pattern is active
        if (matches == 4 && obj_pattern[npattern].active == true) {
            kprintf("Pattern matched (forward): %d", npattern);
            *is_reverse = false;
            player_state_machine.pattern_system.effect_reversed = false;
            return npattern;
        }
        else if (reverse_matches == 4 && obj_pattern[npattern].active == true) {
            kprintf("Pattern matched (reverse): %d", npattern);
            *is_reverse = true;
            player_state_machine.pattern_system.effect_reversed = true;
            return npattern;
        }
    }
    return PTRN_NONE;
}

bool can_use_electric_pattern(void)
{
    if (is_combat_active && enemy_attacking != ENEMY_NONE && 
        enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
        // Can't use thunder during enemy thunder attack
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][6]); // (ES) "Si reproduzco al revés|las notas, podré|contraatacar este hechizo" - (EN) "If I play the notes backwards|I could be able to|counter the spell"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
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
    // Resetear estado solo cuando se completa un patrón
    if (num_played_notes == 4) {
        // Limpiar notas visuales
        for (u8 i = 0; i < 4; i++) {
            show_note(played_notes[i], false);
        }
        
        // Resetear contadores
        num_played_notes = 0;
        time_since_last_note = 0;
        player_state_machine.pattern_time = 0;
        player_state_machine.note_count = 0;
    }
}

void handle_pattern_timeout(void)    // Check for pattern sequence timeout
{
    if (time_since_last_note != 0) {
        time_since_last_note++;
        if (time_since_last_note == calc_ticks(MAX_PATTERN_WAIT_TIME)) {
            // Limpiar estado primero
            reset_pattern_state();
            
            // Limpiar visuales
            hide_rod_icons();
            for (u8 i = 0; i < 4; i++) {
                show_note(i+1, false);
            }
            
            // Notificar timeout a la máquina de estados
            StateMachine_SendMessage(&player_state_machine, MSG_PATTERN_TIMEOUT, 0);
        }
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
    
    if (num_played_notes == 4) {
        // La validación del patrón ahora se maneja en validate_pattern_sequence
        // que enviará MSG_PATTERN_COMPLETE si corresponde
        obj_character[active_character].state = STATE_PATTERN_CHECK;
    } else {
        obj_character[active_character].state = STATE_IDLE;
    }
    update_character_animation();
}

void launch_electric_pattern(void)    // Start electric pattern effect
{
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
    anim_character(active_character, ANIM_MAGIC);
    show_pattern_icon(PTRN_ELECTRIC, true, true);
    SPR_update();
    play_pattern_sound(PTRN_ELECTRIC);
    
    // Inicializar estado del efecto
    player_pattern_effect_in_progress = PTRN_ELECTRIC;
    player_pattern_effect_time = 0;
    
    // Configurar estado en la máquina de estados
    player_state_machine.pattern_system.effect_in_progress = true;
    player_state_machine.pattern_system.effect_type = PTRN_ELECTRIC;
}

void do_electric_pattern_effect(void)    // Process electric pattern visual and combat effects
{
    // Visual thunder effect
    VDP_setHilightShadow((player_pattern_effect_time % 2) == 0);
    SPR_update();
    SYS_doVBlankProcess();
    
    // Apply combat effects
    if (is_combat_active && player_pattern_effect_time == 10) {
        for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active &&
                obj_enemy[nenemy].class_id == ENEMY_CLS_3HEADMONKEY &&
                obj_enemy[nenemy].hitpoints > 0) {
                hit_enemy(nenemy);
                if (enemy_attack_pattern == PTRN_EN_BITE) {
                    enemy_attack_pattern = PTRN_EN_NONE;
                    finish_enemy_pattern_effect();
                }
            }
        }
    }

    player_pattern_effect_time++;
    if (player_pattern_effect_time >= 20) {
        show_pattern_icon(PTRN_ELECTRIC, false, false);
        SPR_update();
        obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
    }
}

void finish_electric_pattern_effect(void)    // Clean up electric pattern state
{
    show_pattern_icon(PTRN_ELECTRIC, false, false);
    SPR_update();
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    update_character_animation();
}

void launch_hide_pattern(void)    // Start hide pattern effect
{
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
    anim_character(active_character, ANIM_MAGIC);
    show_pattern_icon(PTRN_HIDE, true, true);
    play_pattern_sound(PTRN_HIDE);
    movement_active = true;  // Allow movement while hidden
    player_pattern_effect_in_progress = PTRN_HIDE;
    player_pattern_effect_time = 1;
}

void do_hide_pattern_effect(void)    // Process hide pattern transparency effect
{
    u16 max_effect_time = 400;
    movement_active = true;  // Keep movement enabled during effect
    
    if (player_pattern_effect_time != max_effect_time) {
        // Create flickering effect
        if (player_pattern_effect_time % 2 == 0) {
            show_character(active_character, true);
        } else {
            show_character(active_character, false);
        }
        player_pattern_effect_time++;
    }
    else {
        // Effect complete
        show_pattern_icon(PTRN_HIDE, false, false);
        show_character(active_character, true);
        player_pattern_effect_in_progress = PTRN_NONE;
        player_pattern_effect_time = 0;
        obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
        anim_character(active_character, ANIM_IDLE);
    }
}

void finish_hide_pattern_effect(void)    // Clean up hide pattern state
{
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
    movement_active = false;
}

void launch_sleep_pattern(void)    // Start sleep pattern effect
{
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
    show_pattern_icon(PTRN_SLEEP, true, true);
    player_pattern_effect_in_progress = PTRN_SLEEP;
    player_pattern_effect_time = 1;
}

void do_sleep_pattern_effect(void)    // Process sleep pattern effect
{
    // Effect complete
    show_pattern_icon(PTRN_SLEEP, false, false);
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
    obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
}

void finish_sleep_pattern_effect(void)    // Clean up sleep pattern state
{
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
}

void launch_open_pattern(void)    // Start open pattern effect
{
    obj_character[active_character].state = STATE_PATTERN_EFFECT;
    show_pattern_icon(PTRN_OPEN, true, true);
    player_pattern_effect_in_progress = PTRN_OPEN;
    player_pattern_effect_time = 1;
}

void do_open_pattern_effect(void)    // Process open pattern effect
{
    // Effect complete
    show_pattern_icon(PTRN_OPEN, false, false);
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
    obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
}

void finish_open_pattern_effect(void)    // Clean up open pattern state
{
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
}
