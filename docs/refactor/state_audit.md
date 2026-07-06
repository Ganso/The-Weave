# Auditoría del estado inicial — The Weave (pre-refactor)

> Generado en la Fase 0 del refactor (2026-07-06). Refleja el estado del código en la
> rama `master` (commit `eaa9a4e`) al crear la rama `refactor`.
> Referenciado desde `refactorizar.md` §0.

## Métricas

- **53 archivos** fuente en `src/` (`.c`, `.h`, `.s`), **~6.452 líneas**.
- Módulos más grandes: `patterns.c` (674), `dialogs.c` (648), `interface.c` (614),
  `act_1.c` (346), `controller.c` (261), `combat.c` (234).
- Sin Makefile propio: build vía Docker con script externo (ver "Build actual").

## Build actual (documentado antes de tocar nada)

- **Script**: `~/codigo/build-sgdk.sh` (externo al repo, aunque con `PROJECT_DIR`
  hardcodeado a The-Weave). Invocado desde `.vscode/tasks.json`.
- **Mecanismo**: Docker, imagen oficial `ghcr.io/stephane-d/sgdk:latest`, montando el
  proyecto en `/src` con `-u $(id -u):$(id -g)`.
- **Modos**: `build` (incremental, default) / `release` (clean + release) / `clean`.
- **Salida**: `out/rom.bin`.
- **Emulador de pruebas**: BlastEm (task "Run ROM (BlastEm)" en VS Code).
- Docker funciona sin sudo en esta máquina (el `sudo` del script es innecesario aquí).

## Arquitectura actual

### Modelo de includes

Cada `.c` incluye **solo** `globals.h` (75 líneas), que arrastra: `<genesis.h>`, macro
`GAMEVERSION` (statement-expression GNU con `__DATE__`), `dprintf` + `DEBUG_LEVEL`,
los 11 `res_*.h`, y los `.h` de todos los módulos en orden de dependencia. Consecuencias:

- Cualquier cambio de cabecera recompila todo.
- Ningún `.c` declara sus dependencias reales.
- `SCREEN_FPS` es una **variable runtime** (`extern u8`, detección PAL/NTSC), no macro.

### Subsistemas

| Módulo | Responsabilidad | Notas |
|---|---|---|
| `globals.c/h` | umbrella + `next_frame`, `wait_seconds`, `calc_ticks`, `frame_counter` | a partir en el refactor |
| `main.c` | doble `switch (current_act/current_scene)` → `act_1_scene_N()` | 46 líneas |
| `entity.c/h` | `Entity` + `GameState` + `move_entity` (bloqueante por diseño) | base de todo |
| `characters/enemies/items` | encapsulan `Entity` | enemigos: HP, `EnemyMode`, `has_pattern[2]` |
| `combat.c/h` | FSM `CombatState`; `hit_enemy/hit_player/update_combat` | bugs B3, B4, B16 |
| `patterns.c/h` + `patterns/*.c` | validación de notas, tablas `playerPatterns[4]` / `enemyPatterns[MAX_ENEMIES][2]`, callbacks launch/update/canUse/onCounter | bugs B1, B2, B11, B18, B19 |
| `texts.c/h` + `texts_generated.*` | textos ES/EN generados de `texts.csv`; `encode_spanish_text` con malloc (B7) | choices hardcoded en `texts.c:8` |
| `dialogs.c/h` | `talk`, `talk_dialog`, `talk_cluster`, `choice_dialog` | free del malloc (B8) |
| `act_1.c/h` | 4 escenas hardcodeadas (1,2,3,5) con bucles de interacción | B22, B23 |
| `controller.c/h` | input, movimiento, pausa | B6, B10 |
| `interface.c/h` | HUD + pausa | B9 |
| `sound.c/h` | XGM2; jingles por pattern | B17 |
| `intro/geesebumps` | intro y logo | quedan como C |

### Sistema de patterns (hechizos) actual

- Notas `NOTE_MI..NOTE_DO`; cola de 4 notas del jugador; `validate_pattern` devuelve id
  + flag de invertido.
- **Jugador**: `playerPatterns[4]` — thunder (MI FA SOL SOL, `SCREEN_FPS*4`),
  hide (FA SOL SOL FA — palíndromo, `SCREEN_FPS*4`), open (FA SI SOL DO, 45f),
  sleep (FA MI DO LA, 75f). Todos con callbacks `canUse/launch/update`.
- **Enemigo**: `enemyPatterns[MAX_ENEMIES][2]` inicializados por enemigo —
  en_thunder (slot 0, counterable, con `onCounter`) y en_bite (slot 1, 3 notas MI SOL DO).
- **Hallazgo clave**: `update_enemy_pattern` y `try_counter_spell` hardcodean el
  pattern-slot 0 → **en_bite jamás se ejecuta en combate** (B2/B3). Decisión tomada:
  al corregir el bug, bite queda deshabilitado explícitamente (ver `refactorizar.md` §15).
- Los callbacks de pattern muestran diálogos directamente (`pattern_thunder.c:18` hint
  de WeaverGhost que además restaura `combat_state`) — acoplamiento B18.

### Flujo de combate actual

`combat_state` (IDLE → PLAYER/ENEMY_PLAYING → EFFECT...). Durante `COMBAT_STATE_ENEMY_EFFECT`
el jugador puede tocar el patrón invertido → `try_counter_spell`. **Dos efectos pueden
estar vivos a la vez** (el enemigo en efecto + el counter del jugador): el motor nuevo
necesita 2 slots (decisión D12).

### Escenas actuales

- `act_1_scene_1`: bedroom. Bucle re-entrante de 4 items (cualquier orden, repetibles),
  flags por item, activación única de sleep al tocar el cabinet, timeout de 3 s que solo
  corre si `item_interacted[3] && player_has_paused` y se resetea al interactuar
  (`act_1.c:63-101`). **No expresable en un DSL lineal** → hook C en el refactor.
- `act_1_scene_2`: bosque, movimiento + items.
- `act_1_scene_3`: combate **interactivo** contra WeaverGhost (el jugador toca notas
  libremente) con branches win/lose. → op `combat` en la VM, no `cast`.
- `act_1_scene_5`: finale con `SYS_hardReset`.

## Deuda y smells transversales

- Dos convenciones de include guards (`_FOO_H_` / `FOO_H`) — B13.
- Dos vocabularios "side", ambos `bool`, con `SIDE_none == SIDE_left == true` — B12.
- `bool` de SGDK es `u8` (`types.h:121`): `act_1.c:64` lo usa como contador de frames — B23.
- Comentarios corruptos "deTODO" (parece un sed accidental sobre "pending") — B14, B15.
- malloc/free en el hot path de textos — B7/B8; `MEM_alloc` sin check — B9.
- Números mágicos dispersos; timers mezclan ms (`calc_ticks`) y frames.
- Código muerto: `MAX_ENEMY_PATTERNS` (B11), valores de `GameState` (B20, verificar),
  clase 3-head-monkey sin sprite (B21, se elimina), `frame_counter` local (B10).

## Lista completa de bugs

Ver `docs/refactor/bugs.md` (tabla B1-B23 con grupo, estado y fix).

## Assets y herramientas de terceros

Ver sección "Acknowledgements" del README (SGDK + Docker image, BlastEm, Aseprite,
animalese.js de Acedio + librosa/NumPy/SoundFile/pydub/gTTS, Freesound, Pixabay).
