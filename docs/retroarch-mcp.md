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
6. Cerrar RetroArch (`pkill -f 'retroarch -L'`).
