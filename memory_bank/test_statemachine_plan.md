# Plan de Pruebas de la Máquina de Estados

## Implementación Actual

Se ha implementado una integración inicial de la máquina de estados que mantiene la compatibilidad con el sistema existente mientras introduce la nueva funcionalidad:

1. **Inicialización**
   - Nueva instancia global `player_state_machine`
   - Inicialización en `init_patterns()`
   - Configuración de patrones disponibles

2. **Procesamiento de Notas**
   - Integración en `play_note()`
   - Envío de mensajes `MSG_NOTE_PLAYED`
   - Mantenimiento del estado actual para compatibilidad

3. **Validación de Patrones**
   - Actualización de `validate_pattern_sequence()`
   - Envío de `MSG_PATTERN_COMPLETE`
   - Manejo de patrones invertidos

4. **Gestión de Estados**
   - Sincronización en `reset_pattern_state()`
   - Manejo de timeouts con `MSG_PATTERN_TIMEOUT`
   - Actualización de estados del personaje

## Pruebas Recomendadas

1. **Prueba Básica de Notas**
   ```
   1. Presionar una nota
   2. Verificar:
      - Nota se muestra correctamente
      - Estado cambia a PLAYING_NOTE
      - Animación se actualiza
   ```

2. **Prueba de Patrón Eléctrico**
   ```
   1. Activar el patrón eléctrico
   2. Reproducir secuencia: 1,2,3,4
   3. Verificar:
      - Patrón se reconoce
      - Efecto visual se ejecuta
      - Daño se aplica correctamente
   ```

3. **Prueba de Timeout**
   ```
   1. Reproducir 2-3 notas
   2. Esperar MAX_PATTERN_WAIT_TIME
   3. Verificar:
      - Estado se resetea
      - Iconos se ocultan
      - Se puede empezar nuevo patrón
   ```

4. **Prueba de Patrón Invertido**
   ```
   1. Durante ataque eléctrico enemigo
   2. Reproducir secuencia: 4,3,2,1
   3. Verificar:
      - Patrón invertido se detecta
      - Contraataque funciona
   ```

## Puntos de Verificación

1. **Estados**
   - Transiciones correctas entre estados
   - Sincronización entre sistemas
   - Animaciones apropiadas

2. **Efectos Visuales**
   - Notas se muestran/ocultan correctamente
   - Iconos de patrones funcionan
   - Efectos especiales (trueno, etc.)

3. **Combate**
   - Daño se aplica correctamente
   - Interacción con enemigos funciona
   - Contraataques funcionan

4. **Rendimiento**
   - No hay pérdida de frames
   - Animaciones suaves
   - Respuesta inmediata a input

## Problemas Potenciales

1. **Timing**
   - Verificar que los tiempos de notas son correctos
   - Comprobar duración de efectos
   - Validar timeouts

2. **Estado Global**
   - Verificar que no hay conflictos entre sistemas
   - Comprobar limpieza de estado
   - Validar reinicio correcto

3. **Memoria**
   - Monitorear uso de memoria
   - Verificar liberación de recursos
   - Comprobar límites de patrones

## Siguiente Paso

Si las pruebas son exitosas:
1. Implementar los demás patrones (HIDE, SLEEP, OPEN)
2. Refinar el sistema de callbacks
3. Migrar más variables globales a la máquina de estados

Si se encuentran problemas:
1. Identificar el origen exacto
2. Implementar correcciones manteniendo compatibilidad
3. Repetir pruebas afectadas