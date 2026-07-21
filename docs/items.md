# Cómo crear un objeto nuevo — guía completa

Un **objeto** es cualquier cosa colocada en el decorado: una cama, un libro, un
baúl, un árbol, una gaviota. Sirven para dos cosas (o para las dos a la vez):

- **Decorar** — piezas que se dibujan delante o detrás de los personajes para
  dar profundidad a la escena.
- **Interactuar** — el jugador se acerca, pulsa **A** y pasa algo (un comentario,
  aprender un hechizo, abrir una puerta…).

Esta guía va del dibujo hasta verlo funcionar, sin saber programar.

---

## 1. El arte

Un objeto es **un PNG normal** con el dibujo de la cosa. Casi siempre tiene un
solo fotograma; si quieres que se anime (una lámpara que parpadea, una gaviota
que mueve las alas), pon los fotogramas **en fila, uno detrás de otro**.

Reglas:

- Ancho y alto **múltiplos de 8** píxeles.
- El color transparente debe ser **el primero de la paleta**.
- Los objetos suelen usar la **paleta del fondo** de su escena (así se funden con
  el decorado). Dibuja con los colores de esa paleta.
- Si se anima, **todos los fotogramas del mismo tamaño**.

Se guardan por acto y escena:

```
res/gfx/items/act1/bedroom_chest.png
res/gfx/items/act1/hut_staff.png
```

---

## 2. Declarar el arte: `res/res_items.res`

Añade una línea en el bloque de su escena:

```
# Act 2 - Scene 1 (taller)
SPRITE item_taller_anvil "gfx/items/act2/taller_anvil.png" 4 3 BEST
```

| Trozo | Qué es |
|---|---|
| `item_taller_anvil` | el nombre con el que lo conocerá el guion (empieza por `item_`) |
| `"gfx/..."` | la ruta de tu PNG |
| `4 3` | tamaño de UN fotograma en bloques de 8 px → 32×24 píxeles |
| `BEST` | compresión (déjalo así) |
| *(un número más)* | **solo si se anima**: velocidad en fotogramas por segundo |

---

## 3. Colocarlo en la escena

Aquí **no hace falta tocar código**: los objetos se ponen directamente en el
guion de la escena (`data/scenes/<acto>/<escena>.scene`), en el bloque de
montaje, con la orden `item`:

```
item 3 item_taller_anvil PAL_BACKGROUND 200 150 32 0 12 4 CALCULATE_DEPTH
```

Los trozos, en orden:

| Posición | Qué es |
|---|---|
| `3` | **el hueco** que ocupa (0 a 14). Cada objeto de la escena usa uno distinto; es el número con el que lo reconocerás luego. |
| `item_taller_anvil` | el nombre que declaraste en el `.res` |
| `PAL_BACKGROUND` | qué paleta usa (`PAL_BACKGROUND` = la del fondo; es lo normal) |
| `200` | posición **x** dentro del escenario (no de la pantalla: si el nivel es ancho, cuenta desde el principio del todo) |
| `150` | posición **y** del **borde superior** del dibujo |
| `32 0 12 4` | **la caja de interacción** (ver abajo) |
| `CALCULATE_DEPTH` | delante o detrás de los personajes (ver abajo) |

### La caja de interacción (los cuatro números)

Es el rectángulo invisible donde "vive" el objeto: si el jugador está pegado a
él y pulsa A, interactúa. Los cuatro números son
`ancho`, `desplazamiento_x`, `alto`, `desplazamiento_y`, contados desde la
esquina superior izquierda del dibujo.

```
   dibujo del objeto (32 x 24)
   ┌────────────────────────────┐
   │        (offset_y = 4)      │
   │   ┌────────────────────┐   │  ← la caja: 32 de ancho, 12 de alto,
   │   │                    │   │    empezando 0 px a la derecha y 4 abajo
   │   └────────────────────┘   │
   └────────────────────────────┘
```

Consejos que ahorran disgustos:

- **Si el objeto es solo decorado**, pon la caja a `0 0 0 0`… **no**: usa
  `COLLISION_DEFAULT` poniendo los cuatro a los valores del dibujo completo, o
  déjalo sin interacción no dándole caso en el gancho. Una caja de tamaño 0 da
  problemas.
- **Las cajas que quedan a la altura de los pies (y ≥ 145) BLOQUEAN el paso.**
  Es lo que quieres en una cama o un baúl. Si el objeto debe poder atravesarse
  (un cuadro en la pared, una lámpara), **súbelo por encima de esa línea**.
- **Dos objetos pegados se pelean**: gana el más cercano al jugador. Si un
  objeto pequeño está dentro de la caja de otro grande, el grande se lo come.
  Sepáralos o encoge la caja del grande.

### Delante o detrás (el último trozo)

| Valor | Qué hace |
|---|---|
| `FORCE_BACKGROUND` | siempre **detrás** de los personajes (un cuadro, una alfombra) |
| `FORCE_FOREGROUND` | siempre **delante** (una columna en primer plano) |
| `CALCULATE_DEPTH` | **automático** según quién esté más abajo en pantalla (lo normal para muebles: el jugador pasa por detrás o por delante según dónde esté) |

---

## 4. Hacer que responda cuando el jugador pulsa A

Si el objeto es solo decoración, ya has terminado. Si quieres que **haga algo**,
necesitas un **gancho** de C: una función pequeña que se queda escuchando qué
objeto ha tocado el jugador.

En el archivo de la escena (`src/scenes/act2/taller.c`):

```c
void act2_taller_items(void)
{
    while (true) {
        switch (last_interacted_item)     // ¿con cuál ha interactuado?
        {
        case 3:  // el yunque (el hueco que le diste en el guion)
            talk_dialog(&dialogs[ACT2_TALLER][A2_TALLER_ANVIL], false);
            last_interacted_item = ITEM_NONE;   // marcarlo como atendido
            break;

        case 4:  // el martillo: al cogerlo, se acabó la exploración
            last_interacted_item = ITEM_NONE;
            return;                              // salir del bucle

        default:
            break;
        }
        next_frame(true);   // dejar correr el juego un fotograma
    }
}
```

Ideas de lo que puede hacer un objeto, con ejemplos que ya existen:

| Quieres… | Mira cómo lo hace |
|---|---|
| Que diga una frase | el retrato del dormitorio (`bedroom.c`) |
| Que diga varias seguidas | el baúl (`talk_cluster`) |
| Que diga algo distinto la segunda vez | los libros del pasillo (`corridor.c`) |
| Que enseñe un hechizo | la partitura del armario (`activate_spell`) |
| Que desaparezca al cogerlo | el bastón de la choza (`release_item`) |
| Que termine la escena | el bastón otra vez (`return` del bucle) |

El gancho hay que **registrarlo** (dos líneas en `src/scenes/scene_hooks.h` y
`.c`) y llamarlo desde el guion con `call act2_taller_items`. El proceso paso a
paso está en `docs/scenes.md`.

---

## 5. Comprobar que ha salido bien

1. Compila: `./build-theweave.sh build`. Si el nombre del sprite o del diálogo
   está mal, el build **se para y te dice dónde**.
2. Arranca directamente en tu escena poniendo su nombre en `HACK_START_SCENE`
   (`src/core/hack.h`); déjalo en `""` al terminar.
3. Camina hasta el objeto y pulsa **A**.

### Si algo va mal

| Síntoma | Casi siempre es |
|---|---|
| **No pasa nada** al pulsar A | La caja de interacción está lejos del objeto, o el `case` del gancho tiene otro número de hueco. |
| Responde **otro objeto** | Sus cajas se solapan: gana el más cercano. Sepáralas. |
| **No puedo pasar** por ahí | La caja está a la altura de los pies. Súbela por encima de y=145. |
| Aparece **encima** del personaje | Cambia `FORCE_FOREGROUND` por `CALCULATE_DEPTH`. |
| Se ve un **cuadrado de color** alrededor | El color transparente no es el primero de la paleta. |
| Colores raros | Está usando una paleta que no es la del fondo (`PAL_BACKGROUND`). |
| Interactúa **una sola vez** | Falta `last_interacted_item = ITEM_NONE;` en su `case`. |

---

## 6. Resumen: la lista completa

1. Dibujar el PNG (múltiplos de 8, transparente en el primer color, paleta del
   fondo) y guardarlo en `res/gfx/items/<acto>/`.
2. Declararlo en `res/res_items.res`.
3. Colocarlo en el guion con `item <hueco> <sprite> PAL_BACKGROUND <x> <y> <caja> <profundidad>`.
4. Si es interactivo: añadir su `case` al gancho de objetos de la escena,
   registrarlo y llamarlo con `call`.
5. Sus frases van en `data/texts.csv` (guía en `docs/texts.md`).
