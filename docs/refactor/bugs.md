# Bugs conocidos — estado de corrección

> Tabla maestra de `plan.md` §6, con estado vivo. Se actualiza al corregir cada uno.
> Grupos: **A** = no cambia comportamiento observable · **B** = sí lo cambia (playtest + nota en baseline.md) · **doc** = se documenta, no se toca · **(→F4)** = se resuelve en la Fase 4.

| # | Grupo | Archivo:línea | Bug | Fix acordado | Estado |
|---|---|---|---|---|---|
| B1 | B (latente) | `src/patterns/pattern_sleep.c:11` | lee `baseDuration` de HIDE (copy-paste); inobservable porque sleep no es lanzable | usar `PATTERN_SLEEP` | **corregido** (Fase 1) |
| B2 | A* | `src/patterns.c:548` | pattern-slot 0 hardcodeado: EN_BITE nunca se ejecuta | iterar slots + **deshabilitar bite explícitamente** (decisión §15: combate queda idéntico) | **corregido** (Fase 1, commit A) |
| B3 | B | `src/combat.c:75-76` | `try_counter_spell` hardcodea slot 0 | buscar slot del hechizo enemigo activo | **corregido** (Fase 1) |
| B4 | B | `src/combat.c:135-138` | `while(!SPR_isAnimationDone)` bloqueante en `hit_enemy` | estado FSM `COMBAT_STATE_ENEMY_DEFEAT_ANIM` (fix mínimo; defeat handling mejorado → Fase 4) | **corregido** (Fase 1) |
| B5 | doc | `src/entity.c:50-60` | `move_entity` bloqueante | intencional; documentar | **corregido** (Fase 1, commit A) |
| B6 | doc | `src/controller.c:252-255` | espera de followers bloqueante | intencional; documentar | **corregido** (Fase 1, commit A) |
| B7 | A | `src/texts.c:83` | `malloc` en `encode_spanish_text` | buffer estático | **corregido** (Fase 1, commit A) |
| B8 | A | `src/dialogs.c:267,336` | `free` del malloc anterior | eliminar | **corregido** (Fase 1, commit A) |
| B9 | A | `src/interface.c:243,269` | `MEM_alloc` sin NULL check | check + fallback | **corregido** (Fase 1, commit A) |
| B10 | A | `src/controller.c:5` | `frame_counter` local sombrea el global, nunca se lee | eliminar | **corregido** (Fase 1, commit A) |
| B11 | A | `src/patterns.h:16` | `MAX_ENEMY_PATTERNS 8` muerto | eliminar | **corregido** (Fase 1, commit A) |
| B12 | B | `characters.h:21-23` / `texts.h:9-10` | dos vocabularios "side", ambos `bool`, `SIDE_none==SIDE_left==true` | tipo u8 de 3 valores `SIDE_LEFT/RIGHT/NONE` + firmas | **corregido** (Fase 1) |
| B13 | A | include guards | dos convenciones | unificar `_FOO_H_` | **corregido** (Fase 1, commit A) |
| B14 | A | `src/globals.c:48` | comentario garbled "deTODO" | "depending on" | **corregido** (Fase 1, commit A) |
| B15 | A | `src/items.h:29` | idem | idem | **corregido** (Fase 1, commit A) |
| B16 | (→F4) | `src/combat.c:130` | TODO better defeat handling | pospuesto a Fase 4 (decisión §15) | pospuesto |
| B17 | doc | `src/sound.c:85,105` | TODO jingles SLEEP/EN_BITE | documentar como pendiente en AGENTS.md (decisión §15) | pendiente |
| B18 | (→F4) | `pattern_thunder.c:13-27`, `patterns.c:302-345` | callbacks muestran diálogos (acoplamiento) | hook `onRejected` del motor nuevo | **corregido** (Fase 4) |
| B19 | A | `pattern_thunder.c:3-4`, `pattern_en_thunder.c:7-8` | macros PAL_* duplicadas | mover a `patterns.h` (Fase 1) → `constants_spells.h` (Fase 4) | **corregido** (Fase 1, commit A) |
| B20 | A | `src/entity.h:19-30` | valores de `GameState` presuntamente muertos | verificar con grep; eliminar o documentar | **corregido** (Fase 1, commit A) |
| B21 | A | `src/enemies.c:53-54` | clase 3-head-monkey sin sprite | **eliminar la clase** (decisión §15) | **corregido** (Fase 1, commit A) |
| B22 | A | `src/act_1.h:6` | typo "3nd" | "3rd" | **corregido** (Fase 1, commit A) |
| B23 | A (latente) | `src/act_1.c:64` | `bool scene_timeout` como contador de frames (funciona porque bool=u8 en SGDK) | cambiar a `u16` | **corregido** (Fase 1, commit A) |
| B24 | A | `src/characters.c:301` | variable `dy` asignada y nunca leída (warning en build release) | eliminada | **corregido** (Fase 1, commit A) |
| B25 | A | `src/combat.c:128` | `hitpoints` es u16: `hitpoints -= damage` con daño > HP hacía underflow (enemigo inmortal con HP gigante) | comparar antes de restar | **corregido** (Fase 1, con B4) |
| B26 | A | `src/interface.c:578-604` | `update_life_counter`: escaneo de heridos sin check de `active`, y `hitpoints-1` con enemigo moribundo (0 HP) → underflow a anim 255 → corrupción del sprite engine y cuelgue del VDP (aflorado por B4; el check de `active` era un latente preexistente) | exigir `active && hitpoints>0` + validar candidato | **corregido** (Fase 1, post-test) |
