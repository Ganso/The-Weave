# docs/refactor/ — Histórico del refactor v2.0 (COMPLETADO)

Esta carpeta es el **archivo histórico** del refactor de The Weave realizado en
**2026-07** (8 fases, mergeado a `master` como tag `v2.0-refactor`). Se conserva
como registro del proceso, las decisiones y los bugs; **no es documentación viva**.

> La documentación VIVA del proyecto (cómo está el código HOY y cómo trabajar con
> él) está en **`AGENTS.md`** (raíz) y en **`docs/{spells,scenes,texts,build,testing}.md`**.
> Si buscas "cómo añadir un hechizo/escena/diálogo" o "cómo compilar", ve ahí.

## Qué había antes y qué hay ahora (resumen del refactor)

Se partió de ~6.500 líneas con un `globals.h` paraguas, un sistema de "patterns"
con slots hardcodeados y las 4 escenas del acto 1 escritas a mano en `act_1.c`.
Se llegó a: motor de hechizos de **dos slots** (`spells/`), **VM de escenas** con
DSL y codegen validado (`scenes/` + `data/scenes/`), includes por metalibrería de
dominio, build reproducible propio (`build-theweave.sh` + Makefile), smoke ROM y
documentación de autoría. 26 bugs corregidos por el camino.

## Índice de documentos

| Documento | Qué es |
|---|---|
| `plan.md` | El plan completo del refactor (8 fases, decisiones D1-D12, apéndices). Era `refactorizar.md` en la raíz. |
| `state_audit.md` | Auditoría del estado inicial (pre-refactor): métricas, subsistemas, deuda. |
| `bugs.md` | Tabla de los 26 bugs (B1-B26) con grupo, fix y estado. |
| `baseline.md` | Comportamiento de referencia post-Fase-1 (validado por el usuario). |
| `checklist.md` | Checklist de las 8 fases, todas tachadas. |
| `fase5_design.md` | Documento de diseño detallado de la Fase 5 (VM de escenas + DSL). |
| `rom_pre_refactor.bin` | ROM de referencia pre-refactor (gitignored, local). |

## Trabajo posterior al refactor (mejoras sobre v2.0)

Cambios hechos después del merge, documentados en AGENTS.md y en los commits:
banco de pruebas `act1_test` (docs/test_scene.md), hechizos LIGHT/FIRE con iconos,
puzzles de secuencia, metalibrerías de dominio, reorganización de `res/` en
`gfx/`+`sfx/`, y limpieza de `tools/voice/`.
