# Plan — Testing automatizado y debug runtime (testing-v2.1)

> ## Estado del plan (cierre 2026-07-13)
>
> **Fases 1-2 COMPLETAS y verificadas end-to-end.** La automatización de la smoke ROM
> está terminada: la suite AUTO (recorrido *walkthrough* scripted idle → walk → cast
> LIGHT → cast THUNDER → combate, con `smoke_phase` como enum de fases y `smoke_scratch`
> para probar `write_ram`) pasa desatendida (`RESULT: ALL PASS`) tanto a mano como por
> `tools/retroarch/mcp_driver.py`, que ejercita **16/16 tools** del MCP y da exit 0/1.
> Verificado en RetroArch + Genesis Plus GX (`DISPLAY=:0`) en corridas repetidas.
> Código real: `src/smoke/smoke_runner.c` (`run_auto`), `src/smoke/smoke_main.c`,
> `tools/retroarch/mcp_driver.py`. Doc de uso: `docs/retroarch-mcp.md` §9-10.
>
> Endurecimiento del driver (2026-07-13): handshake del gate determinista tras espera de
> arranque en frío (`wait_running`, vence el `frame_counter` congelado por foco/
> `pause_nonactive`) y capturas anti-stale (mtime > comando). Ambos verificados 3/3 runs.
>
> **Fases 3 en adelante: TRABAJO FUTURO** (aún sin implementar). Fase 3 (embudo
> `pad_read` + override de input por RAM) mete las escenas en la verificación
> desatendida; Fase 4 (menú de debug runtime + hacks en caliente) cablea de paso los 6
> toggles muertos de `hack.h`; Fases 5-6, cierre y escenas bajo guion. Las secciones de
> abajo son el **diseño de referencia** de esas fases pendientes (y el histórico de 1-2).

Port a The Weave de la infraestructura de la Fase 4 del refactor de Red Planet
(`../RedPlanet_MD`, v2.0-refactor): smoke ROM desatendida con marcadores RAM,
driver MCP de RetroArch, seam de input override y menú de debug runtime.

- **Referencia de código**: `../RedPlanet_MD` — `src/smoke/`, `src/debug/`,
  `src/core/input.c`, `tools/retroarch/mcp_driver.py`, `docs/retroarch-mcp.md`.
- **Convenciones**: fases completas, un commit por hito
  `Testing Fase N: <resumen>`, verificación al cierre de cada fase antes de
  pasar a la siguiente.
- **Reparto de verificación**: los builds (`--no-run`), greps y comprobaciones de
  `symbol.txt` los hace el agente; todo lo que requiere emulador con GUI
  (BlastEm, RetroArch) lo ejecuta Javier y reporta el resultado.

## Estado (2026-07-12) — histórico del diseño original (table-driven de 13 casos)

> Superado por el walkthrough de `origin/master` (ver el cierre arriba). Se conserva
> por los fixes documentados, que siguen siendo válidos.

Verificado en RetroArch + Genesis Plus GX con vídeo real (DISPLAY=:0):
`mcp_driver.py --auto` recorría los 13 casos y daba **RESULT: ALL PASS
(passed=13 failed=0)**, con la pantalla-resumen legible en captura. Tres fixes
por el camino:

- **Bug real de la ROM (address error)**: `exec_cast` no ponía
  `player_has_rod = true` antes de `init_character(CHR_linus)`, así que cargaba
  `linus_norod` (2 animaciones) en vez de `linus_sprite` (6). Al castear,
  `SPR_setAnim(spr, ANIM_MAGIC=3)` accedía a una animación inexistente →
  `a1=FFFFFFFF` → address error en `SPR_update` (PC 0x0191D8). Fix: poner la
  vara antes de crear el sprite, como hacen `forest.c`/`test.c`. Se manifestaba
  como crash en Genesis Plus GX/hardware; BlastEm lo toleraba (por eso "en
  interactivo iba perfecto").
- **Falso positivo del driver**: `run_auto` daba `ALL PASS` con contadores
  parciales si el AUTO no terminaba. Ahora exige `smoke_phase == 0xFFFF`; si no,
  `TIMEOUT` + exit 2. (Mismo bug en el driver de RedPlanet: portar el fix.)
- **Texto invisible tras fade**: el último `end_level` deja la paleta a negro
  (`PAL_fadeOutAll`) y la pantalla-resumen salía en negro. `smoke_reset_view`
  ahora restaura la paleta de texto (PAL0, glifo blanco).

Pendiente: commits `Testing Fase 1` y `Testing Fase 2` (a la espera de luz verde).

## Estado actual vs objetivo

| Pieza | Hoy en The Weave | Objetivo (modelo Red Planet) |
|---|---|---|
| Smoke ROM | Menú interactivo; cada caso pinta PASS/FAIL y espera botón A | Fila AUTO que encadena CHECK+CAST sin input, con autoarranque por timeout |
| Resultado legible por máquina | No hay | 4 marcadores RAM (`smoke_phase/passed/failed/gate`) leídos por NCI |
| Driver de emulador | No hay | `tools/retroarch/mcp_driver.py` (UDP :55355, capturas, exit code) |
| Lectura de pad | `JOY_readJoypad` disperso en 8 archivos (23 sitios) | Embudo `pad_read()` + override por RAM (`g_inputOverride*`) |
| Hacks de desarrollo | `hack.h` compile-time; **6 de 8 toggles declarados pero sin consultar** | `g_hacks` runtime + menú oculto (X Y X Y) sobre Window plane |

Hallazgo previo (2026-07-12): `HACK_ALL_SPELLS`, `HACK_PLAYER_INVULNERABLE`,
`HACK_ENEMIES_ONE_HP`, `HACK_FAST_DIALOGS`, `HACK_MUTE_MUSIC` y `HACK_MUTE_SFX`
no se consultan en ninguna parte de `src/` — solo `HACK_START_SCENE` y
`HACK_FORCE_LANGUAGE` están cableados (`src/core/main.c`). La Fase 4 incluye
implementar esos puntos de consulta, no solo el menú.

## Orden y dependencias

```
Fase 1 (AUTO + marcadores)  ──►  Fase 2 (driver MCP)
Fase 3 (embudo input + override)  — independiente de 1 y 2
Fase 4 (dominio debug/)  — necesita el embudo de la Fase 3 (detector de código)
Fase 5 (docs + cierre)   — al final
Fase 6 (escenas bajo guion) — opcional, necesita 2 + 3
```

Las Fases 1+2 son el tramo de mayor rentabilidad/esfuerzo y no tocan el juego
real (todo bajo `HACK_SMOKE_BUILD` o en `tools/`).

---

## Fase 1 — Caso AUTO y marcadores RAM en la smoke ROM

Objetivo: que la smoke ROM pueda ejecutar todos los casos automatizables sin
tocar el mando y publique el resultado en RAM.

### 1.1 Separar ejecución de presentación en el runner

Hoy `run_check`/`run_cast` (`src/smoke/smoke_runner.c`) calculan **y** pintan.
Dividir:

- `SmokeResult smoke_exec_case(const SmokeCase *c)` — ejecuta y devuelve
  `SMOKE_PASS/FAIL/NA` sin tocar la pantalla (los `VDP_drawText` salen de ahí).
  CHECK y CAST devuelven PASS/FAIL como hasta ahora; SCENE devuelve `SMOKE_NA`
  (interactiva, no participa en AUTO).
- `smoke_run_case()` queda como envoltorio: ejecuta + pinta pantalla de
  resultado (modo menú, comportamiento actual intacto).
- Añadir `SmokeResult` (enum `SMOKE_FAIL=0, SMOKE_PASS=1, SMOKE_NA=2`) a
  `smoke_cases.h`, y trazar los FAIL por KDebug con nombre de caso y detalle
  (equivalente al `SMOKE_EXPECT` de RP; nuestro modelo es data-driven, no de
  punteros a función, así que basta un `kprintf` en el punto del veredicto).

### 1.2 Marcadores RAM (contrato con el driver)

En `smoke_runner.c`, mismos nombres que RP para que el driver sea intercambiable:

```c
volatile u16 smoke_phase;    // índice del caso en curso; 0xFFFF = AUTO terminado
volatile u16 smoke_passed;   // PASS acumulados
volatile u16 smoke_failed;   // FAIL acumulados (0 al final = ALL PASS)
volatile u16 smoke_gate;     // el driver la escribe ≠0 para pausar entre casos
```

No-static y `volatile`: deben aparecer en `out/symbol.txt` y releerse en cada
iteración. El bucle `while (smoke_gate) SYS_doVBlankProcess();` entre casos
cumple doble función: pausa pedida por el driver y mantener el símbolo vivo
frente a `--gc-sections`.

### 1.3 Caso AUTO

- Nuevo `SmokeKind` `SMOKE_AUTO`, fila 0 de `smoke_cases[]` (`"AUTO TODOS"`).
- `smoke_run_auto()`: recorre la tabla ejecutando CHECK y CAST (SCENE se marca
  N/A), actualiza `smoke_phase` **antes** de cada caso (el driver captura
  pantalla por fase), acumula `smoke_passed/failed`, guarda veredicto por caso
  en `s_autoResult[]`.
- Al terminar: `smoke_phase = 0xFFFF` y pantalla-resumen con una línea por caso
  (PASS/FAIL), total `PASSED n FAILED m` y `RESULT: ALL PASS` / `RESULT: FAIL`
  (el driver espera esos literales solo en RAM, pero la pantalla sirve para el
  playtest a ojo y las capturas).
- 13 de los 18 casos actuales entran en AUTO: 7 CHECK + 6 CAST (ya son
  scripted y miden frames sin input). Los 5 SCENE quedan fuera (Fase 6).

### 1.4 Autoarranque desatendido

Portar el `menu_wait` con timeout de RP (`smoke_menu.c`): ~180 frames (3 s) sin
pulsación en el menú → ejecuta la fila AUTO. Así el flujo
"arrancar ROM y no tocar nada" produce el resultado completo sin intervención.
El menú sigue leyendo el pad físico directamente (`JOY_readJoypad`), **nunca**
el override — lección de RP: si un caso deja el override activo, el menú debe
seguir siendo navegable.

### 1.5 Higiene de pantalla entre casos

Portar `smoke_reset_view()` de RP con sus dos lecciones VDP:

- Ocultar sprites de verdad = `VDP_clearSprites()` **+**
  `VDP_updateSprites(0, DMA_QUEUE)` (lo primero solo toca la caché).
- Resetear el scroll de BG_A/BG_B tras cada CAST (nuestro `new_level` con
  `BG_SCRL_AUTO_RIGHT` desplaza el plano; sin reset, `VDP_drawText` sale
  descuadrado — RP se comió ese bug en su Fase 4).

Verificar además que la secuencia `new_level → cast → end_level` de `run_cast`
deja estado limpio al encadenarse 6 veces seguidas (paletas, `movement_active`,
personaje). Si un CAST encadenado se comporta distinto que suelto, es fuga de
estado del caso anterior: arreglar el teardown, no el caso.

### Archivos y verificación (Fase 1)

| Archivo | Cambio |
|---|---|
| `src/smoke/smoke_cases.h` | `SmokeResult`, `SMOKE_AUTO`, fila AUTO |
| `src/smoke/smoke_runner.c/.h` | split exec/presentación, marcadores, `smoke_run_auto`, `smoke_reset_view` |
| `src/smoke/smoke_main.c` | timeout de autoarranque en el menú |

- Agente: `./build-theweave.sh smoke --no-run` compila;
  `grep -E "smoke_(phase|passed|failed|gate)" out/symbol.txt` → 4 símbolos.
- Javier (BlastEm): dejar la ROM 3 s → AUTO encadena los 13 casos y termina en
  `ALL PASS`; los casos sueltos desde el menú siguen funcionando igual;
  KDebug traza cada caso.

Commit: `Testing Fase 1: caso AUTO + marcadores RAM en la smoke ROM`.

---

## Fase 2 — Driver MCP de RetroArch

Objetivo: correr el AUTO desatendido desde el host y obtener exit code +
informe + capturas, sin humano mirando la pantalla.

### 2.1 Portar el driver

Copiar `../RedPlanet_MD/tools/retroarch/mcp_driver.py` → `tools/retroarch/`.
Es casi agnóstico del juego; cambios reales:

- Textos de ayuda/errores: `build-RedPlanet.sh smoke` → `./build-theweave.sh smoke`.
- Rutas: `out/smoke.bin`, `out/symbol.txt`, salida a `docs/testing/smoke-latest/`
  — idénticas en estructura, no cambian.
- Marcadores: mismos nombres (decisión de la Fase 1), no cambian.

Lo duro ya viene resuelto de RP: Genesis Plus GX no expone mapa de memoria
(`READ_CORE_MEMORY` responde `-1`), así que usa `READ_CORE_RAM`/`WRITE_CORE_RAM`
con offset `símbolo & 0xFFFF` dentro de la work RAM, que además va
**byte-swapped** (`u16` en offset par O = `(b1 << 8) | b0`).

### 2.2 Blindar la pareja smoke.bin/symbol.txt (mejora sobre RP)

En RP, un build normal entre el build smoke y la ejecución del driver
desincroniza `symbol.txt` de `smoke.bin` (documentado como "Ojo" en sus docs).
Mejorarlo aquí: `build-theweave.sh smoke` copia también
`out/symbol.txt → out/smoke-symbol.txt`, y el driver usa `smoke-symbol.txt` si
existe. La pareja queda atómica.

### 2.3 Housekeeping

- `.gitignore`: `tools/retroarch/__pycache__/`, `docs/testing/smoke-latest/`.
- `docs/retroarch-mcp.md` adaptado de RP (requisitos: RetroArch + core Genesis
  Plus GX, `network_cmd_enable = "true"` en `retroarch.cfg` editado con
  RetroArch cerrado; uso: `--auto`, `--launch`, `--watch`; contrato de
  marcadores; direccionamiento RAM).
- Aprovechar para retirar el comentario obsoleto del `Makefile`
  ("Fase 7 añadirá el target smoke" — el modo smoke ya existe vía
  `EXTRA_FLAGS` en el build script).

### Verificación (Fase 2)

- Agente: `python3 tools/retroarch/mcp_driver.py --help` y carga de símbolos en
  seco (`load_symbols()` sobre `out/smoke-symbol.txt`).
- Javier: RetroArch + `out/smoke.bin`, luego
  `python3 tools/retroarch/mcp_driver.py --auto` → exit 0,
  `RESULT: ALL PASS (passed=13 failed=0)`, capturas por fase en
  `docs/testing/smoke-latest/`. (El driver es un cliente UDP sin GUI: una vez
  RetroArch está abierto, puede lanzarlo cualquiera de los dos.)

Commit: `Testing Fase 2: driver MCP RetroArch + docs (smoke desatendida)`.

---

## Fase 3 — Embudo de input y override por RAM

Objetivo: una única función lee el pad en todo el juego; con dos símbolos en
RAM se puede pilotar la ROM real desde fuera (y desde dentro, para futuros SIM).

### 3.1 El seam

Nuevo `src/core/input.c/.h` (se mantiene `controller.c` con la semántica de
gameplay; `input.c` es solo la costura con el hardware):

```c
volatile u16 g_inputOverride;        // máscara BUTTON_* inyectada
volatile u16 g_inputOverrideActive;  // ≠0 → ignorar el mando físico

u16  pad_read(void);                 // JOY_readJoypad(JOY_ALL) u override
void input_setOverride(u16 buttons);
void input_clearOverride(void);
```

No-static (localizables en `symbol.txt`, escribibles por `WRITE_CORE_RAM`).
Añadir `input.h` a la metalibrería `core/core.h`.

### 3.2 Sustitución mecánica de los 23 sitios

| Archivo | Sitios | Nota |
|---|---|---|
| `src/core/controller.c` | 2 | `joy_check` y el bucle de release de `handle_pause_button` |
| `src/narrative/dialogs.c` | 12 | bucles bloqueantes de avance/choices — es el archivo que más gana |
| `src/interface/interface.c` | 4 | pausa/inventario |
| `src/scenes/intro.c` | 2 | selección de idioma |
| `src/scenes/scene_vm.c` | 2 | esperas de botón A del DSL |
| `src/smoke/smoke_main.c` | 2 | **NO se toca**: el menú smoke lee el pad físico a propósito (§1.4) |

Sustitución 1:1 (`JOY_readJoypad(JOY_ALL|JOY_1)` → `pad_read()`); la lógica de
flancos (`joy_state`/`prev_joy_state`) se queda en los llamadores tal cual.
Unificamos de paso la inconsistencia JOY_ALL/JOY_1 (todo pasa a JOY_ALL, que es
lo que ya usa el 80% del código).

### 3.3 Patrón de pulso para los bucles bloqueantes

Los llamadores esperan flancos (release → press). El pilotaje externo debe
emular pulsaciones humanas: escribir la máscara N frames, luego 0 M frames.
Documentar la receta en `docs/retroarch-mcp.md` (sección "pilotar el juego
real"), con el ejemplo de RP adaptado:

```python
ra.write_u16(syms["g_inputOverride"], 0x0040)      # BUTTON_A
ra.write_u16(syms["g_inputOverrideActive"], 1)
# ... ~10 frames ...
ra.write_u16(syms["g_inputOverride"], 0)           # release (¡antes de soltar el override!)
```

### 3.4 Caso smoke nuevo que ejercita el seam (sin emulador)

Añadir un caso `SIM patron notas`: usa `input_setOverride` para inyectar la
secuencia de botones de un patrón de 4 notas (A/B/C/X → MI FA SOL LA…) frame a
frame contra `spell_note_input`/`spell_input_update`, y comprueba que el
hechizo se reconoce. Valida el override desde dentro de la ROM (entra en AUTO)
y de paso cubre la detección de patrones, hoy sin test.

### Verificación (Fase 3)

- Agente: build normal y smoke `--no-run`; `grep -rn JOY_readJoypad src/` → solo
  `input.c` y `smoke_main.c`; `grep g_inputOverride out/symbol.txt` → 2 símbolos.
- Javier: playtest corto del checklist (`docs/testing.md`) — movimiento,
  diálogo con choice, pausa, un combate: idéntico a antes. Con RetroArch:
  receta de §3.3 avanza un diálogo sin tocar el mando.

Commit: `Testing Fase 3: embudo pad_read + input override por RAM (D6)`.

---

## Fase 4 — Dominio `src/debug/`: hacks runtime y menú oculto

Objetivo: los toggles de `hack.h` pasan a ser estado RAM modificable en caliente
desde un menú oculto; con `DEBUG_MENU_ENABLED FALSE` la ROM queda idéntica a hoy.

### 4.1 Cablear los hacks muertos (prerequisito)

Los 6 toggles sin consultar necesitan su punto de consulta, ya directamente
sobre `g_hacks` (un `if` cada uno):

| Hack | Punto de consulta |
|---|---|
| `allSpells` | inicialización del estado de vara/hechizos del jugador (`src/spells/`) + acción de menú `debug_give_spells()` para aplicarlo en caliente |
| `invulnerable` | punto de daño al jugador (`src/combat/`) |
| `enemiesOneHp` | aplicación de daño a enemigos (`src/combat/`) |
| `fastDialogs` | temporización de avance en `src/narrative/dialogs.c` |
| `muteMusic` / `muteSfx` | early-return en `play_music` / `play_sfx` y jingles (`src/audio/sound.c`) |

`HACK_START_SCENE` y `HACK_FORCE_LANGUAGE` se quedan compile-time (solo tienen
sentido en el arranque).

### 4.2 `debug_hacks.c/.h` — modelo de RP tal cual

- `struct DebugHacks` con los 6 campos bool; `g_hacks` global.
- `debug_hacks_init()` carga los defaults de `hack.h`; se llama **una** vez en
  ambos mains (`src/core/main.c` y `src/smoke/smoke_main.c`), no en cada
  escena: los hacks sobreviven a transiciones.
- Con `DEBUG_MENU_ENABLED FALSE`: `g_hacks` se define como literal compuesto
  `const` con los defaults → el compilador pliega los `if (g_hacks.x)` y
  elimina el código muerto; `src/debug/` entero queda fuera del binario
  (guards `#if DEBUG_MENU_ENABLED` en los `.c`).

### 4.3 `debug_code.c/.h` — detector de la secuencia

- Código corto de desarrollo: `X Y X Y` (just-pressed, timeout ~1 s), selector
  `DEBUG_MENU_CODE_SIMPLE` como en RP para cambiar a un código "de producción".
  X/Y solo tocan notas musicales; evaluar en el playtest si X Y X Y se dispara
  tocando patrones — si molesta, cambiar a secuencia con START (inerte fuera
  de pausa).
- Alimentación: al final de `joy_check()` (`src/core/controller.c`), con estado
  prev/current propio del detector. Limitación aceptada v1: el menú no se abre
  dentro de diálogos ni pausa (sus bucles no pasan por `joy_check`).

### 4.4 `debug_menu.c/.h` — UI sobre el Window plane

Particularidad de The Weave que RP no tenía: la window ya es del HUD/diálogos
(VPos 22, filas 22–25). El menú, al abrir, sube la window a pantalla completa
(`VDP_setWindowVPos(TRUE, alto)`) y congela el juego (como RP: solo corre
VBlank + música); al cerrar restaura `VDP_setWindowVPos(TRUE, 22)`, limpia el
plano y repinta el HUD (`interface`).

Entradas del menú:

1. Toggles SI/NO de los 6 hacks (§4.1).
2. `DAR VARA + HECHIZOS` — acción inmediata (`debug_give_spells`).
3. `IR A ESCENA >` — submenú con `scenes[].name`; fija `current_scene_id` y
   levanta un flag `scene_abort_requested` que el bucle del `scene_vm` chequea
   por opcode para retornar limpio (mismo mecanismo que el opcode de fin de
   escena, `scene_vm.c:194-196`). **Riesgo señalado**: el estado a medio
   construir de la escena abandonada (personajes, nivel, paleta) debe pasar por
   el teardown normal; si el VM no tiene un punto único de salida, esta entrada
   se pospone a un commit propio dentro de la fase.
4. `CERRAR` (también con START).

### Verificación (Fase 4)

- Agente: builds normal/smoke `--no-run` con `DEBUG_MENU_ENABLED` TRUE y FALSE;
  con FALSE, `nm`/`symbol.txt` sin símbolos `debug_*` y tamaño de ROM ~igual al
  actual.
- Javier: XYXY abre el menú en gameplay; cada toggle tiene efecto inmediato
  (invulnerable en combate, mute en el acto, fastDialogs en el siguiente
  diálogo); IR A ESCENA aterriza limpio; cerrar restaura HUD intacto.

Commits: `Testing Fase 4: hacks runtime (g_hacks) + puntos de consulta` y
`Testing Fase 4: menú de debug oculto sobre Window plane`.

---

## Fase 5 — Documentación y cierre

- `docs/testing.md`: reescribir — AUTO + autoarranque, marcadores, driver MCP
  (con puntero a `docs/retroarch-mcp.md`), receta de override, menú de debug.
- `docs/build.md`: modo smoke + `smoke-symbol.txt`.
- `AGENTS.md`: sección de testing (comandos del agente vs de Javier).
- Barrido final: `grep JOY_readJoypad`, comentario del Makefile, `.gitignore`.
- Verificación integral: smoke AUTO ALL PASS por driver + playtest del
  checklist completo de `docs/testing.md` + una pasada con
  `DEBUG_MENU_ENABLED FALSE`.

Commit: `Testing Fase 5: documentación y cierre (testing-v2.1)`; tag opcional
`v2.1-testing`.

---

## Fase 6 (opcional, futuro) — Escenas bajo guion

Con driver (F2) + override (F3): guiones de input por escena (lista
frame→máscara, en Python junto al driver) que recorren las 5 SCENE del smoke de
forma desatendida, con `HACK_FAST_DIALOGS` activo y captura por hito. Metería
las escenas en la verificación automática (hoy son el 100% del playtest
manual). No bloquea nada; decidir tras rodar F1–F5.

## Riesgos

| Riesgo | Mitigación |
|---|---|
| CAST encadenados en AUTO se contaminan entre sí (fuga de estado de `new_level`/`end_level`) | §1.5: arreglar teardown; AUTO ejecuta exactamente el mismo código que el caso suelto |
| `symbol.txt` desincronizado de `smoke.bin` | §2.2: `smoke-symbol.txt` atómico en el build |
| El embudo cambia timing de bucles bloqueantes (dialogs) | Sustitución 1:1 sin tocar la lógica de flancos; playtest dirigido a diálogos y choices en F3 |
| X Y X Y se dispara tocando patrones de notas | Selector de código estilo RP (§4.3); cambiar la secuencia es 1 línea |
| `IR A ESCENA` deja estado sucio | Aterriza vía el mecanismo normal de fin de escena; si no hay punto único de salida en el VM, se pospone esa entrada (§4.4) |
| El menú pisa el HUD de la window | Abrir/cerrar restaura VPos 22 y repinta interface (§4.4) |

## Resumen de archivos

| Fase | Nuevos | Modificados |
|---|---|---|
| 1 | — | `src/smoke/{smoke_cases.h, smoke_runner.c/.h, smoke_main.c}` |
| 2 | `tools/retroarch/mcp_driver.py`, `docs/retroarch-mcp.md` | `build-theweave.sh`, `.gitignore`, `Makefile` (comentario) |
| 3 | `src/core/input.c/.h` | `core/core.h`, `controller.c`, `dialogs.c`, `interface.c`, `intro.c`, `scene_vm.c`, `smoke_cases.h` (caso notas) |
| 4 | `src/debug/{debug.h, debug_hacks.c/.h, debug_code.c/.h, debug_menu.c/.h, constants_debug.h}` | `core/hack.h`, `main.c`, `smoke_main.c`, `controller.c`, y los 6 puntos de consulta (§4.1) |
| 5 | — | `docs/testing.md`, `docs/build.md`, `AGENTS.md` |
