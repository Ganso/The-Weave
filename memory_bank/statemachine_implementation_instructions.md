# Instrucciones de Implementación de la Máquina de Estados

## Estado Actual

Se ha completado la implementación base del sistema de estados con las siguientes características:

1. **Estructura Expandida**
   - Sistema de patrones encapsulado en PatternSystem
   - Soporte para callbacks de efectos
   - Manejo de estados mejorado

2. **Patrón Eléctrico como Ejemplo**
   - Implementación completa del ciclo de vida (launch, do, finish)
   - Efectos visuales y de combate integrados
   - Sistema de callbacks funcionando

3. **Conversión de Estados**
   - Mapeo bidireccional entre estados del personaje y estados de la máquina
   - Actualización automática de animaciones

## Próximo Paso Recomendado

Para probar la implementación, se sugiere:

1. Modificar la función `play_note` en character_patterns.c para usar la nueva máquina de estados:
   ```c
   void play_note(u8 nnote) {
       if (!player_state_machine.pattern_system.is_note_playing) {
           StateMachine_SendMessage(&player_state_machine, MSG_NOTE_PLAYED, nnote);
           show_note(nnote, true);
       }
   }
   ```

2. Crear una instancia global de StateMachine en character_patterns.c:
   ```c
   StateMachine player_state_machine;
   ```

3. Inicializar la máquina de estados en init_patterns():
   ```c
   void init_patterns(void) {
       // Inicialización existente de patrones
       obj_pattern[PTRN_ELECTRIC] = (Pattern) {false, {1,2,3,4}, NULL};
       obj_pattern[PTRN_HIDE] = (Pattern) {false, {2,5,3,6}, NULL};
       obj_pattern[PTRN_OPEN] = (Pattern) {false, {2,3,3,2}, NULL};
       obj_pattern[PTRN_SLEEP] = (Pattern) {false, {2,1,6,4}, NULL};

       // Inicializar máquina de estados
       StateMachine_Init(&player_state_machine, active_character);
   }
   ```

4. Modificar check_active_character_state para integrar con la máquina de estados:
   ```c
   void check_active_character_state(void) {
       // Actualizar la máquina de estados
       StateMachine_Update(&player_state_machine, NULL);
       
       // Actualizar el estado del personaje
       update_character_from_sm_state(&obj_character[active_character], 
                                    player_state_machine.current_state);
   }
   ```

## Beneficios de Este Enfoque

1. **Gradual**: Permite probar la nueva implementación sin romper la existente
2. **Verificable**: Cada paso puede ser probado individualmente
3. **Reversible**: Fácil de revertir si se encuentran problemas
4. **Mantenible**: Mejor organización del código

## Consideraciones

1. **Compatibilidad**: Asegurar que los estados se mapean correctamente
2. **Timing**: Mantener la sincronización de efectos y animaciones
3. **Memoria**: Monitorear el uso de memoria con la nueva estructura
4. **Rendimiento**: Verificar que no hay impacto significativo

## Plan de Pruebas

1. **Pruebas Básicas**
   - Reproducir notas individuales
   - Verificar transiciones de estado
   - Comprobar animaciones

2. **Pruebas de Patrones**
   - Probar el patrón eléctrico completo
   - Verificar efectos visuales
   - Comprobar efectos de combate

3. **Pruebas de Error**
   - Intentar patrones inválidos
   - Verificar timeouts
   - Comprobar límites de notas

## Siguientes Pasos

Una vez que esta implementación esté funcionando:

1. Implementar los demás patrones (HIDE, SLEEP, OPEN)
2. Refinar el sistema de callbacks
3. Optimizar el rendimiento si es necesario
4. Documentar la API completa