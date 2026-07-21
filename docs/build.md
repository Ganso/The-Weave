# Build — compilar y ejecutar The Weave

## TL;DR

```bash
./build-theweave.sh              # build incremental + lanzar (MiSTer si online, si no BlastEm)
./build-theweave.sh release -n   # clean + release, sin lanzar nada (verificación)
./build-theweave.sh smoke        # ROM de smoke test (out/smoke.bin) + lanzar
./build-theweave.sh clean
```

## Pipeline

1. **Codegen (host)** — `tools/gen_texts.py`, `gen_choices.py`, `gen_scenes.py`
   regeneran `narrative/texts_data.*`, `narrative/choices_data.*` y
   `scenes/scene_data.*` desde `data/`. Corre en el host porque la imagen Docker
   de SGDK **no incluye python3**. Los generadores validan en FATAL: una
   referencia rota corta el build con archivo:línea.
   Después, `tools/resample_wavs.py` remuestrea los WAV de `res/sfx/` al formato
   del driver de sonido (XGM2 → 13300 Hz mono; XGM → 14000; lee `XGM_VERSION` de
   `src/audio/sound.h`) generando un `<nombre>_resampled.wav` junto a cada
   fuente — **los `.res` referencian siempre el `_resampled.wav`**, el original
   es solo la fuente de autoría. Se salta los que están al día (`--force` para
   rehacerlos, p.ej. tras cambiar de driver); usa `ffmpeg`, pero como los
   `_resampled.wav` van commiteados, un equipo sin ffmpeg puede compilar
   mientras no añada WAV nuevos.
2. **Compilación (contenedor)** — imagen oficial `ghcr.io/stephane-d/sgdk:latest`
   con `--entrypoint make GDK=/sgdk -f /src/Makefile`. El `Makefile` del repo
   **envuelve** el `makefile.gen` de SGDK, que ya hace wildcard de todo `src/`
   (hasta 5 niveles) y `res/*.res` con `-Isrc -Ires`: añadir archivos no
   requiere tocar el Makefile.
3. **Backup** — `out/TheWeave_<fecha>.bin` rotatorio (mantiene los 5 últimos).
4. **Ejecución** — si `MISTER_IP` está configurado y responde: FTP + Remote API
   al MiSTer. Si no: BlastEm. `--no-run` omite este paso.

## Configuración personal

`.build-theweave.local.sh` (gitignored, se carga si existe):

```bash
MISTER_IP="192.168.1.x"
MISTER_PASS="..."
BLASTEM_BIN="/ruta/a/blastem"
SGDK_LOCAL="$HOME/sgdk"   # usar el SGDK local en vez del de la imagen
```

## ROM de smoke test

`./build-theweave.sh smoke` compila el juego con `-DHACK_SMOKE_BUILD`
(clean + release: el flag cambia el binario entero) y produce `out/smoke.bin`.
El `main()` del juego queda excluido por `#ifndef HACK_SMOKE_BUILD` y entra el
menú de smoke (`src/smoke/`). Ver `docs/testing.md`.

## VS Code

Tasks en `.vscode/tasks.json`: Build (Incremental) · Clean + Build (Release) ·
Clean · Build + Run · Run ROM (BlastEm) · Generate Texts · Consolidate.


## Cambiar entre build normal y smoke

Las dos ROMs se compilan del mismo código con `-DHACK_SMOKE_BUILD`, que decide
cuál de los dos `main()` entra (`src/core/main.c` o `src/smoke/smoke_main.c`).
Como make **no recompila por un cambio de flags** (las fuentes no cambian), al
volver de un build smoke sobrevivían los objetos con el otro `main` y el
enlazado fallaba con `multiple definition of main`.

Lo gestiona el script solo: guarda el último modo en `out/.last_build_mode` y,
si el anterior fue smoke, limpia antes de compilar. No hace falta acordarse de
nada, y los builds normales seguidos siguen siendo incrementales.

> No sirve mirar si existen `out/src/smoke/*.o`: el build normal también los
> genera (vacíos, porque el `#ifdef` deja el fichero sin contenido).

## Versiones de SGDK (el Makefile es autocontenido)

El `Makefile` del repo **no incluye el `makefile.gen` de SGDK**: define sus
propias reglas y solo usa `common.mk` (rutas de herramientas). El motivo es que
`makefile.gen` cambia entre versiones y rompía el build al usar un SGDK local
más moderno que el de la imagen Docker. Diferencias reales que nos afectaron:

| | SGDK antiguo (imagen Docker) | SGDK nuevo (local) |
|---|---|---|
| `-Isrc` en los includes | lo ponía él | **ya no** (nuestro código incluye `"core/core.h"`) |
| Cabecera de la ROM | `src/boot/rom_head.c` | `src/rom_header.c` |
| `sega.s` | el del proyecto | el de SGDK, copiado a `out/` |
| Binario que incrusta `sega.s` | `out/rom_head.bin` | `out/rom_header.bin` |
| Carpeta de includes en `common.mk` | `INCLUDE_LIB` | `INC_LIB` |
| Salida | `out/` | `out/<tipo>/` |

Cómo lo resuelve nuestro Makefile, para que **compile con cualquiera de las dos**:

- Pone `-I$(SRC)` en `INCS` él mismo (y deja `EXTRA_FLAGS` libre, que es por
  donde `build-theweave.sh smoke` pasa `-DHACK_SMOKE_BUILD`).
- La cabecera vive en **`src/boot/rom_header.c`** y es NUESTRA (título "The
  Weave", "Geesebumps 2024", región). Se excluye del wildcard y tiene regla
  propia.
- El `sega.s` lo aporta SGDK (`$(SRC_LIB)/boot/sega.s`), así no arrastramos una
  copia que se queda vieja. El nuestro se borró.
- Genera la cabecera con **los dos nombres** (`rom_header.bin` y
  `rom_head.bin`), porque cada `sega.s` la incrusta por nombre de fichero.
- `INC_LIB ?= $(INCLUDE_LIB)` salva el renombrado de la variable.
- Mantiene la salida plana en `out/` (que es lo que espera el script y las
  herramientas de verificación: `out/rom.bin`, `out/symbol.txt`).

También hubo un cambio de **API**: `Z80_loadDriver(Z80_DRIVER_XGM2, ...)` quedó
obsoleta y las versiones nuevas la rechazan al compilar. Se usa
`XGM2_loadDriver(1)` / `XGM_loadDriver(1)`, que existen en ambas.

> Si al cambiar de SGDK algo deja de compilar, mira primero estas diferencias
> antes de tocar el código del juego.

## Problemas típicos

- **"escena/id no existe"** al compilar → validación fatal de un generador:
  el mensaje trae archivo:línea del `.scene`/CSV.
- **Docker sin permisos** → en esta máquina docker corre sin sudo; en otra,
  añade tu usuario al grupo docker.
- **Comparar ROMs**: `GAMEVERSION` incrusta `__DATE__` — builds de días
  distintos difieren SIEMPRE a nivel binario. Compara comportamiento, no bytes.
- **Debug**: todos los toggles en `src/core/hack.h` (ver AGENTS.md §debugear).
