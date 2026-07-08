# Baseline de comportamiento — referencia del refactor (post-Fase-1)

> Este build (rama `refactor`, fin de Fase 1, commit del cierre de fase) es la
> **referencia de comportamiento** para todas las fases siguientes (decisión D10 de
> `plan.md`). La comparación es funcional/visual, nunca binaria.
>
> **VALIDADO por el usuario el 2026-07-06**: el combate de la escena 5 (el punto que
> destapó los bugs B26 y las regresiones de B2/B4) fue re-testeado con un ROM de test
> que arrancaba directamente en el combate, matando a ambos WeaverGhosts sin cuelgues.
> El resto del juego confirmado funcionando.

## Mapa real de escenas (corregido durante la Fase 1)

| Escena | Contenido |
|---|---|
| `act_1_scene_1` | Dormitorio: visita del cisne, bucle de 4 items, aprendizaje del patrón sleep |
| `act_1_scene_2` | Pasillo de los historiadores: libros/puertas/mapas, salida al leer ambos libros |
| `act_1_scene_3` | Hall con Clio y Xander: diálogos + 2 choices ramificados (sin combate) |
| `act_1_scene_5` | Bosque: tutorial de hechizos + **combate contra 2 WeaverGhosts** + finale |

## Cambios de comportamiento respecto al build pre-refactor

Todo lo demás se comporta exactamente como `docs/refactor/rom_pre_refactor.bin`.

| Fix | Cambio observable |
|---|---|
| B4 (derrota sin bloqueo) | Al derrotar a un WeaverGhost, la animación de muerte (1 s, `ENEMY_HURT_DURATION`) se reproduce sin congelar el resto de la pantalla. El resultado final es el mismo. |
| B26 (contador de vida) | El contador de vida desaparece al morir el enemigo en vez de parpadear con valores basura (antes del fix esto colgaba la consola). |
| B12 (side de diálogo) | Ninguno. |
| B1, B2, B3, B25 | Ninguno observable (sleep no lanzable; bite deshabilitado; un solo hechizo counterable; daño siempre 1). |

## Incidencias del cierre de fase (histórico)

1. **Cuelgue al matar al primer enemigo** (reportado por el usuario): tres causas
   encadenadas, todas corregidas —
   - `get_active_enemy_pattern` resolvía por `activePattern`, campo compartido con los
     patterns del jugador (ids solapados) → nuevo campo `activeEnemyPatternSlot`.
   - La liberación del moribundo usaba `SPR_isAnimationDone` el mismo frame del cambio
     de animación → muerte por timer determinista.
   - **B26** (la causa directa del cuelgue): `update_life_counter` calculaba
     `hitpoints - 1` con el moribundo (0 HP) → underflow a anim 255 → `SPR_setAnim`
     corrompía el sprite engine y `SPR_update` congelaba el VDP.
2. La auditoría situaba el combate en la escena 3; está en la **escena 5**. Corregido
   en `state_audit.md` y `plan.md`.

## Referencia de playtest para fases siguientes

Verificar contra este baseline tras cada fase:

- Intro + logo GeeseBumps.
- Escena 1: 4 items en cualquier orden y repetibles; sleep una sola vez en el cabinet;
  salida ~3 s tras cabinet + pausa abierta.
- Escena 2: libros (texto largo la 1.ª vez, corto después), puertas, mapas; no deja
  salir sin leer ambos libros.
- Escena 3: diálogos del hall, 2 choices, respuesta acorde a la elección.
- Escena 5: tutorial, pausa con inventario de hechizos, combate contra 2 ghosts
  (hint al lanzar thunder directo, counter con thunder invertido, muerte fluida de
  cada ghost, contador de vida correcto), mensaje final y reset.
- Diálogos ES/EN: caracteres especiales (ñ, tildes, ¿¡) y posiciones correctas.
