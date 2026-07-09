# Depuración desatendida vía RetroArch + mcp-retroarch

Este documento describe el sistema que permite a un agente de IA (o a una persona)
**depurar la ROM de The Weave sin intervención manual**: arrancarla en un emulador,
leer/escribir su RAM, avanzar frame a frame de forma determinista y sacar capturas
de pantalla que el agente puede *ver*.

Todo se apoya en el **Network Control Interface (NCI)** de RetroArch: un protocolo de
texto sobre **UDP en `127.0.0.1:55355`** que RetroArch expone cuando se habilita. El
paquete [`mcp-retroarch`](https://github.com/dmang-dev/mcp-retroarch) es un servidor
MCP que envuelve ese protocolo y lo ofrece como *tools* a Claude Code. **El MCP no es
imprescindible**: es una capa de comodidad sobre el NCI, y el mismo protocolo se puede
hablar por UDP directo (útil como fallback, ver §5).

> Alcance: esto **no reemplaza** el flujo de depuración del día a día documentado en
> `AGENTS.md` §2 (toggles en `src/core/hack.h`, `dprintf`/KDebug, BlastEm). Lo
> complementa con inspección de memoria y captura programáticas para verificación
> automatizada.

---

## 1. Qué nos abre (capacidades y límites)

| Capacidad | Estado | Notas |
|---|---|---|
| Leer RAM (WRAM del 68K) | ✅ | `READ_CORE_RAM`. **Ojo al byte-swap**, §4. |
| Escribir RAM | ✅ | `WRITE_CORE_RAM` (inyectar estado para reproducir un bug). |
| Estado / info del juego | ✅ | `GET_STATUS` → PLAYING/PAUSED, sistema, CRC32. |
| Pausa / frame-advance | ✅ | `FRAMEADVANCE` avanza **exactamente 1 frame** (determinista). |
| Reset | ✅ | Hard-reset de la ROM. |
| Captura de pantalla | ✅ | PNG a la carpeta de screenshots (§6). El agente puede leerlo. |
| Save/load state | ✅ | Preparar un punto reproducible y volver a él. |
| **Input de gamepad** | ❌ | **El NCI no lo expone.** No se pueden "pulsar botones". §7. |

**Qué desbloquea en la práctica:** verificar invariantes leyendo una variable en RAM
tras un evento, medir duraciones en frames, comprobar a ojo (captura) que un efecto se
ve bien, inyectar estado para reproducir un cuelgue, y hacer todo esto de forma
scripteada/repetible sin tocar el mando.

---

## 2. La configuración que usamos (máquina de referencia)

1. **RetroArch** instalado, con el core **`genesis_plus_gx`**:
   `/usr/lib/x86_64-linux-gnu/libretro/genesis_plus_gx_libretro.so`.
2. **NCI habilitado** en `~/.config/retroarch/retroarch.cfg`:
   ```ini
   network_cmd_enable = "true"
   network_cmd_port   = "55355"
   ```
   (o por GUI: Settings → Network → Network Commands → ON).
3. **`mcp-retroarch`** instalado global (`npm install -g mcp-retroarch`) y **registrado
   en Claude Code a nivel usuario**:
   ```bash
   claude mcp add retroarch --scope user mcp-retroarch
   ```
   Comprobar: `claude mcp list` → `retroarch: ... ✔ Connected`.

> ⚠️ **Las tools MCP se cargan al ARRANCAR la sesión de Claude Code.** Si registras el
> MCP a mitad de una sesión, las tools `retroarch_*` **no** aparecerán hasta reiniciar
> Claude Code. Mientras tanto, usa el fallback UDP directo (§5) — es el mismo protocolo.

---

## 3. Detección automática (¿está este sistema disponible aquí?)

Un agente en una máquina nueva debería ejecutar estas comprobaciones **antes** de
ofrecer este flujo. Si todas pasan, el sistema está listo; si falta alguna, ver
"Arreglo" o proponer instalarla.

```bash
# a) Emulador y core Genesis
command -v retroarch && ls /usr/lib/x86_64-linux-gnu/libretro/genesis_plus_gx_libretro.so
# b) NCI habilitado
grep -E 'network_cmd_enable' ~/.config/retroarch/retroarch.cfg
# c) MCP instalado y registrado
command -v mcp-retroarch ; claude mcp list 2>/dev/null | grep -i retroarch
```

| Falta | Arreglo |
|---|---|
| `retroarch` / core | Instalar RetroArch + core `genesis_plus_gx` (por GUI Online Updater o el paquete de la distro). |
| `network_cmd_enable` | Añadir las dos líneas del §2.2 al `retroarch.cfg` (o por GUI). |
| `mcp-retroarch` | `npm install -g mcp-retroarch` (Node ≥ 22). |
| No registrado en Claude | `claude mcp add retroarch --scope user mcp-retroarch`, y **reiniciar** la sesión. |

Si el sistema **no** está y no se puede instalar, cae al flujo BlastEm de `AGENTS.md` §2.

---

## 4. Leer la RAM: el byte-swap de Genesis (imprescindible)

`genesis_plus_gx` expone la **WRAM del 68000** (`0xFF0000`–`0xFFFFFF`, 64 KB) al NCI a
través del address space de CHEEVOS, con **offset = `addr & 0xFFFF`**. **Pero los bytes
salen intercambiados por palabra de 16 bits.** El 68K es big-endian; el core entrega la
RAM en orden de palabra nativo del host.

- Para reconstruir un valor big-endian real de N bytes leídos `b0 b1 b2 b3 …`,
  **intercambia dentro de cada par**: el valor real es `b1 b0 b3 b2 …`.
- Ejemplo verificado: una global `u32 = 0xCAFEBABE` en `0xFF0000` se lee como
  `FE CA BE BA`.

**Localizar la dirección de una variable** (para saber qué offset leer): tras compilar,
```bash
grep <nombre_global> out/symbol.txt      # p.ej.  e0ff006e b g_frame
```
Aparece como `e0ffXXXX` porque la WRAM del 68K se refleja también sobre `0xE0xxxx`; el
**offset NCI es `XXXX`** (los 16 bits bajos). Solo sirven **globales/estáticas** (viven
en WRAM); las locales están en la pila y no tienen dirección fija.

> ⚠️ **`--gc-sections` descarta globales `volatile` que no se referencien en código.**
> Si quieres inspeccionar una variable centinela, asegúrate de que algo la lee/escribe,
> o el linker la elimina y no aparecerá en `symbol.txt`.

Nota: el README de mcp-retroarch marca la RAM de Genesis como "sparse". En la práctica,
los offsets de WRAM que usa el juego se leen bien; algunos rangos altos pueden fallar
con "no error message". Si una lectura falla, reintenta (el NCI puede perder datagramas
UDP bajo carga) o prueba otro offset.

---

## 5. Fallback: hablar el NCI por UDP directo

Cuando las tools MCP no estén cargadas (sesión sin reiniciar) o quieras un script,
manda comandos NCI por UDP. Snippet de referencia (Python, sin dependencias):

```python
import socket
def nci(cmd, timeout=1.0):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.settimeout(timeout)
    s.sendto(cmd.encode(), ("127.0.0.1", 55355))
    try: return s.recvfrom(4096)[0].decode(errors="replace").strip()
    except socket.timeout: return "<timeout>"

nci("VERSION")                 # -> 1.20.0
nci("GET_STATUS")              # -> GET_STATUS PLAYING mega_drive,rom,crc32=...
nci("READ_CORE_RAM 6e 4")      # -> READ_CORE_RAM 6e b0 b1 b2 b3   (recuerda el swap, §4)
nci("WRITE_CORE_RAM 6e 00 00") # escribir
nci("PAUSE_TOGGLE")            # pausar / reanudar
nci("FRAMEADVANCE")            # +1 frame (solo con efecto en pausa)
nci("SCREENSHOT")              # captura (a veces no responde; localiza el PNG, §6)
nci("RESET")
```

Las tools MCP equivalentes (cuando estén cargadas) son `retroarch_ping`,
`retroarch_get_status`, `retroarch_read_ram` / `retroarch_write_ram`,
`retroarch_pause_toggle`, `retroarch_frame_advance`, `retroarch_reset`,
`retroarch_screenshot`, `retroarch_save_state_current` / `retroarch_load_state_*`.

---

## 6. Lanzar la ROM y capturar pantalla

```bash
# Compilar como siempre (o usar out/rom.bin ya existente):
./build-theweave.sh --no-run          # deja out/rom.bin sin lanzar emulador

# Arrancar RetroArch con el core Genesis y la ROM (necesita display: DISPLAY=:0):
DISPLAY=:0 retroarch -L /usr/lib/x86_64-linux-gnu/libretro/genesis_plus_gx_libretro.so out/rom.bin
```

- Lánzalo **en segundo plano de forma persistente** (en Claude Code: `run_in_background`
  del tool Bash). Un `nohup ... &` disparado desde una tool **no** sobrevive al turno.
- **Capturas:** van a la carpeta de screenshots de RetroArch (por defecto
  `~/.config/retroarch/screenshots`, ver `screenshot_directory` en el cfg). `SCREENSHOT`
  no siempre contesta por UDP; localiza el PNG más reciente y léelo:
  ```bash
  ls -t ~/.config/retroarch/screenshots/*.png | head -1
  ```

---

## 7. Límite clave: sin input de gamepad

El NCI **no** puede pulsar botones. Para pruebas que requieran interacción:

- **Smoke ROM** (`docs/testing.md`): scriptea el caso dentro de la ROM
  (`-DHACK_SMOKE_BUILD`) — es la vía natural para ejercitar hechizos/escenas sin mando.
- **Toggles de arranque** (`src/core/hack.h`, `AGENTS.md` §2): `HACK_START_SCENE` para
  entrar directo a una escena, `HACK_ALL_SPELLS`, etc.
- **Save states**: prepara un estado manualmente una vez y recárgalo con `load_state`.
- Para juegos que *sí* necesiten input programático hay `mcp-mgba` (GBA), fuera de este
  proyecto.

---

## 8. Receta completa (verificación desatendida de un valor)

1. Compilar (`./build-theweave.sh --no-run`) y anotar el offset de la variable de
   interés en `out/symbol.txt` (§4).
2. Lanzar RetroArch en background con la ROM (§6).
3. `GET_STATUS` para confirmar que corre; llegar al estado a comprobar (toggle de
   arranque, o save state, ya que no hay input).
4. `PAUSE_TOGGLE` para congelar; `READ_CORE_RAM <offset> <n>` y **deswapear** (§4).
   Comparar con el valor esperado → PASS/FAIL.
5. Opcional: `FRAMEADVANCE` en bucle para medir duración en frames, y `SCREENSHOT` +
   leer el PNG para juzgar el efecto visual.
6. Cerrar RetroArch. **Usa `pkill -x retroarch`** (nombre exacto), no
   `pkill -f retroarch`: con `-f` el patrón casa con la propia línea de comando que lo
   invoca y el shell se autotermina (exit 144).

---

## 9. Test desatendido de referencia: suite AUTO de la smoke ROM

La smoke ROM (`docs/testing.md`) trae una opción **AUTO** (fila 0 del menú, y auto-corre
al arrancar si no pulsas A en ~3 s) que sin tocar el manda:

1. ejecuta las **7 invariantes `canUse`** (casos `CHECK`) — cubre la lógica del sistema
   de hechizos;
2. hace un **recorrido de movimiento** por el nivel del bosque: Linus aparece con su
   vara (`player_has_rod`, trampa de AGENTS.md §7), espera en reposo y camina a derecha
   e izquierda (mecánica de movimiento);
3. *(planificado)* **castearía** `SPELL_LIGHT` y `SPELL_THUNDER` y **libraría un combate**
   contra un `WeaverGhost` (counter con trueno invertido). **Hoy está COMENTADO** en
   `run_auto` porque cuelga el sprite engine encadenado tras `new_level`+movimiento
   (§10 TODO 1); se descomenta fase a fase cuando se estabilice.
4. deja una **pantalla de resultados** (`CHECKS N OK M FAIL`, `WALK OK`, `RESULT: ALL PASS`).

Durante el recorrido, la ROM escribe la **fase actual** en la global `smoke_phase`
(WRAM), lo que permite al host **sincronizar capturas leyendo RAM** (read_ram) en vez de
adivinar por tiempo. La global `smoke_scratch` es un buffer libre para probar `write_ram`.
La global `smoke_gate` sirve como **puerta de sincronización (RAM Gate)**: la ROM se
congela al inicio del test AUTO (`PH_WAIT_GATE`, `smoke_phase=0xFFFE`) hasta que el host
escribe un valor no nulo (ej. `01 01`) en ella, garantizando que el emulador esté listo
y no se pierdan trazas o frames. Los offsets de todas estas globales se leen
dinámicamente de `out/symbol.txt` (§4); cambian en cada build, así que léelos de ahí.

> El recorrido se limita al **movimiento** a propósito: castear/combatir encadenados
> fuera de la VM de escenas cuelga/corrompe el sprite engine (crash o hang en
> `SPR_update`; ver §10 y AGENTS.md §7). El bloque de cast/combate está **comentado** en
> `run_auto` (`src/smoke/smoke_runner.c`) pendiente de estabilizar; los casos
> `CAST`/`SCENE` individuales del menú SÍ corren (flujo soportado).

Receta (probada):

```bash
# 1. Compilar la smoke ROM (no lanza emulador)
./build-theweave.sh smoke --no-run          # -> out/smoke.bin

# 2. Lanzar en background (en Claude Code: run_in_background del tool Bash)
DISPLAY=:0 retroarch -L /usr/lib/x86_64-linux-gnu/libretro/genesis_plus_gx_libretro.so out/smoke.bin
```

```bash
# 3. Esperar ~6 s (3 s de ventana + checks) y capturar; localizar y leer el PNG.
#    (En Claude Code, mete la espera en un comando run_in_background: los sleep
#    largos en primer plano están bloqueados.)
python3 -c 'import socket; s=socket.socket(2,2); s.settimeout(1); s.sendto(b"SCREENSHOT",("127.0.0.1",55355))'
ls -t ~/.config/retroarch/screenshots/*.png | head -1     # <- leer este PNG
```

**Validación:** el PNG debe mostrar `RESULT: ALL PASS`, `CHECKS  N OK  0 FAIL` (N = filas
`CHECK` en `src/smoke/smoke_cases.h`, 7 hoy) y `WALK OK`. Si sale `RESULT: FAIL`, lista
debajo los casos fallidos. Cerrar con `pkill -x retroarch`.

### Driver host de referencia (ejercita las 17 tools del MCP)

`tools/retroarch/mcp_driver.py` conduce la ROM y ejercita **todos** los comandos NCI que
expone el MCP (mapeo tool→comando en la tabla de abajo), sincronizando capturas con
`smoke_phase` y probando `write_ram` sobre `smoke_scratch`. Uso:

```bash
DISPLAY=:0 retroarch -L .../genesis_plus_gx_libretro.so out/smoke.bin &   # background
python3 tools/retroarch/mcp_driver.py                                     # lee offsets de out/symbol.txt
```
Deja capturas en el scratchpad (`walk_*.png`) y un informe de cobertura por stdout.

| MCP tool | comando NCI | probado |
|---|---|---|
| ping / get_status / get_config | `VERSION` / `GET_STATUS` / `GET_CONFIG_PARAM <n>` | ✅ |
| read_ram / write_ram | `READ_CORE_RAM 0x<off> <n>` / `WRITE_CORE_RAM 0x<off> <hex>` | ✅ (con byte-swap §4) |
| read_memory / write_memory | `READ_CORE_MEMORY` / `WRITE_CORE_MEMORY` | ✅ reporta `no memory map` (esperado en Genesis) |
| pause_toggle / frame_advance | `PAUSE_TOGGLE` / `FRAMEADVANCE` | ✅ +1 frame determinista (si engancha una fase viva) |
| screenshot / show_message | `SCREENSHOT` / `SHOW_MSG <msg>` | ✅ (show_message es fire-and-forget) |
| save_state / load_state / load_state_slot | `SAVE_STATE` / `LOAD_STATE` / `LOAD_STATE_SLOT <n>` | ✅ fire-and-forget; SAVE se verifica por el dir de estados |
| state_slot_plus / minus / reset | `STATE_SLOT_PLUS` / `_MINUS` / `RESET` | ✅ fire-and-forget |

---

## 10. Estado de la implementación y TODOs

**Funciona (verificado en ejecución):**
- Registro del MCP y NCI (§1-3); read/write RAM con byte-swap (§4); frame-advance
  determinista; screenshots leídas por el agente.
- smoke ROM opción **AUTO**: 7 invariantes `canUse` + recorrido de **movimiento** →
  pantalla `RESULT: ALL PASS`. Estable, sin input (§9).
- **RAM Gate** (`smoke_gate`): la ROM se congela en `PH_WAIT_GATE` hasta que el host
  escribe un valor no nulo, garantizando arranque sincronizado y sin pérdida de frames.
- **Despausado forzado** automático en el driver (`GET_STATUS`; si `PAUSED`, `PAUSE_TOGGLE`).
- **Fixes de estabilidad** en el recorrido: `player_has_rod` antes de `init_character`
  (trampa de la vara) y `1440`/`BG_SCRL_USER_RIGHT` en `new_level` (trampa del scroll),
  aplicados también a `cast_run` (casos CAST del menú). Ambos son correcciones según
  AGENTS.md §7; verificados visualmente (capturas del test).
- Driver host: **16/16 tools** del MCP ejercitadas en una pasada; capturas sincronizadas
  por `smoke_phase` (read_ram); `write_ram` confirmado (`CA FE` leído de vuelta).

**Falla / pendiente (TODO):**
1. **Casting y combate en el recorrido desatendido** — el bloque de cast (`SPELL_LIGHT`,
   `SPELL_THUNDER`) y combate (`WeaverGhost` + counter) está **COMENTADO** en `run_auto`
   (`src/smoke/smoke_runner.c`) porque encadenar `spell_narrative_cast` /
   `init_enemy`+`combat_init` tras `new_level` + movimiento/hold **cuelga o crashea** el
   sprite engine (crash/hang en `SPR_update`). Los casos `CAST`/`SCENE` individuales del
   menú SÍ corren (flujo soportado).
   - *Siguiente paso*: descomentar **fase a fase** (primero `PH_CAST_LIGHT` solo, verificar
     con el driver; si pasa, `PH_CAST_THUNDER`; luego `PH_COMBAT`). Alternativa robusta:
     ejecutar una **escena real** con `scene_run` dentro de AUTO (ya existe
     `SMOKE_SCENE`) — es el flujo que el motor soporta — añadiendo un modo "auto-play".
2. **Offsets cambian en cada build** — el driver ya los lee de `out/symbol.txt`; si se
   mueve el driver a CI, mantener esa lectura (no fijar offsets).

*Nota de depuración (resuelto):* El supuesto "cuelgue al arrancar" de la ROM era un crash (`ILLEGAL INSTRUCTION` en `F90001` debido a desreferenciación `NULL` de `background_BGA` en `next_frame` antes de `new_level`). Se solucionó añadiendo una comprobación de seguridad `background_BGA != NULL` en `update_bg` (`src/world/background.c`).

**Ficheros clave:** `src/smoke/smoke_runner.c` (`run_auto`, globales `smoke_phase`/
`smoke_scratch`/`smoke_gate`), `src/smoke/smoke_main.c` (ventana de arranque),
`src/smoke/smoke_cases.h`
(fila `SMOKE_AUTO`), `tools/retroarch/mcp_driver.py` (driver host).

Añadir una invariante nueva = una fila `SMOKE_CHECK` en `smoke_cases.h` (entra sola en
la suite AUTO; ver `docs/testing.md`).
