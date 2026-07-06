# AGENTS.md

> FUNDAMENTAL: Actualiza este fichero después de cada interacción si aplica.
> Mantenlo sincronizado con el estado real del código. Es la fuente de verdad
> para herramientas automáticas y para no re-investigar lo ya sabido.

## 1. Qué es el proyecto

- **The Weave**: fangame secuela de *Loom* (LucasArts) para **Sega Mega Drive / Genesis**.
- C plano compilado con **SGDK 2.x**. Docs SGDK: https://stephane-d.github.io/SGDK/
- Estado: demo técnica (acto 1, 4 escenas) en pleno **refactor** (plan en `refactorizar.md`,
  bitácora en `docs/refactor/`). Fases 0-5 completadas; pendientes: 6 (docs), 7 (smoke ROM + merge).

## 2. Cómo se compila y ejecuta

```
./build-theweave.sh [build|release|full|clean] [--no-run|-n]
```
- Pipeline: **codegen en el host** (python3 `tools/gen_texts.py` — la imagen Docker NO
  trae python3) → make dentro del contenedor `ghcr.io/stephane-d/sgdk:latest`
  (`--entrypoint make GDK=/sgdk -f /src/Makefile`) → backup rotatorio
  `out/TheWeave_<fecha>.bin` (mantiene 5) → MiSTer por FTP si `MISTER_IP` responde,
  si no BlastEm. `--no-run` para builds desatendidos.
- El `Makefile` del repo solo **envuelve** el `makefile.gen` de SGDK (`include`), que ya
  hace wildcard de `src/*.c src/*/*.c res/*.res` con `-Isrc -Ires`. Añadir un `.c` NO
  requiere tocar el Makefile.
- Config personal (MiSTer, BlastEm, SGDK local) en `.build-theweave.local.sh` (gitignored).
- Docker funciona sin sudo en esta máquina. Salida: `out/rom.bin` (~896 KB con padding).
- VS Code: tasks en `.vscode/tasks.json` apuntan al script del repo.

### Cómo debugear (flujo probado)

1. Editar **solo** `src/core/hack.h`: `HACK_START_SCENE "act1_forest"` (directo al
   combate; escenas SIEMPRE por nombre),
   `DEBUG_LEVEL 2-3` (trazas KDebug → consola de BlastEm), `HACK_ENEMIES_ONE_HP`,
   `HACK_FAST_DIALOGS`, `HACK_ALL_SPELLS`, `HACK_PLAYER_INVULNERABLE`,
   `HACK_FORCE_LANGUAGE`, `HACK_MUTE_MUSIC/SFX`.
2. Si BlastEm congela ("Read from VDP data port with invalid source"), el PC que
   imprime se localiza en `out/symbol.txt` (ordenado por dirección) — **ojo: con LTO
   la atribución puede engañar**; la última traza KDEBUG suele señalar mejor.
3. Corrupción del sprite engine (cuelgue dentro de `SPR_update`) = alguien llamó a
   `SPR_setAnim` con índice fuera de rango, usó un `Sprite*` liberado o liberó dos
   veces. Ejemplo histórico: B26 (underflow `hitpoints-1` con enemigo a 0 HP).

## 3. Estructura de directorios

```
data/            → autoría editable: texts.csv (diálogos), choices.csv,
                   scenes/<acto>/<escena>.scene
tools/           → codegen y utilidades python (gen_texts.py; voice/ = animalese)
src/
  boot/          → rom_head.c, sega.s
  core/          → main, init, controller, frame (next_frame/timing), config.h, hack.h
  world/         → background (scroll, límites)
  actors/        → entity (base), characters, enemies, items, collisions
  combat/        → FSM de combate (combat_state, hit_enemy/hit_player)
  spells/        → MOTOR DE HECHIZOS (ver §5)
  narrative/     → texts, texts_data (GEN), choices_data (GEN), dialogs, encode
  scenes/        → scene_vm (intérprete), scene_hooks (enum+tabla), scene_data (GEN),
                   <acto>/<escena>.c/.h (hooks por escena), intro, geesebumps (C)
  interface/     → HUD, pausa, contador de vida
  audio/         → sound (XGM2; jingles por spell id)
res/             → recursos SGDK (res_*.res/.h generados por rescomp) y arte fuente
smoke/           → (Fase 7) ROM de smoke test
```
(globals.h y act_1.c ya NO existen — eliminados en la Fase 5)

## 4. Trampas y datos duros (leer antes de tocar nada)

- **`bool` de SGDK es `u8`** (`~/sgdk/inc/types.h`): no usarlo como contador (bug B23).
- **`SCREEN_FPS` es una VARIABLE runtime** (50 PAL / 60 NTSC, detectada en `initialize`).
  Nada de tablas `const` con duraciones en frames: se rellenan en runtime
  (`SCREEN_FPS * n`). Milisegundos → ticks con `calc_ticks(ms)`.
- **`GAMEVERSION` incrusta `__DATE__`**: dos builds de días distintos NUNCA son
  byte-idénticos. Comparaciones de ROM solo funcionales/visuales.
- **Nunca bloquear con `while(!SPR_isAnimationDone)`** ni consultar esa función el
  mismo frame de un `SPR_setAnim` (estado obsoleto hasta `SPR_update`). Muertes y
  esperas → timers con `modeTimer` (ver bug B4 en `docs/refactor/bugs.md`).
- **`u16` y restas**: comparar antes de restar HP/contadores (underflow, B25/B26).
- `next_frame(bool interactive)` es EL latido: input+combate+animaciones+SPR_update+
  vblank. Los bloqueantes (move_entity, diálogos, wait_seconds) lo llaman por dentro.
- **Mapa real de escenas**: 1 dormitorio · 2 pasillo (libros) · 3 hall Clio/Xander
  (choices, SIN combate) · 5 bosque (tutorial + **combate contra 2 WeaverGhosts** + fin).
- El texto usa charset propio: español codificado (ñ→^ á→# é→$ í→% ó→* ú→/ ¿→< ¡→>)
  vía `encode_spanish_text` (buffer estático — consumir antes de la siguiente llamada).
  Escapes en textos: `|` salto de línea, `@[...@]` color.
- KDebug (`dprintf(nivel, ...)`) solo emite si `nivel <= DEBUG_LEVEL` (hack.h).

## 5. Sistema de hechizos (spells/) — desde la Fase 4

**Arquitectura**: tabla única `spell_defs[SPELL_COUNT]` (ids unificados en
`constants_spells.h`: THUNDER 0, HIDE 1, OPEN 2, SLEEP 3, FIRE 4 | EN_THUNDER 5,
EN_BITE 6; `SPELL_NONE` 254) + motor con **dos slots** (`SPELL_SLOT_PLAYER`,
`SPELL_SLOT_ENEMY`) — ambos pueden estar vivos a la vez (counter). `combat_state`
(combat.c) sigue siendo el director: el motor consulta/actualiza ese FSM.

- `spell.c` — motor: `spell_validate` → `spell_player_cast` (counter o launch),
  `spell_update` (cada frame desde `update_combat`), `spell_try_counter`,
  `spell_cancel`, `spell_reject` (feedback + resume del enemigo), lado enemigo
  (`spell_enemy_try_launch` con recargas por enemigo, cadencia de notas).
- `notes.c` — cola de notas del jugador (debounce `MIN_TIME_BETWEEN_NOTES`, timeout
  `MAX_PATTERN_WAIT_TIME`, lock global) y HUD de notas enemigas.
- `player_spells.c` / `enemy_spells.c` / `fire.c` — defs + hooks de cada hechizo.
- Hooks (todos opcionales): `canUse` → `onRejected` (hints con diálogo) → `onLaunch`
  → `onUpdate` (por frame; el auto-fin por `baseDuration` aplica siempre) →
  `onFinish` (SOLO fin natural) / `onCounter` (contrarrestado) / `onCancel` (cortado).
  **El motor es el único que toca frameCounter, slots y cleanup.**
- Fases declarativas (`SpellPhase`): `PHASE_VISUAL_FLASH` (continuo) y
  `PHASE_LOGIC_DAMAGE` (puntual: usar start==end). Se rellenan en runtime.
- Zona narrativa: `spell_zone` (ZONE_*) — la fijará la escena (Fase 5); los canUse
  la reciben en `ctx->zoneId`.

### Receta: añadir un hechizo nuevo

1. Copiar `src/spells/fire.c/.h` (es el ejemplo canónico comentado).
2. Añadir `SPELL_MIO` en `constants_spells.h` (antes de `SPELL_PLAYER_COUNT` si es
   de jugador) y llamar `mio_init()` desde `init_spells()` en `spell.c`.
3. Si es de jugador con icono: añadir su sprite y el caso en `show_pattern_icon`
   (interface.c). Jingle: caso en `play_spell_jingle` (sound.c).
4. Desbloqueo en juego: `spell_enable(SPELL_MIO)` (silencioso) o
   `activate_spell(SPELL_MIO)` (con jingle y notas, para cutscenes).
5. (Fase 7) añadir caso a la smoke ROM.

### Receta: añadir un enemigo

1. Clase en `actors/enemies.h` (`ENEMY_CLS_*`) + entrada en `init_enemy_classes`
   (enemies.c): HP, follow, y su lista `spell[]` ({SPELL_EN_X, SPELL_NONE}).
2. Sprite en `res/res_enemies.res` + caso en el switch de `init_enemy`.
3. `init_enemy(slot, ENEMY_CLS_*)` + `move_enemy*` en la escena; `combat_init()` arranca.
- **EN_BITE está deshabilitado a propósito** (decisión §15 refactorizar.md): para
  activarlo, añadir `SPELL_EN_BITE` a la lista `spell[]` de una clase y ajustar
  `rechargeInit` con playtest. Nota heredada: bite no aplicaba daño al terminar.

### Receta: añadir diálogos

1. Fila en `data/texts.csv` (`set,id,face,side,time,es,en`; side: SIDE_LEFT/RIGHT/NONE).
2. `python3 tools/gen_texts.py` (o compilar: el build lo lanza) → `narrative/texts_data.*`.
3. En una escena: `say SET ID sound|silent` en el `.scene`. En un hook C:
   `talk_dialog(&dialogs[SET][ID], sound)` / `talk_cluster` (encadena hasta `TERM_*`).

## 5b. Sistema de escenas (scenes/) — desde la Fase 5

**LAS ESCENAS NO TIENEN NÚMERO**: se nombran `<acto>_<nombre>` (act1_bedroom,
act1_corridor, act1_hall, act1_forest) para poder intercalar escenas sin renombrar
nada. El SceneId numérico es un identificador INTERNO generado; en código, DSL,
textos y hacks se habla siempre por nombre. Los diálogos siguen la misma filosofía:
sets `act1_bedroom`... (defines ACT1_BEDROOM...) e ids `A1_BEDROOM_*`; choices
`act1_hall_choice` (ACT1_HALL_CHOICE).

**Diseño completo**: `docs/refactor/fase5_design.md`. Filosofía HÍBRIDA: el DSL
(`data/scenes/actN_sceneM.scene`) expresa la SECUENCIA narrativa (say/say_cluster/
say_response, choice+branch, move/look/show, wait, set, combat, cast, fade_out,
next_scene); la LÓGICA (setup con punteros a recursos, bucles de items, cinemáticas
de paleta) vive en hooks C (`scene_hooks.c`) invocados con `call <hook>`.

- `gen_scenes.py` valida TODO en fatal (labels, diálogos contra texts.csv, choices
  contra choices.csv, hooks contra el enum HOOK_* de scene_hooks.h, spells/zonas) y
  emite `scene_data.c` con las constantes C verbatim — el compilador es la 2ª red.
- La VM (`scene_vm.c`) nunca escribe en los steps (ROM); `last_choice` en RAM; sin
  next_frame entre steps (los ops bloqueantes gestionan sus frames).
- `main.c` = bucle infinito sobre `scene_lookup(current_act, current_scene)`.
- Ops de puzzle (secuencias de hechizos): DISEÑADOS en fase5_design.md §7, no
  implementados aún (no hay puzzles en el guion). `cast/wait_spell/zone` sí están.
- `say_response` responde con `dialogs[set][base + last_choice]` (respuestas a choices).
- `set interface off` solo oculta el HUD (NO toca interface_active — asimetría
  deliberada, ver scene_vm.h).

### Receta: añadir una cutscene

1. Crear `data/scenes/<acto>/<nombre>.scene` (copiar act1/hall.scene como referencia
   narrativa; act1/forest.scene como referencia con combate). El nombre interno es
   `<acto>_<nombre>` y la directiva `scene` debe coincidir.
2. Si necesita setup o lógica: crear `src/scenes/<acto>/<nombre>.c/.h` con sus hooks
   + añadirlos al enum `HOOK_*` de `scene_hooks.h` y a la tabla de `scene_hooks.c`.
3. Sus textos: set `<acto>_<nombre>` en texts.csv, ids `A<n>_<NOMBRE>_*`.
4. Compilar (el build corre gen_scenes.py; valida referencias en fatal).
5. Enlazarla: `next_scene <acto>_<nombre>` desde la escena anterior. (Fase 7:
   añadir caso a la smoke ROM.)

### Receta: añadir un choice

1. Filas en `data/choices.csv` (set,item,face,side,time,es_1..4,en_1..4).
2. En el `.scene`: `choice MI_SET <item>` + `branch <n> goto <label>` o
   `say_response SET BASE_ID` si las respuestas son texto correlativo.

## 6. Reglas de codificación

- `snake_case` funciones/variables; `UPPER_CASE` macros; `PascalCase` tipos.
- Tipos SGDK (`u8/u16/u32/s8/s16/s32/bool`); nunca `int` a pelo.
- Llaves: Allman en funciones; K&R en if/for/while. 4 espacios.
- Comentario `//` junto a cada firma en los `.h`; bloque `/* */` de arquitectura al
  inicio de cabeceras de subsistema. Sin plantillas Doxygen.
- **Includes explícitos** relativos a `-Isrc -Ires` (`"spells/spell.h"`, `"res_sound.h"`).
  Orden: `<genesis.h>`, config/hack, cabecera propia, módulos, recursos.
- Sin malloc/free: pools y buffers estáticos. `static const` para datos inmutables.
- Guards `_FOO_H_`. Funciones privadas `static`. Designated initializers en tablas.
- No editar generados: `narrative/texts_data.*`, `narrative/choices_data.*`,
  `scenes/scene_data.*`, `res/*.h` de rescomp.

## 7. Pendientes conocidos

- Jingles de SLEEP y EN_BITE sin componer (TODO en `play_spell_jingle`, decisión §15).
- "Better enemy defeat handling" (B16): la muerte actual es anim hurt 1s + release;
  mejorarla (anim propia, recompensas) es diseño de juego — el motor lo soporta vía
  `onFinish`/`hit_enemy`.
- act1_scene4 no existe (hueco intencional en el guion; el DSL permite añadirla sin C
  si es lineal).
- Ops de puzzle de la VM pendientes de implementar cuando el guion los pida (§5b).

## 8. Voces y arte

- Voces animalese: `tools/voice/generate_animalese_voices.py` (descarga animalese.wav
  de github.com/Acedio/animalese.js, requiere venv con librosa/numpy/soundfile).
  Salida → `res/Sound/Dialogs/`. Perfiles woman/man/deep.
- Arte: Aseprite (`.ase` en res/). Créditos de terceros: sección Acknowledgements del README.
