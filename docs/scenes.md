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
| `branch n goto <label>` | | salta si `last_choice == n` |
| `label <nombre>` / `goto <nombre>` | | puntos de salto (resueltos por el generador) |
| `move CHR x y` | `move CHR_linus 200 174` | movimiento andado (bloquea) |
| `move_instant CHR x y` | | teletransporte |
| `show CHR on/off` | | mostrar/ocultar personaje |
| `look CHR left/right` | | orientación del sprite |
| `wait <décimas>` | `wait 20` | espera 2,0 s (escala PAL/NTSC) |
| `wait_scroll <offset>` | `wait_scroll 360` | deja jugar hasta ese scroll |
| `set <flag> on/off` | `set interface on` | movement · scroll · interface · spells |
| `combat` | | combate interactivo completo (hasta ganar) |
| `cast SPELL [direct/reversed]` | `cast SPELL_OPEN` | cast scripted (sin canUse) |
| `wait_spell` | | espera a que el hechizo del jugador termine |
| `zone ZONE_X` | | fija la zona narrativa (canUse de puzzles) |
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
