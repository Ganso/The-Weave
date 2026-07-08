# Cutscenes — guía de autoría

Diseño completo en `docs/refactor/fase5_design.md`; arquitectura en
`src/scenes/scene_vm.h`. Filosofía **híbrida**: el `.scene` expresa la SECUENCIA
narrativa; la LÓGICA (setups con recursos, bucles de items, cinemáticas de paleta)
vive en hooks C pequeños invocados con `call`.

**Las escenas no tienen número**: se nombran `<acto>_<nombre>` (p.ej. `act1_bedroom`)
y las transiciones son por nombre — puedes intercalar escenas sin renombrar nada.

## Crear una escena (receta)

1. `data/scenes/<acto>/<nombre>.scene` — copia `act1/hall.scene` (narrativa pura)
   o `act1/forest.scene` (con combate). La directiva `scene <acto>_<nombre>` debe
   coincidir con la ruta.
2. Si necesita setup o lógica: `src/scenes/<acto>/<nombre>.c/.h` con sus hooks +
   entrada en el enum `HOOK_*` de `scene_hooks.h` + la tabla de `scene_hooks.c`.
3. Textos: set `<acto>_<nombre>` en `data/texts.csv`, ids `A<n>_<NOMBRE>_*`
   (ver docs/texts.md).
4. Enlázala: `next_scene <acto>_<nombre>` desde la escena anterior.
5. Compila — `gen_scenes.py` valida TODAS las referencias en fatal.
6. Caso en la smoke ROM (`src/smoke/smoke_cases.h`).

## Referencia del DSL

Comentarios con `#`. Una directiva por línea. Constantes C en MAYÚSCULAS pasan
verbatim (el compilador valida). `sound`/`silent` opcional en los say (def. silent).

| Directiva | Ejemplo | Efecto |
|---|---|---|
| `call <hook>` | `call act1_bedroom_items` | ejecuta el hook C (puede bloquear) |
| `say SET ID [sound]` | `say ACT1_BEDROOM A1_BEDROOM_TOO_LATE` | un diálogo |
| `say_cluster SET ID [sound]` | | encadena diálogos hasta el TERM |
| `say_response SET BASE [sound]` | | diálogo `BASE + last_choice` (respuestas) |
| `choice SET item` | `choice ACT1_HALL_CHOICE 0` | muestra opciones → `last_choice` |
| `branch <n> goto <label>` | `branch 1 goto atajo` | salta si `last_choice == n` |
| `label <nombre>` / `goto <nombre>` | | puntos de salto (resueltos por el generador) |
| `move CHR x y` | `move CHR_linus 200 174` | movimiento andado (bloquea) |
| `move_instant CHR x y` | | teletransporte |
| `show CHR on/off` | | mostrar/ocultar personaje |
| `look CHR left/right` | | orientación del sprite |
| `anim CHR ANIM_*` | `anim CHR_linus ANIM_MAGIC` | fija la animación del personaje |
| `wait <décimas>` | `wait 20` | espera 2,0 s (escala PAL/NTSC) |
| `wait_press` | | pausa la cutscene hasta que se pulse A |
| `wait_scroll <offset>` | `wait_scroll 360` | deja jugar hasta ese scroll |
| `set <flag> on/off` | `set interface on` | movement · scroll · interface · spells |
| `combat` | | combate interactivo completo (hasta ganar) |
| `cast SPELL [direct/reversed]` | `cast SPELL_OPEN` | cast scripted (sin canUse) |
| `wait_spell` | | espera a que el hechizo del jugador termine |
| `zone ZONE_X` | | fija la zona narrativa (canUse de puzzles) |
| `puzzle_sequence <tag> <spell:dir>...` | `puzzle_sequence puerta thunder:direct fire:direct hide:direct` | define y activa un puzzle de secuencia (2-4 pasos) |
| `wait_puzzle <tag>` | | el jugador castea libremente hasta completar la secuencia (fallar reinicia) |
| `if_puzzle_solved <tag> goto <label>` | | salta si el puzzle está resuelto |
| `fade_out <frames>` | `fade_out 120` | fundido a negro |
| `next_scene <escena>` | `next_scene act1_hall` | end_level + transición (por NOMBRE) |
| `hard_reset` | | reset de consola (final de demo) |
| `end` | | fin sin end_level |

Nota: `set interface off` solo oculta el HUD (no toca `interface_active` — los
rechazos de hechizo dependen de él; ver scene_vm.h).

## Cuándo usar un hook C

Cuando hay **estado, bucles o condiciones**: interacción re-entrante con items,
timeouts condicionales, setups con punteros a recursos, manipulación de paletas.
Un hook es una función `void(void)` que puede bloquear con `next_frame()` — el
mismo estilo de siempre. Si un patrón se repite en 3+ escenas, plantearse
promoverlo a opcode (la VM es extensible: op nuevo = case + entrada en
`OPS` de gen_scenes.py).

## Validación del generador (fatal)

labels · sets/ids de diálogo contra texts.csv · choices contra choices.csv ·
hooks contra scene_hooks.h · spells/zonas contra constants_spells.h · escenas
de next_scene contra data/scenes/. Todo error corta el build con archivo:línea.

## Puzzles de secuencia de hechizos

`puzzle_sequence` define la secuencia esperada y la activa (un puzzle activo a la
vez; se resetea al entrar en cualquier escena). Cada cast del jugador (o narrativo)
que TERMINA de forma natural avanza el progreso si coincide con el paso esperado;
un cast equivocado reinicia la secuencia (contando como paso 1 si coincide con el
inicio). Counter y cancelaciones no cuentan. Recuerda habilitar los hechizos
necesarios en el hook de setup (`spell_enable`) y fijar la zona si algún canUse
la exige (`zone ZONE_X`).

## Escena de referencia: act1_test

`data/scenes/act1/test.scene` ejercita TODOS los ops del motor (diálogos, choice
con branch, cast scripted, puzzle de 3 hechizos, scroll, combate). No está
enlazada desde el juego: se llega con `HACK_START_SCENE "act1_test"` o desde la
smoke ROM. Úsala como chuleta del DSL y como test de regresión del motor.

## Caso práctico: crear la escena "act1_claro" paso a paso

Una escena de ejemplo con **todos los conceptos fundamentales**: setup en hook C,
diálogos, un `choice` con dos ramas (branch/goto/label), `say_response`,
movimientos + animación, `wait_press`, un combate y la transición a la siguiente
escena. Guion: "Linus y Clio llegan a un claro; el jugador decide explorar (un
combate) o seguir de largo; ambas ramas confluyen y la escena termina".

### 1. Textos — `data/texts.csv`

Set `act1_claro` (define `ACT1_CLARO`), ids `A1_CLARO_*`. Una fila `NULL` cierra
un cluster. `face`/`side`: `FACE_linus`/`FACE_clio` · `SIDE_LEFT`/`SIDE_RIGHT`.

```
act1_claro,A1_CLARO_ARRIVE,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Un claro en el bosque|El aire está quieto,A forest clearing|The air is still
act1_claro,A1_CLARO_CLIO,FACE_clio,SIDE_RIGHT,DEFAULT_TALK_TIME,¿Exploramos|o seguimos?,Do we explore|or move on?
act1_claro,NULL,,,,,
act1_claro,A1_CLARO_EXPLORE,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Hay algo entre|los árboles...,Something moves|in the trees...
act1_claro,A1_CLARO_SKIP,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Mejor no perder|tiempo,Better not waste|time
act1_claro,A1_CLARO_AFTER,FACE_clio,SIDE_RIGHT,DEFAULT_TALK_TIME,Sigamos el camino,Let's follow the path
```

### 2. Choice — `data/choices.csv`

`act1_claro_choice` (define `ACT1_CLARO_CHOICE`); opciones vacías = no existen.

```
act1_claro_choice,0,FACE_linus,SIDE_RIGHT,DEFAULT_CHOICE_TIME,Explorar el claro,Seguir de largo,,,Explore the clearing,Move on,,
```

### 3. Hooks C — `src/scenes/act1/claro.{c,h}`

La LÓGICA (setup con recursos, aparición del enemigo) va en hooks; la secuencia,
en el `.scene`.

```c
// claro.h
#ifndef _ACT1_CLARO_H_
#define _ACT1_CLARO_H_
void act1_claro_setup(void); // Fondo de bosque + Linus y Clio visibles
void act1_claro_enemy(void); // Aparición de un WeaverGhost para el combate
#endif
```

```c
// claro.c — includes por metalibrería de dominio (ver AGENTS.md §6)
#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "res_all.h"
#include "scenes/act1/claro.h"   // header propio (hooks de esta escena)

void act1_claro_setup(void)
{
    player_has_rod = true;   // ANTES de init_character(CHR_linus): decide su sprite
    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map,
              forest_pal, 1440, BG_SCRL_USER_RIGHT, 3);   // fondo ancho → USER, no AUTO
    set_limits(0,134,275,172);

    init_character(CHR_linus);
    init_character(CHR_clio);
    active_character = CHR_linus;
    move_character_instant(CHR_linus, 140, 154);
    show_character(CHR_linus, true);
    move_character_instant(CHR_clio, 90, 154);
    show_character(CHR_clio, true);

    spell_enable(SPELL_THUNDER);   // para el combate opcional
    spell_enable(SPELL_HIDE);
}

void act1_claro_enemy(void)
{
    show_or_hide_interface(false);
    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA);
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(140));
}
```

### 4. Registrar los hooks — `src/scenes/scene_hooks.{h,c}`

```c
// scene_hooks.h — en el enum (antes de HOOK_COUNT):
    HOOK_ACT1_CLARO_SETUP,
    HOOK_ACT1_CLARO_ENEMY,
```
```c
// scene_hooks.c — include + entradas en la tabla (VERIFICA que ambas están:
// un hueco NULL en la tabla cuelga la consola, ver la guarda del op CALL):
#include "scenes/act1/claro.h"
    [HOOK_ACT1_CLARO_SETUP] = act1_claro_setup,
    [HOOK_ACT1_CLARO_ENEMY] = act1_claro_enemy,
```

### 5. La escena — `data/scenes/act1/claro.scene`

El nombre interno (`act1_claro`) sale de la ruta `act1/claro.scene`; la directiva
`scene` debe coincidir.

```
scene act1_claro

call act1_claro_setup
say_cluster ACT1_CLARO A1_CLARO_ARRIVE sound   # cluster: ARRIVE + CLIO hasta el NULL

# Clio se acerca y mira a Linus
move CHR_clio 120 154
look CHR_clio right
anim CHR_linus ANIM_ACTION
wait 8
anim CHR_linus ANIM_IDLE

set movement on
set spells on

choice ACT1_CLARO_CHOICE 0
say_response ACT1_CLARO A1_CLARO_EXPLORE sound  # respuesta = EXPLORE + last_choice
branch 1 goto seguir                            # opción 1 (Seguir) salta el combate

# Rama "explorar": combate
call act1_claro_enemy
combat
goto confluye

label seguir
say ACT1_CLARO A1_CLARO_SKIP sound

label confluye
set interface off
say ACT1_CLARO A1_CLARO_AFTER sound
wait_press                                      # espera A antes de cerrar
fade_out 60
next_scene act1_forest
```

Nota sobre `say_response`: muestra `dialogs[ACT1_CLARO][A1_CLARO_EXPLORE + last_choice]`,
así que la opción 0 enseña `A1_CLARO_EXPLORE` y la 1 `A1_CLARO_SKIP` (el id
siguiente). Si prefieres textos no correlativos, usa `branch`/`say` como en la
rama de combate.

### 6. Enlazar y probar

- Desde la escena anterior: `next_scene act1_claro`. O directo:
  `HACK_START_SCENE "act1_claro"` en `src/core/hack.h` + build.
- Opcional: añadir un caso `SMOKE_SCENE` en `src/smoke/smoke_cases.h`.

### 7. Compilar

`./build-theweave.sh release` (o `smoke`). `gen_scenes.py` valida en **fatal**
cada referencia (textos, choice, hooks, escena de `next_scene`): un id mal escrito
corta el build con `archivo:línea` antes de compilar C.
