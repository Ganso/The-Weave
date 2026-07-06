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
- [x] Playtest inicial del usuario (notas/capturas para baseline.md — puede hacerse durante la Fase 1)

## Fase 1 — Bugs y baseline

- [x] Commit A (grupo A): B2*, B7, B8, B9, B10, B11, B13, B14, B15, B19, B20, B21, B22, B23
- [x] Commit B1 (duration de sleep) + playtest
- [x] Commit B3 (slot del counter) + playtest combate
- [x] Commit B4 (defeat anim → FSM) + playtest combate
- [x] Commit B12 (SIDE_* a 3 valores) + playtest diálogos
- [x] B5/B6 documentados como bloqueantes intencionales
- [x] `baseline.md` cerrado (comportamiento de referencia + capturas)

## Fase 2 — Directorios, Makefile y build script

- [x] `gibberish/` → `tools/voice/`; scripts → `tools/`; `texts.csv` → `data/`
- [x] `Makefile` propio (wildcards + codegen)
- [x] `build-theweave.sh` (estilo RedPlanet: backup rotatorio + MiSTer/BlastEm)
- [x] `.vscode/tasks.json` y `.gitignore` actualizados
- [x] Build pasa desde el repo sin `~/codigo/build-sgdk.sh`

## Fase 3 — Partición de src/ + includes explícitos

- [x] Subcarpetas por dominio; `git mv` de todos los módulos
- [x] `frame.c/.h`, `config.h`, `hack.h` extraídos de globals
- [x] `constants_*.h` por dominio (AJUSTE: las constantes ya viven agrupadas en el .h de cada módulo — estilo del autor; se crean constants_*.h solo si un dominio lo pide en Fases 4-5)
- [x] `encode.c/.h` extraído de texts; `texts_generated` → `texts_data`
- [x] Includes explícitos en todo salvo `patterns*`/`act_1*` (globals.h transicional)
- [x] Diagramas de arquitectura en `entity.h` y `combat.h`
- [x] Build + playtest de las 4 escenas contra baseline

## Fase 4 — Motor de hechizos

- [x] `spell.h`/`spell.c` (2 slots) + `notes.c` (input + HUD de notas)
- [x] `spell_hooks.c` (AJUSTE: no hay hooks compartidos aún — cada hechizo lleva los suyos; se creará cuando un hook se repita de verdad)
- [x] Migrados: thunder (con onRejected — resuelve B18), hide, open, sleep, en_thunder, en_bite (deshabilitado)
- [x] Integración combat + interface + sound + controller + frame; CombatContext y patterns eliminados
- [x] Hechizo FIRE end-to-end (zona + counter + fases; casos smoke en Fase 7)
- [x] `patterns.*` y `src/patterns/` eliminados
- [ ] Playtest act1_scene3 idéntico a baseline

## Fase 5 — VM de escenas

- [x] `scene_vm` + `scene_hooks` (AJUSTE: sin scene_api — la VM llama las primitivas directamente; puzzles diseñados en fase5_design.md §7, se implementan cuando el guion los pida)
- [x] `gen_scenes.py` con validación fatal (probada con referencias rotas)
- [x] Migradas las 4: scene1/2 (DSL + hooks), scene3 (DSL casi puro, 19 steps), scene5 (DSL + op combat)
- [x] `choices.csv` + `gen_choices.py`
- [x] `main.c` usa `scene_lookup()` + `scene_run()`
- [x] `act_1.*`, `globals.h` y `add_texts_comments.py` eliminados
- [ ] Playtest juego completo contra baseline (pendiente del usuario)

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
