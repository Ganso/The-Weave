# Plan de Refactorización — The Weave (v2)

> Documento de referencia para ejecutar la refactorización de **The Weave** de principio a fin.
> Cualquier IA o desarrollador debe poder seguir este plan sin información adicional.
> Mantener este fichero sincronizado con el estado real del refactor (tachar las fases completadas).
>
> **v2**: revisado tras análisis crítico contra el código real. Cambios principales respecto a v1:
> se elimina el DSL de hechizos (tablas C directas), el DSL de escenas pasa a ser híbrido
> (DSL + hooks C), el motor de hechizos usa dos slots (jugador/enemigo) para soportar
> counters, se reclasifican los bugs según si cambian comportamiento, y se corrigen
> errores del diseño v1 (escritura en ROM desde la VM, argumentos insuficientes en
> `SceneStep`, tipos `u8` que no cabían, escalado PAL/NTSC, orden de fases).

---

## 0. Visión y objetivos

The Weave es un fangame de *Loom* para Sega Mega Drive / Genesis escrito en C con SGDK.
El proyecto tiene ~6.500 líneas en 53 archivos y arrastra deuda técnica que dificulta
autorar contenido nuevo (escenas, hechizos, diálogos) sin tocar mucho código C y sin
riesgo de bugs.

**Objetivo final**: un motor mantenible y fácilmente soportable que permita iteraciones
rápidas a medida que crece el juego, sin pararse en temas técnicos, **manteniendo el
estilo de desarrollo actual del autor** siempre que sea posible.

### Objetivos del refactor (por orden de prioridad)

1. **Legibilidad y mantenibilidad por encima del rendimiento.** No es un proyecto que
   exprima el hardware; es un juego largo que debe poder crecer durante meses sin que la
   parte técnica frene el guión, los gráficos ni la jugabilidad.
2. **Sistema de hechizos versátil y a prueba de bugs.** Definir un hechizo debe ser
   rellenar una struct C (designated initializers) y, si hace falta, un par de hooks.
   El motor gestiona el ciclo de vida completo: los hooks nunca gestionan timers ni
   cleanup. Sin codegen para hechizos: con 6-10 hechizos, una tabla C bien hecha es más
   simple y más cercana al estilo actual que un DSL con generador.
3. **Cutscenes autorables con el mínimo de código C.** Un guionista edita `data/scenes/*.scene`
   y `gen_scenes.py` los integra. El DSL cubre la secuencia lineal (diálogo, movimiento,
   choices, fades); la lógica de puzzle o interacción compleja se escribe como un hook C
   pequeño que el DSL invoca con `call`. **Híbrido asumido desde el diseño**, no como
   parche: las escenas reales del juego ya contienen lógica que ningún DSL razonable
   debe expresar (ver §4.7).
4. **Estructura de directorios y librerías clara.** `gibberish/` y scripts sueltos a
   `tools/`; `src/` en subcarpetas por dominio; módulos grandes partidos por responsabilidad.
5. **Documentación útil y mantenible.** `AGENTS.md` como fuente de verdad exhaustiva +
   `docs/` reducida a las guías que de verdad se abren (hechizos, escenas, textos, build,
   testing). Sin boilerplate de comentarios que divirja del estilo actual.

### Características que se mantienen intactas

- **Estilo de programación actual.** C plano estilo SGDK, `snake_case`,
  `u8/u16/u32/s8/s16/s32/bool`, `static const` en ROM para datos inmutables, pools
  estáticos, comentario `//` junto a la firma de cada función, llaves Allman en
  funciones. Sin C++ ni macros oscuras. El autor debe poder editarlo todo con comodidad.
- **Arquitectura Mega Drive + SGDK.** SGDK instalado en `~/sgdk`. Hardware original
  (64 KB de RAM, sin memoria dinámica en hot paths).
- **Soporte PAL/NTSC en runtime.** `SCREEN_FPS` es una **variable** detectada en runtime
  (`extern u8 SCREEN_FPS`), no una constante. Toda duración se calcula en runtime a
  partir de ella (como hace hoy `init_patterns`). Ningún dato generado ni tabla const
  contiene frames hardcodeados que asuman 60 fps.
- **Comportamiento del juego como referencia "golden" funcional.** Ver §12: la
  referencia es el **build post-Fase-1** (con los bugs corregidos), y la comparación es
  **funcional/visual**, nunca binaria — `GAMEVERSION` incrusta `__DATE__`, así que dos
  builds de días distintos siempre difieren a nivel de bytes.

### Estado actual verificado (resumen de auditoría)

- 53 archivos, ~6.450 líneas en `src/`.
- 4 escenas implementadas en `act_1.c` (1, 2, 3 y 5; la 4 no existe).
- 6 hechizos: 4 de jugador (`patterns.c:88-135`) y 2 de enemigo (`patterns.c:402-428`).
- **El build actual se hace con `~/codigo/build-sgdk.sh`** (script externo compartido
  entre proyectos, invocado desde `.vscode/tasks.json`). No hay Makefile propio del repo.
- No existe `out/rom.bin` versionado; el ROM se regenera con el build.
- `EN_BITE` está definido pero **nunca se ejecuta en combate** (bug B2): el fix cambia
  el gameplay. Ver clasificación de bugs en §6.

### Referencias

- Proyecto de referencia: `/home/ganso/codigo/RedPlanet_MD/` (mismo autor, más moderno).
- SGDK: `/home/ganso/sgdk/` (documentación en https://stephane-d.github.io/SGDK/).
- Auditoría detallada del estado inicial: `docs/refactor/state_audit.md` (se genera en Fase 0).

---

## 1. Decisiones de diseño (ya tomadas)

| # | Decisión | Valor |
|---|---|---|
| D1 | Dependencias entre módulos | **Includes explícitos por archivo**. El Makefile añade `-Isrc -Ires`; los includes se escriben relativos a esas raíces. Sin rutas `../`. `globals.h` desaparece **al final de la Fase 5**. · **ACTUALIZADO 2026-07 (post-refactor):** los `.c` incluyen **metalibrerías de dominio** `<dominio>/<dominio>.h` (una por dominio: core, world, actors, combat, spells, narrative, scenes, interface, audio + `res_all.h`) en vez de decenas de headers sueltos — reduce `bedroom.c` de 21 includes a 10, media 6. Los `.h` siguen con includes específicos mínimos (no metalibrerías). Ver AGENTS.md §6. Es un punto medio entre el globals.h original y los includes 100% granulares: casi la misma legibilidad, sin recompilación total. |
| D2 | Modelo de hechizos | **Tablas C con designated initializers + hooks + fases declarativas opcionales.** Sin DSL ni codegen: los 6 hechizos actuales necesitan hooks todos, así que un generador no aporta. Tabla única `spell_defs[]` inicializada en runtime (para escalar con `SCREEN_FPS`). |
| D3 | Formato de cutscenes | **DSL de texto híbrido** en `data/scenes/*.scene` → `gen_scenes.py` → `src/scenes/scene_data.c`. La VM ejecuta pasos lineales; la lógica compleja vive en hooks C (`src/scenes/scene_hooks.c`) invocados con el op `call`. |
| D4 | Pruebas | **ROM de smoke test** (`out/smoke.bin`) con menú que ejecuta cada hechizo y escena. Verificación **funcional/visual** contra el baseline post-Fase-1 (nunca diff binario). |
| D5 | Organización de `src/` | Subcarpetas por dominio: `core world actors combat spells narrative scenes interface audio boot`. Makefile con wildcards. |
| D6 | Estilo de comentarios | **El actual del proyecto**: `tipo nombre(params); // qué hace` en los `.h`, `//` para notas internas, bloque `/* ... */` de arquitectura al inicio de las cabeceras de subsistema. Sin plantillas `Parámetros:/Retorna:` obligatorias ni Doxygen. |
| D7 | `Makefile` | Propio del repo: wildcard discovery de `src/**/*.c` y `res/*.res`, pre-build codegen (`gen_texts`, `gen_choices`, `gen_scenes`), targets `release/debug/clean/smoke/codegen`. El target `smoke` **excluye `src/core/main.c`** (la smoke ROM tiene su propio `main`). |
| D8 | `res_*.h` | Mantienen el prefijo `res_`. Se incluyen explícitos (`#include "res_sound.h"` gracias a `-Ires`) desde cada `.c` que los usa. |
| D9 | Rama de trabajo | `refactor` a partir de `master`. Commits por fase. |
| D10 | Referencia de comportamiento | **Baseline = build al cerrar la Fase 1** (bugs corregidos y cambios de comportamiento documentados). Desde ahí, cada fase debe reproducir el baseline. |
| D11 | Tiempos y duraciones | Ninguna duración en frames hardcodeados. En C: expresiones con `SCREEN_FPS` o `calc_ticks(ms)` evaluadas en runtime. En el DSL de escenas: **décimas de segundo** (`wait 30` = 3,0 s); la VM convierte a frames en runtime. |
| D12 | Slots de hechizo activos | El motor mantiene **dos slots**: `SPELL_SLOT_PLAYER` y `SPELL_SLOT_ENEMY`. Imprescindible para el counter (el hechizo enemigo sigue vivo mientras el jugador lanza el invertido). |

---

## 2. Estructura de directorios final

```
The-Weave/
├── AGENTS.md                  (exhaustivo, fuente de verdad — ver §9.1)
├── Makefile                   (NUEVO: wildcard + pre-build codegen)
├── build-theweave.sh          (NUEVO: wrapper de build; sustituye a ~/codigo/build-sgdk.sh para este repo)
├── refactorizar.md            (este plan)
├── README.md                  (sin cambios relevantes)
├── .gitignore                 (actualizado: out/, venv/, tools/voice/__pycache__/, etc.)
│
├── data/                      (AUTORÍA EDITABLE — el guionista toca aquí)
│   ├── texts.csv              (diálogos: set,id,face,side,time,es,en — existente, movido de raíz)
│   ├── choices.csv            (NUEVO: choices, hoy hardcoded en texts.c)
│   └── scenes/                (NUEVO: un .scene por escena, DSL)
│       ├── act1_scene1.scene
│       ├── act1_scene2.scene
│       ├── act1_scene3.scene
│       └── act1_scene5.scene
│
├── tools/                     (CODEGEN + UTILIDADES; nada se ejecuta en la ROM)
│   ├── gen_texts.py           (movido de raíz, era generate_texts.py)
│   ├── gen_choices.py         (NUEVO: choices.csv → choices_data.c)
│   ├── gen_scenes.py          (NUEVO: data/scenes/*.scene → scene_data.c)
│   ├── consolidate.py         (movido de raíz)
│   │                          (add_texts_comments.py se mueve aquí en Fase 2 y se retira en Fase 5)
│   └── voice/                 (movido de gibberish/)
│       ├── generate_animalese_voices.py
│       ├── animalese_download/
│       ├── phonemes_animalese/
│       └── synthesis_animalese/
│
├── src/                       (CÓDIGO DEL JUEGO)
│   ├── boot/
│   │   ├── rom_head.c
│   │   └── sega.s
│   ├── core/
│   │   ├── main.c             (bucle principal; EXCLUIDO del target smoke)
│   │   ├── config.h           (SCREEN_WIDTH/HEIGHT, GAME_VERSION, DEBUG_LEVEL, dprintf)
│   │   ├── hack.h             (HACK_* toggles de dev, compile-time)
│   │   ├── frame.c  frame.h   (next_frame, wait_seconds, calc_ticks, frame_counter)
│   │   ├── init.c  init.h     (initialize, new_level, end_level)
│   │   └── controller.c  controller.h  (input del pad, pausa)
│   ├── world/
│   │   ├── background.c  background.h
│   │   └── constants_world.h
│   ├── actors/
│   │   ├── entity.c  entity.h        (Entity, GameState, move_entity)
│   │   ├── characters.c  characters.h
│   │   ├── enemies.c  enemies.h
│   │   ├── items.c  items.h
│   │   ├── collisions.c  collisions.h
│   │   └── constants_actors.h
│   ├── combat/
│   │   ├── combat.c  combat.h        (CombatState, hit_enemy/player, update_combat, combat_run)
│   │   └── constants_combat.h
│   ├── spells/                (SISTEMA DE HECHIZOS — reemplaza patterns/)
│   │   ├── spell.c  spell.h          (motor: validate, launch, update, counter, finish; 2 slots)
│   │   ├── spell_hooks.c  spell_hooks.h  (hooks compartidos reutilizables)
│   │   ├── player_spells.c  player_spells.h  (defs + hooks de thunder/hide/open/sleep)
│   │   ├── enemy_spells.c  enemy_spells.h    (defs + hooks de en_thunder/en_bite)
│   │   ├── fire.c  fire.h             (ejemplo nuevo end-to-end)
│   │   └── constants_spells.h         (SPELL_*, NOTE_*, ZONE_*, límites)
│   ├── narrative/
│   │   ├── texts.c  texts.h
│   │   ├── texts_data.c  texts_data.h   (GENERADO por gen_texts.py — era texts_generated)
│   │   ├── choices_data.c  choices_data.h  (GENERADO por gen_choices.py)
│   │   ├── dialogs.c  dialogs.h         (talk, talk_dialog, talk_cluster, choice_dialog)
│   │   ├── encode.c  encode.h           (encode_spanish_text extraído de texts.c)
│   │   └── constants_narrative.h
│   ├── scenes/                (CUTSCENES — reemplaza act_1.c)
│   │   ├── scene_vm.c  scene_vm.h       (intérprete de SceneStep)
│   │   ├── scene_data.c  scene_data.h   (GENERADO por gen_scenes.py)
│   │   ├── scene_api.c  scene_api.h     (primitivas: scene_say, scene_move, ...)
│   │   ├── scene_hooks.c  scene_hooks.h (hooks C por escena: lógica de puzzle/interacción)
│   │   ├── intro.c  intro.h             (se queda como C, no DSL)
│   │   ├── geesebumps.c  geesebumps.h   (idem)
│   │   └── constants_scenes.h           (PUZZLE_MAX, PUZZLE_SEQ_MAX, ...)
│   ├── interface/
│   │   ├── interface.c  interface.h
│   │   └── constants_interface.h
│   └── audio/
│       ├── sound.c  sound.h
│       └── constants_audio.h            (IDs de SFX con rangos reservados)
│
├── smoke/                     (ROM DE SMOKE TEST — target aparte, propio main)
│   ├── smoke_main.c                     (main del smoke; el del juego se excluye)
│   ├── smoke_menu.c  smoke_menu.h
│   ├── smoke_runner.c  smoke_runner.h
│   └── smoke_cases.h                    (tabla de casos)
│
├── docs/
│   ├── images/                (existente)
│   ├── spells.md              (cómo crear un hechizo: struct, fases, hooks, ejemplo fire)
│   ├── scenes.md              (cómo autorar una cutscene: DSL, hooks, VM, ejemplo)
│   ├── texts.md               (texts.csv, choices.csv, escapes, encoding, voces)
│   ├── build.md               (Makefile, build-theweave.sh, SGDK, BlastEm, VS Code)
│   ├── testing.md             (smoke ROM, checklist de playtest, cómo añadir casos)
│   └── refactor/              (bitácora del refactor)
│       ├── state_audit.md     (Fase 0: estado inicial detallado)
│       ├── bugs.md            (tabla §6 con estado)
│       ├── baseline.md        (Fase 1: descripción del comportamiento baseline + capturas)
│       └── checklist.md       (checklist por fase)
│
└── res/                       (sin cambios estructurales)
    └── (igual que hoy: *.res, *.h generados, subcarpetas de arte fuente)
```

### Notas de organización

- **No hay `data/spells/` ni `gen_spells.py`**: los hechizos se definen en C (D2).
- **No hay `src/lib/`**: se creará solo si aparece un helper realmente compartido.
- La arquitectura general del código se documenta en `AGENTS.md` (no hay
  `docs/architecture.md` separado); el arte y las voces se documentan como secciones
  de `AGENTS.md` (no hay `docs/art.md` ni `docs/voice.md`). Menos ficheros que
  mantener sincronizados.
- `smoke/` vive fuera de `src/` y se compila como target aparte que **excluye
  `src/core/main.c`** y define `-DHACK_SMOKE_BUILD`.
- `controller.c` va a `src/core/` (procesa input global y pausa, no es un actor).

---

## 3. Diseño del sistema de hechizos

### 3.1 Objetivos de diseño

- **Struct + hooks, sin codegen.** Un hechizo se define con designated initializers en
  `player_spells.c` / `enemy_spells.c`. Es el estilo actual (`init_patterns`) pulido.
- **Fases declarativas opcionales.** Para efectos visuales secuenciados por frames, un
  hechizo puede declarar un array de `SpellPhase` y el motor lo ejecuta. Si prefiere
  lógica imperativa, usa el hook `onUpdate` (como los `update` actuales). Ambos pueden
  convivir.
- **Mismo motor para combate y narrativa.** Se lanza igual desde el FSM de combate que
  desde la VM de escena; cambia el `origin`.
- **Dos slots activos (D12).** El counter requiere que el hechizo enemigo y el del
  jugador estén vivos a la vez. El motor mantiene `SPELL_SLOT_PLAYER` y
  `SPELL_SLOT_ENEMY`, cada uno con su `frameCounter` y contexto.
- **Bug-resistant.** El motor es el único que toca frame counters, avance de fases,
  finalización y cleanup. Los hooks no resetean nada. Esto elimina la clase de bugs
  actual (copy-paste de `baseDuration` en B1, slots hardcodeados en B2/B3).
- **Rechazo con feedback.** El caso real de thunder-vs-WeaverGhost (hoy incrustado en
  `pattern_thunder.c:11-29`: muestra diálogo, resetea notas y restaura el estado de
  combate) se modela con el hook `onRejected`, que el motor llama cuando `canUse`
  devuelve false durante combate interactivo. La VM de escena **no** participa en esto:
  durante el combate la VM no está ejecutando.

### 3.2 Estructuras de datos

```c
/* src/spells/spell.h */

/* Origen del lanzamiento: determina quién paga el coste y a quién afecta. */
typedef enum {
    SPELL_ORIGIN_PLAYER    = 0,   /* lanzado por jugador via notas */
    SPELL_ORIGIN_ENEMY     = 1,   /* lanzado por enemigo via IA */
    SPELL_ORIGIN_NARRATIVE = 2    /* lanzado por script de escena (puzzle) */
} SpellOrigin;

/* Slot de ejecución. Dos hechizos pueden estar activos a la vez (counter). */
typedef enum {
    SPELL_SLOT_PLAYER = 0,        /* hechizo del jugador (o de la narrativa) */
    SPELL_SLOT_ENEMY  = 1,        /* hechizo del enemigo activo */
    SPELL_SLOT_COUNT  = 2
} SpellSlot;

/* Tipo de fase declarativa. */
typedef enum {
    PHASE_VISUAL_FLASH   = 0,   /* flash de paleta: p1=PAL_ENTRY(pal,col), p2=color VDP destino */
    PHASE_VISUAL_SPRITE  = 1,   /* mostrar/ocultar sprite de efecto: p1=sprite id, p2=visible */
    PHASE_VISUAL_FADE    = 2,   /* fade global: p1=modo */
    PHASE_LOGIC_DAMAGE   = 3,   /* aplicar daño: p1=PhaseTarget, p2=cantidad */
    PHASE_LOGIC_STATE    = 4,   /* cambiar estado del objetivo: p1=PhaseTarget, p2=GameState */
    PHASE_SFX            = 5    /* disparar sonido: p1=sfx_id */
} PhaseKind;

/* Target de una fase lógica. */
typedef enum {
    PHASE_TARGET_NONE         = 0,
    PHASE_TARGET_PLAYER       = 1,
    PHASE_TARGET_ENEMY_ACTIVE = 2,
    PHASE_TARGET_NARRATIVE    = 3    /* redirige el efecto al estado de puzzle, no a HP */
} PhaseTarget;

/* Una fase del efecto. El motor dispara startFrame..endFrame relativo al lanzamiento.
   startFrame/endFrame se rellenan en runtime con expresiones de SCREEN_FPS (D11).
   p1/p2 son u16: un color VDP (12 bits) o un PAL_ENTRY caben; en v1 eran u8 y no cabían. */
typedef struct {
    u16 startFrame;
    u16 endFrame;
    PhaseKind kind;
    u16 p1, p2;
} SpellPhase;

/* Contexto de un hechizo activo. Lo gestiona el motor; los hooks solo lo leen. */
typedef struct {
    SpellOrigin origin;
    u8  enemyId;           /* enemigo lanzador (slot enemy) o enemigo activo (slot player) */
    u8  zoneId;            /* zona actual dentro de la escena (para canUse de zona) */
    bool reversed;         /* true si se lanzó invertido */
    u16 frameCounter;      /* progreso del efecto — SOLO lo escribe el motor */
} SpellContext;

/* Definición de un hechizo. Es lo que rellena el autor. */
typedef struct {
    u16   id;                     /* SPELL_THUNDER, ... (índice en spell_defs[]) */
    u8    notes[4];               /* secuencia NOTE_MI..NOTE_DO */
    u8    noteCount;              /* 1..4 (en_bite usa 3) */
    bool  isPalindrome;           /* notas == notas invertidas → no distingue reverso */
    bool  counterable;            /* un counter puede cancelarlo */
    u16   baseDuration;           /* frames totales; rellenado en runtime con SCREEN_FPS */
    SpellOrigin defaultOrigin;

    /* Hooks opcionales. NULL si no se necesitan. */
    bool  (*canUse)    (const SpellContext *ctx);  /* false = no se puede lanzar ahora */
    void  (*onRejected)(SpellContext *ctx);        /* feedback al jugador tras canUse=false (hints) */
    void  (*onLaunch)  (SpellContext *ctx);        /* al iniciar, antes de la primera fase */
    bool  (*onUpdate)  (SpellContext *ctx);        /* por frame; true = efecto terminado ya */
    void  (*onCounter) (SpellContext *ctx);        /* al ser contrarrestado */
    void  (*onFinish)  (SpellContext *ctx);        /* al terminar (siempre, incluso tras counter) */

    /* Fases declarativas opcionales (NULL/0 si todo va por onUpdate). */
    const SpellPhase *phases;
    u8  phaseCount;
} SpellDef;

/* Tabla única de hechizos, indexada por SPELL_*. Se rellena en init_spells()
   (runtime, para escalar duraciones con SCREEN_FPS — igual que hoy init_patterns). */
extern SpellDef spell_defs[SPELL_COUNT];

/* Habilitación:
   - Jugador: player_spell_enabled[SPELL_PLAYER_COUNT] (RAM; activate_spell() como hoy).
   - Enemigo: obj_enemy[i].class.has_pattern[] (existente; se renombra a has_spell[]). */
```

```c
/* src/spells/constants_spells.h */
#define SPELL_THUNDER      0
#define SPELL_HIDE         1
#define SPELL_OPEN         2
#define SPELL_SLEEP        3
#define SPELL_FIRE         4        /* nuevo, ejemplo */
#define SPELL_PLAYER_COUNT 5
#define SPELL_EN_THUNDER   5
#define SPELL_EN_BITE      6
#define SPELL_COUNT        7
#define SPELL_NONE         254
/* NOTE_*, ZONE_*, MAX_SPELL_PHASES, macros PAL_ENTRY/PAL0_COL4 (hoy duplicadas — B19) */
```

### 3.3 Motor de hechizos (`src/spells/spell.c`)

Estado interno del motor (estático, privado al módulo):

```c
typedef struct {
    const SpellDef *def;   /* NULL = slot libre */
    SpellContext ctx;
} ActiveSpell;
static ActiveSpell active[SPELL_SLOT_COUNT];
```

API:

1. **`u8 spell_validate(const u8 *notes, u8 count, bool *reversed_out)`** — sustituye a
   `validate_pattern`. Recorre `spell_defs[]` comparando en orden directo e invertido,
   respeta `isPalindrome`. Devuelve `SPELL_NONE` si no hay match.
2. **`bool spell_launch(u8 spellId, SpellSlot slot, SpellContext ctx)`** — llama `canUse`;
   si false, llama `onRejected` (si existe) y devuelve false. Si true: llama `onLaunch`,
   pone `frameCounter = 0`, ocupa el slot y notifica al FSM de combate o a la VM según
   `origin`.
3. **`void spell_update(void)`** — se llama cada frame desde `update_combat` (o desde la
   VM en `wait_spell`). Para **cada slot ocupado**: incrementa `frameCounter`, ejecuta
   las fases cuyo rango contiene `frameCounter`, llama `onUpdate` si existe. El slot
   termina cuando `frameCounter >= baseDuration` o `onUpdate` devuelve true → `spell_finish`.
4. **`bool spell_try_counter(void)`** — si el slot ENEMY tiene un hechizo `counterable`
   activo y el jugador ha validado su secuencia invertida en la ventana adecuada:
   invoca `onCounter` del hechizo enemigo y lo finaliza. El hechizo del jugador puede
   seguir su curso en el slot PLAYER. Sustituye a `try_counter_spell` (`combat.c:71`),
   eliminando el slot-0 hardcodeado (B3).
5. **`void spell_finish(SpellSlot slot)`** — interna: llama `onFinish`, libera el slot,
   devuelve el control a quien lo lanzó (FSM de combate o VM).
6. **`bool spell_slot_active(SpellSlot slot)`** / **`u8 spell_active_id(SpellSlot slot)`**
   — consultas para FSM, VM, HUD y hooks (p. ej. fire pregunta si el enemigo está
   lanzando en_thunder).

El motor es **el único** que escribe `frameCounter` y el estado de los slots.

### 3.4 Hooks compartidos (`src/spells/spell_hooks.c`)

Hooks reutilizables para no duplicar lógica (en C no hay closures, así que los hooks
parametrizados leen su parámetro de la def o del contexto, no de un generador):

- `bool spell_hook_reject_if_no_enemy(const SpellContext *ctx)` — `canUse` que rechaza
  si no hay hechizo enemigo activo.
- `void spell_hook_damage_enemy_and_cancel(SpellContext *ctx)` — `onCounter` genérico:
  daña al enemigo activo y finaliza su hechizo (extraído de `pattern_en_thunder.c`).
- `void spell_hook_hit_player(SpellContext *ctx)` — `onFinish` típico de hechizo
  enemigo no contrarrestado.
- `void spell_hook_narrative_advance(SpellContext *ctx)` — `onFinish` que registra el
  hechizo en el progreso de puzzle (ver §4.8).

### 3.5 Definición de un hechizo (ejemplo real: FIRE)

Caso de uso: "un hechizo fuego que solo se activa en determinada zona, con
comportamiento distinto si hay un hechizo enemigo en curso, y con un efecto visual por
frames que puede ser complejo". Todo en un `.c` pequeño; sin DSL ni generador:

```c
/* src/spells/fire.c */
#include <genesis.h>
#include "spells/spell.h"
#include "spells/constants_spells.h"
#include "combat/combat.h"
#include "audio/sound.h"

static SpellPhase fire_phases[4];   // rellenadas en fire_init() con SCREEN_FPS

static bool fire_can_use(const SpellContext *ctx)
{
    if (ctx->zoneId != ZONE_CAULDRON)
        return false;                                  // solo en la zona del caldero
    u8 enemy_spell = spell_active_id(SPELL_SLOT_ENEMY);
    if (enemy_spell == SPELL_EN_THUNDER) return true;  // puede comer el thunder enemigo
    if (enemy_spell != SPELL_NONE) return false;       // no interfiere con otros
    return true;                                       // uso normal
}

static void fire_on_launch(SpellContext *ctx)
{
    if (spell_active_id(SPELL_SLOT_ENEMY) == SPELL_EN_THUNDER)
        spell_try_counter();                           // fuego come thunder
}

void fire_init(void)    // Registra FIRE en spell_defs[] (llamado desde init_spells)
{
    fire_phases[0] = (SpellPhase){ 0,             SCREEN_FPS/3,   PHASE_VISUAL_FLASH,  PAL0_COL4, COLOR_ORANGE_VDP };
    fire_phases[1] = (SpellPhase){ SCREEN_FPS/3,  SCREEN_FPS,     PHASE_VISUAL_SPRITE, SPR_FIRE_BURST, 1 };
    fire_phases[2] = (SpellPhase){ SCREEN_FPS,    SCREEN_FPS*2,   PHASE_LOGIC_DAMAGE,  PHASE_TARGET_ENEMY_ACTIVE, 2 };
    fire_phases[3] = (SpellPhase){ SCREEN_FPS*2,  SCREEN_FPS*4,   PHASE_VISUAL_FADE,   FADE_OUT_MODE, 0 };

    spell_defs[SPELL_FIRE] = (SpellDef){
        .id = SPELL_FIRE,
        .notes = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 4,                 // 4 segundos, PAL o NTSC
        .defaultOrigin = SPELL_ORIGIN_PLAYER,
        .canUse = fire_can_use, .onLaunch = fire_on_launch,
        .phases = fire_phases, .phaseCount = 4
    };
}
```

Patrón general: **las fases declaran lo secuenciable, los hooks lo condicional, el
motor une ambos**. Añadir un hechizo = un `.c` + una constante `SPELL_*` + una llamada
`*_init()` en `init_spells()` + un caso en la smoke ROM.

### 3.6 Migración de los 6 hechizos actuales

Datos verificados contra `patterns.c:88-135` y `patterns.c:402-428`:

| Hechizo | Nuevo id | Notas | Duración actual | Hooks necesarios |
|---|---|---|---|---|
| `PATTERN_THUNDER` | `SPELL_THUNDER` | MI FA SOL SOL | `SCREEN_FPS*4` | `canUse`+`onRejected` (hint WeaverGhost, ver §3.1), `onLaunch` (flash+jingle), `onUpdate` o fases (flash), `onFinish` (restaurar color) |
| `PATTERN_HIDE` | `SPELL_HIDE` | FA SOL SOL FA (palíndromo) | `SCREEN_FPS*4` | `onLaunch`, `onFinish` (restaurar paleta) |
| `PATTERN_OPEN` | `SPELL_OPEN` | FA SI SOL DO | 45 → `SCREEN_FPS*3/4` | `canUse` (false salvo scripted), stub |
| `PATTERN_SLEEP` | `SPELL_SLEEP` | FA MI DO LA | 75 → `SCREEN_FPS*5/4` | `canUse` (false salvo scripted), stub. Corrige B1 al migrar |
| `PATTERN_EN_THUNDER` | `SPELL_EN_THUNDER` | MI FA SOL SOL | `SCREEN_FPS` | `onLaunch` (flash cielo), `onFinish` (hit_player), `onCounter` (hit_enemy+cancel) |
| `PATTERN_EN_BITE` | `SPELL_EN_BITE` | MI SOL DO (3 notas) | `SCREEN_FPS` | `onLaunch`, `onFinish` (hit_player) |

**Nota importante**: hoy `EN_BITE` nunca se dispara en combate (B2). **Decidido**: el
bug del slot se corrige en Fase 1, pero bite queda **deshabilitado explícitamente**
(`has_pattern[PATTERN_EN_BITE] = false` en la clase WeaverGhost), así que el combate
sigue idéntico al actual. Se migra al motor nuevo igualmente deshabilitado. Activarlo
en el futuro será una decisión de contenido: poner el flag a true y ajustar
`rechargeFrames` con playtest.

### 3.7 Uso en puzzles narrativos

La VM de escena lanza hechizos con `origin = SPELL_ORIGIN_NARRATIVE` en el slot PLAYER.
Mismo motor, mismas fases y hooks, pero las fases `PHASE_LOGIC_DAMAGE` con target
`PHASE_TARGET_NARRATIVE` alimentan el progreso de puzzle en vez de HP. El progreso de
puzzles y su comprobación viven en la VM (§4.8).

---

## 4. Diseño del sistema de cutscenes

### 4.1 Objetivos de diseño

- **Autorar una escena lineal nueva = crear un `.scene` + `make codegen`.** Cero C.
- **Híbrido asumido**: cuando una escena necesita lógica (bucles de interacción con
  items, puzzles con estado, timeouts condicionales), esa lógica se escribe como un
  hook C corto en `scene_hooks.c` y el `.scene` lo invoca con `call <hook>`. Es
  explícitamente **mejor** un hook C de 30 líneas que un DSL con variables,
  condicionales y timers — eso sería un lenguaje de programación malo.
- **El combate interactivo no se scriptea.** El combate de act1_scene5 es el jugador
  tocando notas libremente contra la IA; eso no es un `cast`. El DSL tiene un op
  `combat` que cede el control al FSM de combate hasta victoria/derrota, y ops de salto
  según el resultado.
- **VM pequeña y predecible**: un array de `SceneStep` en ROM (**solo lectura estricta**;
  el estado mutable — `last_choice`, progreso de puzzle — vive en variables de RAM de
  la VM, nunca en los steps), un `switch` por op.
- **Validación estricta en codegen**: labels, ids de diálogo, ids de choice, nombres de
  hechizo y nombres de hook referenciados deben existir.

### 4.2 Estructuras de datos

```c
/* src/scenes/scene_vm.h */

typedef enum {
    SCENE_OP_LEVEL          = 0,   /* level <id> limits <min> <max> */
    SCENE_OP_CHAR_AT        = 1,   /* char <id> at <x> <y> <face_dir> */
    SCENE_OP_CHAR_SHOW      = 2,
    SCENE_OP_CHAR_HIDE      = 3,
    SCENE_OP_MOVE           = 4,   /* move <char> to <x> <y> (bloqueante, como hoy) */
    SCENE_OP_WAIT_FOLLOWERS = 5,
    SCENE_OP_SAY            = 6,   /* say <dialogSet> <dialogId> */
    SCENE_OP_SAY_CLUSTER    = 7,   /* say_cluster <dialogSet> <startId> */
    SCENE_OP_CHOICE         = 8,   /* choice <choiceSet> → escribe last_choice (RAM) */
    SCENE_OP_BRANCH         = 9,   /* branch <idx> goto <label> — salta si last_choice == idx */
    SCENE_OP_GOTO           = 10,  /* goto <label> */
    SCENE_OP_WAIT           = 11,  /* wait <décimas de segundo> — la VM escala con SCREEN_FPS (D11) */
    SCENE_OP_CALL           = 12,  /* call <hook> — ejecuta scene_hook_table[a]() (bloqueante permitido) */
    SCENE_OP_CAST_SPELL     = 13,  /* cast <spellName> <direct|reversed> — scripted, origin NARRATIVE */
    SCENE_OP_WAIT_SPELL     = 14,  /* espera a que el slot PLAYER quede libre */
    SCENE_OP_COMBAT         = 15,  /* combat — cede el control al FSM hasta win/lose */
    SCENE_OP_IF_COMBAT_WON  = 16,  /* if_combat_won goto <label> */
    SCENE_OP_FADE_OUT       = 17,
    SCENE_OP_FADE_IN        = 18,
    SCENE_OP_PLAY_MUSIC     = 19,
    SCENE_OP_STOP_MUSIC     = 20,
    SCENE_OP_NEXT_SCENE     = 21,  /* next_scene <act> <scene> */
    SCENE_OP_PUZZLE_SEQ     = 22,  /* puzzle_sequence → a = índice en puzzle_seqs[] (tabla lateral) */
    SCENE_OP_IF_PUZZLE_SOLVED = 23,/* if_puzzle_solved <tag> goto <label> */
    SCENE_OP_END            = 24
} SceneOp;

/* Un paso. 4 args s16 llegan para todos los ops porque los datos grandes
   (secuencias de puzzle) viven en tablas laterales referenciadas por índice. */
typedef struct {
    u8  op;         /* SceneOp */
    s16 a, b, c, d; /* argumentos según op; -1 si no se usa */
} SceneStep;

typedef struct {
    const char *name;          /* "act1_scene1" */
    const SceneStep *steps;    /* en ROM — la VM JAMÁS escribe aquí */
    u16 stepCount;
} SceneScript;

/* Secuencia esperada de un puzzle (tabla lateral GENERADA — resuelve el problema v1
   de que tag+count+N pares no caben en 4 argumentos). */
typedef struct {
    u8 tag;
    u8 len;                        /* nº de pasos de la secuencia */
    u8 spell[PUZZLE_SEQ_MAX];      /* SPELL_* */
    u8 reversed[PUZZLE_SEQ_MAX];   /* 0=directo, 1=invertido */
} PuzzleSeq;

/* GENERADOS por gen_scenes.py: */
extern const SceneScript scenes[];
extern const u8 scene_count;
extern const PuzzleSeq puzzle_seqs[];

/* Estado de la VM (RAM): */
extern u8 last_choice;
extern u8 puzzle_progress[PUZZLE_MAX][PUZZLE_SEQ_MAX];  /* pasos registrados por tag */
```

### 4.3 Tabla de hooks (`src/scenes/scene_hooks.c`)

```c
/* Firma única: un hook es una función void(void) que puede bloquear usando
   next_frame(), igual que el código de act_1.c hoy. */
typedef void (*SceneHook)(void);

/* La tabla la escribe el autor; gen_scenes.py valida los nombres de `call`
   contra ella parseando este archivo (o un listado hooks.txt junto a él). */
const SceneHook scene_hook_table[] = {
    [HOOK_ACT1_BEDROOM_ITEMS] = act1_bedroom_items,
    [HOOK_ACT1_CORRIDOR_ITEMS] = act1_corridor_items,
};
```

`act1_bedroom_items()` contiene el bucle de interacción actual de `act_1.c:63-101`
(4 items en cualquier orden, flags, activación única de sleep, timeout que solo cuenta
tras cabinet+pausa y se resetea al interactuar). Es exactamente el tipo de lógica que
el DSL **no** debe intentar expresar.

### 4.4 DSL de escenas (`data/scenes/*.scene`)

```
# data/scenes/act1_scene1.scene
scene act1_scene1

# setup
level ACT1_LEVEL_BEDROOM limits 0 3200
char CHR_LINUS at 35 175 face_left
char CHR_LINUS show

say ACT1_DIALOG4 A1D4_OVERSLEPT

# bucle de interacción con los items del dormitorio (lógica en C)
call act1_bedroom_items

say ACT1_DIALOG4 A1D4_MOTHER_CALLS
say ACT1_DIALOG4 A1D4_TOO_LATE
fade_out 10
next_scene 1 2
end
```

Ejemplo con choice, puzzle y combate:

```
# fragmento ilustrativo
choice ACT1_CHOICE1
branch 0 goto path_walk
branch 1 goto path_sleep

label path_walk
  combat
  if_combat_won goto victory
  goto defeat

label victory
  puzzle_sequence DOOR_TAG open:direct fire:reversed thunder:direct
  ...
  if_puzzle_solved DOOR_TAG goto door_open
  goto victory
```

### 4.5 Reglas del DSL

- Comentarios con `#`. Una directiva por línea. Constantes en `MAYÚSCULAS`, labels y
  nombres de escena/hook en `snake_case`.
- `label`/`goto`/`branch`: `gen_scenes.py` resuelve nombres a índices de step.
- `say`/`say_cluster` referencian ids de `data/texts.csv` (validados).
- `choice` referencia un set de `data/choices.csv` (validado). El resultado queda en
  `last_choice` (RAM); `branch <idx> goto <label>` salta si coincide.
- `cast` referencia un `SPELL_*` (validado contra `src/spells/constants_spells.h`).
- `call` referencia un hook (validado contra `scene_hooks.c`).
- `wait <n>` espera n **décimas de segundo** (D11). El DSL nunca expresa frames.
- `puzzle_sequence <tag> <spell:dir>...` — el generador emite la secuencia en
  `puzzle_seqs[]` y el step guarda solo el índice.
- `end` termina la escena (implícito si falta).

### 4.6 VM de escenas (`src/scenes/scene_vm.c`)

```c
/* Ejecuta una escena completa. Bloquea hasta END/next_scene. */
void scene_run(u8 sceneId)
{
    const SceneScript *s = &scenes[sceneId];
    u16 pc = 0;
    while (pc < s->stepCount) {
        const SceneStep *st = &s->steps[pc];    // ROM: solo lectura
        switch (st->op) {
            case SCENE_OP_LEVEL:      scene_level(st->a, st->b, st->c); break;
            case SCENE_OP_CHAR_AT:    scene_char_at(st->a, st->b, st->c, st->d); break;
            case SCENE_OP_SAY:        scene_say(st->a, st->b); break;
            case SCENE_OP_MOVE:       scene_move(st->a, st->b, st->c); break;      // bloqueante (B5, intencional)
            case SCENE_OP_WAIT_FOLLOWERS: scene_wait_followers(); break;           // bloqueante (B6, intencional)
            case SCENE_OP_CHOICE:     last_choice = scene_choice(st->a); break;    // resultado a RAM, nunca al step
            case SCENE_OP_BRANCH:     if (last_choice == st->a) { pc = st->b; continue; } break;
            case SCENE_OP_GOTO:       pc = st->a; continue;
            case SCENE_OP_WAIT:       scene_wait_tenths(st->a); break;             // escala con SCREEN_FPS
            case SCENE_OP_CALL:       scene_hook_table[st->a](); break;
            case SCENE_OP_CAST_SPELL: scene_cast_spell(st->a, st->b); break;
            case SCENE_OP_WAIT_SPELL: scene_wait_spell(); break;
            case SCENE_OP_COMBAT:     scene_combat_run(); break;                   // FSM hasta win/lose
            case SCENE_OP_IF_COMBAT_WON: if (scene_combat_won()) { pc = st->b; continue; } break;
            case SCENE_OP_PUZZLE_SEQ: scene_puzzle_define(&puzzle_seqs[st->a]); break;
            case SCENE_OP_IF_PUZZLE_SOLVED:
                if (scene_puzzle_solved(st->a)) { pc = st->b; continue; } break;
            case SCENE_OP_NEXT_SCENE: current_act = st->a; current_scene = st->b; return;
            case SCENE_OP_END:        return;
            /* ... resto de ops ... */
        }
        pc++;
    }
}
```

Notas de diseño de la VM (correcciones sobre v1):

- **La VM nunca escribe en los steps** (están en ROM; en Mega Drive la escritura se
  ignora en silencio y produce bugs indetectables). Todo estado mutable va a RAM.
- **La VM no llama `next_frame(true)` entre steps.** Los ops instantáneos (setup,
  branch, goto) se encadenan sin ceder frame; los ops que consumen tiempo (`say`,
  `move`, `wait`, `call`, `combat`) ya bloquean internamente y gestionan sus frames,
  como el código actual. Así el jugador no recibe input a mitad del setup.
- Las primitivas (`scene_say`, `scene_move`, ...) viven en `scene_api.c` y son wrappers
  finos sobre funciones existentes (`talk_dialog`, `move_entity`, `spell_launch` con
  origin NARRATIVE, `PAL_fadeOut`...).

### 4.7 Migración de las 4 escenas actuales

`act_1.c` tiene `act_1_scene_1/2/3/5` (la 4 no existe). Traducción honesta — qué parte
va al DSL y qué parte queda como hook C:

| Escena | DSL | Hooks C (`scene_hooks.c`) |
|---|---|---|
| `act1_scene1` | setup, diálogos, fade, next_scene | `act1_bedroom_items` (bucle de items con flags, sleep una sola vez, timeout condicionado a pausa — `act_1.c:63-101`) |
| `act1_scene2` | setup, movimientos, diálogos, fade | `act1_corridor_items` (bucle de libros/puertas/mapas del pasillo + condición de salida, `act_1.c:146-200`) |
| `act1_scene3` | setup, diálogos, choices con branch (hall de Clio/Xander) | ninguno |
| `act1_scene5` | setup del bosque, tutorial, `combat`, finale (`SYS_hardReset` → `end`) | posible hook para el setup de enemigos/scroll previo al combate (`act_1.c:313-339`) |

`act_1_scene_4` se deja pendiente; el DSL permite añadirla después sin C (si es lineal)
o con un hook (si no).

### 4.8 Puzzles narrativos

- `puzzle_sequence` define la secuencia esperada (tabla lateral `puzzle_seqs[]`).
- Cada `cast ... direct/reversed` que termina (y cada hechizo del jugador con origin
  NARRATIVE lanzado durante la escena) registra `(spell, reversed)` en
  `puzzle_progress[tag]` vía `spell_hook_narrative_advance` (§3.4).
- `if_puzzle_solved <tag>` compara el progreso con la secuencia esperada.

### 4.9 `gen_scenes.py`

Entrada: `data/scenes/*.scene`. Salida: `src/scenes/scene_data.c/.h`
(+ `enum SceneId`, tabla `scenes[]`, tabla `puzzle_seqs[]`).

Pasos: parsear → validar (labels, dialog ids contra `texts.csv`, choice sets contra
`choices.csv`, spells contra `constants_spells.h`, hooks contra `scene_hooks.c`) →
resolver labels a índices → emitir con cabecera `// Generated by tools/gen_scenes.py — DO NOT EDIT`.

Regla de robustez: cualquier referencia no resuelta es **error fatal** del generador
(no warning). Es la línea de defensa principal contra escenas rotas.

---

## 5. Higiene de cabeceras e includes (D1)

### 5.1 Retirada de `globals.h` (gradual)

Hoy cada `.c` incluye solo `globals.h`, que arrastra `<genesis.h>`, todos los `res_*.h`
y todos los `.h` de módulos. Consecuencias: recompilación total ante cualquier cambio,
dependencias ocultas, acoplamiento.

**Nuevo modelo**: el Makefile añade `-Isrc -Ires` y cada `.c` lista lo que usa, con
rutas relativas a esas raíces (sin `../`):

```c
/* src/combat/combat.c */
#include <genesis.h>
#include "combat/combat.h"
#include "combat/constants_combat.h"
#include "spells/spell.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "core/frame.h"
#include "audio/sound.h"
#include "res_sound.h"          /* solo los recursos que usa; -Ires lo resuelve */
```

**Calendario realista (corrección sobre v1)**: convertir los includes de `patterns.c`
(674 líneas) y `act_1.c` (346 líneas) sería trabajo tirado, porque las Fases 4 y 5 los
reescriben o eliminan. Por tanto:

- **Fase 3**: se convierten todos los módulos **excepto** `patterns.*`, `patterns/*` y
  `act_1.*`, que se mueven de sitio pero siguen incluyendo un `globals.h` transicional
  (reducido a lo que esos dos módulos necesitan).
- **Fase 4**: el código nuevo de `spells/` nace con includes explícitos; `patterns.*`
  desaparece.
- **Fase 5**: el código nuevo de `scenes/` nace con includes explícitos; `act_1.*` y
  **`globals.h` desaparecen definitivamente**.

`globals.c` se parte en Fase 3: `frame.c` (`next_frame`, `wait_seconds`, `calc_ticks`,
`frame_counter`), `config.h` (constantes, `GAME_VERSION`, `dprintf`), `init.c` (ya existe).

### 5.2 Diagramas de arquitectura en cabeceras

Las cabeceras de subsistemas grandes (`entity.h`, `spell.h`, `combat.h`, `scene_vm.h`)
empiezan con un bloque `/* ... */` que explica: qué hace el subsistema, qué archivos lo
componen, el data flow en un frame típico y su relación con otros subsistemas.

Ejemplo (`src/spells/spell.h`):

```c
/*
 * src/spells/spell.h — Sistema de hechizos
 * -----------------------------------------
 * Un hechizo es una SpellDef (datos + fases declarativas opcionales + hooks).
 * El motor (spell.c) gestiona el ciclo de vida en DOS slots simultáneos
 * (jugador y enemigo — necesarios para el counter): launch, update por
 * frames, counter, finish. Los hooks implementan la lógica específica.
 *
 * Archivos:
 *   spell.c         — motor (validate, launch, update, try_counter, finish)
 *   spell_hooks.c   — hooks compartidos reutilizables
 *   player_spells.c — defs + hooks de hechizos del jugador
 *   enemy_spells.c  — defs + hooks de hechizos de enemigo
 *   fire.c          — ejemplo end-to-end de hechizo con zona y counter
 *
 * Data flow en un frame de combate:
 *   controller → spell_validate(notas) → spell_launch(id, SLOT_PLAYER, ctx)
 *   update_combat → spell_update() → fases/onUpdate de ambos slots → spell_finish
 *   counter: jugador valida invertido durante efecto enemigo → spell_try_counter()
 *
 * Uso en narrativa:
 *   scene_vm → SCENE_OP_CAST_SPELL → spell_launch(id, SLOT_PLAYER, ctx{NARRATIVE})
 *
 * Ver docs/spells.md para la guía de autoría.
 */
```

### 5.3 `constants_*.h` + `config.h` + `hack.h`

Los números mágicos se centralizan por dominio:

- `src/core/config.h` — `SCREEN_WIDTH/HEIGHT`, `GAME_VERSION`, `DEBUG_LEVEL`, `dprintf`.
  (`SCREEN_FPS` **no** va aquí: es una variable runtime, queda `extern` en `frame.h`.)
- `src/core/hack.h` — `HACK_INFINITE_HEALTH`, `HACK_SKIP_INTRO`, `HACK_START_SCENE`,
  `HACK_SMOKE_BUILD`, etc. Compile-time.
- `src/actors/constants_actors.h` — `MAX_CHR`, `MAX_ENEMIES`, `MAX_ITEMS`, velocidades,
  colisiones, distancias de follow.
- `src/combat/constants_combat.h` — duraciones de hurt, timeouts (en ms para
  `calc_ticks`, o expresiones `SCREEN_FPS`).
- `src/spells/constants_spells.h` — `SPELL_*`, `NOTE_*`, `ZONE_*`, `MAX_SPELL_PHASES`,
  macros de paleta hoy duplicadas (B19).
- `src/narrative/constants_narrative.h` — `DEFAULT_TALK_TIME`, `MAX_CHOICES`, `SIDE_*`.
- `src/scenes/constants_scenes.h` — `PUZZLE_MAX`, `PUZZLE_SEQ_MAX`, `HOOK_*`.
- `src/audio/constants_audio.h` — IDs de SFX con rangos reservados.

---

## 6. Bugs conocidos a corregir (Fase 1)

Verificados uno a uno contra el código. **Clasificados según su efecto** (corrección
sobre v1, que exigía "comportamiento idéntico al golden" a la vez que cambiaba
comportamiento — contradicción):

- **Grupo A — no cambian comportamiento observable**: se corrigen primero, en un commit.
- **Grupo B — SÍ cambian comportamiento**: se corrigen uno a uno, cada fix con playtest
  y una nota en `docs/refactor/baseline.md` describiendo el cambio esperado.
- **Grupo C — decisiones de producto**: preguntar al usuario antes (ver §15).

El build al cerrar la Fase 1 es el **baseline** para todo el refactor (D10).

| # | Grupo | Archivo:línea | Bug | Fix |
|---|---|---|---|---|
| B1 | B (latente) | `src/patterns/pattern_sleep.c:11` | `player_sleep_update` lee `playerPatterns[PATTERN_HIDE].baseDuration` (copy-paste). Hoy inobservable: sleep no es lanzable (`can_use` siempre false) | Usar `PATTERN_SLEEP` |
| B2 | A* | `src/patterns.c:548` | `update_enemy_pattern` hardcodea el pattern-slot 0; **EN_BITE (slot 1) nunca se ejecuta en combate** | **Decidido**: iterar slots activos + deshabilitar bite explícitamente (`has_pattern=false`). El combate queda idéntico al actual (por eso pasa a grupo A). Activar bite es decisión de contenido futura (§3.6) |
| B3 | **B** | `src/combat.c:75-76` | `try_counter_spell` hardcodea slot 0 | Buscar el slot del hechizo enemigo activo |
| B4 | **B** | `src/combat.c:135-138` | `while(!SPR_isAnimationDone)` bloqueante dentro de `hit_enemy` (llamado per-frame) | Convertir en estado FSM (`COMBAT_STATE_ENEMY_DEFEAT_ANIM`). Visualmente igual; cambia el timing interno |
| B5 | doc | `src/entity.c:50-60` | `move_entity` bloqueante con `next_frame` por step | **Intencional** (la VM de escena lo invoca en contexto blocking). Documentar, no tocar |
| B6 | doc | `src/controller.c:252-255` | espera de followers bloqueante | Idem B5 |
| B7 | A | `src/texts.c:83` | `malloc` en `encode_spanish_text` | Buffer estático `static char scratch[MAX_TEXT_LEN]` |
| B8 | A | `src/dialogs.c:267,336` | `free` del malloc anterior | Eliminar al pasar a buffer estático |
| B9 | A | `src/interface.c:243,269` | `MEM_alloc` sin NULL check | Añadir check + fallback |
| B10 | A | `src/controller.c:5` | `static u16 frame_counter` local: sombrea el global, se incrementa y jamás se lee | Eliminar |
| B11 | A | `src/patterns.h:16` | `MAX_ENEMY_PATTERNS 8` muerto (la real es `MAX_PATTERN_ENEMY 2` en `enemies.h:10`) | Eliminar |
| B12 | **B** | `src/characters.h:21-23` vs `src/texts.h:9-10` | Dos vocabularios "side" y ambos son `bool`: `SIDE_left=true`, `SIDE_right=false`, **`SIDE_none=true`** (¡igual que left!) vs `SIDE_LEFT/SIDE_RIGHT`. **No es un rename**: unificar en un tipo de 3 valores (`u8`: `SIDE_LEFT=0, SIDE_RIGHT=1, SIDE_NONE=2`) cambia firmas de funciones y el significado de "none" | Nuevo tipo + actualizar firmas y llamadas. `texts.csv` ya usa `SIDE_LEFT`, compatible |
| B13 | A | include guards | Dos convenciones (`_FOO_H_` vs `FOO_H`) | Unificar en `_FOO_H_` |
| B14 | A | `src/globals.c:48` | Comentario garbled "deTODO" (sed accidental de "depending") | "depending on" |
| B15 | A | `src/items.h:29` | Idem | Idem |
| B16 | (→F4) | `src/combat.c:130` | `// TODO: Better enemy defeat handling` | **Decidido**: en Fase 1 solo el fix mínimo de B4 (mismo resultado visual); el manejo mejorado de la derrota se diseña con el motor nuevo en Fase 4 |
| B17 | doc | `src/sound.c:85,105` | `// TODO` jingles de SLEEP y EN_BITE | **Decidido**: documentar como pendiente en AGENTS.md (es trabajo de contenido/audio, no de refactor); el motor nuevo los soporta cuando existan |
| B18 | (→F4) | `src/patterns/pattern_thunder.c:13-27`, `patterns.c:302-345` | Callbacks de pattern muestran diálogos (`dialogs[ACT1_DIALOG3]`) — acoplamiento | Se resuelve en Fase 4 con el hook `onRejected` (§3.1). El diálogo sigue viviendo junto al hechizo (es su feedback), pero pasa por un mecanismo del motor, no incrustado en `launch` |
| B19 | A | `pattern_thunder.c:3-4`, `pattern_en_thunder.c:7-8` | Macros `PAL_ENTRY`/`PAL0_COL4` duplicadas | Mover a un header común (futuro `constants_spells.h`; en Fase 1, a `patterns.h`) |
| B20 | A | `src/entity.h:19-30` | `GameState` con valores presuntamente muertos (`STATE_PATTERN_FINISHED`, `STATE_PATTERN_CHECK`, `STATE_ATTACK_FINISHED`, `STATE_FOLLOWING`) | Verificar uso real con grep; eliminar los muertos o documentar |
| B21 | A | `src/enemies.c:53-54` | Sprite de 3-head-monkey comentado, clase sin implementar | **Decidido**: eliminar la clase (código muerto). Con el motor nuevo, re-añadir un enemigo cuando exista su arte será trivial (receta en AGENTS.md) |
| B22 | A | `src/act_1.h:6` | Typo "3nd" | "3rd" (el archivo muere en Fase 5 igualmente) |
| B23 | A (latente) | `src/act_1.c:64` | `bool scene_timeout=0` usado como **contador de frames** comparado contra `SCREEN_FPS*3` (=180). Funciona solo porque SGDK define `bool` como `u8` (`types.h:121`); desbordaría en timeouts > 4,25 s | Cambiar a `u16` |

---

## 7. Makefile y build

### 7.1 Estado actual (documentado para poder partir de él)

Hoy el proyecto se compila con **`~/codigo/build-sgdk.sh`**, un script externo
compartido entre proyectos, invocado desde `.vscode/tasks.json`. Usa el `makefile.gen`
de SGDK (`~/sgdk`). No hay Makefile en el repo. La Fase 0 debe anotar la invocación
exacta y sus flags antes de tocar nada.

### 7.2 `Makefile` nuevo (propio del repo) — IMPLEMENTADO en Fase 2

Ajuste sobre el plan original: en vez de reimplementar el makefile.gen, el `Makefile`
del repo **lo envuelve** (`include $(GDK)/makefile.gen`), porque makefile.gen ya hace
todo lo que D7 pedía: wildcard discovery de `src/*.c`, `src/*/*.c` y `res/*.res`,
include paths `-Isrc -Ires`, targets `release`/`debug`/`clean` y los warnings del
proyecto. Menos código propio que mantener y cero divergencia con SGDK.

- El Makefile corre **dentro del contenedor** (la imagen aporta el toolchain):
  `docker run ... --entrypoint make <imagen> GDK=/sgdk -f /src/Makefile <target>`.
- **El codegen corre en el HOST** (la imagen SGDK no incluye python3): lo lanza
  `build-theweave.sh` antes de compilar (`tools/gen_texts.py`; la Fase 5 añade
  `gen_choices.py` y `gen_scenes.py`).
- **Target `smoke`** (Fase 7): se añadirá al Makefile del repo tras el include;
  si el filtrado de `main.c` resulta incómodo envolviendo makefile.gen, la
  alternativa es `#ifndef HACK_SMOKE_BUILD` alrededor del `main()` del juego.

### 7.3 `build-theweave.sh`

Wrapper propio del repo (sustituye a `build-sgdk.sh`) — IMPLEMENTADO en Fase 2,
adaptado de `build-RedPlanet.sh`:

- Modos: `build` (incremental), `release`/`full` (clean+release), `clean`
  (`smoke` se añade en Fase 7). Flag `--no-run` / `-n`.
- Pipeline: codegen en host → make en contenedor → backup rotatorio
  `out/TheWeave_YYYYMMDD_HHMMSS.bin` (mantiene 5) → MiSTer (FTP + Remote API) si
  está online, si no BlastEm.
- SGDK: por defecto el de la imagen (`/sgdk`, lo que se ha usado y validado hasta
  ahora); `SGDK_LOCAL=~/sgdk` lo monta readonly y lo usa en su lugar.
- Configuración personal (IP/credenciales MiSTer, BlastEm, SGDK local) en
  `.build-theweave.local.sh`, no versionado (gitignored).

### 7.4 Integración con VS Code

`.vscode/tasks.json` se actualiza para apuntar a `./build-theweave.sh` (hoy apunta a
`${env:HOME}/codigo/build-sgdk.sh`):

- "Build (Incremental)" → `./build-theweave.sh build`
- "Clean + Build (Release)" → `./build-theweave.sh release`
- "Build Smoke ROM" → `./build-theweave.sh smoke`
- "Run ROM (BlastEm)" / "Run Smoke ROM (BlastEm)"
- "Generate Data" → `make codegen`

---

## 8. ROM de smoke test (`smoke/`)

### 8.1 Objetivo

Un ROM aparte (`out/smoke.bin`) con un menú que ejecuta cada hechizo y cada escena de
forma aislada, para verificación rápida en BlastEm sin jugar el juego entero. Es el
test principal (D4).

### 8.2 Estructura

- `smoke_main.c` — `main()` propio (el del juego queda excluido del target).
- `smoke_menu.c` — menú con secciones **Spells** y **Scenes**; UP/DOWN + A.
- `smoke_runner.c` — ejecuta el caso:
  - **Spell**: monta un `SpellContext` mock (zona y hechizo enemigo activo según el
    caso), lanza, corre `spell_update` hasta terminar y muestra invariantes: daño
    aplicado, si `canUse` rechazó, frames consumidos. PASS/FAIL en pantalla.
  - **Scene**: `scene_run(sceneId)` y al terminar muestra "OK" o el step donde paró.
- `smoke_cases.h` — tabla de casos:

```c
typedef enum { SMOKE_SPELL, SMOKE_SCENE } SmokeKind;
typedef struct { const char *name; SmokeKind kind; u8 id; u8 zoneId; u8 enemySpell; } SmokeCase;
static const SmokeCase smoke_cases[] = {
    {"thunder (player)",       SMOKE_SPELL, SPELL_THUNDER,    0,             SPELL_NONE},
    {"thunder (counter)",      SMOKE_SPELL, SPELL_THUNDER,    0,             SPELL_EN_THUNDER},
    {"hide",                   SMOKE_SPELL, SPELL_HIDE,       0,             SPELL_NONE},
    {"fire (no zone)",         SMOKE_SPELL, SPELL_FIRE,       0,             SPELL_NONE},  /* espera rechazo */
    {"fire (zone cauldron)",   SMOKE_SPELL, SPELL_FIRE,       ZONE_CAULDRON, SPELL_NONE},
    {"fire (counter thunder)", SMOKE_SPELL, SPELL_FIRE,       ZONE_CAULDRON, SPELL_EN_THUNDER},
    {"en_thunder",             SMOKE_SPELL, SPELL_EN_THUNDER, 0,             SPELL_NONE},
    {"en_bite",                SMOKE_SPELL, SPELL_EN_BITE,    0,             SPELL_NONE},
    {"act1_scene1",            SMOKE_SCENE, SCENE_ACT1_SCENE1, 0, 0},
    {"act1_scene2",            SMOKE_SCENE, SCENE_ACT1_SCENE2, 0, 0},
    {"act1_scene3",            SMOKE_SCENE, SCENE_ACT1_SCENE3, 0, 0},
    {"act1_scene5",            SMOKE_SCENE, SCENE_ACT1_SCENE5, 0, 0},
};
```

### 8.3 Verificación

- **Automática donde sea posible**: invariantes chequeables (HP tras en_thunder,
  rechazo de fire fuera de zona, hechizo termina en `baseDuration`±1). PASS/FAIL.
- **Visual**: el humano compara contra el baseline. `docs/testing.md` incluye
  screenshots de referencia del baseline para cada caso.
- **Sin diff binario ni "log de VDP"**: v1 proponía comparar logs de estado VDP contra
  el golden; se descarta por coste/beneficio (BlastEm no lo da de serie y el árbitro
  final es humano igualmente).

### 8.4 Cómo añadir un caso

1. Añadir una fila a `smoke_cases[]`.
2. (Opcional) Screenshot de referencia a `docs/testing/`.
3. Rebuild smoke. Sin tocar el menú ni el runner.

---

## 9. Documentación

### 9.1 `AGENTS.md` exhaustivo (fuente de verdad)

Secciones:

1. **Qué es el proyecto** — Loom fangame, Mega Drive, SGDK, C, estado actual.
2. **Cómo se compila y ejecuta** — `build-theweave.sh` (todos los modos), Makefile,
   requisitos, BlastEm, troubleshooting.
3. **Estructura de directorios** — árbol con descripciones de una línea.
4. **Arquitectura del código** — por subsistema: archivos, structs clave, data flow.
   (Aquí vive lo que v1 quería en `docs/architecture.md`.)
5. **Constantes y parámetros clave** — `config.h`, `hack.h`, `constants_*.h`.
6. **Generación de datos** — los 3 scripts de `tools/`, qué lee y emite cada uno,
   regla "no editar ficheros generados".
7. **Recursos (`res/`) y arte** — qué hay en cada `.res`, convenciones.
8. **Voces (`tools/voice/`)** — pipeline animalese, cómo regenerar.
9. **Reglas de codificación** — tipos, memoria, includes explícitos, estilo actual
   (snake_case, Allman en funciones, `//` junto a firmas), no editar generados.
10. **Tareas comunes** — recetas paso a paso: añadir un hechizo (un `.c` + constante +
    init + caso smoke), añadir una cutscene (`.scene` + codegen + caso smoke), añadir
    diálogos (`texts.csv`), añadir un choice (`choices.csv`), añadir un hook de escena,
    añadir un enemigo, añadir un item, crear un puzzle de secuencia.
11. **Estado y pendientes** — act1_scene4, actos futuros, decisiones abiertas.

Cabecera con directiva de sync:
> FUNDAMENTAL: Actualiza este fichero después de cada interacción si aplica. Mantenlo
> sincronizado con el estado real del código.

### 9.2 `docs/` técnica (reducida respecto a v1)

Solo las guías que se abren de verdad — menos ficheros que mantener sincronizados:

- `docs/spells.md` — guía de autoría de hechizos (struct, fases, hooks, ejemplo fire,
  tabla de hechizos, debugging con smoke ROM).
- `docs/scenes.md` — guía de autoría de cutscenes (DSL completo, tabla de opcodes,
  cuándo usar un hook C, ejemplo end-to-end, puzzles).
- `docs/texts.md` — `texts.csv`/`choices.csv`, escapes (`|`, `@[...@]`), encoding, voces.
- `docs/build.md` — Makefile, script, SGDK, BlastEm, VS Code.
- `docs/testing.md` — smoke ROM, checklist de playtest, screenshots de referencia.
- `docs/refactor/` — bitácora (ver §11).

La arquitectura, el arte y las voces se documentan en `AGENTS.md` (§9.1), no en
ficheros propios.

### 9.3 Comentarios en funciones (estilo actual, no plantillas)

Se mantiene el estilo vigente del proyecto:

- **Declaración en `.h`**: `tipo nombre(params); // qué hace en una línea` — hoy ya es
  así en la mayoría del código; se completa donde falte.
- **Definición en `.c`**: comentario `//` junto a la firma si aporta; `//` para notas
  internas no obvias.
- **Cabeceras de subsistema**: bloque `/* ... */` de arquitectura (§5.2).
- **Sin** plantillas `Parámetros:/Retorna:` ni Doxygen: v1 las imponía y chocan con el
  estilo del autor. Un comentario de una línea bien escrito documenta mejor que
  boilerplate que nadie actualiza.

---

## 10. Plan de ejecución por fases

Cada fase es un commit (o varios) en la rama `refactor`. Entre fases: build + smoke
(desde que exista) deben pasar. Si una fase rompe el build, se arregla antes de avanzar.

### ~~Fase 0 — Preparación~~ ✔ COMPLETADA (2026-07-06)

**Objetivo**: dejar el terreno listo sin perder el estado actual.

> Hecho: rama `refactor`, build documentado y verificado (Docker SGDK, sin warnings),
> ROM de referencia en `docs/refactor/rom_pre_refactor.bin` (gitignored), bitácora
> completa en `docs/refactor/`. Pendiente no bloqueante: playtest inicial del usuario
> (alimenta `baseline.md` durante la Fase 1).

1. `git checkout -b refactor`.
2. **Anotar el build actual**: comando exacto de `~/codigo/build-sgdk.sh` que usan las
   tasks de VS Code, flags, y verificar que produce un ROM funcional. Documentarlo en
   `docs/refactor/state_audit.md`.
3. Build limpio y guardar el ROM como referencia informal (`docs/refactor/rom_pre_refactor.bin`
   fuera de git o en LFS — recordar que la comparación será funcional, no binaria).
4. Generar `docs/refactor/state_audit.md` (informe de auditoría completo),
   `docs/refactor/bugs.md` (tabla §6) y `docs/refactor/checklist.md` (fases y pasos).
5. Playtest completo del juego actual grabando el comportamiento de las 4 escenas y
   los hechizos (notas + screenshots) → material base de `baseline.md`.

**Verificación**: rama creada, `docs/refactor/` poblado, build actual funciona.
**Rollback**: `git checkout master`.

### ~~Fase 1 — Corrección de bugs y baseline~~ ✔ COMPLETADA (2026-07-06)

> Hecho: B1-B15 y B19-B26 corregidos (6 commits + 3 de fixes post-test), B16-B18
> pospuestos según lo decidido, baseline validado por el usuario (incluido re-test
> dirigido del combate de la escena 5 tras corregir el cuelgue B26).

**Objetivo**: corregir §6 en dos tandas y **fijar el baseline** de comportamiento.

1. **Commit A — Grupo A** (no cambian comportamiento): B7, B8, B9, B10, B11, B13, B14,
   B15, B19, B20 (tras verificar con grep), B21 (eliminar clase 3-head-monkey), B22,
   B23, y B2 (iterar slots + bite deshabilitado explícitamente, §3.6). Build +
   playtest rápido.
2. **Commits B — Grupo B, uno por bug**: B1, B3, B4, B12. Cada commit: fix + playtest
   del área afectada + nota en `docs/refactor/baseline.md` describiendo el cambio
   observable, si lo hay.
3. **Decisiones ya tomadas** (§15): B16 pospuesto a Fase 4 (en Fase 1 solo el fix
   mínimo de B4); B17 se documenta como pendiente en AGENTS.md.
4. Documentar B5 y B6 como intencionalmente bloqueantes (comentario en el código).
5. **Cerrar baseline**: playtest completo, capturas, `baseline.md` terminado. Este
   build es la referencia de las fases siguientes (D10).

**Verificación**: build pasa; `bugs.md` actualizado con el estado de cada bug;
`baseline.md` describe el comportamiento de referencia.
**Rollback**: reset al commit de fin de Fase 0.

### ~~Fase 2 — Reestructuración de directorios (sin tocar `src/`)~~ ✔ COMPLETADA (2026-07-06)

> Hecho: `gibberish/`→`tools/voice/` (139 ficheros con `git mv`), scripts a `tools/`,
> `texts.csv`→`data/` (gen_texts.py regenera output idéntico), Makefile propio
> envolviendo makefile.gen, build-theweave.sh estilo RedPlanet (codegen + backup +
> MiSTer/BlastEm), .vscode/tasks.json y .gitignore actualizados. Build release
> verificado end-to-end con el pipeline nuevo, sin warnings.

1. `mkdir -p tools data/scenes smoke docs/refactor`.
2. `git mv gibberish tools/voice`.
3. `git mv generate_texts.py tools/gen_texts.py` (actualizar paths internos: lee
   `data/texts.csv`, escribe `src/texts_generated.{c,h}` por ahora).
4. `git mv add_texts_comments.py consolidate.py tools/`.
5. `git mv texts.csv data/texts.csv`.
6. Actualizar `.gitignore` (`out/`, `venv/`, `tools/voice/__pycache__/`...).
7. Crear `Makefile` (§7.2) y `build-theweave.sh` (§7.3) y verificar que el build del
   repo ya no depende de `~/codigo/build-sgdk.sh`. Actualizar `.vscode/tasks.json`.
8. Build + playtest.

**Verificación**: build pasa con el Makefile propio, `gen_texts.py` funciona desde
`tools/`, `gibberish/` no existe.
**Rollback**: reset al commit de fin de Fase 1.

### ~~Fase 3 — Partición de `src/` + includes explícitos (salvo módulos condenados)~~ ✔ COMPLETADA (2026-07-06)

> Hecho: src/ particionado en core/world/actors/combat/narrative/scenes/interface/audio
> (git mv, historia conservada); globals.c partido en core/frame.c + core/config.h +
> core/hack.h (con HACK_START_SCENE, el toggle que nació en la caza del cuelgue);
> encode.c/.h extraído de texts.c; texts_generated → narrative/texts_data (generador
> y add_texts_comments.py actualizados); 16 .c con includes explícitos y 14 .h
> auto-contenidos; globals.h reducido a umbrella transicional solo para patterns*/act_1*;
> diagramas de arquitectura en entity.h y combat.h. Ajuste documentado: sin
> constants_*.h por dominio (las constantes ya viven en el .h de cada módulo, estilo
> del autor). Build release limpio, 0 warnings.

**La fase más larga.** Sub-commits por módulo, build tras cada uno.

1. Crear subcarpetas `src/{core,world,actors,combat,spells,narrative,scenes,interface,audio}/`.
2. Mover módulos según el mapeo de §17 (con `git mv` para conservar historia).
3. Extraer de `globals.c/h`: `frame.c/.h`, `config.h`, `hack.h`. Crear los
   `constants_*.h` por dominio y mover a ellos los números mágicos.
4. Extraer `encode.c/.h` de `texts.c`. Renombrar `texts_generated` → `texts_data`
   (actualizar `gen_texts.py`).
5. **Convertir a includes explícitos** todos los módulos **excepto** `patterns.*`,
   `patterns/*` y `act_1.*` (§5.1): estos se mueven de sitio pero mantienen un
   `globals.h` transicional reducido, porque las Fases 4-5 los eliminan.
6. Diagramas de arquitectura (§5.2) en `entity.h`, `combat.h` (los de `spell.h` y
   `scene_vm.h` nacen con sus fases).
7. Verificar que el Makefile con wildcards compila todo sin editarlo.

**Verificación**: build pasa, cada `.c` (salvo los condenados) lista sus includes,
playtest de las 4 escenas idéntico al baseline.
**Rollback**: reset al fin de Fase 2. Si la conversión de includes de un módulo se
atasca, commit intermedio con ese módulo aún en `globals.h` transicional.

### ~~Fase 4 — Sistema de hechizos~~ ✔ COMPLETADA (2026-07-06)

> Hecho: motor de 2 slots en spells/ (spell.c + notes.c + player_spells + enemy_spells
> + fire como ejemplo canónico), espacio de ids UNIFICADO (el solape player/enemy del
> sistema viejo causó el cuelgue de la Fase 1), hooks incl. onRejected (B18 resuelto:
> los hints de thunder salen del flujo de notas), fases declarativas (FLASH + DAMAGE),
> CombatContext eliminado (el estado vive en el motor), patterns.* borrado. Ajustes:
> notes.c añadido (input/HUD separado del motor); sin spell_hooks.c (no hay hooks
> compartidos reales aún); jingle unificado play_spell_jingle. La retirada de
> patterns.h destapó includes enmascarados (los pattern_*.h arrastraban globals.h) —
> corregidos. AGENTS.md reescrito con la arquitectura y las recetas.

1. Structs y motor (§3.2, §3.3): `spell.h`, `spell.c` con los **dos slots**.
   Diagrama de arquitectura en la cabecera.
2. Hooks compartidos (`spell_hooks.c/.h`).
3. Migrar hechizos **uno a uno**, cada uno con su smoke check (con un main temporal
   hasta la Fase 7 o probando en juego): thunder (incluido `onRejected` con el hint
   WeaverGhost — resuelve B18), hide, open, sleep, en_thunder, en_bite. Comportamiento
   idéntico al baseline (que ya incluye los fixes B1-B4).
4. Integrar con `combat.c`: `update_combat` llama `spell_update()`;
   `spell_try_counter()` sustituye a `try_counter_spell`; eliminar la FSM vieja de
   patterns. Añadir `combat_run()` / `combat_result` (lo usará la VM en Fase 5).
5. Integrar con `interface.c` (iconos desde `spell_defs[]`) y `sound.c` (jingles por id).
6. Hechizo FIRE end-to-end (§3.5) con sus 3 casos smoke.
7. Eliminar `patterns.c/.h` y `src/patterns/`.

**Verificación**: los 6 hechizos + fire correctos contra baseline; playtest de
act1_scene5 (combate) idéntico al baseline.
**Rollback**: reset al fin de Fase 3.

### ~~Fase 5 — Sistema de cutscenes~~ ✔ COMPLETADA (código, 2026-07-06; pendiente playtest)

> Hecho según docs/refactor/fase5_design.md (documento de detalle previo, pedido por
> el usuario): VM (scene_vm) + 12 hooks C (scene_hooks, trasplante literal de act_1.c)
> + gen_scenes.py y gen_choices.py con validación fatal probada + 4 escenas en DSL
> (scene3 = 19 steps casi puro DSL) + choices.csv. main.c = bucle sobre scene_lookup.
> ELIMINADOS: act_1.*, globals.h (ya sin consumidores), add_texts_comments.py.
> Ajustes documentados: sin scene_api.c (la VM llama las primitivas directamente);
> ops de puzzle diseñados pero diferidos hasta que el guion los pida; op say_response
> nuevo (respuestas correlativas a choices).

1. Structs y VM (§4.2, §4.6): `scene_vm.c/.h`, estado en RAM, tabla lateral de puzzles.
2. API (`scene_api.c/.h`): wrappers sobre funciones existentes.
3. Hooks (`scene_hooks.c/.h`): `act1_bedroom_items` (traslada `act_1.c:63-101`),
   `act1_corridor_items`, tabla `scene_hook_table[]`.
4. `gen_scenes.py` (§4.9) con validación fatal de referencias.
5. Migrar escenas **una a una** con playtest completo tras cada una:
   scene1 (DSL + hook items), scene2 (DSL + hook items), scene3 (DSL puro con
   choices/branch), scene5 (DSL + op `combat` + hook de setup si hace falta).
6. `choices.csv` + `gen_choices.py`: mover `act1_choice1[]` de `texts.c` al pipeline.
7. `main.c`: sustituir el doble `switch` por `scene_run()` según `current_act/scene`.
8. Eliminar `act_1.c/.h`. Mover `intro` y `geesebumps` a `src/scenes/` como C normal.
9. **Eliminar `globals.h`/`globals.c` definitivamente** (ya no queda ningún consumidor).
10. **Retirar `add_texts_comments.py`** (decidido, §15): los `.scene` son legibles por
    sí mismos. Eliminarlo de `tools/` y anotarlo en AGENTS.md.

**Verificación**: playtest del juego completo (intro → act1 entero, ambas rutas del
choice, win y lose del combate) idéntico al baseline. `globals.h` no existe.
**Rollback**: reset al fin de Fase 4.

### ~~Fase 6 — Documentación~~ ✔ COMPLETADA (2026-07-06)

> Hecho: docs/{build,spells,scenes,texts,testing}.md escritas (guías de autoría y
> operación; la arquitectura vive en AGENTS.md y en las cabeceras de subsistema);
> cabeceras de archivo completadas en los módulos antiguos. AGENTS.md se ha ido
> manteniendo al día fase a fase (regla de sync en su cabecera).

1. Reescribir `AGENTS.md` (11 secciones de §9.1) — es la fuente de verdad y la usan
   las herramientas: máxima prioridad de exactitud.
2. Escribir `docs/spells.md`, `docs/scenes.md`, `docs/texts.md`, `docs/build.md`,
   `docs/testing.md`.
3. Completar los comentarios `//` de una línea que falten en los `.h` (estilo §9.3).
4. Cabecera breve (`// qué es este archivo`) en cada `.c`/`.h` que no la tenga.

**Verificación**: un desarrollador nuevo puede añadir un hechizo, una escena y un
diálogo siguiendo solo la doc.
**Rollback**: no rompe nada.

### ~~Fase 7 — Smoke ROM + verificación final~~ ✔ COMPLETADA (2026-07-07)

> Hecho: smoke ROM en src/smoke/ (menú con 14 casos: 5 CHECK de canUse automáticos,
> 5 CAST con medición de duración, 4 SCENE completas), compilada con
> -DHACK_SMOKE_BUILD vía `./build-theweave.sh smoke` → out/smoke.bin (el main del
> juego queda excluido por #ifndef). Ambas ROMs compilan limpias de cero. Merge a
> master + tag v2.0-refactor. Los screenshots de referencia quedan como paso del
> próximo playtest (docs/testing.md).

1. `smoke/` completo (§8): main propio, menú, runner, tabla de casos.
2. Target `smoke` del Makefile (excluyendo `src/core/main.c`) → `out/smoke.bin`.
3. `build-theweave.sh smoke`.
4. Pasar todos los casos; capturar screenshots de referencia a `docs/testing/`.
5. Playtest final completo contra `baseline.md`. Discrepancias: documentar en
   `checklist.md` y decidir (aceptar o arreglar).
6. Merge `refactor` → `master` (solo si todo pasa). Tag `v2.0-refactor` (opcional).

**Verificación**: `out/smoke.bin` todo PASS; playtest final conforme al baseline;
`master` actualizado.
**Rollback**: `git reset --hard <pre-merge>` en master.

---

## 11. Bitácora del refactor (`docs/refactor/`)

- `state_audit.md` — estado inicial + build actual documentado (Fase 0).
- `bugs.md` — tabla §6 con estado (corregido/pendiente/aceptado).
- `baseline.md` — comportamiento de referencia post-Fase-1, incluyendo los cambios
  introducidos por los fixes del Grupo B (Fase 1).
- `checklist.md` — fases y pasos, se tacha al completar.

Se actualiza al cerrar cada fase.

---

## 12. Pruebas (resumen, D4)

- **Referencia**: el **baseline post-Fase-1** (D10), descrito en `baseline.md` con
  notas y screenshots. **Nunca diff binario de ROMs** (`GAMEVERSION` incrusta
  `__DATE__`: dos builds de días distintos siempre difieren).
- **Test principal**: la smoke ROM (§8), con invariantes automáticos donde se pueda y
  comparación visual donde no.
- **Playtest manual con checklist** (`docs/testing.md`) tras cada fase desde la 3.
- **Unit tests de lógica pura en PC** (futuro, no en este plan): la arquitectura los
  permite — `spell_validate`, `encode_spanish_text` y la VM no tocan hardware.

**Criterio de no-regresión**: desde la Fase 4, todos los casos smoke disponibles PASS
tras cada fase.

---

## 13. Convenciones de estilo (las actuales del proyecto, aplicadas con consistencia)

- **Indentación**: 4 espacios, sin tabs.
- **Nombres**: `snake_case` funciones y variables; `UPPER_CASE` macros/constantes;
  `PascalCase` tipos (`Entity`, `SpellDef`, `SceneStep`).
- **Tipos**: SGDK fixed-width (`u8/u16/u32/s8/s16/s32/bool`). Nunca `int`/`short`
  bare. Ojo: `bool` es `u8` en SGDK — no usarlo como contador (B23).
- **Llaves**: Allman en funciones (estilo actual); K&R same-line en
  `if/for/while/switch` (lo más común en el código actual).
- **Comentarios**: `//` junto a la firma y para notas; bloques `/* */` solo para
  cabeceras de arquitectura. Sin plantillas, sin emojis, sin texto garbled.
- **Includes**: explícitos por archivo (D1). Orden: `<genesis.h>`, cabecera propia,
  módulos del proyecto, recursos (`res_*.h`), constants. Rutas relativas a `-Isrc -Ires`.
- **Memoria**: sin `malloc`/`free`. Pools y buffers estáticos. `static const` para
  tablas inmutables (ROM). Las tablas que necesitan `SCREEN_FPS` se inicializan en
  runtime (RAM), como hoy.
- **Globals**: `extern` en `.h`, definidos en un `.c` del subsistema. Mínimos.
- **Funciones privadas**: `static`.
- **Data tables**: designated initializers (`[SPELL_THUNDER] = {...}`, `.campo = valor`).
- **Tiempos**: siempre `SCREEN_FPS`-relativos o `calc_ticks(ms)`, evaluados en runtime
  (D11). Prohibido `240` a pelo.
- **Include guards**: `_FOO_H_`.
- **Generados**: no editar `*_data.c/.h` ni `res_*.h`; cabecera `// Generated by ...`.

---

## 14. Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| Los fixes B3/B4/B12 cambian el gameplay de forma indeseada | Se hacen en Fase 1, uno por commit, con playtest antes de fijar el baseline. Nada posterior los enmascara. (B2 ya no es riesgo: bite queda deshabilitado, §15.) |
| Retirada de `globals.h` rompe mucho de golpe | Gradual: módulo a módulo en Fase 3, los módulos condenados esperan a Fases 4-5, `globals.h` muere al final de la 5. |
| Migración de hechizos cambia comportamiento sutil | Un hechizo por commit contra baseline; el motor de 2 slots reproduce el flujo real del counter en vez de aproximarlo. |
| Migración de escenas pierde un branch o interacción | Una escena por commit con playtest completo; la lógica difícil NO se traduce a DSL sino que se traslada casi literal a un hook C; `gen_scenes.py` falla en duro ante referencias rotas. |
| El DSL se queda corto para una escena futura | Primera opción: hook C (`call`) — es el mecanismo previsto, no un parche. Segunda: nuevo `SceneOp` si el patrón se repite en 3+ escenas. |
| `gen_scenes.py` tiene bugs | Validación fatal + el generado se compila (el compilador C es la segunda red) + smoke ROM por escena. |
| Makefile propio no replica flags de SGDK | Partir de `makefile.gen`, comparar el ROM resultante con el del build viejo el mismo día (mismo `__DATE__` → comparable) antes de retirar el script antiguo. |
| Performance por capas nuevas (VM, fases) | No prioritario (objetivo 1). El hardware va sobrado para este juego; medir solo si se nota. |
| Tamaño del refactor | 8 fases con commits independientes; se puede parar entre fases con el juego funcionando. |

---

## 15. Decisiones tomadas con el usuario (2026-07-06)

Todas las preguntas que bloqueaban fases están resueltas:

| Tema | Decisión |
|---|---|
| B2 (EN_BITE inactivo) | Corregir el bug del slot pero **mantener bite deshabilitado** explícitamente (`has_pattern=false`). El combate queda idéntico. Activarlo será decisión de contenido futura (flag + ajustar recarga con playtest). |
| B16 (defeat handling) | **Posponer a Fase 4**: en Fase 1 solo el fix mínimo de B4 (mismo resultado visual). |
| B17 (jingles SLEEP/EN_BITE) | **Documentar como pendiente** en AGENTS.md; es trabajo de contenido/audio, no de refactor. |
| B21 (3-head-monkey) | **Eliminar la clase** (código muerto); re-añadirla será trivial con el motor nuevo cuando exista el arte. |
| `build-theweave.sh` | **Replicar RedPlanet**: backup rotatorio de ROMs (mantiene 5) + FTP a MiSTer si está online, si no BlastEm. Usar `build-RedPlanet.sh` como referencia. |
| `add_texts_comments.py` | **Retirar en Fase 5**: los `.scene` son legibles por sí mismos. |

Única cuestión abierta (no bloquea el refactor): cuándo activar EN_BITE y con qué
recarga — decisión de diseño de juego, posterior al refactor.

---

## 16. Cómo ejecutar este plan ("venga, refactoriza")

1. Confirmar que `refactorizar.md` está al día y las decisiones de §1 cuadran.
   (Las preguntas de implementación ya están resueltas — §15.)
2. Ejecutar fases 0 → 7 en orden. Tras cada fase:
   - Build: `./build-theweave.sh build` (desde Fase 2; antes, el build actual anotado
     en Fase 0).
   - Smoke: `./build-theweave.sh smoke` (desde Fase 7; antes, playtest manual dirigido).
   - Playtest de la escena/hechizo afectado contra `baseline.md`.
   - Actualizar `docs/refactor/checklist.md` y tachar la fase aquí.
   - Commit `Refactor Fase N: <resumen>`.
3. Si una fase revela que una decisión de §1 es mala: parar, actualizar este documento
   y `AGENTS.md`, y continuar.
4. Al final (Fase 7): merge a master, tag `v2.0-refactor`.

**Regla de oro**: si el build no pasa o el playtest revela una regresión respecto al
baseline, se arregla antes de avanzar. No se acumula deuda durante el refactor.

---

## 17. Apéndice: mapeo archivo-viejo → archivo-nuevo

| Archivo actual | Archivo refactorizado | Fase |
|---|---|---|
| `src/globals.h` | transicional en Fase 3 → **eliminado** | 5 |
| `src/globals.c` | `src/core/frame.c/.h` + `src/core/config.h` | 3 |
| `src/main.c` | `src/core/main.c` | 3 |
| `src/init.c/.h` | `src/core/init.c/.h` | 3 |
| `src/controller.c/.h` | `src/core/controller.c/.h` | 3 |
| `src/entity.c/.h` | `src/actors/entity.c/.h` | 3 |
| `src/characters.c/.h` | `src/actors/characters.c/.h` | 3 |
| `src/enemies.c/.h` | `src/actors/enemies.c/.h` | 3 |
| `src/items.c/.h` | `src/actors/items.c/.h` | 3 |
| `src/collisions.c/.h` | `src/actors/collisions.c/.h` | 3 |
| `src/background.c/.h` | `src/world/background.c/.h` | 3 |
| `src/combat.c/.h` | `src/combat/combat.c/.h` | 3 |
| `src/interface.c/.h` | `src/interface/interface.c/.h` | 3 |
| `src/sound.c/.h` | `src/audio/sound.c/.h` | 3 |
| `src/texts.c/.h` | `src/narrative/texts.c/.h` + `encode.c/.h` | 3 |
| `src/texts_generated.c/.h` | `src/narrative/texts_data.c/.h` | 3 |
| `src/dialogs.c/.h` | `src/narrative/dialogs.c/.h` | 3 |
| `src/intro.c/.h` | `src/scenes/intro.c/.h` (sigue siendo C) | 3 |
| `src/geesebumps.c/.h` | `src/scenes/geesebumps.c/.h` (idem) | 3 |
| `src/patterns.c/.h` | **eliminados** → `src/spells/spell.c/.h` + `player_spells.c` + `enemy_spells.c` | 4 |
| `src/patterns/*.c/.h` | **eliminados** → hooks en `player_spells.c`/`enemy_spells.c`/`fire.c` | 4 |
| `src/act_1.c/.h` | **eliminados** → `data/scenes/*.scene` + `scene_vm` + `scene_api` + `scene_hooks` + `scene_data` (gen) | 5 |
| `gibberish/` | `tools/voice/` | 2 |
| `generate_texts.py` | `tools/gen_texts.py` | 2 |
| `add_texts_comments.py` | `tools/add_texts_comments.py` (Fase 2) → **eliminado** (Fase 5, §15) | 2/5 |
| `consolidate.py` | `tools/consolidate.py` | 2 |
| `texts.csv` | `data/texts.csv` | 2 |
| (n/a) | `data/choices.csv` + `tools/gen_choices.py` + `src/narrative/choices_data.c/.h` | 5 |
| (n/a) | `data/scenes/*.scene` + `tools/gen_scenes.py` + `src/scenes/scene_data.c/.h` | 5 |
| (n/a) | `smoke/` (main propio + menú + runner + casos) | 7 |
| (n/a) | `Makefile` + `build-theweave.sh` (propios del repo) | 2 |
| `AGENTS.md` | `AGENTS.md` reescrito + `docs/{spells,scenes,texts,build,testing}.md` | 6 |

---

*Fin del plan. Mantener sincronizado con el estado real del refactor.*
