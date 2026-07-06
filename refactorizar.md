# Plan de Refactorización — The Weave

> Documento de referencia para ejecutar la refactorización de **The Weave** de principio a fin.
> Cualquier IA o desarrollador debe poder seguir este plan sin información adicional.
> Mantener este fichero sincronizado con el estado real del refactor (tacha las fases completadas).

---

## 0. Visión y objetivos

The Weave es un fangame de *Loom* para Sega Mega Drive / Genesis escrito en C con SGDK.
El proyecto ha crecido hasta ~6.500 líneas en 53 archivos y arrastra deuda técnica que
dificulta autorar contenido nuevo (escenas, hechizos, diálogos) sin tocar mucho código C
y sin riesgo de bugs.

### Objetivos del refactor (por orden de prioridad)

1. **Legibilidad y mantenibilidad por encima del rendimiento.** No es un proyecto que
   exprima el hardware; es un juego largo que debe poder crecer durante meses sin que la
   parte técnica frene el guión, los gráficos ni la jugabilidad.
2. **Sistema de hechizos versátil y declarativo.** Definir un hechizo (efecto visual por
   frames, condiciones de zona, contrarrestros, uso en puzzles narrativos) debe ser tan
   simple como rellenar una struct y, si hace falta, un par de hooks. No se quieren bugs
   sorpresa: si el objeto del hechizo está bien definido, se ejecuta y punto.
3. **Cutscenes autorables con el mínimo de código C.** Un guionista edita ficheros de
   texto en `data/scenes/` y un script de Python los integra a `scene_data.c`. Cero C
   nuevo por escena nueva (salvo lógica de puzzle muy específica).
4. **Estructura de directorios y librerías clara.** Mover la carpeta `gibberish/` y los
   scripts sueltos a `tools/`; dividir `src/` en subcarpetas por dominio; partir módulos
   grandes en varios archivos por responsabilidad (estilo RedPlanet_MD).
5. **Documentación exhaustiva.** `AGENTS.md` como fuente de verdad (~600 líneas, estilo
   RedPlanet_MD) y `docs/` con documentación técnica por subsistema. Todas las funciones
   comentadas. Un nuevo desarrollador debe poder unirse al proyecto leyendo la doc.

### Características que se mantienen intactas

- **Estilo de programación y de estructuración del código.** C plano estilo SGDK,
  `snake_case`, `u8/u16/u32/s8/s16/s32/bool`, `static const` en ROM, pools estáticos,
  sin fancy C++ ni macros oscuras. El autor debe poder editarlo perfectamente.
- **Arquitectura Mega Drive + SGDK.** SGDK instalado en `~/sgdk`. Hardware original
  (64 KB de RAM, no memoria dinámica en hot paths).
- **Comportamiento del juego actual como referencia "golden".** El resultado del refactor
  debe reproducir el comportamiento de las 4 escenas migradas y los 6 hechizos actuales.
  El ROM actual (`out/rom.bin`) es la referencia para el smoke test.

### Referencias

- Proyecto de referencia: `/home/ganso/codigo/RedPlanet_MD/` (mismo autor, más moderno).
- SGDK: `/home/ganso/sgdk/` (documentación en https://stephane-d.github.io/SGDK/).
- Exploración exhaustiva del estado actual: ver "Informe de estado" en `docs/refactor/state_audit.md` (se genera en la Fase 0).

---

## 1. Decisiones de diseño (ya tomadas)

| # | Decisión | Valor |
|---|---|---|
| D1 | Dependencias entre módulos | **Includes explícitos por archivo** (sin `globals.h` paraguas, estilo RedPlanet_MD). Cada `.c` lista solo lo que necesita. |
| D2 | Modelo de hechizos | **Fases declarativas + hooks opcionales**. Cada hechizo define una lista de fases (rango de frames + tipo de efecto) y hooks `canUse / onLaunch / onCounter / onFinish`. |
| D3 | Formato de cutscenes | **DSL de texto** en `data/scenes/*.scene` → `gen_scenes.py` → `src/scenes/scene_data.c` con un array de `SceneStep` por escena. Una VM pequeña en C ejecuta los pasos. |
| D4 | Pruebas | **ROM de smoke test** con un menú extra que ejecuta cada hechizo y cada escena para verificación rápida en BlastEm. Comparación contra ROM golden del build actual. |
| D5 | Organización de `src/` | **Subcarpetas por dominio**: `core world actors combat spells narrative scenes interface audio lib boot`. Makefile con wildcards. |
| D6 | Estilo de comentarios | Estilo RedPlanet_MD: `/* función — Parámetros: — Retorna: */` para funciones públicas, `//` para notas internas. Doxygen ligero (`/** @brief */`) solo en API pública de `lib/`. |
| D7 | `Makefile` | Propio del repo: wildcard discovery de `src/**/*.c` y `res/*.res`, pre-build codegen (`tools/gen_*.py`), targets `release/debug/clean/smoke`. |
| D8 | `res_*.h` | Mantienen el prefijo `res_` (menos churn). Se incluyen explícitos desde cada `.c` que los usa. |
| D9 | Rama de trabajo | `refactor` a partir de `master`. Commits por fase. |
| D10 | Reproducir comportamiento | Las 4 escenas migradas y los 6 hechizos deben comportarse idéntico al build actual (golden). |

---

## 2. Estructura de directorios final

```
The-Weave/
├── AGENTS.md                  (exhaustivo ~600 líneas, fuente de verdad)
├── Makefile                   (NUEVO: wildcard + pre-build codegen)
├── build-theweave.sh          (wrapper con --no-run para verificación CI)
├── refactorizar.md            (este plan)
├── README.md                  (sin cambios relevantes)
├── .gitignore                 (actualizado: out/, venv/, tools/voice/__pycache__/, etc.)
│
├── data/                      (AUTORÍA EDITABLE — el guionista toca aquí)
│   ├── texts.csv              (diálogos: set,id,face,side,time,es,en — existente)
│   ├── choices.csv            (NUEVO: choices generados, hoy hardcoded en texts.c)
│   ├── spells/                (NUEVO: un .spell por hechizo, DSL)
│   │   ├── thunder.spell
│   │   ├── hide.spell
│   │   ├── open.spell
│   │   ├── sleep.spell
│   │   ├── en_thunder.spell
│   │   ├── en_bite.spell
│   │   └── fire.spell         (ejemplo nuevo end-to-end)
│   └── scenes/                (NUEVO: un .scene por escena, DSL)
│       ├── act1_scene1.scene
│       ├── act1_scene2.scene
│       ├── act1_scene3.scene
│       └── act1_scene5.scene
│
├── tools/                     (CODEGEN + UTILIDADES)
│   ├── gen_texts.py           (movido de raíz; existente)
│   ├── gen_choices.py         (NUEVO: choices.csv → choices_data.c)
│   ├── gen_spells.py          (NUEVO: data/spells/*.spell → spells_data.c)
│   ├── gen_scenes.py          (NUEVO: data/scenes/*.scene → scene_data.c)
│   ├── add_texts_comments.py  (movido de raíz; existente)
│   ├── consolidate.py         (movido de raíz; existente)
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
│   ├── core/                  (núcleo del bucle y configuración)
│   │   ├── main.c  main.h
│   │   ├── config.h           (SCREEN_W/H/FPS, GAME_VERSION)
│   │   ├── hack.h             (HACK_* toggles de dev)
│   │   ├── frame.c  frame.h   (next_frame, wait_seconds, frame_counter)
│   │   └── init.c  init.h     (initialize, new_level, end_level)
│   ├── world/                 (mundo y scroll)
│   │   ├── background.c  background.h
│   │   └── constants_world.h
│   ├── actors/                (entidades)
│   │   ├── entity.c  entity.h        (Entity, GameState, move_entity)
│   │   ├── characters.c  characters.h
│   │   ├── enemies.c  enemies.h
│   │   ├── items.c  items.h
│   │   ├── collisions.c  collisions.h
│   │   └── constants_actors.h
│   ├── combat/                (combate FSM)
│   │   ├── combat.c  combat.h        (CombatState, hit_enemy/player, update_combat)
│   │   └── constants_combat.h
│   ├── spells/                (SISTEMA DE HECHIZOS — reemplaza patterns/)
│   │   ├── spell.c  spell.h          (SpellDef, motor de fases, validate, launch/update)
│   │   ├── spell_hooks.c  spell_hooks.h  (hooks compartidos: zone checks, counter logic)
│   │   ├── spells_data.c  spells_data.h  (GENERADO por gen_spells.py)
│   │   ├── player_spells.c  player_spells.h  (thunder/hide/open/sleep — solo hooks específicos)
│   │   ├── enemy_spells.c  enemy_spells.h    (en_thunder/en_bite)
│   │   ├── fire.c  fire.h             (ejemplo nuevo)
│   │   └── constants_spells.h
│   ├── narrative/             (textos y diálogos)
│   │   ├── texts.c  texts.h
│   │   ├── texts_data.c  texts_data.h   (GENERADO por gen_texts.py — era texts_generated)
│   │   ├── choices_data.c  choices_data.h  (GENERADO por gen_choices.py)
│   │   ├── dialogs.c  dialogs.h         (talk, talk_dialog, talk_cluster, choice_dialog)
│   │   ├── encode.c  encode.h           (encode_spanish_text extraído de texts.c)
│   │   └── constants_narrative.h
│   ├── scenes/                (CUTSCENES — reemplaza act_1.c hardcoded)
│   │   ├── scene_vm.c  scene_vm.h       (intérprete de SceneStep)
│   │   ├── scene_data.c  scene_data.h   (GENERADO por gen_scenes.py)
│   │   ├── scene_api.c  scene_api.h     (primitivas que la VM llama: scene_say, scene_move...)
│   │   └── constants_scenes.h
│   ├── interface/             (HUD + pausa)
│   │   ├── interface.c  interface.h
│   │   └── constants_interface.h
│   ├── audio/                 (sonido)
│   │   ├── sound.c  sound.h
│   │   └── constants_audio.h
│   └── lib/                   (cabeceras paraguas opcionales + helpers compartidos)
│       └── (vacío por defecto; rellenar solo si hay helpers realmente compartidos)
│
├── smoke/                     (ROM DE SMOKE TEST)
│   ├── smoke_menu.c  smoke_menu.h      (menú que lista hechizos/escenas)
│   ├── smoke_runner.c  smoke_runner.h  (ejecuta cada caso y compara)
│   └── smoke_cases.h                   (tabla de casos: id, tipo, esperado)
│
├── docs/                      (DOCUMENTACIÓN TÉCNICA)
│   ├── images/                (existente)
│   ├── architecture.md        (visión general del código, módulos, data flow)
│   ├── spells.md              (cómo crear un hechizo: struct, fases, hooks, ejemplo fire)
│   ├── scenes.md              (cómo autorar una cutscene: DSL, SceneStep, VM, ejemplo)
│   ├── texts.md               (cómo añadir diálogos: texts.csv, gen_texts.py, choices.csv)
│   ├── build.md               (cómo compilar: Makefile, build-theweave.sh, SGDK, BlastEm)
│   ├── art.md                 (organización de res/, convenciones para grafistas)
│   ├── voice.md               (pipeline de voces animalese: tools/voice/)
│   ├── testing.md             (smoke ROM, checklist de playtest, cómo añadir casos)
│   └── refactor/              (auditoría y seguimiento del refactor)
│       ├── state_audit.md     (generado en Fase 0: estado actual detallado)
│       ├── bugs.md            (bugs conocidos y su corrección)
│       └── checklist.md       (checklist por fase, se tacha al completar)
│
└── res/                       (sin cambios estructurales; ver docs/art.md)
    ├── resources.res / .h
    ├── res_backgrounds.res / .h
    ├── res_characters.res / .h
    ├── res_faces.res / .h
    ├── res_enemies.res / .h
    ├── res_items.res / .h
    ├── res_interface.res / .h
    ├── res_sound.res / .h
    ├── res_geesebumps.res / .h
    ├── res_intro.res / .h
    ├── res_dialogs.res / .h
    ├── Backgrounds/  Geesebumps/  Intro/  Sound/  Sprites/  font.png
    └── (subcarpetas de arte fuente existentes)
```

### Notas de organización

- `data/` es lo único que toca el guionista. Todo lo demás es código o recursos.
- `tools/` contiene solo scripts Python de codegen y utilidades. Nada se ejecuta en la ROM.
- `src/` se divide por dominio. Cada dominio tiene sus `constants_*.h`.
- `smoke/` es código C pero vive fuera de `src/` para no contaminar el juego; se compila
  como un target aparte del Makefile que produce `out/smoke.bin`.
- `docs/` es documentación humana; `docs/refactor/` es la bitácora del refactor.
- `gibberish/` desaparece como carpeta raíz; su contenido se mueve a `tools/voice/`.
- `consolidate.py`, `add_texts_comments.py`, `generate_texts.py` se mueven a `tools/`.
- `texts.csv` se mueve a `data/`. `generate_texts.py` se actualiza para leer de `data/texts.csv`
  y escribir a `src/narrative/texts_data.{c,h}` (antes `src/texts_generated.{c,h}`).

---

## 3. Diseño del sistema de hechizos

### 3.1 Objetivos de diseño

- **Declarativo por defecto.** Un hechizo se define rellenando una `SpellDef` con fases.
  El 90% de los hechizos no necesita escribir lógica: solo datos.
- **Hooks cuando haga falta.** Comportamientos especiales (condición de zona, contrarrestro,
  reacción según hechizo enemigo activo) se implementan en hooks opcionales.
- **Mismo motor para combate y narrativa.** Un hechizo se lanza igual desde el FSM de
  combate que desde la VM de escena. La diferencia es el `origin` y el `narrativeTag`.
- **Bug-resistant.** El motor gestiona el frame counter, el avance de fases, la finalización,
  el cleanup. Los hooks no tienen que acordarse de resetear nada.
- **Validación estricta en codegen.** `gen_spells.py` valida que las fases no se solapen
  mal, que los rangos sean coherentes, que los hooks referenciados existan.

### 3.2 Estructuras de datos

```c
/* src/spells/spell.h */

/* Origen del lanzamiento: determina quién paga el coste y a quién afecta. */
typedef enum {
    SPELL_ORIGIN_PLAYER    = 0,   /* lanzado por jugador via notas */
    SPELL_ORIGIN_ENEMY     = 1,   /* lanzado por enemigo via IA */
    SPELL_ORIGIN_NARRATIVE = 2    /* lanzado por script de escena (puzzle) */
} SpellOrigin;

/* Tipo de fase: qué hace esta fase del efecto. */
typedef enum {
    PHASE_VISUAL_FLASH   = 0,   /* flashear paleta: p1=color index, p2=color destino */
    PHASE_VISUAL_SPRITE  = 1,   /* mostrar/ocultar sprite de efecto: p1=sprite id */
    PHASE_VISUAL_FADE    = 2,   /* fade global: p1=MODE (PAL fade in/out) */
    PHASE_LOGIC_DAMAGE   = 3,   /* aplicar daño: p1=target, p2=cantidad */
    PHASE_LOGIC_STATE    = 4,   /* cambiar estado de objetivo: p1=target, p2=GameState */
    PHASE_SFX            = 5,   /* disparar sonido: p1=sfx_id */
    PHASE_END            = 6    /* fin del efecto (siempre al final) */
} PhaseKind;

/* Target de una fase lógica. */
typedef enum {
    PHASE_TARGET_NONE        = 0,
    PHASE_TARGET_PLAYER      = 1,
    PHASE_TARGET_ENEMY_ACTIVE = 2,
    PHASE_TARGET_SCENE_ZONE  = 3,   /* p2 = zone id, para puzzles de puerta */
    PHASE_TARGET_NARRATIVE   = 4    /* avanzar tag narrativo */
} PhaseTarget;

/* Una fase del efecto. El motor la ejecuta cuando frameCounter está en [start,end]. */
typedef struct {
    u16 startFrame;        /* frame relativo al inicio del efecto (0 = primer frame) */
    u16 endFrame;          /* último frame en que la fase está activa */
    PhaseKind kind;
    u8  target;            /* PhaseTarget */
    u8  p1, p2;            /* parámetros según kind (color, sprite_id, daño, sfx_id, zone...) */
} SpellPhase;

/* Contexto que se pasa a los hooks. Les da acceso a todo lo que necesitan
   sin que tengan que reach into globals arbitrarios. */
typedef struct {
    SpellOrigin origin;
    u8  enemyId;           /* SPELL_ORIGIN_ENEMY: enemigo que lanza; SPELL_ORIGIN_PLAYER: enemigo activo si lo hay */
    u8  activeEnemySpell;  /* id del hechizo enemigo en curso, o SPELL_NONE si ninguno */
    u8  reversed;          /* 1 si se lanzó invertido (para contrarrestros) */
    u8  sceneId;           /* escena actual (para canUse de zona) */
    u8  zoneId;            /* zona dentro de la escena (para canUse de zona) */
    u16 frameCounter;      /* progreso del efecto (lo gestiona el motor) */
} SpellContext;

/* Definición de un hechizo. Es lo que rellena el autor. */
typedef struct {
    u16   id;                     /* SPELL_THUNDER, SPELL_FIRE, ... */
    const char *name;             /* "thunder" — para el DSL y el smoke test */
    u8    notes[4];               /* secuencia de notas NOTE_MI..NOTE_DO */
    u8    noteCount;              /* 1..4 (algunos enemigos usan 3) */
    bool  palindrome;             /* true si notas == notas invertidas (no se puede invertir) */
    bool  counterable;            /* true si un enemigo puede contrarrestarlo / si contrarresta a enemigo */
    u16   baseDuration;           /* duración total del efecto en frames (suma de fases) */
    SpellOrigin origin;           /* quién lo lanza por defecto */
    u8    narrativeTag;           /* id de puzzle, 0 si no es de narrativa */

    /* Hooks opcionales. NULL si no se necesitan. */
    bool  (*canUse)   (SpellContext *ctx);   /* false = no se puede lanzar ahora */
    void  (*onLaunch) (SpellContext *ctx);   /* al iniciar, antes de la primera fase */
    void  (*onCounter)(SpellContext *ctx);   /* al ser contrarrestado */
    void  (*onFinish) (SpellContext *ctx);   /* al terminar el efecto */

    /* Fases del efecto. */
    const SpellPhase *phases;
    u8  phaseCount;
} SpellDef;

/* Tabla global de hechizos (GENERADA por gen_spells.py desde data/spells/*.spell,
   o definida a mano en player_spells.c/enemy_spells.c si el hechizo tiene hooks
   que no se pueden expresar en el DSL — ver §3.5). */
extern const SpellDef *spells[];        /* indexado por SPELL_* */
extern const u8 spell_count;
```

### 3.3 Motor de hechizos (`src/spells/spell.c`)

Responsabilidades del motor:

1. **`spell_validate(notes, count, reversed_out) -> spellId`** — sustituye a
   `validate_pattern`. Recorre `spells[]`, compara notas en orden directo e invertido,
   respeta `palindrome`. Devuelve `SPELL_NONE` si no hay match.
2. **`spell_launch(spellId, ctx) -> bool`** — llama `canUse`; si false, devuelve false y
   no hace nada. Si true, llama `onLaunch`, resetea `ctx.frameCounter = 0`, marca el
   hechizo como activo, notifica al FSM de combate o a la VM de escena.
3. **`spell_update() -> bool`** — se llama cada frame desde `next_frame` (o desde la VM).
   Incrementa `frameCounter`. Ejecuta todas las fases cuyo `[startFrame,endFrame]`
   contiene `frameCounter`. Devuelve `true` cuando el efecto ha terminado (frameCounter
   >= baseDuration o fase `PHASE_END`).
4. **`spell_counter(spellId, ctx)`** — invoca `onCounter` del hechizo activo y lo finaliza.
   Lo llama el FSM de combate cuando el jugador lanza el invertido de un hechizo enemigo
   `counterable` en la ventana adecuada.
5. **`spell_finish()`** — interna: llama `onFinish`, limpia el estado activo, devuelve
   el control a quien lo lanzó (FSM de combate → `COMBAT_STATE_IDLE`; VM de escena →
   siguiente `SceneStep`).

El motor es **el único** que toca `ctx.frameCounter` y el estado "hechizo activo". Los
hooks nunca tienen que gestionar temporizadores ni cleanup. Esto elimina la clase de
bugs del sistema actual (copy-paste de `baseDuration`, transiciones inconsistentes,
slot-0 hardcodeado).

### 3.4 Hooks típicos (`src/spells/spell_hooks.c`)

Hooks reutilizables compartidos entre hechizos, para no duplicar lógica:

- `spell_hook_zone_eq(u8 zoneId)` — devuelve una función `canUse` que chequea
  `ctx->zoneId == zoneId`. Para "fuego solo en la zona del caldero".
- `spell_hook_counter_if_enemy(SpellContext *ctx)` — `onCounter` genérico: aplica daño al
  enemigo activo y finaliza su hechizo. Reemplaza la lógica duplicada de
  `pattern_en_thunder.c:on_counter`.
- `spell_hook_reject_if_no_enemy(SpellContext *ctx)` — `canUse` que rechaza si
  `ctx->activeEnemySpell == SPELL_NONE`. Para hechizos que solo tienen sentido en combate.
- `spell_hook_narrative_advance(SpellContext *ctx)` — `onFinish` que avanza el
  `narrativeTag` de la escena. Para puzzles de secuencia.

### 3.5 DSL de hechizos (`data/spells/*.spell`)

Formato de texto plano que `gen_spells.py` transforma en `spells_data.c`. Un hechizo
sin hooks especiales se define íntegramente en el DSL. Si necesita hooks, se define el
`skeleton` en el DSL y los hooks se enlazan a mano en `player_spells.c` / `enemy_spells.c`
(referenciando el id del DSL).

```
# data/spells/fire.spell
spell FIRE
  id        SPELL_FIRE
  name      "fire"
  notes     MI FA SOL LA
  duration  240            # 4 segundos a 60fps
  origin    PLAYER
  counterable true
  narrativeTag 0

  hook canUse   zone_eq 3           # solo en zona del caldero
  hook onLaunch sfx SFX_FIRE_START

  phase 0  20  VISUAL_FLASH   color 4 dest 0xEEE   # flash inicial
  phase 20 60  VISUAL_SPRITE  sprite SPR_FIRE_BURST
  phase 60 120 LOGIC_DAMAGE   target ENEMY_ACTIVE amount 2
  phase 120 240 VISUAL_FADE   mode FADE_OUT
  phase 240 240 END
```

`gen_spells.py` valida:
- `notes` son constantes `NOTE_*` válidas y `noteCount` <= 4.
- `duration` >= último `endFrame` de las fases.
- Fases no se solapan en el mismo `kind`+`target` (warning, no error).
- `hook` referenciado existe en una tabla de hooks conocida (`spell_hook_*`).
- `palindrome` se calcula automáticamente y se verifica contra el declarado (si existe).

Salida: `src/spells/spells_data.c` con un array `const SpellDef spells_table[]` indexado
por id, y `spells[]` / `spell_count` apuntando a él. Los hooks se resuelven como
function pointers en una tabla `spell_hook_table[]` que `gen_spells.py` referencia por
nombre simbólico.

### 3.6 Hechizo ejemplo: FIRE (end-to-end, nuevo)

Caso de uso del usuario: "un nuevo hechizo fuego que solo se active si estamos en
determinada zona, que tenga un comportamiento distinto según si se está lanzando otro
hechizo enemigo, que tenga un efecto visual que dure un número de frames y que pueda
ser complejo".

**Definición en DSL** (`data/spells/fire.spell`): ver §3.5.

**Comportamiento distinto según hechizo enemigo activo**: se implementa con un hook
`canUse` custom en `src/spells/fire.c` que no se puede expresar en el DSL puro:

```c
/* src/spells/fire.c */
static bool fire_can_use(SpellContext *ctx)
{
    if (ctx->zoneId != ZONE_CAULDRON)
        return false;                                  /* solo en la zona del caldero */
    if (ctx->activeEnemySpell == SPELL_EN_THUNDER)
        return true;                                   /* contrarresta thunder enemigo */
    if (ctx->activeEnemySpell != SPELL_NONE)
        return false;                                  /* no interfiere con otros hechizos */
    return true;                                       /* uso normal */
}

static void fire_on_launch(SpellContext *ctx)
{
    if (ctx->activeEnemySpell == SPELL_EN_THUNDER) {
        spell_counter(ctx->activeEnemySpell, ctx);     /* fuego come thunder */
    }
    /* el resto del efecto lo definen las fases */
}

/* En spells_data.c, generado, fire queda enlazado así:
   spells_table[SPELL_FIRE] = (SpellDef){
       .id = SPELL_FIRE, .name = "fire",
       .notes = {NOTE_MI,NOTE_FA,NOTE_SOL,NOTE_LA}, .noteCount = 4,
       .palindrome = false, .counterable = false, .baseDuration = 240,
       .origin = SPELL_ORIGIN_PLAYER, .narrativeTag = 0,
       .canUse = fire_can_use, .onLaunch = fire_on_launch, .onCounter = NULL, .onFinish = NULL,
       .phases = fire_phases, .phaseCount = 5
   };
   fire_phases[] se genera desde el DSL.
*/
```

Esto muestra el patrón: **el DSL define las fases (lo declarativo), el .c define los
hooks (lo imperativo)**. El motor une ambas cosas. El autor solo toca el .spell y, si
hay lógica especial, un hook en un .c pequeño.

### 3.7 Migración de los 6 hechizos actuales

| Hechizo actual | Nuevo id | Origen | Notas | Hooks |
|---|---|---|---|---|
| `PATTERN_THUNDER` | `SPELL_THUNDER` | PLAYER | MI FA SOL SOL | `canUse` (counter window + reject vs WeaverGhost hint), `onLaunch` (flash + jingle) |
| `PATTERN_HIDE` | `SPELL_HIDE` | PLAYER | FA SOL SOL FA | `onLaunch`, `onFinish` (restore palette) |
| `PATTERN_OPEN` | `SPELL_OPEN` | PLAYER | FA SI SOL DO | `canUse` (siempre false salvo scripted), `onLaunch` (stub) |
| `PATTERN_SLEEP` | `SPELL_SLEEP` | PLAYER | FA MI DO LA | `canUse` (siempre false salvo scripted), `onLaunch` (stub) |
| `PATTERN_EN_THUNDER` | `SPELL_EN_THUNDER` | ENEMY | MI FA SOL SOL | `onLaunch` (flash sky/purple), `onFinish` (hit_player), `onCounter` (hit_enemy + cancel) |
| `PATTERN_EN_BITE` | `SPELL_EN_BITE` | ENEMY | MI SOL DO | `onLaunch`, `onFinish` (hit_player) |

Comportamiento **idéntico** al actual. Las fases se extraen del `update()` actual de cada
`pattern_*.c`. Los bugs detectados (copy-paste `pattern_sleep.c:11`, slot-0 hardcodeado)
se corrigen al migrar — ver §10.

### 3.8 Uso en puzzles narrativos

La VM de escena (§4) puede lanzar hechizos con `origin = NARRATIVE` y `narrativeTag != 0`.
El motor los ejecuta igual que en combate (mismas fases, mismos hooks), pero el daño se
redirige a `PHASE_TARGET_NARRATIVE` (no afecta a HP). Al terminar, `onFinish` avanza el
estado del puzzle.

**Ejemplo: abrir una puerta con secuencia de 3 hechizos directos/reversos.**

```
# data/scenes/act2_door.scene
label door_puzzle
  cast open   direct
  wait_spell
  cast fire   reversed    # fuego invertido = apagar
  wait_spell
  cast thunder direct
  wait_spell
  # la VM chequea narrativeTag sequence y abre la puerta
  if_puzzle_solved goto door_open
  goto door_puzzle_fail
label door_open
  say ACT2_DIALOG DOOR_OPENS face_left 90
  ...
```

El puzzle state lo lleva `scene_vm.c` con un small array `puzzle_progress[]` indexado
por `narrativeTag`. Cada `cast ... direct/reversed` que termina añade su tag + dirección
al progreso; `if_puzzle_solved` comprueba la secuencia esperada. La definición de la
secuencia esperada va en el propio `.scene` (directiva `puzzle_sequence TAG 3 open:direct fire:reversed thunder:direct`), y `gen_scenes.py` la emite como datos.

---

## 4. Diseño del sistema de cutscenes

### 4.1 Objetivos de diseño

- **Autorar una escena nueva = crear un `.scene` + re-ejecutar `gen_scenes.py`.** Cero C.
- **DSL legible** que un guionista pueda escribir sin conocer C.
- **VM pequeña y predecible** en C que ejecuta un array de `SceneStep`. Un `case` por op.
- **Mismas primitivas que el código actual**: `say`, `move`, `wait`, `choice`, `cast`,
  `fade`, `next_scene`. Lo que hoy son funciones en `act_1.c` se convierten en opcodes.
- **Soporte de flujo**: `label`, `goto`, `branch` (según choice), `if_puzzle_solved`.
- **Validación estricta en codegen**: labels referenciados existen, ids de diálogo
  existen, ids de hechizo existen, tipos de argumento correctos.

### 4.2 Estructuras de datos

```c
/* src/scenes/scene_vm.h */

typedef enum {
    SCENE_OP_LEVEL          = 0,   /* level <id> limits <min> <max> */
    SCENE_OP_CHAR_AT        = 1,   /* char <id> at <x> <y> <face> */
    SCENE_OP_CHAR_SHOW      = 2,
    SCENE_OP_CHAR_HIDE      = 3,
    SCENE_OP_MOVE           = 4,   /* move <char> to <x> <y> speed <s> */
    SCENE_OP_WAIT_FOLLOWERS = 5,
    SCENE_OP_SAY            = 6,   /* say <dialogSet> <dialogId> <face> <side> <time> */
    SCENE_OP_SAY_CLUSTER    = 7,   /* say_cluster <dialogSet> <startId> <endId> <face> <side> */
    SCENE_OP_CHOICE         = 8,   /* choice <choiceSet> */
    SCENE_OP_BRANCH         = 9,   /* branch <idx> goto <labelIdx> */
    SCENE_OP_LABEL          = 10,  /* marker, no-op en runtime; gen_scenes resuelve a índice */
    SCENE_OP_GOTO           = 11,  /* goto <labelIdx> */
    SCENE_OP_WAIT_FRAMES    = 12,
    SCENE_OP_WAIT_ITEM      = 13,  /* wait_item <itemId> — bloquea hasta last_interacted_item */
    SCENE_OP_CAST_SPELL     = 14,  /* cast <spellName> <direct|reversed> */
    SCENE_OP_WAIT_SPELL     = 15,
    SCENE_OP_FADE_OUT       = 16,
    SCENE_OP_FADE_IN        = 17,
    SCENE_OP_PLAY_MUSIC     = 18,
    SCENE_OP_STOP_MUSIC     = 19,
    SCENE_OP_NEXT_SCENE     = 20,  /* next_scene <act> <scene> */
    SCENE_OP_PUZZLE_SEQ     = 21,  /* puzzle_sequence <tag> <count> <spell:dir>... */
    SCENE_OP_IF_PUZZLE_SOLVED = 22,/* if_puzzle_solved <tag> goto <labelIdx> */
    SCENE_OP_END            = 23
} SceneOp;

typedef struct {
    SceneOp op;
    s16 a, b, c, d;   /* argumentos: dependen del op. -1 si no se usa. */
} SceneStep;

/* Una escena es un array de SceneStep. */
typedef struct {
    const char *name;          /* "act1_scene1" */
    const SceneStep *steps;
    u16 stepCount;
} SceneScript;

/* Tabla de escenas (GENERADA por gen_scenes.py). */
extern const SceneScript scenes[];
extern const u8 scene_count;

/* Progreso de puzzles (lo mantiene la VM en RAM). */
extern u8 puzzle_progress[PUZZLE_MAX][PUZZLE_SEQ_MAX];
```

### 4.3 DSL de escenas (`data/scenes/*.scene`)

```
# data/scenes/act1_scene1.scene
scene act1_scene1

# setup
level ACT1_LEVEL_BEDROOM limits 0 3200
char CHR_LINUS at 160 200 face_left
char CHR_LINUS show

# diálogo inicial
say ACT1_DIALOG1 A1D1_OVERSLEPT face_left side_left 90

# movimiento + espera
move CHR_LINUS to 300 200 speed 1
wait_followers

# choice
choice ACT1_CHOICE1
branch 0 goto morning_walk
branch 1 goto overslept

label morning_walk
  cast thunder direct
  wait_spell
  fade_out 60
  next_scene 1 2
  end

label overslept
  say ACT1_DIALOG1 A1D1_LATE face_left side_left 60
  fade_out 60
  next_scene 1 5
  end
```

### 4.4 Reglas del DSL

- Comentarios con `#`.
- Una directiva por línea. Identificadores en `MAYÚSCULAS` (constantes) o `snake_case`
  (labels y nombres de escena).
- `label <name>` define un punto de salto. `goto <name>` y `branch <idx> goto <name>`
  saltan a él. `gen_scenes.py` resuelve los nombres a índices de step.
- `say`/`say_cluster` referencian ids de `data/texts.csv` (validados contra el CSV).
- `choice` referencia un choice set de `data/choices.csv` (validado).
- `cast` referencia un nombre de hechizo de `data/spells/*.spell` (validado).
- `puzzle_sequence <tag> <count> <spell:dir>...` define la secuencia esperada para
  resolver el puzzle `<tag>`. La VM la guarda y `if_puzzle_solved` la compara.
- `end` termina la escena. Si no se pone, `gen_scenes.py` añade uno implícito.
- Strings entre comillas dobles si contienen espacios (raro: solo `say` con texto literal,
  pero preferimos referenciar ids).

### 4.5 `gen_scenes.py`

Entrada: `data/scenes/*.scene`.
Salida: `src/scenes/scene_data.c` + `src/scenes/scene_data.h`.

Pasos:
1. **Parsear** cada `.scene` en una lista de steps con labels simbólicos.
2. **Validar**:
   - Todo `goto`/`branch` referencia un `label` existente en la misma escena.
   - Todo `say`/`say_cluster` referencia un dialog set + id existente en `data/texts.csv`.
   - Todo `choice` referencia un choice set existente en `data/choices.csv`.
   - Todo `cast` referencia un hechizo existente en `data/spells/*.spell`.
   - `puzzle_sequence` tiene `count` == número de pares `spell:dir`.
3. **Resolver labels** a índices de step (segunda pasada).
4. **Emitir** `scene_data.c` con un array `static const SceneStep act1_scene1_steps[]`
   por escena, y la tabla `const SceneScript scenes[]` indexada por id de escena.
   Emitir también `enum SceneId { SCENE_ACT1_SCENE1, ... }` en `scene_data.h`.
5. **Cabecera** `// Generated by tools/gen_scenes.py — DO NOT EDIT`.

### 4.6 VM de escenas (`src/scenes/scene_vm.c`)

```c
/* Ejecuta una escena completa. Bloquea hasta SCENE_OP_END/next_scene. */
void scene_run(u8 sceneId)
{
    const SceneScript *s = &scenes[sceneId];
    u16 pc = 0;
    while (pc < s->stepCount) {
        const SceneStep *st = &s->steps[pc];
        switch (st->op) {
            case SCENE_OP_LEVEL:      scene_level(st->a, st->b, st->c); break;
            case SCENE_OP_CHAR_AT:    scene_char_at(st->a, st->b, st->c, st->d); break;
            case SCENE_OP_SAY:        scene_say(st->a, st->b, st->c, st->d); break;
            case SCENE_OP_MOVE:       scene_move(st->a, st->b, st->c); break;
            case SCENE_OP_WAIT_FOLLOWERS: scene_wait_followers(); break;
            case SCENE_OP_CHOICE:     st->a = scene_choice(st->a); break;   /* devuelve idx */
            case SCENE_OP_BRANCH:     if (last_choice == st->a) { pc = st->b; continue; } break;
            case SCENE_OP_GOTO:       pc = st->a; continue;
            case SCENE_OP_CAST_SPELL: scene_cast_spell(st->a, st->b); break;
            case SCENE_OP_WAIT_SPELL: scene_wait_spell(); break;
            case SCENE_OP_FADE_OUT:   scene_fade_out(st->a); break;
            case SCENE_OP_NEXT_SCENE: current_act = st->a; current_scene = st->b; return;
            case SCENE_OP_PUZZLE_SEQ: scene_puzzle_define(st->a, st->b, &st->c); break;
            case SCENE_OP_IF_PUZZLE_SOLVED:
                if (scene_puzzle_solved(st->a)) { pc = st->b; continue; } break;
            case SCENE_OP_END:        return;
            /* ... resto de ops ... */
        }
        pc++;
        next_frame(true);
    }
}
```

Las primitivas (`scene_say`, `scene_move`, etc.) viven en `src/scenes/scene_api.c` y
son wrappers finos sobre las funciones existentes (`talk_dialog`, `move_character`,
`spell_launch` con `origin=NARRATIVE`, etc.). Migran la lógica de `act_1.c` a una capa
reutilizable.

### 4.7 Migración de las 4 escenas actuales

`act_1.c` tiene `act_1_scene_1/2/3/5` (scene 4 falta). Cada una se traduce a un `.scene`:

| Escena | Pasos aprox. | Notas |
|---|---|---|
| `act1_scene1` | ~30 | bedroom, diálogo overslept, choice, move, cast thunder (scripted), fade |
| `act1_scene2` | ~25 | forest, movimiento, diálogo, wait_item (interacción con items) |
| `act1_scene3` | ~40 | corridor, combate con WeaverGhost, win/lose branches |
| `act1_scene5` | ~15 | finale, diálogo, `SYS_hardReset` → se sustituye por `end` |

El gameplay loop (el `while { switch(last_interacted_item) } next_frame(true)` de
`act_1.c:67-98`) se modela con `SCENE_OP_WAIT_ITEM` + `branch` según el item. El combate
se modela con `cast` + `wait_spell` + `if_combat_won`/`if_combat_lost` (dos ops nuevos
que se añaden si hace falta; la VM los implementa leyendo `combat_state`).

`act_1_scene_4` se deja pendiente (no existe hoy); el DSL permite añadirla después sin C.

---

## 5. Higiene de cabeceras e includes (D1)

### 5.1 Retirada de `globals.h`

Hoy cada `.c` empieza con `#include "globals.h"` que a su vez incluye `<genesis.h>`,
todos los `res_*.h` y todos los `.h` de módulos. Esto causa:

- Recompilación total ante cualquier cambio de cabecera.
- Dependencias ocultas: un `.c` no declara qué módulos usa.
- Acoplamiento circular conceptual (`patterns.h` ↔ `globals.h`).

**Nuevo modelo (D1, estilo RedPlanet_MD):** cada `.c` incluye explícitamente:

```c
/* src/combat/combat.c */
#include <genesis.h>
#include "combat.h"
#include "../spells/spell.h"
#include "../actors/characters.h"
#include "../actors/enemies.h"
#include "../core/frame.h"
#include "../audio/sound.h"
#include "../res/res_sound.h"        /* solo los recursos que usa */
#include "constants_combat.h"
```

`globals.h` **desaparece**. `globals.c` se parte: `frame.c` (next_frame, frame_counter),
`config.h` (constantes), `init.c` (inicialización). Los recursos `res_*.h` se incluyen
desde el `.c` que los usa, no desde un paraguas.

### 5.2 Diagramas de arquitectura en cabeceras (estilo RedPlanet_MD `enemy.h`)

Las cabeceras de subsistemas grandes (`entity.h`, `spell.h`, `combat.h`, `scene_vm.h`)
empiezan con un bloque `/* ... */` que explica:

- Qué hace el subsistema.
- Qué archivos lo componen y su responsabilidad.
- Data flow en un frame típico.
- Relación con otros subsistemas.

Ejemplo (`src/spells/spell.h`):

```c
/*
 * src/spells/spell.h — Sistema de hechizos
 * -----------------------------------------
 * Un hechizo se define como una SpellDef (fases declarativas + hooks opcionales).
 * El motor (spell.c) gestiona el ciclo de vida: launch, update por frames,
 * counter, finish. Los hooks implementan lógica específica (zona, contrarrestro).
 *
 * Archivos:
 *   spell.c        — motor (validate, launch, update, counter, finish)
 *   spell_hooks.c  — hooks reutilizables (zone_eq, counter_if_enemy, ...)
 *   spells_data.c  — GENERADO por tools/gen_spells.py desde data/spells/*.spell
 *   player_spells.c — hooks específicos de hechizos de jugador
 *   enemy_spells.c  — hooks específicos de hechizos de enemigo
 *
 * Data flow en un frame de combate:
 *   controller → spell_validate(notes) → spell_launch(id, ctx)
 *   next_frame → spell_update() → ejecuta fases → spell_finish() → combat FSM IDLE
 *
 * Uso en narrativa:
 *   scene_vm → SCENE_OP_CAST_SPELL → spell_launch(id, ctx{origin=NARRATIVE})
 *   scene_vm → SCENE_OP_WAIT_SPELL → espera spell_update() == true
 *
 * Ver docs/spells.md para guía de autoría.
 */
```

### 5.3 `constants_*.h` + `config.h` + `hack.h`

Todos los números mágicos se centralizan:

- `src/core/config.h` — `SCREEN_WIDTH`, `SCREEN_HEIGHT`, `SCREEN_FPS`, `GAME_VERSION`.
- `src/core/hack.h` — `HACK_INFINITE_HEALTH`, `HACK_SKIP_INTRO`, `DEBUG_SPELL_LOG`,
  `HACK_START_SCENE` (para smoke test), etc. Compil-time.
- `src/actors/constants_actors.h` — `MAX_CHR`, `MAX_ENEMIES`, `MAX_ITEMS`, `MAX_FACE`,
  velocidades, tamaños de colisión.
- `src/combat/constants_combat.h` — `PLAYER_HURT_DURATION`, `MAX_PATTERN_WAIT_TIME`,
  timeouts de combate.
- `src/spells/constants_spells.h` — `MAX_SPELL_NOTES`, `SPELL_NONE`, `NOTE_*`,
  `MAX_SPELL_PHASES`, `ZONE_*`.
- `src/narrative/constants_narrative.h` — `DEFAULT_TALK_TIME`, `MAX_DIALOG_LINES`.
- `src/scenes/constants_scenes.h` — `PUZZLE_MAX`, `PUZZLE_SEQ_MAX`.
- `src/audio/constants_audio.h` — IDs de SFX (`SFX_THUNDER`, `SFX_FIRE_START`, ...)
  con rangos reservados (0-63 driver, 64-199 gameplay, 200+ UI), estilo RedPlanet_MD.

Los timers se escalan con `SCREEN_FPS` (ej. `SCREEN_FPS * 4` = 4 segundos) en lugar de
`240` hardcoded.

---

## 6. Bugs conocidos a corregir (Fase 1)

Lista extraída de la auditoría. Se corrigen en la Fase 1 (limpieza, sin mover archivos)
para que el comportamiento pre-refactor sea correcto y comparable.

| # | Archivo:línea | Bug | Fix |
|---|---|---|---|
| B1 | `src/patterns/pattern_sleep.c:11` | `player_sleep_update` lee `playerPatterns[PATTERN_HIDE].baseDuration` (copy-paste) | Usar `PATTERN_SLEEP` |
| B2 | `src/patterns/patterns.c:547` | `update_enemy_pattern` hardcodea slot 0; slot 1 nunca se actualiza | Iterar slots activos |
| B3 | `src/combat/combat.c:75-76` | `try_counter_spell` hardcodea slot 0 | Buscar slot activo |
| B4 | `src/combat/combat.c:135-138` | `while(!SPR_isAnimationDone)` bloqueante dentro de `hit_enemy` (per-frame) | Convertir en estado FSM |
| B5 | `src/entity/entity.c:50-60` | `move_entity` bloqueante con `next_frame` por step | Dejar bloqueante pero documentar; la VM de escena ya lo espera así |
| B6 | `src/controller/controller.c:252-255` | `wait_for_followers` bloqueante | Documentar; idem |
| B7 | `src/texts/texts.c:83` | `malloc` para `encode_spanish_text` | Buffer estático `static u8 scratch[MAX_TEXT_LEN]` |
| B8 | `src/dialogs/dialogs.c:267,336` | `free` de lo anterior | Eliminar el free al ir a buffer estático |
| B9 | `src/interface/interface.c:243,269` | `MEM_alloc` sin NULL check | Añadir check + fallback |
| B10 | `src/controller/controller.c:5` | `static u16 frame_counter` local sombrea el global y no se lee | Eliminar |
| B11 | `src/patterns/patterns.h:16` | `MAX_ENEMY_PATTERNS 8` muerto (la real es `MAX_PATTERN_ENEMY 2`) | Eliminar |
| B12 | `src/characters.h:21-23` vs `src/texts.h:9-10` | Dos vocabularios "side" (`SIDE_left`/`SIDE_left` vs `SIDE_LEFT`/`SIDE_RIGHT`) | Unificar en `SIDE_LEFT`/`SIDE_RIGHT`/`SIDE_NONE` |
| B13 | Include guards | Dos convenciones (`_FOO_H_` vs `FOO_H`) | Unificar en `_FOO_H_` |
| B14 | `src/globals.c:48` | Comentario garbled "deTODO" | "depending on" |
| B15 | `src/items.h:29` | Comentario garbled "deTODO" | "depending on" |
| B16 | `src/combat.c:130` | `// TODO: Better enemy defeat handling` | Resolver con FSM (relacionado con B4) |
| B17 | `src/sound.c:85,105` | `// TODO` jingles de SLEEP y EN_BITE | Añadir o documentar como intencional |
| B18 | `src/patterns/pattern_thunder.c:13-27` y `patterns.c:302-345` | Pattern callbacks reach into `dialogs[ACT1_DIALOG3]` (coupling) | Mover hints a la capa de escena |
| B19 | `src/patterns/pattern_thunder.c:3-4` y `pattern_en_thunder.c:7-8` | Macros `PAL_ENTRY`/`PAL0_COL4` duplicados | Mover a `constants_spells.h` o `spell.h` |
| B20 | `src/entity/entity.h` | `GameState` tiene valores muertos (`PATTERN_FINISHED`, `PATTERN_CHECK`, `ATTACK_FINISHED`, `FOLLOWING`) | Eliminar o documentar como intencional |
| B21 | `src/enemies.c:53-54` | Sprite de 3-head-monkey comentado | O se implementa o se elimina la clase; decidir con el usuario |
| B22 | `act_1.h:6` | Typo "3nd" | "3rd" o eliminar (se reescribe en Fase 5) |

B5 y B6 son **intencionalmente bloqueantes** (la VM de escena los invoca en contexto
blocking). Se documentan pero no se eliminan. B4 sí se arregla porque está dentro del
per-frame del FSM de combate.

---

## 7. Makefile y build

### 7.1 `Makefile` nuevo (propio del repo)

Basado en el `makefile.gen` de SGDK pero con:

- **Wildcard discovery**: `SRC_C = $(wildcard src/*.c) $(wildcard src/*/*.c)
  $(wildcard src/*/*/*.c)` — añadir un `.c` no requiere editar el Makefile.
- **Wildcard de recursos**: `RES_RES = $(wildcard res/*.res)`.
- **Pre-build codegen**: target `codegen` que ejecuta `tools/gen_texts.py`,
  `tools/gen_choices.py`, `tools/gen_spells.py`, `tools/gen_scenes.py` antes de compilar.
  Se chain a `all: codegen compile`.
- **Include paths**: `-Isrc -Isrc/core -Isrc/world ... -Ires` (o usar rutas relativas
  `../spells/spell.h` desde los `.c`).
- **Target `smoke`**: compila `smoke/smoke_*.c` + el código del juego con
  `-DHACK_SMOKE_BUILD` y produce `out/smoke.bin`.
- **Targets**: `release` (default, -O3 + LTO), `debug` (-O1 -g), `clean`, `smoke`,
  `codegen`.
- **Warnings**: `-Wall -Wextra -Wno-shift-negative-value -Wno-main -Wno-unused-parameter`.

### 7.2 `build-theweave.sh`

Wrapper análogo a `build-RedPlanet.sh`:

- Modos: `build` (incremental), `release` (clean + release), `clean`, `smoke`.
- Flag `--no-run` / `-n` para CI (no lanza emulador).
- Backup rotatorio de `out/TheWeave_YYYYMMDD_HHMMSS.bin` (mantiene 5).
- Si hay MiSTer online → FTP upload; si no → BlastEm.
- Usa Docker con SGDK local montado readonly (igual que RedPlanet) **o** SGDK nativo
  si el host lo tiene (decidir en Fase 2 según preferencia del usuario).

### 7.3 Integración con VS Code

`.vscode/tasks.json` se actualiza:

- "Build (Incremental)" → `./build-theweave.sh build`
- "Clean + Build (Release)" → `./build-theweave.sh release`
- "Build Smoke ROM" → `./build-theweave.sh smoke`
- "Run ROM (BlastEm)" → lanza `out/rom.bin`
- "Run Smoke ROM (BlastEm)" → lanza `out/smoke.bin`
- "Generate Data" → `make codegen` (ejecuta los 4 scripts)
- "Consolidate Code" → `python3 tools/consolidate.py`

---

## 8. ROM de smoke test (`smoke/`)

### 8.1 Objetivo

Un ROM aparte (`out/smoke.bin`) que presenta un menú y permite ejecutar cada hechizo y
cada escena de forma aislada, para verificación rápida en BlastEm sin jugar el juego
entero. Es el test principal (D4).

### 8.2 Estructura

- `smoke/smoke_menu.c` — menú principal con dos secciones: **Spells** y **Scenes**.
  Navegación con UP/DOWN, A para seleccionar. Usa el sistema de texto del juego.
- `smoke/smoke_runner.c` — ejecuta el caso seleccionado:
  - **Spell case**: inicializa un contexto mock (`SpellContext` con `enemyId`,
    `zoneId`, `activeEnemySpell` configurables desde el menú), lanza el hechizo y
    muestra el resultado (daño aplicado, estado final, frames tardados).
  - **Scene case**: llama `scene_run(sceneId)` y al terminar muestra "OK" o el paso
    donde se detuvo.
- `smoke/smoke_cases.h` — tabla de casos:
  ```c
  typedef enum { SMOKE_SPELL, SMOKE_SCENE } SmokeKind;
  typedef struct { const char *name; SmokeKind kind; u8 id; u8 zoneId; u8 activeEnemySpell; } SmokeCase;
  static const SmokeCase smoke_cases[] = {
      {"thunder (player)",      SMOKE_SPELL, SPELL_THUNDER,      0, SPELL_NONE},
      {"thunder (counter)",     SMOKE_SPELL, SPELL_THUNDER,      0, SPELL_EN_THUNDER},
      {"hide",                  SMOKE_SPELL, SPELL_HIDE,         0, SPELL_NONE},
      {"fire (no zone)",        SMOKE_SPELL, SPELL_FIRE,         0, SPELL_NONE},   /* espera fail */
      {"fire (zone cauldron)",  SMOKE_SPELL, SPELL_FIRE,         ZONE_CAULDRON, SPELL_NONE},
      {"fire (counter thunder)",SMOKE_SPELL, SPELL_FIRE,         ZONE_CAULDRON, SPELL_EN_THUNDER},
      {"en_thunder",            SMOKE_SPELL, SPELL_EN_THUNDER,   0, SPELL_NONE},
      {"en_bite",               SMOKE_SPELL, SPELL_EN_BITE,      0, SPELL_NONE},
      {"act1_scene1",           SMOKE_SCENE, SCENE_ACT1_SCENE1,  0, 0},
      {"act1_scene2",           SMOKE_SCENE, SCENE_ACT1_SCENE2,  0, 0},
      {"act1_scene3",           SMOKE_SCENE, SCENE_ACT1_SCENE3,  0, 0},
      {"act1_scene5",           SMOKE_SCENE, SCENE_ACT1_SCENE5,  0, 0},
  };
  ```

### 8.3 Verificación

- **Automática donde sea posible**: el runner chequea invariantes (p.ej. tras
  `en_thunder`, el HP del jugador bajó 1; tras `fire` en zona incorrecta, `canUse`
  devolvió false y no se lanzó). Muestra PASS/FAIL en pantalla.
- **Visual**: el humano mira que el efecto visual se ve igual que en el juego golden.
  `docs/testing.md` incluye screenshots de referencia del golden para cada caso.
- **Comparación contra golden**: `build-theweave.sh smoke --compare` (opcional, Fase 7)
  genera un log de frames (VDP state simplificado) del smoke ROM y lo diff contra un
  log del golden. Es best-effort; el humano sigue siendo el árbitro final.

### 8.4 Cómo añadir un caso

1. Añadir una fila a `smoke_cases[]` en `smoke/smoke_cases.h`.
2. (Opcional) Añadir un screenshot de referencia a `docs/testing/`.
3. Rebuild smoke. Sin tocar `smoke_menu.c` ni `smoke_runner.c`.

---

## 9. Documentación

### 9.1 `AGENTS.md` exhaustivo (estilo RedPlanet_MD, ~600 líneas)

Secciones:

1. **QUÉ ES EL PROYECTO** — Loom fangame, Mega Drive, SGDK, C, estado actual.
2. **CÓMO SE COMPILA Y EJECUTA** — `build-theweave.sh` con todos los modos, Makefile,
   requisitos (SGDK / Docker), BlastEm, MiSTer, troubleshooting.
3. **ESTRUCTURA DE DIRECTORIOS** — árbol ASCII completo con descripciones de una línea.
4. **ARQUITECTURA DEL CÓDIGO (ESTADO REAL)** — por subsistema: core, world, actors,
   combat, spells, narrative, scenes, interface, audio. Cada uno con sus archivos,
   structs clave, data flow. Referencias a `docs/*` para detalle.
5. **CONSTANTES Y PARÁMETROS CLAVE** — tabla de `config.h`, `hack.h`, `constants_*.h`.
6. **GENERACIÓN DE DATOS** — los 4 scripts de `tools/`, qué lee cada uno, qué emite,
   regla "no editar ficheros generados".
7. **RECURSOS (res/)** — qué hay en cada `.res`, organización del arte fuente.
8. **VOCES (tools/voice/)** — pipeline animalese, cómo regenerar.
9. **REGLAS DE CODIFICACIÓN** — tipos, memoria, VDP, includes explícitos, estilo
   (snake_case, Allman para funciones, comentarios `/* */` en API pública), no editar
   generados, build invocation.
10. **TAREAS COMUNES** — recetas paso a paso:
    - **Añadir un hechizo nuevo** (crear `.spell` + hooks si hace falta + caso smoke)
    - **Añadir una cutscene nueva** (crear `.scene` + re-run gen_scenes + caso smoke)
    - **Añadir diálogos** (editar `texts.csv` + re-run gen_texts)
    - **Añadir una opción de choice** (editar `choices.csv` + re-run gen_choices)
    - **Añadir un enemigo** (tocar `enemies.h` + `enemies.c` + spell de enemigo)
    - **Añadir un item** (tocar `items` + `res_items.res`)
    - **Cambiar la zona de un hechizo** (editar el `.spell` o el hook)
    - **Crear un puzzle de secuencia de hechizos** (editar `.scene` con
      `puzzle_sequence` + `if_puzzle_solved`)
11. **ESTADO DE PENDIENTES** — features futuras (act1_scene4, más actos, etc.).

Cabecera con la directiva de sync obligatoria (estilo RedPlanet_MD):
> FUNDAMENTAL: Actualiza este fichero después de cada interacción si aplica. Mantenlo
> sincronizado con el estado real del código.

### 9.2 `docs/` técnica

- `docs/architecture.md` — visión general: módulos, data flow por frame, diagrama de
  dependencias entre subsistemas. Expande §4 de AGENTS.md.
- `docs/spells.md` — guía de autoría de hechizos: struct `SpellDef`, fases, hooks,
  DSL, ejemplo `fire` end-to-end, tabla de hechizos existentes, debugging con smoke ROM.
- `docs/scenes.md` — guía de autoría de cutscenes: DSL completo, tabla de opcodes,
  ejemplo `act1_scene1` end-to-end, cómo hacer puzzles, debugging con smoke ROM.
- `docs/texts.md` — `texts.csv` y `choices.csv`, `gen_texts.py`/`gen_choices.py`,
  escapes (`|`, `@[...@]`), encoding español, voces.
- `docs/build.md` — Makefile, `build-theweave.sh`, SGDK, Docker, BlastEm, MiSTer,
  VS Code tasks.
- `docs/art.md` — organización de `res/`, convenciones para grafistas (paletas, tamaños,
  nombres), cómo añadir un sprite/fondo.
- `docs/voice.md` — pipeline de `tools/voice/`, perfiles de voz, cómo regenerar.
- `docs/testing.md` — smoke ROM, cómo añadir casos, checklist de playtest, cómo
  comparar contra golden.
- `docs/refactor/` — bitácora del refactor (ver §11).

### 9.3 Comentarios en funciones

- **API pública** (cabeceras `.h`): bloque `/* ... */` con:
  ```
  /*
   * nombre_funcion
   * --------------
   * Breve descripción de qué hace.
   *
   * Parámetros:
   *   param1 — significado
   *   param2 — significado
   *
   * Retorna:
   *   qué devuelve, o void.
   */
  ```
- **Funciones internas** (en `.c`): comentario `//` de una línea si no es obvio.
- **Doxygen** (`/** @brief ... */`) solo en `src/lib/` si aparece algo realmente
  compartido y reusable. No se impone en todo el código.

Todas las funciones existentes se documentan en la Fase 6.

---

## 10. Plan de ejecución por fases

Cada fase es un commit (o varios) en la rama `refactor`. Entre fases: build + smoke test
debe pasar. Si una fase rompe el build, se arregla antes de pasar a la siguiente.

### Fase 0 — Preparación

**Objetivo**: dejar el terreno listo para refactorizar sin perder el estado actual.

**Pasos**:
1. `git checkout -b refactor`.
2. Snapshot del ROM actual: `cp out/rom.bin docs/refactor/golden_rom.bin` (si existe);
   si no, hacer un build limpio y guardarlo.
3. Generar `docs/refactor/state_audit.md` con el informe de auditoría completo (el que
   produjo la exploración inicial: bugs, smells, estructura, acoplamiento). Referenciar
   desde `refactorizar.md` §0.
4. Generar `docs/refactor/bugs.md` con la tabla del §6.
5. Generar `docs/refactor/checklist.md` con las 8 fases y sus pasos, para tachar.
6. Confirmar que el build actual funciona: `./build-theweave.sh build` (o el comando
   que use el usuario hoy). Anotar cualquier warning.

**Criterio de verificación**: `refactor` branch creada, `docs/refactor/` poblado, build
actual funciona.

**Rollback**: `git checkout master`.

### Fase 1 — Limpieza y consistencia (sin mover archivos)

**Objetivo**: corregir los bugs del §6 y unificar convenciones, sin reestructurar nada.
El diff debe ser pequeño y revisable.

**Pasos**:
1. Fix B1 (pattern_sleep copy-paste).
2. Fix B2, B3 (slot-0 hardcodeado en update_enemy_pattern / try_counter_spell).
3. Fix B4 (hit_enemy blocking → FSM state `COMBAT_STATE_ENEMY_DEFEAT_ANIM`).
4. Fix B7, B8 (malloc/free en encode_spanish_text → buffer estático).
5. Fix B9 (MEM_alloc NULL check).
6. Fix B10 (frame_counter local muerto).
7. Fix B11 (MAX_ENEMY_PATTERNS muerto).
8. Fix B12 (unificar SIDE_* en `SIDE_LEFT`/`SIDE_RIGHT`/`SIDE_NONE`).
9. Fix B13 (unificar include guards a `_FOO_H_`).
10. Fix B14, B15, B22 (comentarios garbled / typo).
11. Fix B18 (mover hints de dialog de pattern_thunder a la capa de escena — para esto
    se anticipa un poco de la Fase 5; si es intrusivo, posponer a Fase 5).
12. Fix B19 (macros PAL_* duplicados a un sitio común).
13. Fix B20 (limpiar GameState muertos o documentar).
14. B16, B17, B21: decidir con el usuario (ver "Preguntas pendientes" al final).
15. Documentar B5, B6 como intencionalmente bloqueantes (comentario).

**Criterio de verificación**: build pasa, smoke test (aún sin smoke ROM — usar playtest
manual de las 4 escenas) comportamiento idéntico al golden. `docs/refactor/bugs.md`
actualizado con cada fix.

**Rollback**: `git reset --hard <commit_anterior_fase1>`.

### Fase 2 — Reestructuración de directorios

**Objetivo**: mover `gibberish/` a `tools/voice/`, scripts a `tools/`, `texts.csv` a
`data/`, crear `data/spells/`, `data/scenes/`, `smoke/`, `docs/refactor/`. Aún sin
particionar `src/` (eso es la Fase 3).

**Pasos**:
1. `mkdir -p tools data/spells data/scenes smoke docs/refactor`.
2. `mv gibberish tools/voice`; `rm -rf gibberish`.
3. `mv generate_texts.py tools/gen_texts.py`; actualizar paths dentro (lee `data/texts.csv`,
   escribe `src/texts_generated.{c,h}` por ahora — se mueve en Fase 3).
4. `mv add_texts_comments.py tools/`; `mv consolidate.py tools/`.
5. `mv texts.csv data/texts.csv`.
6. Actualizar `.gitignore` (`venv/`, `tools/voice/__pycache__/`, etc.).
7. Actualizar `.vscode/tasks.json` con los nuevos paths.
8. Build + playtest. Confirmar que todo sigue funcionando (los paths relativos del
   script de textos se actualizan).

**Criterio de verificación**: build pasa, `generate_texts.py` funciona desde su nueva
ubicación, `gibberish/` ya no existe.

**Rollback**: `git checkout -- .` o reset al commit de fin de Fase 1.

### Fase 3 — Partición de `src/` en subcarpetas + includes explícitos

**Objetivo**: mover los `.c`/`.h` a las subcarpetas de §2, retirar `globals.h`, pasar a
includes explícitos, introducir `config.h`/`hack.h`/`constants_*.h`. **Es la fase más
larga y la que más riesgo tiene** — hacerla en sub-commits.

**Pasos (en orden, un commit cada varios)**:
1. **Crear subcarpetas**: `src/{core,world,actors,combat,spells,narrative,scenes,interface,audio,lib}/`.
2. **Mover boot/**: `src/boot/` ya existe.
3. **Mover core**: `main.c`, `init.c` → `src/core/`; crear `config.h`, `hack.h`,
   `frame.c/.h` (extraer `next_frame`/`wait_seconds`/`frame_counter` de `globals.c`).
4. **Mover world**: `background.c/.h` → `src/world/`; crear `constants_world.h`.
5. **Mover actors**: `entity`, `characters`, `enemies`, `items`, `collisions` →
   `src/actors/`; crear `constants_actors.h`.
6. **Mover combat**: `combat.c/.h` → `src/combat/`; `constants_combat.h`.
7. **Mover spells**: renombrar `patterns/` → `spells/` y mover a `src/spells/` (los
   `.c` de callbacks se renombran `pattern_*.c` → `*_spells.c` o se integran en
   `player_spells.c`/`enemy_spells.c`); `constants_spells.h`. **Ojo**: aquí solo se
   mueve y renombra; la conversión a `SpellDef` con fases es la Fase 4.
8. **Mover narrative**: `texts`, `texts_generated` → `texts_data`, `dialogs` →
   `src/narrative/`; extraer `encode.c/.h` de `texts.c`; `constants_narrative.h`.
   Actualizar `gen_texts.py` para escribir a `src/narrative/texts_data.{c,h}`.
9. **Mover scenes**: `act_1.c/.h`, `intro.c/.h`, `geesebumps.c/.h` → `src/scenes/` por
   ahora (la conversión a DSL/VM es la Fase 5); `constants_scenes.h`.
10. **Mover interface**: `interface.c/.h` → `src/interface/`; `constants_interface.h`.
11. **Mover audio**: `sound.c/.h` → `src/audio/`; `constants_audio.h` (SFX IDs).
12. **Retirar `globals.h`/`globals.c`**: para cada `.c`, reemplazar
    `#include "globals.h"` por la lista explícita de includes que necesita (basándose
    en qué externs/functions/resources usa). Este es el paso más tedioso; hacerlo
    módulo a módulo y build tras cada uno.
13. **Makefile nuevo** (§7.1) con wildcard discovery. Verificar que compila sin tocar
    el Makefile al añadir archivos.
14. **Diagramas en cabeceras** (§5.2) para `entity.h`, `combat.h`, `spell.h` (ex
    `patterns.h`), `scene_vm.h` (ex `act_1.h`, temporal).

**Criterio de verificación**: build pasa con el Makefile nuevo, `globals.h` ya no
existe, cada `.c` lista sus includes explícitamente, playtest de las 4 escenas OK.

**Rollback**: reset al commit de fin de Fase 2. Si la retirada de `globals.h` da muchos
problemas, se puede hacer un commit intermedio con `globals.h` aún presente pero los
archivos ya movidos, y terminar la retirada en un segundo commit.

### Fase 4 — Sistema de hechizos (SpellDef + fases + hooks)

**Objetivo**: implementar el nuevo motor de hechizos y migrar los 6 hechizos actuales
con comportamiento idéntico. Crear el hechizo FIRE de ejemplo.

**Pasos**:
1. **Definir structs** (`src/spells/spell.h`): `SpellDef`, `SpellPhase`, `SpellContext`,
   `SpellOrigin`, `PhaseKind`, `PhaseTarget`. Con diagrama de arquitectura en la cabecera.
2. **Implementar motor** (`src/spells/spell.c`): `spell_validate`, `spell_launch`,
   `spell_update`, `spell_counter`, `spell_finish`. State estático: `active_spell`,
   `active_ctx`.
3. **Hooks compartidos** (`src/spells/spell_hooks.c/.h`): `zone_eq`, `counter_if_enemy`,
   `reject_if_no_enemy`, `narrative_advance`.
4. **`gen_spells.py`** (`tools/gen_spells.py`): parsea `data/spells/*.spell`, valida,
   emite `src/spells/spells_data.{c,h}`. Soporta hechizos puro-DSL (sin hooks) y
   hechizos con hooks (referenciados por nombre, enlazados en `player_spells.c`/
   `enemy_spells.c`).
5. **Migrar thunder** (`data/spells/thunder.spell` + hooks en
   `src/spells/player_spells.c`): reproducir `pattern_thunder.c` exacto (flash paleta,
   jingle, counter window, hint vs WeaverGhost — el hint se mueve a la capa de escena
   en Fase 5). Smoke test: `thunder (player)` PASS.
6. **Migrar hide**: smoke test PASS.
7. **Migrar open, sleep** (stubs): smoke test PASS (canUse false salvo scripted).
8. **Migrar en_thunder, en_bite**: smoke test PASS.
9. **Integrar motor con `combat.c`**: `update_combat` llama `spell_update` en
   `COMBAT_STATE_PLAYER_EFFECT` / `COMBAT_STATE_ENEMY_EFFECT`. `try_counter_spell`
   llama `spell_counter`. Eliminar la FSM vieja de patterns.
10. **Integrar motor con `interface.c`**: HUD lee `spells[]` para iconos.
11. **Integrar motor con `sound.c`**: jingles por `spellId`.
12. **Hechizo FIRE** (`data/spells/fire.spell` + `src/spells/fire.c` con hooks custom):
    end-to-end, zona cauldron, contrarrestro vs en_thunder, fases visuales. Smoke test
    de los 3 casos (no zone / zone / counter) PASS.
13. **`constants_spells.h`**: `ZONE_CAULDRON` y demás zones, `SFX_*` IDs.

**Criterio de verificación**: smoke ROM (que se completa en Fase 7, pero los casos de
hechizo se pueden probar ya con un main temporal) muestra los 6 hechizos migrados +
fire con resultado PASS. Playtest de `act1_scene3` (combate) idéntico al golden.

**Rollback**: reset al commit de fin de Fase 3.

### Fase 5 — Sistema de cutscenes (DSL + VM + gen_scenes)

**Objetivo**: implementar la VM de escenas y migrar las 4 escenas actuales a `.scene`
con comportamiento idéntico.

**Pasos**:
1. **Definir structs** (`src/scenes/scene_vm.h`): `SceneOp`, `SceneStep`, `SceneScript`,
   `scenes[]`. Con diagrama de arquitectura.
2. **Implementar VM** (`src/scenes/scene_vm.c`): `scene_run`, switch por op, soporte
   de `label`/`goto`/`branch`/`puzzle_*`. State: `pc`, `last_choice`,
   `puzzle_progress[][]`.
3. **Implementar API** (`src/scenes/scene_api.c/.h`): `scene_level`, `scene_char_at`,
   `scene_say`, `scene_move`, `scene_choice`, `scene_cast_spell`, `scene_wait_spell`,
   `scene_fade_out/in`, `scene_next_scene`, `scene_puzzle_define/solved`. Wrappers sobre
   funciones existentes (`new_level`, `init_character`, `talk_dialog`, `move_character`,
   `spell_launch` con `origin=NARRATIVE`, `PAL_fadeOut`, etc.).
4. **`gen_scenes.py`** (`tools/gen_scenes.py`): parsea `data/scenes/*.scene`, valida
   (labels, dialog ids, choice ids, spell names), resuelve labels, emite
   `src/scenes/scene_data.{c,h}` con `enum SceneId`.
5. **Migrar act1_scene1** (`data/scenes/act1_scene1.scene`): traducir `act_1.c:act_1_scene_1`
   a DSL. Mover el hint de thunder (B18) aquí como `say` condicional. Smoke test: escena
   completa idéntica al golden.
6. **Migrar act1_scene2, act1_scene3, act1_scene5**. La scene 3 (combate) usa
   `cast`/`wait_spell`/`if_combat_won`/`if_combat_lost` (dos ops nuevos si hace falta).
7. **`main.c`**: reemplazar el `switch(current_act)/switch(current_scene)` que llama
   `act_1_scene_N()` por `scene_run(SCENE_ACT1_SCENE_N)`.
8. **Eliminar `act_1.c/.h`**: ya no se necesita.
9. **`choices.csv` + `gen_choices.py`**: mover `act1_choice1[]` de `texts.c` a
   `data/choices.csv`, generar `src/narrative/choices_data.{c,h}`. Unificar con el
   pipeline de textos.
10. **`add_texts_comments.py`**: actualizar para que funcione con los nuevos paths y
    con `scene_data.c` (añade comentarios bilingües al lado de cada `say` del DSL —
    opcional, el DSL ya es legible).

**Criterio de verificación**: smoke ROM ejecuta las 4 escenas y terminan donde deben.
Playtest del juego completo (intro → act1 completo) idéntico al golden.

**Rollback**: reset al commit de fin de Fase 4.

### Fase 6 — Documentación

**Objetivo**: AGENTS.md exhaustivo + docs/ completa + todas las funciones comentadas.

**Pasos**:
1. **Reescribir `AGENTS.md`** con las 11 secciones de §9.1.
2. **Escribir `docs/architecture.md`**, `docs/spells.md`, `docs/scenes.md`,
   `docs/texts.md`, `docs/build.md`, `docs/art.md`, `docs/voice.md`, `docs/testing.md`.
3. **Comentar todas las funciones públicas** con el formato `/* ... */` de §9.3.
4. **Comentar funciones internas** con `//` donde no sea obvio.
5. **Cabeceras de archivo**: cada `.h` y `.c` empieza con un comentario `//` de qué es.
6. **Verificar** que no hay funciones sin comentar (script `tools/check_docs.py` que
   grep por firmas de funciones y verifica que hay un `/*` encima — opcional).

**Criterio de verificación**: un desarrollador nuevo lee AGENTS.md + docs/ y entiende
cómo añadir un hechizo, una escena, un diálogo. `check_docs.py` (si se hace) pasa.

**Rollback**: la documentación no rompe nada; rollback solo si se quiere descartar.

### Fase 7 — Smoke ROM + verificación final

**Objetivo**: completar el smoke ROM, añadir todos los casos, verificar contra golden,
merge a master.

**Pasos**:
1. **`smoke/smoke_menu.c`**: menú con secciones Spells/Scenes, navegación.
2. **`smoke/smoke_runner.c`**: ejecuta cada caso, muestra PASS/FAIL.
3. **`smoke/smoke_cases.h`**: tabla completa (todos los hechizos + 4 escenas + fire).
4. **Target `smoke` del Makefile**: compila `out/smoke.bin`.
5. **`build-theweave.sh smoke`**: build + run en BlastEm.
6. **Playtest golden vs refactor**: ejecutar el juego actual (master) y el refactorizado
   side-by-side. Comparar las 4 escenas y los 6 hechizos. Documentar cualquier
   discrepancia en `docs/refactor/checklist.md` y decidir si es aceptable o fix.
7. **Screenshots de referencia**: capturar los casos del smoke ROM y guardarlos en
   `docs/testing/` para futuras regresiones.
8. **`docs/testing.md`**: completo con checklist de playtest y cómo añadir casos.
9. **Merge `refactor` → `master`**: solo si todo pasa. Fast-forward o merge commit
   `Refactor: spells declarativos, scene VM, includes explícitos, docs exhaustivas`.
10. **Tag**: `git tag v2.0-refactor` (opcional).

**Criterio de verificación**: `out/smoke.bin` todos PASS, playtest golden idéntico
(salvo discrepancias documentadas y aceptadas), `master` actualizado.

**Rollback**: si el merge revela problemas, `git reset --hard <pre-merge>` en master.

---

## 11. Bitácora del refactor (`docs/refactor/`)

- `state_audit.md` — informe de auditoría del estado inicial (generado en Fase 0).
- `bugs.md` — tabla de bugs (§6) con estado (corregido/pendiente/aceptado).
- `checklist.md` — las 8 fases con sus pasos, para tachar al completar.
- Se actualiza tras cada fase.

---

## 12. Pruebas (resumen, D4)

El test principal es la **smoke ROM** (§8). Adicionalmente:

- **Playtest manual con checklist** (`docs/testing.md`): lista por escena/hechizo con
  comportamiento esperado, comparando contra el golden.
- **Comparación visual**: screenshots de referencia del golden en `docs/testing/`.
- **Unit tests de lógica pura** (opcional, si el usuario los quiere en el futuro):
  compilar `spell_validate`, `encode_spanish_text`, `scene_vm` (con mocks) con gcc en
  PC y un `main()` de tests. No es parte del plan actual (D4 eligió smoke ROM), pero
  la arquitectura lo permite porque el motor de hechizos y la VM están separados del
  hardware.

**Criterio de "no regressión"**: el smoke ROM de todos los casos debe dar PASS tras
cada fase desde la Fase 4 en adelante.

---

## 13. Convenciones de estilo (a aplicar en todo el refactor)

- **Indentación**: 4 espacios, sin tabs.
- **Nombres**: `snake_case` para funciones y variables; `UPPER_CASE` para macros y
  `#define`; `PascalCase` para tipos (`Entity`, `SpellDef`, `SceneStep`).
- **Tipos**: SGDK fixed-width (`u8`, `u16`, `u32`, `s8`, `s16`, `s32`, `bool`). Nunca
  `int`/`short` bare.
- **Llaves de función**: Allman (brace en la línea siguiente) — **estilo actual de The
  Weave**, se mantiene (a diferencia de RedPlanet_MD que usa K&R). Llaves de
  `if/for/while/switch`: consistentes (elegir K&R same-line, que es lo más común en el
  código actual).
- **Declaración de función**: `tipo nombre(params)` en una línea si cabe; si no,
  `tipo\nnombre(params)`. Comentario `//` opcional a la derecha de la firma.
- **Comentarios**: `/* ... */` con bloque `Parámetros:`/`Retorna:` en API pública
  (cabeceras); `//` para notas internas. Sin comentarios garbled. Sin emojis.
- **Includes**: explícitos por archivo (D1). Orden: `<genesis.h>` primero, luego
  módulos del proyecto (`"../xxx/yyy.h"` o `"yyy.h"` según include path), luego
  recursos (`"../res/res_zzz.h"`), luego `constants_*.h`.
- **Memoria**: sin `malloc`/`free` en hot paths. Pools estáticos. `static const` para
  tablas en ROM. Buffer estático para `encode_spanish_text`.
- **Globals**: `extern` en `.h`, definidos en un `.c` por subsistema. Mínimos.
- **Functions privadas**: `static`.
- **Data tables**: designated initializers `[SPELL_THUNDER] = { ... }`.
- **Timers**: escalados con `SCREEN_FPS` (`SCREEN_FPS * 4` = 4s), no hardcoded `240`.
- **Include guards**: `_FOO_H_`.
- **No editar generados**: `*_data.c/.h`, `res_*.h`. Cabecera `// Generated by ...`.

---

## 14. Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| Retirada de `globals.h` rompe todo de golpe | Hacerla módulo a módulo con build tras cada uno (Fase 3). Commit intermedio con `globals.h` aún presente si hace falta. |
| Migración de hechizos cambia comportamiento sutil | Smoke ROM por hechizo tras cada migración (Fase 4). Golden como referencia. |
| Migración de escenas pierde un branch/route | Migrar una escena, playtest completo, antes de la siguiente. `gen_scenes.py` valida labels/refs. |
| `gen_scenes.py` / `gen_spells.py` tienen bugs | Tests de golden-file para los generadores (opcional, ver §12). Validación estricta en el script. |
| El DSL no expresa algo que la escena necesita | Añadir un `SceneOp` nuevo (la VM es extensible). Documentar en `docs/scenes.md`. |
| Performance: más capas (VM, motor de fases) | No es prioritario (D1: legibilidad > rendimiento). Medir si preocupa. El hardware lo asume (no se acerca al límite). |
| Tamaño del refactor | 8 fases, cada una commit independiente. Se puede parar entre fases y el juego sigue funcionando. |

---

## 15. Preguntas pendientes (resolver antes de la fase correspondiente)

- **B16, B17, B21** (Fase 1): ¿Better enemy defeat handling se hace ahora o se aplaza al
  nuevo FSM de combate (Fase 4)? ¿Jingles de SLEEP/EN_BITE se añaden o se documentan
  como intencional? ¿3-head-monkey se implementa o se elimina la clase?
- **Build en Docker vs SGDK nativo** (Fase 2): ¿`build-theweave.sh` usa Docker con SGDK
  local (como RedPlanet) o SGDK nativo del host? Depende de cómo compile hoy el usuario.
- **`res_*.h` con o sin prefijo** (Fase 3): confirmado mantener prefijo `res_` (D8).
- **Smoke ROM: ¿menú en la misma ROM o ROM aparte?** (Fase 7): confirmado ROM aparte
  (`out/smoke.bin`, target `smoke` del Makefile).
- **Unit tests de PC** (futuro): no en este plan (D4), pero la arquitectura los permite.

---

## 16. Cómo ejecutar este plan ("venga, refactoriza")

1. Confirmar que `refactorizar.md` está al día y las decisiones de §1 cuadran.
2. `git checkout -b refactor` (Fase 0 ya lo hace).
3. Ejecutar fases 0 → 7 en orden. Tras cada fase:
   - Build: `./build-theweave.sh build` (o `make release`).
   - Smoke (desde Fase 4): `./build-theweave.sh smoke`.
   - Playtest manual de la escena/hechizo afectado.
   - Actualizar `docs/refactor/checklist.md` (tachar la fase).
   - Commit con mensaje `Refactor Fase N: <resumen>`.
4. Si una fase revela que una decisión de §1 es mala, parar, actualizar `refactorizar.md`
   y `AGENTS.md`, y continuar.
5. Al final (Fase 7): merge a master, tag `v2.0-refactor`.

**Regla de oro**: si en cualquier punto el build no pasa o el smoke/playtest revela una
regresión, se arregla antes de avanzar. No se acumula deuda durante el refactor.

---

## 17. Apéndice: mapeo archivo-viejo → archivo-nuevo

| Archivo actual | Archivo refactorizado | Fase |
|---|---|---|
| `src/globals.h` | (eliminado) | 3 |
| `src/globals.c` | `src/core/frame.c/.h`, `src/core/config.h` | 3 |
| `src/main.c` | `src/core/main.c` | 3 |
| `src/init.c/.h` | `src/core/init.c/.h` | 3 |
| `src/entity.c/.h` | `src/actors/entity.c/.h` | 3 |
| `src/characters.c/.h` | `src/actors/characters.c/.h` | 3 |
| `src/enemies.c/.h` | `src/actors/enemies.c/.h` | 3 |
| `src/items.c/.h` | `src/actors/items.c/.h` | 3 |
| `src/collisions.c/.h` | `src/actors/collisions.c/.h` | 3 |
| `src/background.c/.h` | `src/world/background.c/.h` | 3 |
| `src/controller.c/.h` | `src/core/controller.c/.h` (o `src/actors/` — decidir en Fase 3) | 3 |
| `src/combat.c/.h` | `src/combat/combat.c/.h` | 3 |
| `src/interface.c/.h` | `src/interface/interface.c/.h` | 3 |
| `src/sound.c/.h` | `src/audio/sound.c/.h` | 3 |
| `src/patterns.c/.h` | `src/spells/spell.c/.h` + `spells_data.c/.h` | 4 |
| `src/patterns/*.c` | `src/spells/player_spells.c/.h` + `enemy_spells.c/.h` + `fire.c/.h` | 4 |
| `src/texts.c/.h` | `src/narrative/texts.c/.h` + `encode.c/.h` | 3 |
| `src/texts_generated.c/.h` | `src/narrative/texts_data.c/.h` | 3 |
| `src/dialogs.c/.h` | `src/narrative/dialogs.c/.h` | 3 |
| `src/act_1.c/.h` | (eliminado) → `data/scenes/act1_scene*.scene` + `src/scenes/scene_vm.c/.h` + `scene_api.c/.h` + `scene_data.c/.h` | 5 |
| `src/intro.c/.h` | `src/scenes/intro.c/.h` (o se queda como código C, no DSL) | 3 |
| `src/geesebumps.c/.h` | `src/scenes/geesebumps.c/.h` (idem) | 3 |
| `gibberish/` | `tools/voice/` | 2 |
| `generate_texts.py` | `tools/gen_texts.py` | 2 |
| `add_texts_comments.py` | `tools/add_texts_comments.py` | 2 |
| `consolidate.py` | `tools/consolidate.py` | 2 |
| `texts.csv` | `data/texts.csv` | 2 |
| (n/a) | `data/choices.csv` + `tools/gen_choices.py` + `src/narrative/choices_data.c/.h` | 5 |
| (n/a) | `data/spells/*.spell` + `tools/gen_spells.py` + `src/spells/spells_data.c/.h` | 4 |
| (n/a) | `data/scenes/*.scene` + `tools/gen_scenes.py` + `src/scenes/scene_data.c/.h` | 5 |
| (n/a) | `smoke/smoke_menu.c/.h` + `smoke_runner.c/.h` + `smoke_cases.h` | 7 |
| `AGENTS.md` | `AGENTS.md` (rewrito) + `docs/*` | 6 |
| `Makefile` (externo) | `Makefile` (propio, wildcard + codegen) | 3 |
| (n/a) | `build-theweave.sh` | 3 |

---

*Fin del plan. Mantener sincronizado con el estado real del refactor.*
