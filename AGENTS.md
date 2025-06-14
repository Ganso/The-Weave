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
