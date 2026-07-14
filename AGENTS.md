# AGENTS.md — biblia técnica de The Weave

> Esta es la **fuente de verdad** del proyecto para humanos y herramientas
> automáticas. Contiene todo lo necesario para entender y tocar el código **leyendo
> los archivos fuente lo mínimo posible**. Mantenlo sincronizado con el código real:
> **actualízalo en la misma tanda en que cambies algo que aquí se describe.**
>
> **Qué NO está aquí (y dónde está):** la *autoría de contenido* tiene guías propias,
> exhaustivas y para no técnicos, en `docs/` (ver §10). AGENTS.md documenta el
> **motor, la arquitectura y las trampas**; las guías documentan **cómo escribir**
> hechizos, escenas y textos. No dupliques aquí lo que ya está en esas guías: enlázalas.

## Flujo de trabajo Git (multi-ordenador — CRÍTICO)

> Javier trabaja este proyecto **desde varios ordenadores**. En cualquier sesión el
> remoto puede tener commits que este clon no tiene, y este clon puede tener trabajo
> sin subir. Sincronizar mal = trabajo duplicado o perdido. **Esto no es opcional y
> anula la cautela por defecto de "subir solo si me lo piden": aquí SE SUBE siempre.**

**Al EMPEZAR cada sesión — comprobar sincronización REAL antes de tocar código:**

1. `git fetch --all --prune` — **siempre lo primero**. `git status` a secas MIENTE: se
   calcula contra el último fetch y puede decir "al día" o "adelantado por N" cuando en
   realidad las ramas han divergido.
2. `git status -sb` y `git rev-list --left-right --count @{upstream}...HEAD`
   (izquierda = commits solo en el remoto; derecha = commits solo locales).
3. Interpretar el conteo `L  R`:
   - `0  0` → sincronizado, adelante.
   - `N  0` (solo remoto por delante) → `git pull --ff-only`.
   - `0  N` (solo local por delante) → trabajo sin subir de otra sesión; lo subes al terminar.
   - `N  M` (**divergencia**) → NO hagas pull/merge a ciegas. Mira los commits de cada lado
     (`git log --oneline @{upstream}..HEAD` y `... HEAD..@{upstream}`) y los ficheros que
     tocan. Si es **trabajo paralelo sobre lo mismo**, compara las dos implementaciones y
     quédate con la mejor; **etiqueta con `git tag` lo que vayas a descartar antes de un
     `reset --hard`** para que sea recuperable. Ante duda real, pregunta.

**Cómo se hacen los commits (bien):**
- Un commit por hito, asunto concreto en imperativo (qué cambia y por qué), nunca "wip"/"cambios".
- Actualiza este AGENTS.md y las guías de `docs/` **en la misma tanda** que el código que describen.
- Cierra el mensaje con el trailer `Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>`.

**Antes de TERMINAR la sesión — subir TODO (obligatorio):**
- No dejes commits locales sin empujar: otro ordenador los va a necesitar. `git push`.
- Verifica que quedó sincronizado: `git rev-list --left-right --count @{upstream}...HEAD` == `0  0`.
- Nunca `push --force` a `master` sin analizar antes qué hay en el remoto (puede ser trabajo de otro PC).

## 1. Qué es el proyecto

- **The Weave**: fangame secuela de *Loom* (LucasArts) para **Sega Mega Drive / Genesis**.
- **C plano** sobre **SGDK 2.x**. Docs SGDK: https://stephane-d.github.io/SGDK/
- **Estado**: demo técnica del acto 1. Flujo narrativo:
  `act1_bedroom → act1_corridor → act1_hall → act1_forest → reset`. Pilares del motor:
  hechizos de dos slots (§5), VM de cutscenes con DSL propio (§6), codegen validado
  desde `data/` (§4), y una smoke ROM para probar hechizos/escenas aislados.
- Hay una bitácora histórica del rediseño que dio lugar a esta arquitectura en
  `docs/refactor/` (baseline, bugs numerados B\*, plan). Es **referencia histórica**,
  no hace falta para el día a día.

## 2. Compilar, ejecutar y debugear

```
./build-theweave.sh [build|release|full|clean|smoke] [--no-run|-n]
```

El **pipeline y la configuración personal** están documentados en `docs/build.md`. En
una línea: codegen python en el **host** (la imagen Docker de SGDK no trae python3;
incluye `tools/resample_wavs.py`, que genera los `*_resampled.wav` de `res/sfx/` que
referencian los .res — 13300 Hz mono para XGM2) → `make` dentro de
`ghcr.io/stephane-d/sgdk:latest` → backup rotatorio → MiSTer (FTP) o BlastEm. Salida `out/rom.bin` (~896 KB con padding). La smoke ROM
(`build-theweave.sh smoke`, `out/smoke.bin`) y el checklist de playtest están en
`docs/testing.md`.

Datos que conviene tener a mano aquí:
- El `Makefile` del repo solo **envuelve** el `makefile.gen` de SGDK, que hace
  wildcard de `src/**/*.c` y `res/*.res` con `-Isrc -Ires`. **Añadir un `.c` o un
  `.res` NO requiere tocar el Makefile.**
- Los generadores validan en **FATAL** con `archivo:línea`: una referencia rota
  (id de diálogo, hook, escena, hechizo) corta el build antes de compilar.

### Cómo debugear (flujo probado)

1. Editar **solo** `src/core/hack.h`. Toggles disponibles: `HACK_START_SCENE
   "act1_forest"` (arranca directo en una escena — **siempre por nombre**),
   `DEBUG_LEVEL 2-3` (trazas KDebug a la consola de BlastEm), `HACK_ENEMIES_ONE_HP`,
   `HACK_FAST_DIALOGS`, `HACK_ALL_SPELLS`, `HACK_PLAYER_INVULNERABLE`,
   `HACK_FORCE_LANGUAGE`, `HACK_MUTE_MUSIC/SFX`.
2. `dprintf(nivel, ...)` (KDebug) solo emite si `nivel <= DEBUG_LEVEL`.
   La **smoke ROM muestra siempre FPS y carga de CPU** en la esquina superior
   derecha (VDP_showFPS/VDP_showCPULoad en next_frame, solo con HACK_SMOKE_BUILD):
   úsala para detectar ralentizaciones.
3. Si BlastEm congela con *"Read from VDP data port with invalid source"*, el PC
   impreso se localiza en `out/symbol.txt` (ordenado por dirección). **Con LTO la
   atribución del PC puede engañar**; la última traza KDebug suele señalar mejor.
4. Cuelgue dentro de `SPR_update` = corrupción del sprite engine: alguien llamó a
   `SPR_setAnim` con índice fuera de rango, usó un `Sprite*` liberado, o hubo un
   underflow de contador. Ver la trampa de `SPR_setAnim` en §7.

**Depuración desatendida (opcional):** si en esta máquina está instalado RetroArch +
`mcp-retroarch` (detección y setup en `docs/retroarch-mcp.md`), un agente puede arrancar
la ROM y **leer/escribir RAM, avanzar frame a frame de forma determinista y sacar
capturas** por el Network Control Interface (UDP :55355) — útil para verificación
automatizada. No hay input de gamepad por esa vía: para interacción, usa la smoke ROM o
los toggles de `hack.h`. Antes de ofrecerlo, comprueba disponibilidad con las
comprobaciones del §3 de ese doc.

## 3. Estructura de directorios

Cada dominio de `src/` tiene una **metalibrería** `<dominio>/<dominio>.h` que
reexporta sus headers públicos; los `.c` incluyen esa, no los headers sueltos (§8).

```
data/            → autoría editable: texts.csv (diálogos), choices.csv,
                   scenes/<acto>/<escena>.scene
tools/           → codegen python (gen_texts.py, gen_choices.py, gen_scenes.py) +
                   voice/ (síntesis de voces animalese)
src/
  boot/          → rom_head.c, sega.s
  core/          → main (bucle de escenas), init, controller, frame
                   (next_frame/timing), config.h (SCREEN_WIDTH…), hack.h (toggles)
  world/         → background: scroll, límites, new_level/end_level
  actors/        → entity (base común), characters, enemies, items, collisions
  combat/        → FSM de combate por hechizos (combat_state, hit_enemy/hit_player)
                   + melee.c (combate físico sin hechizos: jabalíes, golpe con A)
  spells/        → motor de hechizos (§5)
  narrative/     → texts, texts_data (GEN), choices_data (GEN), dialogs, encode
  scenes/        → scene_vm (intérprete), scene_hooks (enum+tabla), scene_data (GEN),
                   <acto>/<escena>.c/.h (hooks de lógica por escena), intro, geesebumps
  interface/     → HUD, pausa, contador de vida
  audio/         → sound (XGM2; jingles por spell id)
  smoke/         → smoke ROM: menú de casos (solo compila con -DHACK_SMOKE_BUILD)
res/             → res_*.res (rescomp) + res_*.h/.c (GEN) + assets fuente:
  gfx/           → font.png, backgrounds/act1, characters, enemies, faces,
                   interface, items/act1, geesebumps, intro
  sfx/           → dialogs/{deep,man,woman}, effects, music, notes, patterns, player
```

## 4. Codegen: de `data/` a C (todo generado, no editar a mano)

Tres generadores en `tools/` convierten los datos de autoría en tablas C. Corren en
el host (el build los lanza; también manualmente con `python3 tools/gen_*.py`). Todos
**validan en FATAL** y emiten las constantes C **verbatim** (el compilador es la 2ª
red de validación).

| Generador | Entrada | Salida (NO editar) | Valida |
|---|---|---|---|
| `gen_texts.py`   | `data/texts.csv`   | `narrative/texts_data.{c,h}`   | — |
| `gen_choices.py` | `data/choices.csv` | `narrative/choices_data.{c,h}` | — |
| `gen_scenes.py`  | `data/scenes/*/*.scene` | `scenes/scene_data.{c,h}` | labels, diálogos vs texts.csv, choices vs choices.csv, hooks vs enum `HOOK_*`, hechizos/zonas vs `constants_spells.h`, escena destino de `next_scene` |

- **Nombres, no números**: escenas, sets de diálogo y choices se nombran
  `<acto>_<nombre>` (`act1_bedroom`, `act1_hall_choice`). El generador emite el enum
  numérico interno (`SceneId`, `ACT1_BEDROOM`…) pero en DSL/textos/hacks se habla
  SIEMPRE por nombre, para poder intercalar sin renumerar.
- `gen_scenes.py` también emite **tablas laterales** para los datos que no caben en
  los 4 args `s16` de un `SceneStep` o que llevan punteros a recursos:
  `scene_levels[]`, `scene_items[]`, `scene_palettes[]` y `puzzle_seqs[]`.

## 5. Sistema de hechizos (`spells/`)

**Ids** (`constants_spells.h`): jugador THUNDER 0, HIDE 1, OPEN 2, SLEEP 3, FIRE 4,
LIGHT 5 (`SPELL_PLAYER_COUNT` = 6); enemigo EN_THUNDER 6, EN_BITE 7
(`SPELL_COUNT` = 8); `SPELL_NONE` = 254. Zonas: `ZONE_NONE` 0, `ZONE_CAULDRON` 1.
Botón→nota: A→MI B→FA C→SOL X→LA Y→SI Z→DO.

**Arquitectura**: tabla única `spell_defs[SPELL_COUNT]` + motor con **dos slots**
(`SPELL_SLOT_PLAYER`, `SPELL_SLOT_ENEMY`), ambos pueden estar vivos a la vez (para el
counter). `combat_state` (combat.c) es el director del combate; el motor consulta y
actualiza ese FSM.

Aparte existe el **combate físico** (`combat/melee.c`, `melee_combat_run`): cuerpo a
cuerpo sin hechizos (acto 1 antes de la vara). No toca `combat_state` (queda en
COMBAT_NO); dirige a los enemigos activos (persiguen con pausas aleatorias, muerden
de cerca, y al ser golpeados huyen hacia el lado de pantalla más cercano y vuelven;
locomoción con el enemigo en STATE_WALKING) y resuelve el golpe del jugador con A
(reutiliza STATE_PLAYING_NOTE → ANIM_ACTION). El tutorial "Eso ha dolido" de
update_character_animations solo salta con combat_state != COMBAT_NO.

**Vida del jugador** (ambos combates): `player_hitpoints` se reinicia a
`player_max_hitpoints` (5 por defecto; fijarla en el hook del combate si procede) en
cada `combat_init`/`melee_combat_run`; `hit_player` la resta y a 0 marca
`player_defeated`. El op `combat` del DSL sale entonces vía `combat_abort()` (libera
enemigos y cierra), y el melee libera y retorna. El op **`if_defeated goto`** permite
a la escena mostrar el fallo y reintentar (ver act1_test).

- `spell.c` — motor: `spell_validate` → `spell_player_cast` (counter o launch),
  `spell_update` (cada frame desde `update_combat`), `spell_try_counter`,
  `spell_cancel`, `spell_reject` (feedback + resume del enemigo), y el lado enemigo
  (`spell_enemy_try_launch`, recargas por enemigo, cadencia de notas).
- `notes.c` — cola de notas del jugador (debounce `MIN_TIME_BETWEEN_NOTES`, timeout
  `MAX_PATTERN_WAIT_TIME`, lock global) y HUD de notas enemigas. `player_has_rod`
  vive aquí y decide el sprite de Linus (ver la trampa en §7).
- `player_spells.c` / `enemy_spells.c` / `fire.c` / `light.c` — las `SpellDef` y los
  hooks de cada hechizo. Cada hechizo hace su `*_init()`, llamado desde
  `init_spells()` en `spell.c`.
- **Hooks (todos opcionales)**: `canUse` → `onRejected` (hint con diálogo si no se
  puede) → `onLaunch` → `onUpdate` (por frame; el auto-fin por `baseDuration` aplica
  siempre) → `onFinish` (SOLO fin natural) / `onCounter` (contrarrestado) /
  `onCancel` (cortado). **El motor es el único que toca `frameCounter`, los slots y el
  cleanup**; los hooks nunca liberan ni avanzan el contador.
- **Fases declarativas** (`SpellPhase`): efectos descritos como datos, no como código.
  `PHASE_VISUAL_FLASH` (continuo en un rango de frames) y `PHASE_LOGIC_DAMAGE`
  (puntual: `start==end`). Se rellenan en runtime porque dependen de `SCREEN_FPS`.
- **Zona narrativa**: `spell_zone` (`ZONE_*`) la fija la escena con el op `zone`; los
  `canUse` la reciben en `ctx->zoneId`.
- Desbloqueo: `spell_enable(id)` (silencioso; op `enable_spell` del DSL) o
  `activate_spell(id)` (con jingle y notas, para cutscenes).
- **`EN_BITE`** lo usa el jabalí (`ENEMY_CLS_BOAR`), primera clase de juego con
  mordisco (no counterable); ajustar `rechargeInit`/follow con playtest.

→ **Para crear un hechizo nuevo**: guía paso a paso en `docs/spells.md`. Puntos que
tocan el motor: `SPELL_*` en `constants_spells.h` (los de jugador antes de
`SPELL_PLAYER_COUNT`; renumerar arrastra `PLAYER_COUNT`/`COUNT`), `*_init()` en
`init_spells()`, icono en `show_pattern_icon` (interface.c), jingle en
`play_spell_jingle` (sound.c), caso en la smoke ROM.

## 6. Sistema de escenas / cutscenes (`scenes/`)

Una escena es un **array de `SceneStep` en ROM** (`{u8 op; s16 a,b,c,d;}`) generado
desde `data/scenes/<acto>/<escena>.scene`. `scene_run()` lo interpreta con un `switch`
por opcode. **Filosofía híbrida**:

- **El DSL `.scene` expresa todo lo declarativo**: el *setup* (fondo, límites, paleta,
  personajes, objetos, hechizos, vara) y la *secuencia narrativa* (diálogos, choices,
  movimientos, esperas, combate, casts, transiciones).
- **Los hooks C (`scene_hooks.c`) solo tienen la lógica imperativa** que una lista de
  órdenes no puede expresar: bucles de items, cinemáticas de paleta, aparición de
  enemigos. Se invocan desde el DSL con `call <hook>`. **Hay escenas sin ningún hook**
  (p.ej. `act1_hall`, 100% declarativa).

Internals del intérprete (esto NO está en la guía de autoría):
- **La VM nunca escribe en los steps** (están en ROM). Todo el estado mutable
  (`last_choice`, `pc`) vive en RAM.
- **No hay `next_frame` entre steps**: los ops instantáneos se encadenan; los
  bloqueantes (say, move, wait, combat, call) gestionan sus propios frames.
- `main.c` = `while(true) scene_run(&scenes[current_scene_id])`; cada escena deja
  `current_scene_id` apuntando a la siguiente (op `next_scene`) o resetea/termina.
- **Tablas laterales** (§4): `level`/`item`/`palette` guardan un índice en el step y
  los datos reales (con punteros a recursos) en `scene_levels[]`/`scene_items[]`/
  `scene_palettes[]`. Ojo: la paleta se guarda como `const Palette *` (copiarla por
  valor no es constante en un inicializador estático).
- **Puzzles de secuencia**: `puzzle_sequence <tag> <spell:dir>…` + `wait_puzzle` +
  `if_puzzle_solved`. El motor de hechizos llama a `scene_puzzle_notify()` al terminar
  cada cast para avanzar/reiniciar el puzzle activo (tabla lateral `PuzzleSeq`).
- **HUD durante diálogos**: los ops SAY/SAY_CLUSTER/SAY_RESPONSE/CHOICE ocultan el HUD
  de hechizos mientras hablan y lo restauran si estaba activo (no se solapa con el
  texto). `show_or_hide_interface()` no toca `interface_active`.
- **Asimetría deliberada de `set interface`**: `on` pone `interface_active=true` y
  muestra el HUD; `off` SOLO oculta el HUD (no toca `interface_active`, porque los
  rechazos de hechizo dependen de ese flag). Documentado en `scene_vm.h`.

**Escena de referencia del motor**: `act1_test` (`data/scenes/act1/test.scene`)
ejercita TODOS los ops — diálogos, choice+branch con bucles, cast scripted, dos
puzzles (uno con paso invertido), scroll y dos oleadas de combate. No está enlazada
al juego (`HACK_START_SCENE "act1_test"` o smoke ROM); es la chuleta canónica del DSL
(desglose sección a sección en `docs/test_scene.md`).

→ **Para crear una escena o un choice**: guía paso a paso en `docs/scenes.md`
(referencia completa de ops, formatos de CSV, recetas). Solo si la escena necesita
lógica imperativa creas `src/scenes/<acto>/<nombre>.c/.h` y registras cada hook en
DOS sitios: el enum `HOOK_*` de `scene_hooks.h` **y** la tabla de `scene_hooks.c`
(olvidar uno → salto a NULL → cuelgue; la VM avisa por KDebug si el hueco es NULL).

## 7. Trampas y datos duros (leer antes de tocar nada)

- **`bool` de SGDK es `u8`** (`sgdk/inc/types.h`): no lo uses como contador.
- **`SCREEN_FPS` es una VARIABLE runtime** (50 PAL / 60 NTSC, detectada en
  `initialize`). Nada de tablas `const` con duraciones en frames: rellénalas en
  runtime (`SCREEN_FPS * n`). Milisegundos → ticks con `calc_ticks(ms)`.
- **`u16` y restas**: compara antes de restar HP/contadores (underflow silencioso).
- **`next_frame(bool interactive)` es EL latido**: input + combate + animaciones +
  `SPR_update` + vblank. Los bloqueantes (`move_entity`, diálogos, `wait_seconds`) lo
  llaman por dentro. No dibujes ni proceses input fuera de él.
- **Nunca bloquees con `while(!SPR_isAnimationDone)`** ni la consultes el mismo frame
  de un `SPR_setAnim` (estado obsoleto hasta el siguiente `SPR_update`). Muertes y
  esperas → timers con `modeTimer`.
- **`SPR_update()` es CARO: exactamente uno por frame** (el de `next_frame`). Jamás
  lo llames en helpers por-entidad que corren cada frame: recorre y reprocesa TODOS
  los sprites. Caso real: `update_enemy()` lo llamaba y el melee con 5 jabalíes caía
  de 50 a ~25 FPS (medido con el contador de la smoke). Los `SPR_set*` son baratos
  (early-out); acumula cambios y deja que el `SPR_update` global los recoja. En
  helpers de SETUP/cutscene (poco frecuentes) sí es aceptable.
- **`SPR_setAnim` con un índice que el sprite no tiene** lee un puntero fuera de rango
  y congela el VDP (crash constante). Caso histórico: la **vara de Linus** (hoy los
  tres sprites tienen las 6 filas, pero la regla sigue). Linus tiene TRES formas:
  antorcha (`linus_has_torch`, override visual) > vara (`player_has_rod`, además
  puerta de la magia) > sin nada. Fija ambas ANTES de crear a Linus
  (`character CHR_linus` / `init_character`); para cambiar de forma a mitad de
  escena usa `reinit_character_sprite` (ver el hook del melee de act1_test).
- **Scroll y `new_level`/op `level`**: la anchura y el modo deben cuadrar con el mapa.
  `auto_*` sobre una anchura menor que el mapa deriva cada frame y acaba leyendo fuera
  del tilemap → cuelgue *"unmapped read"*. Fondos anchos → `user_*` con la anchura
  real (p.ej. forest = 1440, user_right). Ante la duda, copia los valores de una
  escena que use el mismo fondo.
- **`GAMEVERSION` incrusta `__DATE__`**: dos builds de días distintos NUNCA son
  byte-idénticos. Compara comportamiento (funcional/visual), no bytes.
- **Charset propio del texto de diálogo**: el español se codifica
  (ñ→^ á→# é→\$ í→% ó→\* ú→/ ¿→< ¡→>) vía `encode_spanish_text` (buffer estático:
  consúmelo antes de la siguiente llamada). Escapes de texto: `|` salto de línea,
  `@[...@]` color.
- **Fuente en UI de texto CRUDO** (`VDP_drawText`, no diálogos): la fuente tiene los
  glifos españoles EN las posiciones ASCII de `/ < > ^ # $ % *`. En menús/HUD crudos
  evita esos caracteres. Los diálogos normales no sufren esto (ya pasan por
  `encode_spanish_text`).

## 8. Reglas de codificación

- `snake_case` funciones/variables; `UPPER_CASE` macros; `PascalCase` tipos.
- **Tipos SGDK** (`u8/u16/u32/s8/s16/s32/bool`); nunca `int` a pelo.
- Llaves: **Allman** en funciones; **K&R** en if/for/while. 4 espacios.
- Comentario `//` junto a cada firma en los `.h`; bloque `/* */` de arquitectura al
  inicio de las cabeceras de subsistema. Sin plantillas Doxygen.
- **Includes por METALIBRERÍA de dominio** (relativos a `-Isrc -Ires`). Cada `.c`
  incluye `<dominio>/<dominio>.h` de los dominios que usa, no los headers sueltos.
  Metalibrerías: `core/core.h`, `world/world.h`, `actors/actors.h`, `combat/combat.h`,
  `spells/spells.h`, `narrative/narrative.h`, `scenes/scenes.h`, `interface/interface.h`,
  `audio/audio.h`, y `res_all.h` (todos los recursos). Orden del bloque: `<genesis.h>`
  → metalibrerías (ese orden) → headers específicos que no estén en ninguna meta
  (p.ej. `scenes/act1/<escena>.h`, o `spells/player_spells.h` que solo usa spell.c).
  - Los **`.h` NO incluyen metalibrerías**: solo sus dependencias mínimas de tipos.
  - `combat` e `interface` usan `combat.h`/`interface.h` como metalibrería (combat.h reexporta melee.h).
- Sin `malloc/free`: pools y buffers estáticos. `static const` para datos inmutables.
  Funciones privadas `static`. Guards `_FOO_H_`. Designated initializers en tablas.
- **No editar generados**: `narrative/texts_data.*`, `narrative/choices_data.*`,
  `scenes/scene_data.*`, `res/*.h` de rescomp.

## 9. Recursos (`res/`)

- Assets fuente en `gfx/` (gráficos) y `sfx/` (sonido), con subcarpetas por tipo de
  actor/escena. Los `.res` (definiciones rescomp) y los `res_*.h/.c` generados viven
  en la **raíz** de `res/`. `res_all.h` los agrega todos.
- Al añadir un asset: ponlo en su carpeta `gfx/`/`sfx/` y referéncialo en el `.res`
  con esa ruta (`SPRITE nombre "gfx/interface/x.png" …`). **El NOMBRE declarado en el
  `.res` (no la ruta) es lo que genera el símbolo C** → mover un asset no cambia el
  código, solo la ruta del `.res`.
- Arte fuente: Aseprite (`.ase`/`.aseprite`) en `res/gfx/…`. Créditos de terceros en
  la sección Acknowledgements del README.

## 10. Mapa de documentación (`docs/`)

AGENTS.md es la biblia del **motor**. La **autoría de contenido** y los flujos de
trabajo tienen guías dedicadas — no dupliques su contenido aquí, enlázalas:

| Documento | Cubre |
|---|---|
| `docs/build.md`    | Pipeline de build completo, configuración personal, VS Code, problemas típicos. |
| `docs/testing.md`  | Smoke ROM (tipos de caso, cómo añadir uno) y checklist de playtest. |
| `docs/retroarch-mcp.md` | Depuración desatendida vía RetroArch + `mcp-retroarch` (NCI UDP :55355): detección, leer/escribir RAM con byte-swap, frame-advance, capturas. |
| `docs/spells.md`   | Guía para no técnicos: crear un hechizo de principio a fin (hooks, fases, zonas, receta). |
| `docs/scenes.md`   | Guía para no técnicos: crear una escena (todos los ops del DSL, formatos de texts.csv/choices.csv, recetas). |
| `docs/texts.md`    | Guía para no técnicos: escribir diálogos, clusters, choices y voces. |
| `docs/test_scene.md` | Desglose de qué prueba cada sección de `act1_test`. |
| `docs/refactor/`   | Histórico del rediseño (baseline, bugs B\*, plan). Referencia, no día a día. |

Las guías `spells.md`/`scenes.md`/`texts.md` son **deliberadamente autocontenidas y no
técnicas** (no remiten a otros documentos): al mantenerlas, conserva ese tono y no las
acortes al estilo terso de AGENTS.

## 11. Voces (animalese)

- `tools/voice/generate_animalese_voices.py` sintetiza las voces (perfiles woman / man
  / deep) a partir de `tools/voice/animalese_download/animalese.wav`
  (github.com/Acedio/animalese.js; requiere venv con librosa/numpy/soundfile). Rutas
  ancladas al directorio del script (ejecutable desde cualquier cwd).
- Los **fonemas A-Z se escriben DIRECTAMENTE en `res/sfx/dialogs/<voz>/`** (donde
  `res_dialogs.res` los referencia): regenerar = actualizar los assets en su sitio.
- Se versiona solo el script y `animalese.wav`. La voz raw y las síntesis de prueba
  (`phonemes_animalese/`, `synthesis_animalese/`) son salida de exploración,
  gitignored. Los `typewriter*.wav` NO los genera este script (son efectos externos).

## 12. Pendientes conocidos

- Jingles de SLEEP y EN_BITE sin componer (TODO en `play_spell_jingle`).
- Mejor manejo de la derrota de un enemigo: la muerte actual es anim hurt ~1 s +
  release; una anim propia y recompensas es diseño de juego (el motor lo soporta vía
  `onFinish`/`hit_enemy`).
- No hay escena entre `act1_hall` y `act1_forest` para una posible ampliación; el DSL
  permite añadirla sin C si es lineal.
- Capturas de referencia de playtest pendientes (`docs/testing/`).
