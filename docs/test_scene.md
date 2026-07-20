# act1_test — banco de pruebas del motor

`act1_test` es la escena que **ejercita todos los ops del DSL y todas las mecánicas de
motor** en un solo sitio. No forma parte del flujo del juego: es la chuleta canónica
del DSL (referencia cruzada de AGENTS.md §6) y el checklist de regresión cuando se toca
el motor de escenas, de hechizos o de combate.

- **Guion**: `data/scenes/act1/test.scene`. **Lógica**: `src/scenes/act1/test.c`.
- **Cómo entrar**: `HACK_START_SCENE "act1_test"` en `src/core/hack.h` + build, o la
  smoke ROM (`./build-theweave.sh smoke` → caso "SCENE act1_test"). Tras probar con
  smoke, `./build-theweave.sh release` restaura `out/`.

## Estructura: intro + HUB de secciones

Intro corta (cluster + `wait_press`) → menú de 4 opciones que se repite al acabar cada
sección:

1. **Diálogos y quiz**
2. **Hechizos y puzzles**
3. **Combate** → menú anidado: **por hechizos** (dos oleadas) o **físico: jabalíes**
   (contacto, Linus sin vara). En ambos, el jugador tiene **5 puntos de vida** por
   combate; al llegar a cero, mensaje de derrota y reintento.
4. **Terminar test** → fade + `next_scene` al dormitorio.

## Qué prueba cada sección

### Intro
- Cluster de 3 pantallas con el mapeo botón→nota.
- `wait_press`: la escena espera a que pulses A.

### 1. Diálogos y quiz
- Cara de **Clio a la DERECHA** y de **Xander a la izquierda** (variedad de face/side).
- `anim`: Linus hace la animación de MAGIA 1,5 s por opcode del DSL y vuelve a idle.
- **Quiz anidado** (choices dentro de choices, con bucles de reintento):
  - Q1 "¿Cuántas notas tiene un patrón?" → correcta **Cuatro** (opción 2). Fallar →
    mensaje + repetición de la pregunta (bucle `goto`).
  - Q2 "¿Qué hechizo es palíndromo?" → correcta **Esconderse** (opción 3).
  - Confirmación anidada "¿Estás seguro?" → **No** vuelve a Q2; **Sí** supera el quiz.
- Vuelta al hub.

### 2. Hechizos y puzzles
- Cast scripted de **sleep** (hechizo solo-guion) con `wait_spell`.
- Cast scripted de **LUZ** (hechizo por fases declarativas): el cielo pasa a **cian**
  (0,75 s) y luego a **blanco** (0,75 s) y se restaura.
- Selector de puzzle (choice de 2):
  - **Puzzle 1 (básico)**: trueno → fuego → esconderse (directos). Requiere
    `zone ZONE_CAULDRON` (fire). Notas en el propio diálogo.
  - **Puzzle 2 (con INVERTIDO)**: trueno → **luz invertida (DO SI LA SOL)** →
    esconderse. Prueba el matching `reversed` en `PuzzleSeq` (LUZ es el único hechizo
    casteable invertido libremente, sin `canUse`).
- Fallar la secuencia la reinicia (con crédito si el fallo es el 1.er paso).
- `if_puzzle_solved` tras el puzzle 1 (el centinela "ERROR" nunca debe verse).
- Vuelta al hub (se puede entrar otra vez y elegir el otro puzzle).

### 3. Combate por hechizos (dos oleadas en la misma escena)
- **Oleada 1**: WeaverGhost clásico — hint al thunder directo, counter (SOL SOL FA MI)
  durante su efecto, muerte con 2 counters.
- **Oleada 2**: **ENEMY_CLS_TESTGHOST** (clase SOLO test, sprite del ghost) con DOS
  hechizos: thunder (counterable, recarga 3 s) y **mordisco** (MI SOL DO, 3 notas, NO
  counterable, recarga 2 s, 1 de daño). Verifica la alternancia de ataques por recargas
  y que el counter solo funciona contra el thunder (contra el mordisco: esconderse o
  comerse el golpe).
- `combat` dos veces en la misma escena (re-init limpio del FSM).
- **Derrota y reintento**: dejarse golpear 5 veces → "Te han derrotado" y la oleada
  vuelve a empezar (por oleada: perder en la 2 repite solo la 2).
- Vuelta al hub.

### 3b. Combate de contacto: jabalíes (`combat/contact.c`)
- Al entrar en Combate, **Clio aparece y sigue a Linus** (en ambos combates).
- Al empezar el combate, Linus pierde la vara y saca la **antorcha** (`linus_torch`; el
  golpe con A es el de antorcha) y Clio, si estaba delante, se recoloca andando detrás
  de él mirando a la derecha y se queda inmóvil todo el combate.
- Entran **5 jabalíes**: tres por la derecha y dos por la izquierda, a distintas
  alturas, escalonados (~0,7 s).
- Los jabalíes **persiguen** a Linus con **pausas aleatorias** (~1 s quietos, con
  cooldown por enemigo) y, al llegar a su altura, **muerden** (daño a mitad del ciclo;
  flash + stun del jugador, sin diálogos).
- **Golpe con A** (sin notas, `set spells off`): anim ACTION; si hay un jabalí delante
  (según hacia dónde mira Linus) y a su altura, recibe el golpe (flash) y **huye
  galopando al borde de pantalla más cercano**, y desde fuera vuelve a la carga.
  Cooldown entre golpes (~0,5 s).
- Al **sexto golpe conectado** (a cualquier jabalí), todos huyen y desaparecen; termina
  el combate.
- **Derrota y reintento**: 5 mordiscos → "Te han derrotado" y el combate se reinicia.
- Linus recupera la vara (sprite normal) y vuelta al hub.

### Salida
- Opción 4 del hub: despedida + `fade_out` + `next_scene` al dormitorio.
- Desde la smoke ROM: al terminar vuelve al menú de smoke.

## Lo que queda por hacer (banco de pruebas)

Mejoras opcionales, no bloqueantes (el banco cubre ya todos los ops actuales):

- Op `music`/`sfx` desde el DSL (necesita tabla lateral de recursos: los punteros no
  caben en los args `s16` del `SceneStep`).
- Puzzle con pasos de 4 hechizos (`PUZZLE_SEQ_MAX` ya lo permite).
- Sección de items interactuables en el test (bucle tipo bedroom con hook).
