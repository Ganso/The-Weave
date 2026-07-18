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
Fase 3 (embudo input + override)  — independiente; habilita 6
Fase 4 (menú de debug runtime)    — independiente
Fase 6 (escenas bajo guion)       — necesita la Fase 3
```

Las tres son independientes entre sí salvo que la Fase 6 necesita el override de la
Fase 3. La Fase 3 es la de mayor rentabilidad: mete las **escenas** en la verificación
automática, hoy 100 % playtest manual.

---

## Fase 3 — Embudo de input y override por RAM

Objetivo: que una **única** función lea el pad en todo el juego y que, con dos símbolos
en RAM, se pueda pilotar la ROM real desde fuera (RetroArch) o desde dentro (casos SIM
de la smoke). Hoy hay ~23 `JOY_readJoypad` dispersos (`grep -rn JOY_readJoypad src/`).

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

- Código de desarrollo `X Y X Y` (just-pressed, timeout ~1 s), con selector
  `DEBUG_MENU_CODE_SIMPLE` para cambiar a un código "de producción". X/Y solo tocan
  notas musicales; en el playtest, evaluar si se dispara tocando patrones — si molesta,
  pasar a una secuencia con START (inerte fuera de pausa).
- Alimentación: al final de `joy_check()` (`src/core/controller.c`). Limitación
  aceptada v1: el menú no se abre dentro de diálogos ni pausa (sus bucles no pasan por
  `joy_check`).

### 4.3 `debug_menu.c/.h` — UI sobre el Window plane

La window ya es del HUD/diálogos (VPos 22). El menú, al abrir, la sube a pantalla
completa (`VDP_setWindowVPos(TRUE, alto)`) y congela el juego (solo VBlank + música); al
cerrar restaura VPos 22, limpia el plano y repinta el HUD. Entradas:

1. Toggles SI/NO de los hacks de gameplay.
2. `DAR VARA + HECHIZOS` — acción inmediata (`debug_give_spells`).
3. `IR A ESCENA >` — submenú con `scenes[].name`; fija `current_scene_id` y levanta un
   flag que el bucle del `scene_vm` chequea por opcode para retornar limpio. **Riesgo**:
   el estado a medio construir de la escena abandonada debe pasar por el teardown
   normal; si el VM no tiene un punto único de salida, esta entrada se pospone a un
   commit propio.
4. `CERRAR` (también con START).

### Verificación (Fase 4)

- Agente: builds normal/smoke `--no-run` con `DEBUG_MENU_ENABLED` TRUE y FALSE; con
  FALSE, `symbol.txt` sin símbolos `debug_*` y ROM de tamaño ~igual al actual.
- Javier: XYXY abre el menú en gameplay; cada toggle tiene efecto inmediato; IR A
  ESCENA aterriza limpio; cerrar restaura el HUD intacto.

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
