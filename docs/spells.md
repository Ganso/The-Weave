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

## Depurar

`src/core/hack.h`: `HACK_ALL_SPELLS`, `HACK_ENEMIES_ONE_HP`,
`HACK_PLAYER_INVULNERABLE`, `HACK_START_SCENE "act1_forest"`, `DEBUG_LEVEL 2`
(trazas del motor por KDebug en BlastEm). Smoke ROM: `docs/testing.md`.
