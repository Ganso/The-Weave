# AGENTS.md

Este archivo proporciona pautas y notas para herramientas automáticas que interactúan con el código de **The Weave**.

## Descripción general
- **The Weave** es un fangame de *Loom* desarrollado para **Sega Genesis/Megadrive**.
- Todo el código está escrito en **C** y se compila con **SGDK** (Sega Genesis Development Kit).
- La documentación de SGDK puede consultarse en https://stephane-d.github.io/SGDK/
- Las fuentes están en `src/` y los recursos generados (sprites, fondos, sonidos...) en `res/`.

## Estilo de código
- Cada archivo `.c` **solo** incluye `globals.h`. Dicho encabezado enlaza `<genesis.h>`, todos los recursos de `res/` y las cabeceras de cada módulo del juego.
- Las funciones se declaran en los `.h` como `tipo nombre(parámetros);` y se definen en los `.c` siguiendo el formato:
  ```c
  tipo nombre(parámetros)    // Breve comentario explicativo
  {
      // cuerpo de la función
  }
  ```
- Se usa indentación de 4 espacios y las llaves de funciones y bucles abren en la línea siguiente.
- Los nombres de variables y funciones usan `snake_case`; las constantes y macros se escriben en `MAYÚSCULAS`.
- Las variables globales se declaran con `extern` en su `.h` correspondiente y se definen en un archivo `.c`.
- Los comentarios son `//` para una línea y `/* ... */` para bloques multi-línea.

## Hardware y limitaciones
- El juego está diseñado para el hardware original de **Sega Genesis**, que dispone de solo 64 KB de RAM.
- Se evita la memoria dinámica y se usan tipos de tamaño reducido (`u8`, `u16`, `bool`...) para minimizar el consumo de RAM.
- Siempre que sea posible, las tablas y datos constantes se definen como `static const` para que se almacenen en ROM y no en RAM.
- Los arrays y estructuras se declaran de forma estática; se intenta mantener el tamaño del *stack* lo más pequeño posible.

## Uso de librerías y recursos
- `globals.h` gestiona la inclusión de todas las cabeceras y recursos, de modo que cada `.c` empieza con:
  ```c
  #include "globals.h"
  ```
- Los archivos `res_*.h` generados por SGDK se encuentran en `res/` y se incluyen de forma indirecta desde `globals.h`.

## Compilación
- La compilación se realiza con las herramientas de **SGDK**. Habitualmente se llama a `make` utilizando el `Makefile` proporcionado por SGDK.
- Asegúrese de tener configuradas las variables de entorno de SGDK antes de compilar.

## Scripts
- Los scripts de utilidades (`add_texts_comments.py`, `generate_texts.py`) están escritos en Python 3 y siguen la sintaxis estándar del lenguaje.

Estas indicaciones permiten a Codex y a otras herramientas comprender la estructura del proyecto, su estilo de programación y las limitaciones de memoria propias del hardware.

## Lógica del juego
La carpeta `src/` contiene diversos módulos C organizados por temática. Cada módulo cuenta con un `.c` y su cabecera `.h`, y se incluye de forma indirecta mediante `globals.h`. A grandes rasgos:

- `entity.[ch]` define el tipo `Entity` con posición, sprites, caja de colisión y el enumerado `GameState`. Es la base de personajes, enemigos e items.
- `characters.[ch]` gestiona los personajes controlables y sus caras.
- `enemies.[ch]` define las clases e instancias de enemigo junto a funciones de IA y movimiento.
- `items.[ch]` representa los objetos del escenario que usan internamente un `Entity`.
- `background.[ch]` maneja el scroll y los límites del escenario.
- `controller.[ch]` procesa la entrada del pad, el movimiento y la pausa.
- `collisions.[ch]` ofrece funciones para detectar colisiones y distancias.
- `interface.[ch]` dibuja la HUD y la pantalla de pausa.
- `combat.[ch]` implementa la máquina de estados de combate y las acciones `hit_enemy`, `hit_player`, `update_combat`…
- `patterns.[ch]` gestiona el sistema de hechizos: validación de notas, ejecución y control de efectos. Los ficheros `patterns/*.c` contienen las callbacks de cada hechizo.
- Otros módulos (`init`, `intro`, `act_1`, `geesebumps`, `sound`, `dialogs`, `texts`…) se encargan de la inicialización, escenas y recursos.

### Sistema de patrones
Las notas se codifican con constantes `NOTE_MI`…`NOTE_DO`. Jugador y enemigos almacenan una cola de notas (máx. 4). `validate_pattern` comprueba la secuencia y devuelve el ID del patrón, pudiendo indicar si está invertido. Cada `PlayerPattern` o `EnemyPattern` define callbacks `launch`, `update` y si la magia es contrarrestable. Las definiciones se encuentran en las tablas `playerPatterns` y `enemyPatterns`.

### Entidades
`Entity` incluye estado (`GameState`), posición, tamaño, prioridad, animación y datos de colisión. `characters`, `enemies` e `items` encapsulan este tipo para sus propias necesidades. Los enemigos además almacenan puntos de vida y un `EnemyMode` para distinguir sus fases.
Desde esta actualización las entidades manejan coordenadas de tipo fijo (`fix16`) en `fx`/`fy` y una velocidad `velocity` expresada también en ese formato para permitir movimiento subpixel.

### Combate
`combat.c` gestiona un bucle basado en `CombatState`. Durante `COMBAT_STATE_IDLE` los enemigos pueden lanzar patrones si su `rechargeFrames` ha terminado. El lanzamiento y actualización de patrones modifican `combat_state` y `combatContext` (temporizadores, notas en curso, enemigo activo…). Las funciones `hit_enemy` y `hit_player` aplican daño y activan la animación de `HURT`. `update_combat` se llama cada fotograma desde `next_frame` para avanzar la máquina de estados.

Estas notas resumen la lógica actual del juego para facilitar futuras modificaciones por parte de herramientas automáticas.
