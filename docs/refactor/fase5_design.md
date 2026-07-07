# Fase 5 — Diseño de implementación: VM de escenas + DSL

> Documento de detalle previo a la implementación (pedido expresamente por el usuario).
> Complementa `refactorizar.md` §4 con las decisiones finales tomadas al confrontar el
> diseño con el código real de `act_1.c`. Tras implementarse, sirve como referencia
> del sistema junto a AGENTS.md §"Sistema de escenas".

## 0. Objetivo y filosofía

Autorar una escena = editar `data/scenes/actN_sceneM.scene` + compilar. El DSL cubre
la **secuencia narrativa** (diálogos, choices con ramas, movimientos, esperas, combate,
transiciones). Lo que es **lógica** (setup con punteros a recursos, bucles de
interacción con items, cinemáticas con manipulación de paletas) vive en **hooks C**
pequeños en `scene_hooks.c`, invocados desde el DSL con `call`. Híbrido por diseño,
no como parche.

## 1. Archivos

| Archivo | Contenido |
|---|---|
| `data/scenes/act1_scene{1,2,3,5}.scene` | Las 4 escenas en DSL |
| `data/choices.csv` | Choices (hoy hardcoded en `narrative/texts.c`) |
| `tools/gen_scenes.py` | `*.scene` → `src/scenes/scene_data.{c,h}` (validación fatal) |
| `tools/gen_choices.py` | `choices.csv` → `src/narrative/choices_data.{c,h}` |
| `src/scenes/scene_vm.{c,h}` | Tipos + intérprete `scene_run()` |
| `src/scenes/scene_hooks.{c,h}` | Hooks C por escena + `scene_hook_table[]` |
| `src/scenes/scene_data.{c,h}` | GENERADO — steps, tabla `scenes[]`, lookup act/escena |
| `src/narrative/choices_data.{c,h}` | GENERADO — `ChoiceItem` + tabla `choices[]` |

**Ajustes sobre el plan §4** (documentados aquí, reflejados luego en refactorizar.md):
- **Sin `scene_api.c`**: los wrappers no aportaban nada — los `case` de la VM llaman
  directamente a las primitivas existentes (`talk_dialog`, `move_character`, ...).
- **Ops de puzzle (`puzzle_sequence`/`if_puzzle_solved`) DISEÑADOS pero NO
  implementados**: hoy no existe ningún puzzle de secuencia; se implementarán con la
  tabla lateral `PuzzleSeq` (ver §7) cuando el guion los pida. Evitamos código muerto
  sin test. `cast`/`wait_spell`/`zone` SÍ se implementan (base de esos puzzles).
- **`say_response`**: op nuevo no previsto — la escena 3 responde con
  `dialogs[SET][BASE + last_choice]`, y expresarlo con branches triplicaría el DSL.

## 2. Estructuras (scene_vm.h)

```c
typedef struct {
    u8  op;         // SceneOp
    s16 a, b, c, d; // argumentos según op (los no usados, 0)
} SceneStep;

typedef struct {
    const char *name;          // "act1_scene1" (para logs/smoke)
    const SceneStep *steps;    // en ROM — la VM JAMÁS escribe aquí
    u16 stepCount;
} SceneScript;

extern u8 last_choice;         // resultado del último CHOICE (RAM de la VM)
```

Los argumentos que en el fuente son constantes C (`CHR_linus`, `ACT1_DIALOG4`,
`A1D4_SLEPT_BAD`, `SPELL_SLEEP`, `HOOK_*`) **se emiten como identificadores** en el
array generado: el generador valida estructura y referencias conocidas, y el
compilador C es la segunda red (un identificador inexistente no compila). Esto evita
tablas laterales para punteros y mantiene el generador simple.

## 3. Opcodes (los que las 4 escenas reales necesitan)

| Op | Args | Semántica (bloquea = gestiona sus frames) |
|---|---|---|
| `CALL` | a=HOOK_* | `scene_hook_table[a]()` — bloqueante permitido |
| `SAY` | a=set, b=id, c=sound | `talk_dialog(&dialogs[a][b], c)` — bloquea |
| `SAY_CLUSTER` | a=set, b=startId, c=sound | `talk_cluster(...)` — bloquea |
| `SAY_RESPONSE` | a=set, b=baseId, c=sound | `talk_dialog(&dialogs[a][b + last_choice], c)` |
| `CHOICE` | a=choiceSet, b=item | `last_choice = choice_dialog(&choices[a][b])` — bloquea |
| `BRANCH` | a=valor, b=stepIdx | si `last_choice == a` → pc = b |
| `GOTO` | a=stepIdx | pc = a |
| `MOVE` | a=chr, b=x, c=y | `move_character(a,b,c)` — bloquea (B5 intencional) |
| `MOVE_INSTANT` | a=chr, b=x, c=y | `move_character_instant` |
| `SHOW` | a=chr, b=visible | `show_character` |
| `LOOK` | a=chr, b=right | `look_left(a,b)` (b sigue la semántica actual) |
| `WAIT` | a=décimas de segundo | frames = a*SCREEN_FPS/10 con `next_frame(true)` (D11: el DSL nunca expresa frames) |
| `WAIT_SCROLL` | a=offset | `while (offset_BGA < a) next_frame(true)` (escena 5) |
| `SET` | a=flag, b=on | flags: MOVEMENT, SCROLL, INTERFACE (con show_or_hide), SPELLS |
| `COMBAT` | — | `combat_init(); while (combat_state != COMBAT_NO) next_frame(true);` |
| `CAST` | a=spell, b=reversed | lanzamiento scripted (origin NARRATIVE) vía motor |
| `WAIT_SPELL` | — | espera a que el slot PLAYER quede libre |
| `ZONE` | a=ZONE_* | `spell_zone = a` (para canUse de puzzle) |
| `FADE_OUT` | a=frames | `PAL_fadeOutAll(a,false)` |
| `NEXT_SCENE` | a=act, b=scene | `end_level()` + current_act/scene = a/b + return |
| `HARD_RESET` | — | `SYS_hardReset()` (final de la demo) |
| `END` | — | return (sin end_level) |

Reglas de la VM:
- **Nunca escribe en los steps** (ROM). Estado mutable (`last_choice`, pc) en RAM.
- **No hay `next_frame` entre steps**: los ops instantáneos se encadenan; los
  bloqueantes ya gestionan sus frames (mismo comportamiento que act_1.c).
- Op desconocido → `dprintf(1,...)` y avanzar (robustez).

## 4. DSL (data/scenes/*.scene)

```
# comentarios con #
scene act1_scene3            # debe coincidir con el nombre del archivo

call act1_scene3_setup       # setup C: level, personajes, posiciones

say_cluster ACT1_DIALOG2 A1D2_GUILD_YEAR sound
move CHR_linus 200 174
say_cluster ACT1_DIALOG2 A1D2_CLIO_LATE sound
...
choice ACT1_CHOICE1 0
say_response ACT1_DIALOG2 A1D2_XANDER_ABILITY sound
...
label some_point
goto some_point              # gen_scenes resuelve labels a índices
next_scene 1 5
```

- Una directiva por línea; args separados por espacios.
- `sound` / `silent` como último arg de los say (default `silent`).
- `on` / `off` para `set` (`set interface on`).
- Labels en `snake_case`; constantes en `MAYÚSCULAS` (pasan verbatim al C).

## 5. gen_scenes.py — validación (fatal, nunca warning)

1. Sintaxis: op conocido, número de args correcto.
2. `label`/`goto`/`branch`: el label existe en la escena (2 pasadas).
3. `say*`: set + id existen en `data/texts.csv` (fuente de verdad; se derivan
   `ACT1_DIALOG4` de `act1_dialog4` y los ids literales).
4. `choice`: set existe en `data/choices.csv` y el item está en rango.
5. `call`: el hook existe en `scene_hooks.h` (parsea el enum `HOOK_*`).
6. `cast`: el `SPELL_*` existe en `constants_spells.h`.
7. Nombre de escena `actN_sceneM` → tabla de lookup `{act, scene, idx}` para main.

Salida `scene_data.c`: un array `static const SceneStep act1_scene1_steps[]` por
escena + `const SceneScript scenes[]` + `const SceneScript* scene_lookup(u8 act, u8 sc)`.
Cabecera `// Generated by tools/gen_scenes.py — DO NOT EDIT`.

## 6. choices.csv y gen_choices.py

```
set,item,face,side,time,es_1,es_2,es_3,es_4,en_1,en_2,en_3,en_4
act1_choice1,0,FACE_linus,SIDE_RIGHT,DEFAULT_CHOICE_TIME,¿Los Tejedores?,...
act1_choice1,1,FACE_linus,SIDE_RIGHT,DEFAULT_CHOICE_TIME,Tengo que ir a la isla,...
```
- `num_options` se calcula contando columnas no vacías.
- Emite `choices_data.c` (arrays + tabla `choices[]` + terminador como hoy) y
  `choices_data.h` (`#define ACT1_CHOICE1 0`, movido desde texts.h).
- `narrative/texts.c` pierde los arrays hardcoded (B-pendiente del plan).

## 7. Puzzles de secuencia — IMPLEMENTADO (2026-07-07, escena act1_test)

Tabla lateral generada (los pasos no caben en 4 args s16):
```c
typedef struct { u8 tag, len; u8 spell[PUZZLE_SEQ_MAX]; u8 reversed[PUZZLE_SEQ_MAX]; } PuzzleSeq;
```
Ops `PUZZLE_SEQ a=idx` y `IF_PUZZLE_SOLVED a=tag b=target`; el progreso se registra
en `puzzle_progress[]` cuando un CAST con origin NARRATIVE termina. La base (CAST,
WAIT_SPELL, ZONE) queda implementada en esta fase.

## 8. Hooks por escena (scene_hooks.c) — mapa de migración

| Hook | Origen (act_1.c) | Contenido |
|---|---|---|
| `act1_scene1_setup` | 8-23 | new_level, set_limits, paleta swan, init chars, 5 init_item |
| `act1_scene1_swan` | 25-45 | cinemática del cisne (backup de paleta + flashes + cluster) |
| `act1_scene1_wake` | 47-61 | fade a día, say NEXT_MORNING, release_item(4), paleta chars, mostrar a Linus, movement/scroll on |
| `act1_scene1_items` | 63-101 | bucle de 4 items con flags + sleep una vez + timeout condicional |
| `act1_scene2_setup` | 111-130 | new_level pasillo + 12 init_item + init linus |
| `act1_scene2_entry` | 135-137 | move_instant + move de entrada |
| `act1_scene2_items` | 146-205 | bucle de libros/puertas/mapas + condición de salida + move out |
| `act1_scene3_setup` | 213-227 | new_level hall + 3 chars + posiciones + looks |
| `act1_scene5_setup` | 262-295 | new_level bosque + 7 items + rod + chars + follow + spell_enable ×4 + move + stop |
| `act1_scene5_pad_hint`| 299-302 | cluster condicional según tipo de mando |
| `act1_scene5_day` | 303-304 | PAL_fadeTo a paleta de día |
| `act1_scene5_enemies` | 318-334 | paleta ghosts + stop chars + init_enemy ×2 + moves |

Lo que queda en DSL por escena: scene1 = 6 steps (calls + says finales + next_scene);
scene2 = 6; **scene3 = ~20 steps, casi puro DSL** (la escena narrativa por excelencia:
clusters, moves, looks, 2 choices con say_response); scene5 = ~14 (calls + moves +
clusters + wait_scroll + set + combat + fade + hard_reset).

El diálogo dentro de los hooks (items) usa `talk_dialog` directo, como hoy: correcto —
es lógica, no secuencia.

## 9. Integración y limpieza

- `main.c`: el doble switch → `scene_run(scene_lookup(current_act, current_scene))`;
  si no hay escena, quedarse en bucle (comportamiento actual con default).
- `build-theweave.sh`: añadir `gen_choices.py` y `gen_scenes.py` al paso de codegen.
- **Eliminar**: `act_1.c/.h`, `src/globals.h` (ya sin consumidores),
  `tools/add_texts_comments.py` (decisión §15).
- `texts.h`: fuera `ACT1_CHOICE1` (va a choices_data.h).
- AGENTS.md: sección "Sistema de escenas" + receta "añadir una cutscene".

## 10. Verificación

- Build limpio + `gen_scenes.py`/`gen_choices.py` con validación fatal probada
  (romper una referencia a propósito y ver el error).
- Los steps generados de cada escena se revisan contra act_1.c línea a línea.
- Playtest del usuario: juego completo (intro → 4 escenas, ambas rutas de cada
  choice, combate win) contra `baseline.md`. La lógica portada a hooks es
  copy-paste del original → el riesgo se concentra en el orden de steps del DSL.
```

## 11. Adenda (2026-07-06): escenas por NOMBRE, no por número

A petición del usuario tras la implementación inicial:

- Escenas nombradas `<acto>_<nombre>` (act1_bedroom/corridor/hall/forest); ficheros en
  `data/scenes/<acto>/<nombre>.scene`. El SceneId numérico es un detalle interno del
  generador: NADA en el código habla de números de escena. `next_scene <nombre>`.
  `current_act`/`current_scene` eliminados → `current_scene_id` (SceneId) en la VM.
- Hooks divididos por acto y escena: `src/scenes/<acto>/<escena>.c/.h` (un par por
  escena, con su nombre — mejor que N ficheros homónimos scene_hooks.c por directorio);
  `scene_hooks.c/h` queda como enum + tabla de despacho.
- Textos renombrados con la misma filosofía: sets `act1_bedroom`... → defines
  `ACT1_BEDROOM`...; ids `A1_BEDROOM_*`, `A1_CORRIDOR_*`, `A1_HALL_*`, `A1_FOREST_*`.
  Choices: `act1_hall_choice` → `ACT1_HALL_CHOICE`. gen_texts deriva ahora el prefijo
  de TERM/COUNT de los dos primeros tokens (`A1_BEDROOM`), no del primero.
- `HACK_START_SCENE` pasa a ser un string con el nombre de escena ("" = off).
- El makefile.gen de SGDK 2.12 ya wildcardea hasta 5 niveles: src/scenes/act1/ compila
  sin tocar el Makefile.
