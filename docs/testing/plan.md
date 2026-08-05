# Roadmap de automatización del testing

Este documento es la **hoja de ruta de lo que queda** para llevar la verificación
desatendida del juego más allá de donde está hoy. Para el estado y el uso de lo que ya
funciona, ver `docs/testing.md` (smoke ROM y checklist) y `docs/retroarch-mcp.md`
(driver, marcadores, gate).

## Ya disponible (contexto)

- **Suite AUTO de la smoke ROM + driver MCP de RetroArch**: la ROM corre desatendida
  (`RESULT: ALL PASS`) publicando su progreso en RAM, y `tools/retroarch/mcp_driver.py`
  la conduce desde el host con exit code y capturas. Cubre hechizos (invariantes
  `canUse` + casts medidos), movimiento y combate de patrones. Detalle en
  `docs/retroarch-mcp.md`.
- **Toggles de `hack.h` cableados**: los 8 toggles compile-time
  (`HACK_ALL_SPELLS`, `HACK_PLAYER_INVULNERABLE`, `HACK_ENEMIES_ONE_HP`,
  `HACK_FAST_DIALOGS`, `HACK_MUTE_MUSIC`, `HACK_MUTE_SFX`, `HACK_START_SCENE`,
  `HACK_FORCE_LANGUAGE`) tienen ya su punto de consulta en código real.

Lo que sigue **no** está implementado. Referencia de código para el port: Red Planet
(`../RedPlanet_MD` — `src/core/input.c`, `src/debug/`), que ya tiene el embudo de input
y el menú de debug runtime.

Convenciones al abordarlo: fases completas, un commit por hito
(`Testing Fase N: <resumen>`), verificación al cierre de cada fase. Reparto: los builds
(`--no-run`), greps y comprobaciones de `symbol.txt` los hace el agente; todo lo que
requiere emulador con GUI (BlastEm, RetroArch) lo ejecuta Javier y reporta.

## Qué queda, y en qué orden

```
Fase 3 (embudo input + override)  — HECHA, salvo el caso smoke SIM (§3.4)
Fase 4 (menú de debug runtime)    — independiente; pendiente
Fase 6 (escenas bajo guion)       — ya desbloqueada por la Fase 3; pendiente
```

Las tres son independientes entre sí salvo que la Fase 6 necesita el override de la
Fase 3 — que ya está, así que la 6 puede abordarse cuando se quiera: es la que mete las
**escenas** en la verificación automática, hoy 100 % playtest manual.

---

## Fase 3 — Embudo de input y override por RAM ✅ HECHA

Objetivo: que una **única** función lea el pad en todo el juego y que, con dos símbolos
en RAM, se pueda pilotar la ROM real desde fuera (RetroArch) o desde dentro (casos SIM
de la smoke).

> **Estado**: hechos el seam (§3.1), la sustitución de los 24 llamadores (§3.2), la
> receta de pilotaje (§3.3, en `docs/retroarch-mcp.md` §7) y el caso smoke
> `SIM patron notas` (§3.4). De propina, la suite AUTO publica ahora su veredicto en la
> global `smoke_result` (máscara de fallos, 0 = ALL PASS), para que una run desatendida
> no dependa de mirar una captura.
>
> ⚠️ **El caso SIM está compilado pero NUNCA EJECUTADO**: RetroArch 1.20 con
> `video_driver = "null"` ya no corre el bucle principal (carga la ROM, pero
> `READ_CORE_RAM` da timeout y `frame_counter` no avanza), así que no hay vía headless
> hoy. Hay que correrlo con emulador de verdad antes de darlo por bueno.
>
> Lo de abajo se conserva como registro de lo que se hizo y por qué.

### 3.1 El seam

Nuevo `src/core/input.c/.h` (se mantiene `controller.c` con la semántica de gameplay;
`input.c` es solo la costura con el hardware):

```c
volatile u16 g_inputOverride;        // máscara BUTTON_* inyectada
volatile u16 g_inputOverrideActive;  // ≠0 → ignorar el mando físico

u16  pad_read(void);                 // JOY_readJoypad(JOY_ALL) u override
void input_setOverride(u16 buttons);
void input_clearOverride(void);
```

No-static (localizables en `symbol.txt`, escribibles por `WRITE_CORE_RAM`). Añadir
`input.h` a la metalibrería `core/core.h`.

### 3.2 Sustituir los sitios que leen el pad

Sustitución mecánica 1:1 (`JOY_readJoypad(JOY_ALL|JOY_1)` → `pad_read()`) en todos los
llamadores salvo el menú de la smoke ROM (`src/smoke/smoke_main.c`), que **lee el pad
físico a propósito**: si un caso deja el override activo, el menú debe seguir siendo
navegable. La lógica de flancos (`joy_state`/`prev_joy_state`) se queda en cada
llamador tal cual. Aprovechar para unificar la inconsistencia `JOY_ALL`/`JOY_1` a
`JOY_ALL`. Para localizar los sitios: `grep -rn JOY_readJoypad src/` (el archivo que
más gana es `narrative/dialogs.c`, con los bucles de avance/choices).

### 3.3 Patrón de pulso para los bucles bloqueantes

Los llamadores esperan flancos (release → press). El pilotaje externo debe emular
pulsaciones humanas: escribir la máscara N frames, luego 0 durante M frames.
Documentar la receta en `docs/retroarch-mcp.md` (sección "pilotar el juego real"):

```python
ra.write_u16(syms["g_inputOverride"], 0x0040)      # BUTTON_A
ra.write_u16(syms["g_inputOverrideActive"], 1)
# ... ~10 frames ...
ra.write_u16(syms["g_inputOverride"], 0)           # release (¡antes de soltar el override!)
```

### 3.4 Caso smoke que ejercita el seam (sin emulador)

Añadir un caso `SIM patron notas`: usa `input_setOverride` para inyectar la secuencia
de botones de un patrón de 4 notas frame a frame contra la detección de patrones, y
comprueba que el hechizo se reconoce. Valida el override desde dentro de la ROM (entra
en AUTO) y de paso cubre la detección de patrones, hoy sin test.

### Verificación (Fase 3)

- Agente: build normal y smoke `--no-run`; `grep -rn JOY_readJoypad src/` → solo
  `input.c` y `smoke_main.c`; `grep g_inputOverride out/symbol.txt` → 2 símbolos.
- Javier: playtest corto del checklist (`docs/testing.md`) — movimiento, diálogo con
  choice, pausa, un combate: idéntico a antes. Con RetroArch: la receta de §3.3 avanza
  un diálogo sin tocar el mando.

---

## Fase 4 — Menú de debug runtime sobre el Window plane

Objetivo: que los toggles de `hack.h` se puedan cambiar **en caliente** desde un menú
oculto, sin recompilar. Con `DEBUG_MENU_ENABLED FALSE` la ROM queda idéntica a hoy.

> Los 6 toggles de gameplay ya tienen su punto de consulta (ver "Ya disponible"). Esta
> fase los pasa de constantes compile-time a estado RAM (`g_hacks`) modificable, y
> añade el menú; `HACK_START_SCENE` y `HACK_FORCE_LANGUAGE` se quedan compile-time
> (solo tienen sentido en el arranque).

> **Estado: PENDIENTE, pero con el diseño ya cerrado.** Lo de abajo se revisó contra el
> código real el 2026-08-05, y las correcciones respecto al plan original están marcadas
> como ⚠️. La referencia a portar es `../RedPlanet_MD/src/debug/` (unas 600 líneas entre
> los cuatro módulos), que ya resuelve la forma de la tabla de entradas y la FSM del
> código de acceso.

### 4.0 Los seis puntos de consulta (ya localizados)

Uno por toggle, la conversión es una línea en cada sitio:

| Toggle | Dónde se consulta hoy |
|---|---|
| `fastDialogs` | `src/narrative/dialogs.c:26` |
| `muteMusic` / `muteSfx` | `src/audio/sound.c:10` y `:23` |
| `enemiesOneHp` / `playerInvulnerable` | `src/combat/combat.c:60` y `:92` |
| `allSpells` | `src/spells/spell.c:79` — ⚠️ **NO es un toggle** |

⚠️ `HACK_ALL_SPELLS` se consulta **una sola vez, dentro de `init_spells()`**. Como
casilla del menú no haría nada, así que va como **acción** (ver 4.3).

### 4.1 `debug_hacks.c/.h`

- `struct DebugHacks` con los campos bool de los toggles de gameplay; `g_hacks` global.
- `debug_hacks_init()` carga los defaults de `hack.h`; se llama **una** vez en ambos
  mains (`src/core/main.c` y `src/smoke/smoke_main.c`), no por escena: los hacks
  sobreviven a transiciones.
- Los puntos de consulta actuales (`if (HACK_X)`) pasan a `if (g_hacks.x)`.
- Con `DEBUG_MENU_ENABLED FALSE`: `g_hacks` se define como literal compuesto `const`
  con los defaults → el compilador pliega los `if` y elimina el código muerto;
  `src/debug/` entero queda fuera del binario (guards `#if DEBUG_MENU_ENABLED`).

### 4.2 `debug_code.c/.h` — detector de secuencia

FSM estilo Konami sobre just-pressed con timeout (~1 s), portada de
`../RedPlanet_MD/src/debug/debug_code.c`. Dos correcciones sobre el plan original:

- ⚠️ **El código NO puede ser `X Y X Y`**: en The Weave X/Y/Z **son notas musicales**
  (LA/SI/DO), así que se dispararía tocando patrones. Usar una secuencia terminada en
  START, que es inerte fuera de la pausa (p. ej. `A B C START`, o el Konami completo).
- ⚠️ **Engancharlo en `next_frame()`** (`src/core/frame.c`), **no** en `joy_check()`:
  `joy_check` solo corre con `movement_active`, mientras que `next_frame` es el latido
  universal — los diálogos lo llaman en sus diez bucles (`dialogs.c` usa
  `next_frame(false)`) y las cutscenes también. Enganchar ahí hace que el menú se abra
  **también dentro de diálogos y cutscenes**, que es donde más falta hace. Queda fuera
  solo el bucle de `pause_screen`, que usa `SYS_doVBlankProcess` directo.
- Como en `next_frame` no interactivo nadie lee el pad, `debug_code_tick()` debe ser
  **autónomo**: que llame él a `pad_read()` y calcule el flanco con su propio `prev`
  estático (RedPlanet se apoya en un `input_justPressed()` global que aquí no existe).
  Al pasar por `pad_read()`, el código de acceso también es pilotable por RAM.

### 4.3 `debug_menu.c/.h` — UI sobre el Window plane

⚠️ **Modelarlo sobre `pause_screen()`** (`src/interface/interface.c:386`), no sobre el
`debug_menu.c` de RedPlanet: allí el juego separa update de render y el menú es un
estado más, mientras que aquí lo natural es un **bucle bloqueante propio** que llama
`SYS_doVBlankProcess()`. Nunca `next_frame()`, que es desde donde se le invoca (sería
reentrante). `pause_screen` ya hace exactamente este ciclo y tiene los helpers hechos.

Ciclo, calcado del de la pausa:

1. Abrir: `hide_interface_sprites()` / `hide_all_sprites()` y `save_animation_states()`
   (ya existen en `interface.h`); subir la window a pantalla completa con
   `VDP_setWindowVPos(FALSE, 26)`.
2. Bucle: UP/DOWN mueven el cursor, LEFT/RIGHT/A alternan, B o START cierran —
   esperando a que el pad quede en reposo antes de devolver el control, para que la
   pulsación de cierre no dispare una nota.
3. Cerrar: restaurar `VDP_setWindowVPos(TRUE, 22)` (el valor que fija `init.c:58`),
   `VDP_clearPlane(WINDOW, true)` y repintar el HUD con `show_or_hide_interface(true)`.

Entradas:

1. Casillas SI/NO: INVULNERABLE, ENEMIGOS 1 HP, DIALOGOS RAPIDOS, SIN MUSICA, SIN SFX.
2. `DAR VARA Y HECHIZOS` — acción que replica el bloque de `spell.c:79`. ⚠️ **Trampa
   crítica** (AGENTS §7): cambiar la forma de Linus a mitad de escena exige
   `reinit_character_sprite`; darle la vara sin eso deja el sprite sin las filas de
   animación que se le pedirán y **congela el VDP** en el siguiente `SPR_setAnim`.
3. `CURAR` y `MATAR ENEMIGOS` — acciones baratas sobre `player_hitpoints` y `hit_enemy`.
4. Línea INFO: escena actual (`scenes[current_scene_id].name`), FPS y RAM libre.
5. `CERRAR` (también con START).

⚠️ **Trampa de la fuente** (AGENTS §7): en texto CRUDO (`VDP_drawTextBG`, que es lo que
usa el menú) la fuente tiene los glifos españoles en las posiciones ASCII de
`/ < > ^ # $ % *`. El cursor `">"` de RedPlanet se dibujaría como `¡`. Usar las marcas
que ya emplean `choice_dialog` y el menú de la smoke (`}` … `{`) y evitar esos ocho
caracteres en todas las etiquetas.

### 4.4 `IR A ESCENA` — deliberadamente fuera de la v1

Se pospone a un commit propio: la VM **no tiene un punto único de salida**, y abandonar
una escena a medio construir exige que los ops bloqueantes (`say`, `move`, `combat`)
sepan abortar. La vía más limpia cuando se aborde: un flag que `scene_run()` compruebe
en la cabecera de su bucle de steps y que ejecute la misma rutina que
`SCENE_OP_NEXT_SCENE` (`end_level` + return), de modo que el teardown sea el normal.
Mientras tanto, `HACK_START_SCENE` sigue cubriendo el caso a coste de recompilar.

### Verificación (Fase 4)

- Agente: builds normal/smoke `--no-run` con `DEBUG_MENU_ENABLED` TRUE y FALSE; con
  FALSE, `symbol.txt` sin símbolos `debug_*` y ROM de tamaño ~igual al actual.
- Javier: el código abre el menú **en gameplay y también dentro de un diálogo**; cada
  casilla tiene efecto inmediato; DAR VARA Y HECHIZOS no cuelga el juego (es la trampa
  del sprite); al cerrar, el HUD queda intacto.

---

## Fase 6 — Escenas bajo guion de input (opcional)

Con el override de la Fase 3: guiones de input por escena (lista frame→máscara, en
Python junto al driver) que recorren las escenas `SCENE` de la smoke de forma
desatendida, con `HACK_FAST_DIALOGS` activo y captura por hito. Metería las escenas en
la verificación automática (hoy son el 100 % del playtest manual). No bloquea nada.

## Riesgos de lo que queda

| Riesgo | Mitigación |
|---|---|
| El embudo cambia el timing de los bucles bloqueantes (dialogs) | Sustitución 1:1 sin tocar la lógica de flancos; playtest dirigido a diálogos y choices |
| `X Y X Y` se dispara tocando patrones de notas | Selector de código estilo Red Planet (§4.2); cambiar la secuencia es 1 línea |
| `IR A ESCENA` deja estado sucio | Aterriza vía el mecanismo normal de fin de escena; si no hay punto único de salida en el VM, se pospone esa entrada |
| El menú pisa el HUD de la window | Abrir/cerrar restaura VPos 22 y repinta interface |
