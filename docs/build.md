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

## Problemas típicos

- **"escena/id no existe"** al compilar → validación fatal de un generador:
  el mensaje trae archivo:línea del `.scene`/CSV.
- **Docker sin permisos** → en esta máquina docker corre sin sudo; en otra,
  añade tu usuario al grupo docker.
- **Comparar ROMs**: `GAMEVERSION` incrusta `__DATE__` — builds de días
  distintos difieren SIEMPRE a nivel binario. Compara comportamiento, no bytes.
- **Debug**: todos los toggles en `src/core/hack.h` (ver AGENTS.md §debugear).
