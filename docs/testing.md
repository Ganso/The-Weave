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

- **CHECK** — invariantes automáticos de canUse (p.ej. "fire sin zona → rechazado",
  "fire en ZONE_CAULDRON → permitido"). PASS/FAIL instantáneo en pantalla.
- **CAST** — lanza el hechizo con un cast scripted en un nivel de pruebas y mide
  los frames hasta terminar. PASS si acaba en `baseDuration ± 2`. El efecto visual
  lo juzga el humano.
- **SCENE** — ejecuta la escena completa (interactiva: se juega). Al terminar
  vuelve al menú. Nota: `act1_forest` termina con reset de consola (es su final
  real) — es el comportamiento esperado.

### Añadir un caso

Una fila en `smoke_cases[]` (`src/smoke/smoke_cases.h`) y rebuild. Sin tocar el
menú ni el runner.

## Test automatizado: tools/smoke-test.sh

```bash
./tools/smoke-test.sh        # exit 0 = todos los casos automáticos PASS
```

Compila la smoke ROM en modo AUTORUN (`-DSMOKE_AUTORUN`): al arrancar ejecuta
solos los casos CHECK y CAST, emite cada resultado por KDebug y termina con
`SMOKE RESULT: n/m PASS`. El script lanza BlastEm capturando su stdout, espera
esa línea (timeout `SMOKE_TIMEOUT`, 120 s por defecto), cierra el emulador y
devuelve exit code 0/1 con el detalle por caso.

- Ejecutar desde la sesión gráfica: la ventana de BlastEm aparece ~15-20 s y
  se cierra sola. (BlastEm 0.6.3 no tiene modo headless utilizable — probado
  SDL dummy, render software y Xvfb sin éxito; detalles en el propio script.)
- Las escenas (SCENE) no entran en el autorun: requieren jugador — para ellas,
  `./build-theweave.sh smoke` y el menú interactivo.

## Checklist de playtest (contra docs/refactor/baseline.md)

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

El comportamiento de referencia es el baseline post-Fase-1
(`docs/refactor/baseline.md`). Las comparaciones son funcionales/visuales — nunca
binarias (`GAMEVERSION` incrusta la fecha de build).

Capturas de referencia: pendiente de añadir en `docs/testing/` (hacerlas en el
próximo playtest completo).
