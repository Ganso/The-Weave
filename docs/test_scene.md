# act1_test — banco de pruebas del motor (documento vivo)

> QUÉ ES: inventario de todo lo implementado en la escena de test y CÓMO probarlo.
> Si una sesión se corta a medias, este fichero es la fuente de verdad.
>
> CÓMO ENTRAR: `HACK_START_SCENE "act1_test"` en `src/core/hack.h` + build, o
> smoke ROM (`./build-theweave.sh smoke` → caso "SCENE act1_test (motor)").
> Tras probar con smoke: `./build-theweave.sh release` para restaurar out/.

## Estado: ESTABILIZADO (sesión 2026-07-08)

### Bugs reportados por el usuario y su arreglo (2026-07-08)
- **No se veía al personaje**: `act1_test_setup` hacía `init_character` (crea el
  sprite OCULTO) pero nunca `show_character` ni lo posicionaba. Arreglado:
  posiciona y muestra a Linus.
- **HUD tapaba los diálogos**: ahora los ops SAY/SAY_CLUSTER/SAY_RESPONSE/CHOICE
  de la VM ocultan el HUD de hechizos al hablar y lo restauran al terminar (solo
  si `interface_active`). Aplica a TODAS las escenas, no solo al test.
- **Opción de test en blanco en el menú smoke**: el menú crecía hasta filas que
  el overscan NTSC recortaba. Menú compactado (empieza en fila 0/1, casos desde
  la fila 3) y nombre acortado a "SCENE act1_test".
- **Fondo**: el test usa ahora el fondo de BOSQUE (antes el pasillo).
- **Cuelgue al tocar la primera nota del puzzle** ("unmapped read from B96608"):
  causa REAL (el auto-scroll era un problema aparte, ya corregido) — `act1_test_setup`
  fijaba `player_has_rod = true` DESPUÉS de `init_character(CHR_linus)`. Ese flag
  decide el sprite de Linus: con vara `linus_sprite` (tiene ANIM_ACTION de tocar
  notas), sin vara `linus_norod_sprite` (NO la tiene). Al quedar con el sprite sin
  vara, la primera nota → `STATE_PLAYING_NOTE` → `anim_character(..., ANIM_ACTION)`
  → `SPR_setAnim` leía una animación inexistente (puntero basura fijo, de ahí la
  dirección constante). Arreglado moviendo `player_has_rod = true` ANTES de
  `init_character`, igual que act1_forest_setup. Bisecado con marcas `F:xxx` en
  next_frame (el crash caía en `F:chranim` = update_character_animations).

## Estructura: intro + HUB de secciones

Intro corta (cluster + **wait_press**, op nuevo) → menú de 4 opciones que se
repite al acabar cada sección:

1. **Diálogos y quiz**
2. **Hechizos y puzzles**
3. **Combate** (dos oleadas)
4. **Terminar test** → fade + next_scene al dormitorio

## Qué probar, sección a sección

### Intro
- [ ] Cluster de 3 pantallas con el mapeo botón→nota.
- [ ] `wait_press`: la escena espera a que pulses A (op nuevo de la VM).

### 1. Diálogos y quiz
- [ ] Cara de **Clio a la DERECHA** y de **Xander a la izquierda** (variedad de
      face/side en DialogItem).
- [ ] `anim`: Linus hace la animación de MAGIA 1,5 s por opcode del DSL y vuelve a idle.
- [ ] **Quiz anidado** (choices dentro de choices, con bucles de reintento):
  - Q1 "¿Cuántas notas tiene un patrón?" → correcta **Cuatro** (opción 2).
    Fallar → mensaje + repetición de la pregunta (bucle goto).
  - Q2 "¿Qué hechizo es palíndromo?" → correcta **Esconderse** (opción 3).
  - Confirmación anidada "¿Estás seguro?" → **No** vuelve a Q2; **Sí** supera el quiz.
- [ ] Vuelta al hub.

### 2. Hechizos y puzzles
- [ ] Cast scripted de **sleep** (hechizo solo-guion) con `wait_spell`.
- [ ] Cast scripted de **LUZ** — hechizo NUEVO definido por fases declarativas:
      el cielo pasa a **cian** (0,75 s) y luego a **blanco** (0,75 s) y se restaura.
- [ ] Selector de puzzle (choice de 2):
  - **Puzzle 1 (básico)**: trueno → fuego → esconderse (directos). Requiere
    `zone ZONE_CAULDRON` (fire). Notas en el propio diálogo.
  - **Puzzle 2 (con INVERTIDO)**: trueno → **luz invertida (DO SI LA SOL)** →
    esconderse. Primera prueba real del matching `reversed` en PuzzleSeq.
    LUZ es el único hechizo casteable invertido libremente (sin canUse).
- [ ] Fallar la secuencia la reinicia (con crédito si el fallo es el 1er paso).
- [ ] `if_puzzle_solved` tras el puzzle 1 (el centinela "ERROR" nunca debe verse).
- [ ] Vuelta al hub (se puede entrar otra vez y elegir el otro puzzle).

### 3. Combate (dos oleadas en la misma escena)
- [ ] **Oleada 1**: WeaverGhost clásico — hint al thunder directo, counter
      (SOL SOL FA MI) durante su efecto, muerte con 2 counters.
- [ ] **Oleada 2**: **ENEMY_CLS_TESTGHOST** (clase SOLO test, sprite del ghost)
      con DOS hechizos: thunder (counterable, recarga 3 s) y **mordisco**
      (MI SOL DO, 3 notas, NO counterable, recarga 2 s, ahora hace 1 de daño).
      Verificar: alternancia de ataques por recargas, y que el counter solo
      funciona contra el thunder (contra el mordisco: esconderse o comerse el golpe).
- [ ] `combat` dos veces en la misma escena (re-init limpio del FSM).
- [ ] Vuelta al hub.

### Salida
- [ ] Opción 4 del hub: despedida + fade_out + next_scene al dormitorio.
- [ ] Desde la smoke ROM: al terminar vuelve al menú de smoke.

## Casos nuevos de smoke ROM (menú)
- [ ] `CHK light directo - SI` y `CHK light inverso - SI` (único invertido libre).
- [ ] `CAST light (cian-blanco)`: duración 90/90 frames NTSC + efecto visual.

## Cambios de motor de esta sesión (2026-07-07)

| Cambio | Archivos | Nota |
|---|---|---|
| Op `anim <chr> <ANIM_*>` | scene_vm, gen_scenes | animación de personaje desde DSL |
| Op `wait_press` | scene_vm, gen_scenes | pausa de cutscene hasta pulsar A (release previo) |
| `SPELL_LIGHT` | spells/light.c/.h, constants (ids renumerados), sound.c, spell.c | 1er hechizo por fases; 1er invertible libre |
| Mordisco funcional | enemy_spells.c | onFinish con hit_player(1) (antes no-op heredado) |
| `ENEMY_CLS_TESTGHOST` | enemies.h/.c | SOLO test; 1er enemigo multi-hechizo (recargas alternas) |
| Guarda hook NULL | scene_vm.c (op CALL) | hook sin registrar avisa por KDebug, no cuelga |
| Iconos HUD opcionales | interface.c | hechizo sin icono no se dibuja, no crashea (FIRE y LUZ ya tienen icono: F y L) |

## Ideas pendientes (no implementadas)

- Op `music`/`sfx` desde DSL (necesita tabla de recursos: los punteros no caben
  en los args s16 del SceneStep).
- Puzzle con pasos de 4 hechizos (PUZZLE_SEQ_MAX ya lo permite).
- Sección de items interactuables en el test (bucle tipo bedroom con hook).
- Derrota del jugador en combate (hoy no existe: sin HP de jugador).
