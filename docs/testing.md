# Testing — smoke ROM y checklist de playtest

The Weave se valida por dos vías complementarias:

- **Smoke ROM** — hechizos y mecánicas de motor probados de forma **aislada y
  automatizable** (incluida una suite AUTO que corre sin mando).
- **Checklist de playtest** — las **escenas** del juego, que hoy se juegan a mano
  (meterlas en la verificación desatendida es trabajo futuro, ver el TODO al final).

## Smoke ROM

```bash
./build-theweave.sh smoke        # compila out/smoke.bin y lo lanza
```

La smoke ROM (`src/smoke/`, compilada con `-DHACK_SMOKE_BUILD`) arranca en un menú
que permite ejecutar cada hechizo y cada escena de forma aislada, sin jugar el
juego entero. Controles: UP/DOWN navegar · A ejecutar · (al terminar un caso se
vuelve al menú).

### Tipos de caso

Los tipos viven en el enum `SmokeKind` (`src/smoke/smoke_cases.h`):

- **AUTO** — recorrido desatendido por **cada mecánica fundamental** del juego (ver
  "La suite AUTO" abajo). Es la fila 0 del menú y **corre sola al arrancar** si no
  pulsas A en unos segundos: así la ROM se valida sin mando, a mano o vía RetroArch.
- **CHECK** — invariante automático de `canUse` (p.ej. "fire sin zona → rechazado",
  "fire en `ZONE_CAULDRON` → permitido"). PASS/FAIL instantáneo en pantalla, sin
  render. La suite AUTO ejecuta **todas** las filas CHECK de la tabla.
- **CAST** — lanza el hechizo con un cast scripted en un nivel de pruebas y mide los
  frames hasta terminar. PASS si acaba en `baseDuration ± 2`; el efecto visual lo
  juzga el humano. Cada hechizo se prueba en su **contexto real**: el TRUENO en el
  bosque OSCURO (`forest_dark_pal`), donde su destello cambia toda la paleta; el resto
  en el bosque de día.
- **SCENE** — ejecuta la escena completa (interactiva: se juega). Al terminar vuelve
  al menú. Nota: `act1_forest` (vieja demo técnica, fuera del flujo del juego) termina
  con reset de consola — es su final real, comportamiento esperado.

### Añadir un caso

Una fila en `smoke_cases[]` (`src/smoke/smoke_cases.h`) y rebuild. Sin tocar el menú
ni el runner. El total es `SMOKE_CASE_COUNT` (se calcula solo del tamaño de la tabla);
para saber cuántos casos de cada tipo hay hoy, cuenta las filas por su `SmokeKind`.
Un `SMOKE_CHECK` nuevo entra **solo** en la suite AUTO.

### La suite AUTO

La fila AUTO (`smoke_run_auto`, `src/smoke/smoke_runner.c`) hace, sin tocar el mando:

1. **Invariantes `canUse`**: ejecuta todas las filas `SMOKE_CHECK` de la tabla y
   cuenta PASS/FAIL.
2. **Recorrido de movimiento**: Linus aparece con su vara (`player_has_rod`, trampa de
   AGENTS.md §7), espera en reposo y camina a derecha e izquierda por el bosque
   (movimiento + scroll).
3. **Casts**: castea `SPELL_LIGHT` y `SPELL_THUNDER` (cast scripted; PASS si cada uno
   termina en `baseDuration ± 2` frames).
4. **Combate**: libra un combate contra un `WeaverGhost` que toca sus notas y lanza su
   rayo; el jugador lo contrarresta con **trueno invertido** (counter) y el enemigo
   muere. Cada espera lleva una guarda de frames: si la IA no progresa, el combate se
   marca FAIL y la suite sigue (nunca se cuelga).
5. **Pantalla de resultados** con el desglose por bloque (`CHECKS n OK / m FAIL`,
   `WALK`, `CAST LIGHT/THUNDER` con los frames medidos, `COMBAT`) y la línea final
   `RESULT: ALL PASS` / `RESULT: FAIL`.

Durante el recorrido la ROM publica su estado en globales de WRAM: `smoke_phase` (la
fase actual; `0xFFFF` = resultados en pantalla), `smoke_gate` (puerta de
sincronización que el host abre escribiendo un valor no nulo) y `smoke_scratch` (buffer
libre para probar `write_ram`). Así un host puede leerlas por RAM y sincronizar
capturas. El detalle del contrato de marcadores, del gate y del driver que conduce la
ROM desde el host está en **`docs/retroarch-mcp.md`** (§9-10).

## Checklist de playtest

Las escenas se validan jugándolas. El guion y el estado escena por escena están en
`docs/acto1.md`; esto es el recorrido mínimo de regresión del acto 1 completo
(arranca en `act1_bedroom`, o usa `HACK_START_SCENE`/la smoke ROM para una escena
suelta):

- **Intro + logo**: GeeseBumps y la intro; selección de idioma (ES/EN).
- **act1_bedroom**: cisne (flash-aparición-diálogo-flash); items en cualquier orden y
  repetibles; la partitura enseña DORMIR (teórico); cierra con la llamada de Clio.
- **act1_corridor**: los dos libros (texto largo la 1.ª vez, corto después); no deja
  salir sin leer ambos.
- **act1_hall**: diálogos con Clio y Xander; dos choices — la respuesta debe
  corresponder a la opción elegida.
- **act1_coast**: tramo de paso; la gaviota alza el vuelo a la altura del árbol rojo
  (objeto examinable).
- **act1_island**: 1.er combate de **contacto** (jabalíes, golpe con A); tras la
  pelea, Clio canta CURACIÓN (jingle + notas + menú de pausa), aprendida pero no
  jugable aún.
- **act1_hut**: coger el bastón dispara la cutscene (rayo, patrón ELECTRICIDAD, voz de
  Bobbin); entra el **límite de notas** (una nota por encima de SOL avisa y cancela).
- **act1_return**: mismo bosque de derecha a izquierda (scroll `user_left`); combate de
  contacto con arma **TRUENO** y combate de **patrones** contra los espectros; ambos
  con reintento por derrota (mensaje de Bobbin al caer).
- **act1_coast_end**: cierre en la nave; pausa interactiva, fundido a negro y susurro
  final. FIN DEL ACTO 1.
- **Comprobar en todo el recorrido**: caracteres especiales (ñ, tildes, ¿¡) correctos
  en ES y EN; contador de vida y reintentos de combate; que el HUD no se solape con
  los diálogos.

Además, `act1_test` (fuera del flujo) es el banco de pruebas del motor: ejercita todos
los ops del DSL. Su desglose sección a sección está en `docs/test_scene.md`.

## Referencia

Las comparaciones entre builds son **funcionales/visuales**, nunca binarias:
`GAMEVERSION` incrusta `__DATE__`, así que dos builds de días distintos nunca son
byte-idénticos.

## Lo que queda por hacer (testing)

- **Meter las escenas en la verificación desatendida**: hoy la smoke AUTO cubre
  hechizos, movimiento y combate de patrones, pero las **escenas** (`SCENE`) son
  interactivas y solo se validan con el checklist de playtest manual de arriba.
  Automatizarlas necesita un embudo de input + override por RAM para pilotar la ROM;
  el diseño está en `docs/testing/plan.md` (Fases 3 y 6).
- **Menú de debug runtime**: los toggles de `hack.h` son compile-time; un menú oculto
  para cambiarlos en caliente es la Fase 4 de `docs/testing/plan.md`.
- **Capturas de referencia de playtest**: pendientes de añadir en `docs/testing/`
  (hacerlas en el próximo playtest completo).
