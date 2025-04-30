# Plan Final para la Máquina de Estados y Sistema de Combate

## Resumen

Este documento presenta el plan final para completar la implementación de la máquina de estados y el sistema de combate en "The Weave". La implementación se basa en los documentos previos y las discusiones mantenidas, consolidando toda la información en un único plan coherente.

## Arquitectura General

La arquitectura se basa en los siguientes componentes principales:

1. **Máquina de Estados (StateMachine)**: Componente central que gestiona los estados de personajes y enemigos.
2. **Sistema de Patrones**: Gestiona los patrones de notas que pueden ejecutar tanto el jugador como los enemigos.
3. **Sistema de Combate**: Coordina las interacciones entre personajes y enemigos durante el combate.

## Implementación de la Máquina de Estados

### Estructura Principal (statemachine.h/c)

```c
typedef struct StateMachine {
    // Estados y temporizadores base
    SM_State current_state;      // Estado actual
    u16 timer;                   // Temporizador general
    
    // Sistema de notas
    u8 notes[4];                 // Notas del patrón actual
    u8 note_count;               // Número de notas reproducidas
    u8 current_note;             // Nota actual que se está reproduciendo
    u16 note_time;               // Tiempo que lleva reproduciendo la nota
    u16 pattern_time;            // Tiempo desde la última nota
    
    // Sistema de patrones base
    u16 active_pattern;          // Patrón activo (si hay alguno)
    bool is_reversed;            // Si el patrón es invertido
    u16 effect_time;             // Tiempo que lleva el efecto activo
    u16 entity_id;               // ID de la entidad (jugador o enemigo)
    
    // Sistema de patrones expandido
    PatternSystem pattern_system;
    
    // Callbacks para efectos específicos
    EffectCallback launch_effect;  // Función para iniciar un efecto
    EffectCallback do_effect;      // Función para procesar un efecto
    EffectCallback finish_effect;  // Función para finalizar un efecto
} StateMachine;
```

### Estados de la Máquina

```c
typedef enum {
    SM_STATE_IDLE,               // Sistema inactivo
    SM_STATE_PLAYING_NOTE,       // Reproduciendo una nota
    SM_STATE_PATTERN_CHECK,      // Verificando si el patrón es válido
    SM_STATE_PATTERN_EFFECT,     // Ejecutando el efecto del patrón
    SM_STATE_PATTERN_EFFECT_FINISH, // Finalizando el efecto del patrón
    SM_STATE_ATTACK_FINISHED     // Ataque finalizado, en enfriamiento
} SM_State;
```

### Mensajes para Comunicación

```c
typedef enum {
    MSG_PATTERN_COMPLETE,        // Patrón completado
    MSG_COMBAT_START,            // Inicio de combate
    MSG_COMBAT_END,              // Fin de combate
    MSG_ENEMY_DEFEATED,          // Enemigo derrotado
    MSG_PLAYER_HIT,              // Jugador golpeado
    MSG_ENEMY_HIT,               // Enemigo golpeado
    MSG_NOTE_PLAYED,             // Nota reproducida
    MSG_PATTERN_TIMEOUT          // Tiempo de espera del patrón agotado
} MessageType;
```

## Flujo de Ejecución

1. **Inicialización**:
   - Se inicializan las máquinas de estado para el jugador y los enemigos.
   - Se configuran los patrones disponibles.

2. **Durante el Combate**:
   - En cada frame, se llama a combat_update para actualizar todas las máquinas de estado.
   - Las máquinas de estado procesan los mensajes y actualizan sus estados.
   - Los callbacks de los patrones se ejecutan según el estado actual.

3. **Reproducción de Notas**:
   - Cuando el jugador reproduce una nota, se envía un mensaje MSG_NOTE_PLAYED a la máquina de estados.
   - La máquina de estados actualiza su estado a SM_STATE_PLAYING_NOTE.

4. **Verificación de Patrones**:
   - Después de reproducir 4 notas, se verifica si forman un patrón válido.
   - Si se encuentra un patrón, se envía un mensaje MSG_PATTERN_COMPLETE a la máquina de estados.

5. **Ejecución de Efectos**:
   - La máquina de estados asigna los callbacks correspondientes según el tipo de patrón.
   - Se ejecutan los callbacks para iniciar, procesar y finalizar el efecto.

6. **Contraataque**:
   - Si el jugador reproduce el patrón eléctrico invertido durante un ataque eléctrico enemigo, se activa el contraataque.
   - Se muestra un mensaje de éxito y se golpea al enemigo.

## Correcciones Adicionales

### 1. Mejora de la Detección de Contraataques

Se ha mejorado la detección de contraataques para asegurarse de que solo se puedan realizar durante un ataque enemigo en progreso:

```c
// Handle thunder counter (reverse thunder during enemy thunder)
else if (matched_pattern == PTRN_ELECTRIC && is_reverse_match) {
    kprintf("Reverse thunder spell detected! Combat active: %d, Enemy attacking: %d, Effect in progress: %d", 
            is_combat_active, enemy_attacking, enemy_attack_effect_in_progress);
    
    if (is_combat_active && enemy_attacking != ENEMY_NONE && enemy_attack_effect_in_progress) {
        kprintf("COUNTER-ATTACK ACTIVATED!");
        
        // Mostrar mensaje de éxito
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "¡Contraataque!" - (EN) "Counter-attack!"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        
        // Activar el contraataque
        StateMachine_SendMessage(&player_state_machine, MSG_PATTERN_COMPLETE, PTRN_ELECTRIC);
        obj_character[active_character].state = STATE_PATTERN_EFFECT;
        player_pattern_effect_in_progress = PTRN_ELECTRIC;
        player_pattern_effect_reversed = true;
        
        // Establecer la bandera de contraataque exitoso
        counter_spell_success = true;
        
        // Golpear al enemigo
        hit_enemy(enemy_attacking);
        
        // Resetear el estado del enemigo
        enemy_attack_effect_in_progress = false;
        enemy_attack_effect_time = 0;
        enemy_attack_pattern = PTRN_EN_NONE;
        
        // Asegurarse de que el efecto visual se desactive
        VDP_setHilightShadow(false);
        
        // Limpiar el estado del patrón
        reset_pattern_state();
        
        // Actualizar sprites
        SPR_update();
    } else {
        // Si no estamos en combate o no hay un ataque en progreso, mostrar mensaje de error
        kprintf("Reverse thunder spell not usable in current context");
        show_pattern_icon(matched_pattern, true, true);
        play_pattern_sound(PTRN_NONE);
        show_or_hide_interface(false);
        talk_dialog(&dialogs[SYSTEM_DIALOG][0]); // (ES) "No puedo usar ese patrón|ahora mismo" - (EN) "I can't use that pattern|right now"
        show_or_hide_interface(true);
        show_pattern_icon(matched_pattern, false, false);
        obj_character[active_character].state = STATE_IDLE;
    }
}
```

### 2. Mejora de la Función play_note

Se ha mejorado la función play_note para que sea más fiable en la detección de la primera nota:

```c
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
        
        // Enviar mensaje después de actualizar las notas
        StateMachine_SendMessage(&player_state_machine, MSG_NOTE_PLAYED, nnote);
    }
}
```

### 3. Corrección del Efecto Visual del Patrón Eléctrico

Se ha corregido el problema con el fondo oscuro que no se restablecía correctamente después de un contraataque:

```c
void do_electric_pattern_effect(void)    // Process electric pattern visual and combat effects
{
    // ...
    
    // Asegurarse de que el efecto visual se desactive
    VDP_setHilightShadow(false);
    show_pattern_icon(PTRN_ELECTRIC, false, false);
    SPR_update();
    
    // ...
}

void finish_electric_pattern_effect(void)    // Clean up electric pattern state
{
    // Asegurarse de que el efecto visual se desactive
    VDP_setHilightShadow(false);
    show_pattern_icon(PTRN_ELECTRIC, false, false);
    SPR_update();
    
    // ...
}
```

### 4. Mejora del Ataque Enemigo para Permitir Contraataques

Se ha modificado la función do_electric_enemy_pattern_effect para que el ataque enemigo no termine si el jugador está tocando notas, dándole tiempo para completar un contraataque:

```c
void do_electric_enemy_pattern_effect(void)    // Process electric pattern effect and counter-spell checks
{
    // ...
    
    // Check if player is currently playing notes
    bool player_is_playing_notes = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                  num_played_notes > 0);
    
    // ...
    
    // If player is playing notes, extend the effect time to give them a chance to counter
    if (player_is_playing_notes && enemy_attack_effect_time >= calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30) {
        // Keep the effect going a bit longer
        enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30;
        kprintf("Player is playing notes - extending enemy attack time");
    }
}
```

## Conclusión

La implementación de la máquina de estados y el sistema de combate proporciona una arquitectura modular, mantenible y extensible para el juego. Los cambios realizados han corregido los problemas identificados y mejorado la funcionalidad general del sistema.

La arquitectura permite añadir fácilmente nuevos patrones y comportamientos en el futuro, y mantiene la compatibilidad con el código existente mientras añade la nueva funcionalidad basada en máquinas de estado.

Las correcciones adicionales han mejorado la detección de contraataques y la fiabilidad en la reproducción de notas, lo que proporciona una experiencia de juego más fluida y satisfactoria.

### 5. Prevención de Daño Durante Contraataques

Se ha modificado la función finish_electric_enemy_pattern_effect para que no haga daño al jugador si está tocando notas, dándole tiempo para completar un contraataque:

```c
void finish_electric_enemy_pattern_effect(void)    // Complete electric pattern and apply damage if not countered
{
    kprintf("FINISH_ELECTRIC_ENEMY_PATTERN_EFFECT called: enemy=%d", enemy_attacking);
    
    VDP_setHilightShadow(false);
    
    // Check if player is currently playing notes
    bool player_is_playing_notes = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                  num_played_notes > 0);
    
    if (enemy_attacking != ENEMY_NONE) {
        if (player_is_playing_notes) {
            // Player is still trying to counter, give them more time
            kprintf("Player is still playing notes - delaying damage");
            
            // Extend the effect time to give them a chance to counter
            enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30;
            
            // Don't apply damage yet
            return;
        } else {
            // Player failed to counter
            hit_caracter(active_character);
            show_or_hide_interface(false);
            show_or_hide_enemy_combat_interface(false);
            talk_dialog(&dialogs[ACT1_DIALOG3][2]); // (ES) "Eso ha dolido" - (EN) "That hurts"
            
            // Only show the hint if they haven't successfully countered before
            if (!counter_spell_success) {
                talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "Quizá deba pensar|al revés" - (EN) "I should maybe|think backwards"
            }
            
            show_or_hide_interface(true);
            show_or_hide_enemy_combat_interface(true);
        }
    }
    
    kprintf("FINISH_ELECTRIC_ENEMY_PATTERN_EFFECT completed");
}
```

Este cambio asegura que el jugador tenga suficiente tiempo para completar un contraataque, y no reciba daño mientras está intentando tocar las notas del contraataque.

### 6. Corrección del Mensaje de Contraataque

Se ha modificado la función do_electric_enemy_pattern_effect para mostrar el mensaje correcto cuando se lanza un contraataque con éxito:

```c
void do_electric_enemy_pattern_effect(void)    // Process electric pattern effect and counter-spell checks
{
    // ...
    
    // Check for player counter-spell
    if (player_pattern_effect_in_progress == PTRN_ELECTRIC && player_pattern_effect_reversed == true) {
        // Set the counter-spell success flag
        counter_spell_success = true;
        
        // Stop all visual effects immediately
        VDP_setHilightShadow(false);
        
        // Show success message
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "¡Contraataque!" - (EN) "Counter-attack!"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        
        // Damage the enemy
        hit_enemy(enemy_attacking);
        
        // ...
    }
    
    // ...
}
```

Este cambio asegura que se muestre el mensaje "¡Contraataque!" en lugar de "Quizá deba pensar al revés" cuando el jugador lanza un contraataque con éxito.

### 7. Corrección de la Bandera de Contraataque Exitoso

Se ha modificado la función check_active_character_state en character_patterns.c para asegurarnos de que se establezca correctamente la bandera counter_spell_success:

```c
// Handle thunder counter (reverse thunder during enemy thunder)
else if (matched_pattern == PTRN_ELECTRIC && is_reverse_match) {
    kprintf("Reverse thunder spell detected! Combat active: %d, Enemy attacking: %d, Effect in progress: %d",
            is_combat_active, enemy_attacking, enemy_attack_effect_in_progress);
    
    if (is_combat_active && enemy_attacking != ENEMY_NONE && enemy_attack_effect_in_progress) {
        kprintf("COUNTER-ATTACK ACTIVATED!");
        
        // Establecer la bandera de contraataque exitoso PRIMERO
        counter_spell_success = true;
        kprintf("Counter spell success flag set to: %d", counter_spell_success);
        
        // Mostrar mensaje de éxito
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "¡Contraataque!" - (EN) "Counter-attack!"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        
        // ... resto del código ...
    }
}
```

Y también se ha modificado la función finish_electric_enemy_pattern_effect para que no haga nada si ya se ha realizado un contraataque exitoso:

```c
void finish_electric_enemy_pattern_effect(void)    // Complete electric pattern and apply damage if not countered
{
    kprintf("FINISH_ELECTRIC_ENEMY_PATTERN_EFFECT called: enemy=%d, counter_success=%d",
            enemy_attacking, counter_spell_success);
    
    // If counter-spell already succeeded, don't do anything
    if (counter_spell_success) {
        kprintf("Counter spell already successful, skipping damage and dialog");
        VDP_setHilightShadow(false);
        return;
    }
    
    // ... resto del código ...
}
```

Estos cambios aseguran que no se muestre el mensaje "Quizá deba pensar al revés" cuando el jugador ya ha realizado un contraataque exitoso.