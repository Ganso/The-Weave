# Checklist del refactor — fase a fase

> Se tacha al completar. La descripción completa de cada fase está en `refactorizar.md` §10.
> Regla de oro: si el build no pasa o el playtest revela regresión contra el baseline,
> se arregla antes de avanzar.

## Fase 0 — Preparación

- [x] Rama `refactor` creada desde `master`
- [x] Build actual documentado (`state_audit.md`: Docker + ghcr.io/stephane-d/sgdk, modos, salida)
- [x] Build de verificación pasa sin warnings (`out/rom.bin`, 896 KB)
- [x] ROM de referencia guardado (`docs/refactor/rom_pre_refactor.bin`, fuera de git)
- [x] `state_audit.md` generado
- [x] `bugs.md` generado (B1-B23 con grupos y decisiones)
- [x] `checklist.md` generado
- [ ] Playtest inicial del usuario (notas/capturas para baseline.md — puede hacerse durante la Fase 1)

## Fase 1 — Bugs y baseline

- [ ] Commit A (grupo A): B2*, B7, B8, B9, B10, B11, B13, B14, B15, B19, B20, B21, B22, B23
- [ ] Commit B1 (duration de sleep) + playtest
- [ ] Commit B3 (slot del counter) + playtest combate
- [ ] Commit B4 (defeat anim → FSM) + playtest combate
- [ ] Commit B12 (SIDE_* a 3 valores) + playtest diálogos
- [ ] B5/B6 documentados como bloqueantes intencionales
- [ ] `baseline.md` cerrado (comportamiento de referencia + capturas)

## Fase 2 — Directorios, Makefile y build script

- [ ] `gibberish/` → `tools/voice/`; scripts → `tools/`; `texts.csv` → `data/`
- [ ] `Makefile` propio (wildcards + codegen)
- [ ] `build-theweave.sh` (estilo RedPlanet: backup rotatorio + MiSTer/BlastEm)
- [ ] `.vscode/tasks.json` y `.gitignore` actualizados
- [ ] Build pasa desde el repo sin `~/codigo/build-sgdk.sh`

## Fase 3 — Partición de src/ + includes explícitos

- [ ] Subcarpetas por dominio; `git mv` de todos los módulos
- [ ] `frame.c/.h`, `config.h`, `hack.h` extraídos de globals
- [ ] `constants_*.h` por dominio
- [ ] `encode.c/.h` extraído de texts; `texts_generated` → `texts_data`
- [ ] Includes explícitos en todo salvo `patterns*`/`act_1*` (globals.h transicional)
- [ ] Diagramas de arquitectura en `entity.h` y `combat.h`
- [ ] Build + playtest de las 4 escenas contra baseline

## Fase 4 — Motor de hechizos

- [ ] `spell.h`/`spell.c` (2 slots, validate/launch/update/try_counter/finish)
- [ ] `spell_hooks.c` (hooks compartidos)
- [ ] Migrados uno a uno contra baseline: thunder (con onRejected), hide, open, sleep, en_thunder, en_bite (deshabilitado)
- [ ] Integración combat + interface + sound; FSM viejo de patterns eliminado
- [ ] Hechizo FIRE end-to-end (3 casos)
- [ ] `patterns.*` y `src/patterns/` eliminados
- [ ] Playtest act1_scene3 idéntico a baseline

## Fase 5 — VM de escenas

- [ ] `scene_vm` + `scene_api` + `scene_hooks` (+ tabla lateral de puzzles)
- [ ] `gen_scenes.py` con validación fatal
- [ ] Migradas una a una: scene1 (hook items), scene2 (hook items), scene3 (op combat), scene5
- [ ] `choices.csv` + `gen_choices.py`
- [ ] `main.c` usa `scene_run()`
- [ ] `act_1.*`, `globals.*` y `add_texts_comments.py` eliminados
- [ ] Playtest juego completo contra baseline (ambas rutas, win y lose)

## Fase 6 — Documentación

- [ ] `AGENTS.md` reescrito (11 secciones)
- [ ] `docs/spells.md`, `docs/scenes.md`, `docs/texts.md`, `docs/build.md`, `docs/testing.md`
- [ ] Comentarios `//` completados en los `.h`
- [ ] Cabecera de una línea en cada archivo

## Fase 7 — Smoke ROM + verificación final

- [ ] `smoke/` completo; target `smoke` (excluye `src/core/main.c`)
- [ ] Todos los casos PASS
- [ ] Screenshots de referencia en `docs/testing/`
- [ ] Playtest final contra `baseline.md`
- [ ] Merge `refactor` → `master` (+ tag `v2.0-refactor`)
