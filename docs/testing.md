# Testing — smoke ROM y checklist de playtest

## Smoke ROM

```bash
./build-theweave.sh smoke        # compila out/smoke.bin y lo lanza
```

La smoke ROM (`src/smoke/`, compilada con `-DHACK_SMOKE_BUILD`) arranca en un menú
que permite ejecutar cada hechizo y cada escena de forma aislada, sin jugar el
juego entero. Controles: UP/DOWN navegar · A ejecutar · (al terminar un caso se
vuelve al menú).

Tipos de caso (`src/smoke/smoke_cases.h`):

- **AUTO** (primera fila del menú) — recorrido desatendido por **cada mecánica
  fundamental**: los 7 `CHECK` de canUse, movimiento por el bosque, cast de LIGHT y
  THUNDER (duración medida en frames) y un combate contra un WeaverGhost resuelto
  con counter de trueno invertido. Deja una pantalla de resultados con el desglose
  (`CHECKS/WALK/CAST/COMBAT` + `RESULT: ALL PASS`/`FAIL` y los casos fallidos).
  Además corre **sola al arrancar** si no pulsas A en ~3 s: así la ROM se puede
  validar sin mando, a mano o vía RetroArch (ver `docs/retroarch-mcp.md` §9, que
  incluye la receta del driver `tools/retroarch/mcp_driver.py` — capturas con fecha
  en `docs/testing/smoke-latest/` — y la de la run a mano).
- **CHECK** — invariantes automáticos de canUse (p.ej. "fire sin zona → rechazado",
  "fire en ZONE_CAULDRON → permitido"). PASS/FAIL instantáneo en pantalla.
- **CAST** — lanza el hechizo con un cast scripted en un nivel de pruebas y mide
  los frames hasta terminar. PASS si acaba en `baseDuration ± 2`. El efecto visual
  lo juzga el humano. Cada hechizo se prueba en su **contexto real**: el TRUENO en
  el bosque OSCURO (`forest_dark`), donde su destello cambia toda la paleta; el
  resto en el bosque de día.
- **SCENE** — ejecuta la escena completa (interactiva: se juega). Al terminar
  vuelve al menú. Nota: `act1_forest` termina con reset de consola (es su final
  real) — es el comportamiento esperado.

### Añadir un caso

Una fila en `smoke_cases[]` (`src/smoke/smoke_cases.h`) y rebuild. Sin tocar el
menú ni el runner.

### Estado y trabajo futuro

La **validación automatizada de la smoke ROM está completa y verificada**: la suite
AUTO pasa desatendida (`RESULT: ALL PASS`) tanto a mano como por el driver MCP de
RetroArch (`docs/retroarch-mcp.md`), que ejercita las 16 tools, sincroniza capturas por
`smoke_phase` y da exit code 0/1 para scripts. El handshake del gate y las capturas son
deterministas (arranque en frío + anti-stale).

El resto del plan (`docs/testing/plan.md`) es **trabajo futuro**: embudo de input +
override por RAM para pilotar la ROM real (Fase 3, mete las **escenas** en la
verificación desatendida), y un menú de debug runtime con hacks en caliente (Fase 4).
Hasta entonces, las escenas y el resto del juego se validan con el **checklist de
playtest** de abajo.

## Checklist de playtest

- **Intro + logo**: GeeseBumps y la intro, selección de idioma.
- **act1_bedroom**: cisne (flash-aparición-diálogo-flash); 4 items en cualquier
  orden y repetibles; sleep una sola vez en el armario; la escena acaba ~3 s tras
  armario + pausa abierta.
- **act1_corridor**: aviso de demo; libros (texto largo la 1.ª vez, corto después);
  puertas y mapas; no deja salir sin leer ambos libros.
- **act1_hall**: diálogos, 2 choices — la respuesta debe corresponder a la opción.
- **act1_forest**: tutorial; pausa con inventario; combate contra 2 ghosts
  (thunder directo → hint; invertido fuera de ventana → hint; counter en ventana;
  hide corta el thunder; muerte fluida de cada ghost; contador de vida correcto);
  mensaje final y reset.
- **Idiomas**: ES y EN; caracteres especiales (ñ, tildes, ¿¡) correctos.

## Referencia

Las comparaciones entre builds son funcionales/visuales — nunca binarias
(`GAMEVERSION` incrusta la fecha de build, así que dos builds nunca son
byte-idénticos).

Capturas de referencia: pendiente de añadir en `docs/testing/` (hacerlas en el
próximo playtest completo).
