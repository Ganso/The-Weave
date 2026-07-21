# Documentación de The Weave

Este es el índice. Empieza por la fila que describa lo que quieres hacer.

## Quiero crear algo para el juego

Una guía por elemento, escritas **para no técnicos**: cada una va del **arte** a
declararlo en los recursos, al **código** mínimo y a **usarlo en una escena**,
con una tabla de "si algo va mal" al final.

| Quiero crear… | Guía |
|---|---|
| Un **personaje** (anda, habla, tiene retrato) | [characters.md](characters.md) |
| Un **enemigo** (de contacto o a distancia) | [enemies.md](enemies.md) |
| Un **objeto** del decorado o interactuable | [items.md](items.md) |
| Un **fondo** / escenario | [backgrounds.md](backgrounds.md) |
| Un **sonido** o música | [audio.md](audio.md) |
| Un **hechizo** | [spells.md](spells.md) |
| Una **escena** (cutscene) | [scenes.md](scenes.md) |
| **Diálogos** y preguntas al jugador | [texts.md](texts.md) |

## Quiero entender cómo funciona algo

| Tema | Documento |
|---|---|
| El **apartado gráfico** (VRAM, índice de tiles, planos, paletas, scroll) | [graphics.md](graphics.md) |
| El **combate** (director único, encuentros declarativos, roles) | [combat.md](combat.md) |
| El **motor** entero, la arquitectura y las trampas | [../AGENTS.md](../AGENTS.md) |
| En qué **estado** está el acto 1 y qué falta | [acto1.md](acto1.md) |

## Quiero compilar, probar o depurar

| Tarea | Documento |
|---|---|
| Compilar y configurar el entorno | [build.md](build.md) |
| Probar: smoke ROM y checklist de playtest | [testing.md](testing.md) |
| Qué prueba cada sección de la escena de test | [test_scene.md](test_scene.md) |
| Depuración desatendida con RetroArch | [retroarch-mcp.md](retroarch-mcp.md) |
| Roadmap de testing pendiente | [testing/plan.md](testing/plan.md) |

---

## Reglas de la casa (para quien mantenga esta documentación)

- Las guías de **creación de elementos** son autocontenidas y no técnicas: no
  mandan al lector a otros documentos salvo para temas vecinos bien acotados.
  Conserva ese tono; no las acortes al estilo terso de AGENTS.md.
- Todas siguen la **misma estructura**: qué necesitas → arte → declarar en
  `.res` → código → usarlo en la escena → comprobar (con tabla de fallos
  típicos) → resumen final.
- **AGENTS.md documenta el motor**; estas guías documentan cómo crear
  contenido. No dupliques: enlaza.
- Actualiza la documentación **en la misma tanda** que el código que describe.
