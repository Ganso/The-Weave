# Hechizos — guía de autoría

Arquitectura completa en `src/spells/spell.h` (bloque de cabecera) y AGENTS.md §5.
Resumen: tabla única `spell_defs[SPELL_COUNT]` + motor de **dos slots** (jugador y
enemigo simultáneos — la base del counter). El motor gestiona TODO el ciclo de vida;
los hooks solo aportan la lógica específica y nunca tocan timers ni cleanup.

## Crear un hechizo nuevo (receta)

1. **Copia `src/spells/fire.c/.h`** — es el ejemplo canónico comentado (zona,
   comportamiento condicional, fases declarativas, hooks mínimos).
2. Añade `SPELL_MIO` en `src/spells/constants_spells.h` (antes de
   `SPELL_PLAYER_COUNT` si es de jugador; después si es de enemigo) y ajusta los
   contadores.
3. Llama a `mio_init()` desde `init_spells()` (`spell.c`).
4. Icono (jugador): sprite en `res/res_interface.res` + caso en `show_pattern_icon`
   (interface.c). Jingle: caso en `play_spell_jingle` (sound.c).
5. Desbloqueo: `spell_enable(SPELL_MIO)` (silencioso, en un hook de setup) o
   `activate_spell(SPELL_MIO)` (con jingle y notas — para cutscenes).
6. Añade un caso a la smoke ROM (`src/smoke/smoke_cases.h`).

## SpellDef — campos

| Campo | Significado |
|---|---|
| `notes[4]` / `noteCount` | Secuencia NOTE_MI..NOTE_DO (en_bite usa 3) |
| `isPalindrome` | notas == invertidas → no existe forma invertida (hide) |
| `counterable` | el jugador puede contrarrestarlo (hechizos de enemigo) |
| `baseDuration` | frames de efecto — SIEMPRE `SCREEN_FPS * n` (runtime, PAL/NTSC) |
| `rechargeInit` | recarga inicial al entrar en combate (enemigos) |
| `enabled` | desbloqueado (jugador) |
| `phases` / `phaseCount` | fases declarativas opcionales |
| `icon` | icono HUD/pausa (se carga bajo demanda) |

## Hooks (todos opcionales, NULL si no hacen falta)

| Hook | Cuándo | Uso típico |
|---|---|---|
| `canUse(ctx)` | antes de lanzar | restricción de zona, ventana de counter |
| `onRejected(ctx)` | canUse devolvió false | hint con diálogo (thunder vs ghost) |
| `onLaunch(ctx)` | al empezar el efecto | guardar paleta, jingle, comerse otro hechizo |
| `onUpdate(ctx)` | cada frame de efecto | efectos imperativos (flash alternante); `true` = terminar ya. El auto-fin por `baseDuration` aplica SIEMPRE |
| `onFinish(ctx)` | SOLO fin natural | daño diferido (en_thunder → hit_player), restaurar |
| `onCounter(ctx)` | contrarrestado | daño al lanzador + restaurar |
| `onCancel(ctx)` | cortado sin counter | solo limpiar (restaurar paleta) |

El `ctx` trae: `spellId`, `origin` (PLAYER/ENEMY/NARRATIVE), `reversed`,
`frameCounter` (solo lectura), `enemyId`, `zoneId`.

## Fases declarativas

```c
// [startFrame..endFrame] relativo al inicio del efecto; rellenar en runtime:
fases[0] = (SpellPhase){ 0, SCREEN_FPS*2, PHASE_VISUAL_FLASH, PAL0_COL4, COLOR_X };
fases[1] = (SpellPhase){ SCREEN_FPS, SCREEN_FPS, PHASE_LOGIC_DAMAGE, PHASE_TARGET_ENEMY_ACTIVE, 2 };
```
- `PHASE_VISUAL_FLASH`: fija un color de paleta cada frame del rango.
- `PHASE_LOGIC_DAMAGE`: acción puntual — usar `start == end` (dispara una vez).

## Zonas (puzzles narrativos)

`spell_zone` (ZONE_* en constants_spells.h) la fija la escena con el op `zone`;
los `canUse` la reciben en `ctx->zoneId`. Ejemplo: FIRE solo castea en
`ZONE_CAULDRON`. Cast scripted desde una escena: op `cast` (origin NARRATIVE,
sin canUse — el guion manda).

## Hechizos actuales

| Hechizo | Notas | Duración | Particularidades |
|---|---|---|---|
| THUNDER | MI FA SOL SOL | 4 s | invertido = counter del thunder enemigo; hints vía onRejected |
| HIDE | FA SOL SOL FA | 4 s | palíndromo; corta el hechizo enemigo en curso |
| OPEN | FA SI SOL DO | 0,75 s | solo scripted (canUse false) |
| SLEEP | FA MI DO LA | 1,25 s | solo scripted |
| FIRE | MI FA SOL LA | 2 s | ejemplo: zona + come thunder + fases; no desbloqueado |
| EN_THUNDER | MI FA SOL SOL | 1 s | counterable; daño al jugador al fin natural |
| EN_BITE | MI SOL DO | 1 s | DESHABILITADO (decisión de diseño; sin daño heredado) |

## Caso práctico: crear el hechizo VIENTO paso a paso

Un hechizo de jugador de ejemplo que toca **todos los conceptos fundamentales**:
`canUse` condicional + su `onRejected`, `onLaunch`, fases declarativas (una visual
y una de daño), `onFinish`, y el cableado completo (id, init, icono, jingle,
desbloqueo, smoke). Diseño: "una ráfaga que solo puede lanzarse contra un enemigo
en combate; destella verde y le hace 1 de daño a mitad del efecto".

### 1. Reservar el id — `src/spells/constants_spells.h`

Los ids de jugador van antes de `SPELL_PLAYER_COUNT`; al insertar uno, los de
enemigo y los contadores se corren (son índices internos, sin problema):

```c
#define SPELL_FIRE         4
#define SPELL_LIGHT        5
#define SPELL_WIND         6      // nuevo
#define SPELL_PLAYER_COUNT 7     // era 6
// Enemy spells
#define SPELL_EN_THUNDER   7     // era 6
#define SPELL_EN_BITE      8     // era 7
#define SPELL_COUNT        9     // era 8
```

### 2. El hechizo — `src/spells/wind.c` y `wind.h`

```c
// wind.h
#ifndef _WIND_H_
#define _WIND_H_
void wind_init(void); // Registra VIENTO en spell_defs[] (lo llama init_spells)
#endif
```

```c
// wind.c
#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "spells/spell.h"
#include "spells/wind.h"
#include "combat/combat.h"
#include "audio/sound.h"
#include "narrative/dialogs.h"
#include "narrative/texts_data.h"
#include "actors/enemies.h"

#define COLOR_WIND_VDP  RGB24_TO_VDPCOLOR(0x66FF88)   // verde ráfaga

static SpellPhase wind_phases[2];   // se rellenan en runtime (SCREEN_FPS)
static u16 wind_saved_color;

// canUse: solo directo y solo si hay un enemigo activo en combate.
static bool wind_can_use(const SpellContext *ctx)
{
    if (ctx->reversed) return false;             // no tiene forma invertida útil
    if (ctx->enemyId == ENEMY_NONE) return false; // requiere blanco
    return true;
}

// onRejected: feedback cuando canUse devolvió false (aquí, diálogo genérico).
static void wind_on_rejected(SpellContext *ctx)
{
    (void)ctx;
    talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN], false);
}

static void wind_on_launch(SpellContext *ctx)
{
    (void)ctx;
    wind_saved_color = PAL_getColor(PAL0_COL4);  // guardar para restaurar luego
    play_spell_jingle(SPELL_WIND);
    // el resto (flash + daño) lo declaran las fases
}

static void wind_on_finish(SpellContext *ctx)
{
    (void)ctx;
    PAL_setColor(PAL0_COL4, wind_saved_color);   // restaurar el cielo
}

void wind_init(void)    // llamado desde init_spells()
{
    // Fase visual: destello verde el primer segundo.
    wind_phases[0] = (SpellPhase){ 0, SCREEN_FPS, PHASE_VISUAL_FLASH, PAL0_COL4, COLOR_WIND_VDP };
    // Fase de daño: PUNTUAL (start==end) a mitad del efecto → 1 de daño al enemigo activo.
    wind_phases[1] = (SpellPhase){ SCREEN_FPS/2, SCREEN_FPS/2, PHASE_LOGIC_DAMAGE, PHASE_TARGET_ENEMY_ACTIVE, 1 };

    spell_defs[SPELL_WIND] = (SpellDef){
        .id = SPELL_WIND,
        .notes = { NOTE_LA, NOTE_SOL, NOTE_FA, NOTE_MI }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS,              // 1 segundo, PAL o NTSC
        .enabled = false,                        // lo desbloquea la escena
        .canUse = wind_can_use, .onRejected = wind_on_rejected,
        .onLaunch = wind_on_launch, .onFinish = wind_on_finish,
        .phases = wind_phases, .phaseCount = 2,
        // sin onUpdate/onCounter/onCancel: este hechizo no los necesita
    };
}
```

Conceptos que ilustra y **cuándo NO** usar cada hook: no hay `onUpdate` porque el
efecto visual es declarativo (fases), no imperativo; no hay `onCounter` porque no
es `counterable`; no hay `onCancel` porque no hay nada que limpiar si se corta
(el `onFinish` que restaura el color solo corre en fin natural — si necesitaras
restaurar también al cancelar, añadirías `onCancel`).

### 3. Registrar en el motor — `src/spells/spell.c`

```c
#include "spells/wind.h"
// ... dentro de init_spells(), junto a los otros *_init():
    wind_init();
```

### 4. Icono de HUD — `res/` + `interface.c`

Crea `res/Sprites/Interface/pattern_wind.png` (32×32, 4×4 tiles, misma paleta
indexada que los demás — ver cómo se generaron los de FIRE/LUZ en el commit
"Iconos de HUD para FIRE y LUZ"). Decláralo y cablea el caso en las DOS funciones
que dibujan iconos:

```
# res/res_interface.res
SPRITE int_pattern_wind "Sprites/Interface/pattern_wind.png" 4 4 BEST
```
```c
// interface.c — en show_pattern_icon Y en show_icon_in_pause_list:
if (npattern==SPELL_WIND) nsprite = &int_pattern_wind;
```

### 5. Jingle — `src/audio/sound.c`

```c
// en play_spell_jingle(), reutilizando un sample existente:
case SPELL_WIND:
    play_sample(snd_pattern_open, sizeof(snd_pattern_open));
    break;
```

### 6. Desbloqueo y prueba

- En un hook de setup de escena: `spell_enable(SPELL_WIND);` (silencioso) o
  `activate_spell(SPELL_WIND);` (con jingle y notas, para una cutscene de "has
  aprendido...").
- Smoke ROM (`src/smoke/smoke_cases.h`): un CHECK y un CAST.
  ```c
  {"CHK wind sin enemigo - NO", SMOKE_CHECK, SPELL_WIND, false, 0, false, NULL},
  {"CAST wind (1s verde)",      SMOKE_CAST,  SPELL_WIND, false, 0, false, NULL},
  ```
  (El CAST usa `spell_narrative_cast`, que no pasa por `canUse`, así que el
  destello se ve aunque no haya enemigo; el daño de la fase no hace nada sin
  blanco — `enemyId == ENEMY_NONE`.)

### 7. Compilar

`./build-theweave.sh release` (o `smoke`). El build corre el codegen y compila;
si olvidaste el `wind_init()` o un include, el enlazador o el compilador lo dicen.
Notas del hechizo: **LA SOL FA MI** (botones X C B A).

## Depurar

`src/core/hack.h`: `HACK_ALL_SPELLS`, `HACK_ENEMIES_ONE_HP`,
`HACK_PLAYER_INVULNERABLE`, `HACK_START_SCENE "act1_forest"`, `DEBUG_LEVEL 2`
(trazas del motor por KDebug en BlastEm). Smoke ROM: `docs/testing.md`.
