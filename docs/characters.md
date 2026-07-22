# Cómo crear un personaje nuevo — guía completa

Esta guía te lleva de principio a fin: **desde el dibujo hasta verlo andar y
hablar en una escena**, aunque no sepas programar. Un "personaje" es alguien que
aparece en el mundo y puede moverse, animarse y hablar con su retrato: Linus,
Clio, Xander, el cisne. (Los **enemigos** son otra cosa y tienen su propia guía;
los **objetos** del decorado, también.)

---

## 1. Qué necesitas antes de empezar

Un personaje completo son **tres piezas de arte** y unos pocos apuntes en el
código:

| Pieza | Qué es | ¿Obligatoria? |
|---|---|---|
| La **hoja de sprites** | El dibujo del personaje con todas sus animaciones | Sí |
| La **sombra** | Una manchita ovalada bajo los pies | No (el cisne no tiene) |
| La **cara** | El retrato que sale en los diálogos | Solo si habla |

---

## 2. El arte

### 2.1 La hoja de sprites (el personaje en sí)

Es **una sola imagen PNG** con todos los fotogramas ordenados en una rejilla:
cada **fila es una animación** y cada **columna un fotograma** de esa animación.

El juego espera las filas **en este orden**:

| Fila | Animación | Para qué se usa |
|---|---|---|
| 0 | `IDLE` | quieto, respirando |
| 1 | `WALK` | andando |
| 2 | `ACTION` | acción genérica (golpear, señalar, tocar notas) |
| 3 | `MAGIC` | lanzar un hechizo |
| 4 | `HURT` | recibir daño |
| 5 | *extra* | opcional; su significado lo decide cada personaje: agacharse a coger algo, correr, o una **pose sostenida** (Clio herida en el suelo tras el mordisco) |

Reglas de oro:

- **Todos los fotogramas miden lo mismo** y la rejilla es regular (si un
  fotograma es de 48×48, todos lo son).
- El **ancho y el alto en píxeles deben ser múltiplos de 8** (la consola dibuja
  en bloques de 8×8).
- **El personaje mira a la DERECHA.** El juego voltea el dibujo solo cuando anda
  hacia la izquierda. Si lo dibujas mirando a la izquierda, saldrá andando de
  espaldas: es el error más típico. Compruébalo ampliando la cara.
- El **color de fondo** (el que quieres que sea transparente) tiene que ser el
  **primer color de la paleta**.
- Si una fila tiene menos fotogramas que las demás, **repite el último** hasta
  completar la rejilla.

> Si la fila extra es una **pose que debe mantenerse** (un personaje herido que
> sigue en el suelo mientras hablan), díselo a quien programe: hace falta fijarla
> para que ni el motor ni los diálogos la deshagan. Ya está resuelto, es una
> línea de código.

Ejemplo real: `res/gfx/characters/linus.png` mide 816×384 con fotogramas de
136×48 (6 columnas × 8 filas).

### 2.2 La paleta de colores

Los personajes **comparten una paleta común**: `res/gfx/characters/characters.pal`
(16 colores). Dibuja tu personaje usando **solo esos colores** y todo encajará
sin tocar nada más.

> Si tu personaje necesita **colores propios** (como el cisne, que tiene su
> `swan_pal`), se puede: la escena carga su paleta al mostrarlo y la devuelve
> después. Pero deja dos huecos con el color que toca, porque el juego los usa
> para otras cosas aunque cambies el resto:
>
> - **el índice 13** debe ser un color **oscuro** (es el de las sombras),
> - **el índice 15**, un color **legible sobre la caja de diálogo** (es el de las
>   palabras resaltadas con `@[...@]`).
>
> Con eso hecho, todo funciona igual. Es lo que se hizo con el cisne.

### 2.3 La sombra

Una imagen aparte, del mismo ancho que un fotograma y **muy bajita** (una tira),
con una elipse oscura. El juego la coloca solo bajo los pies. Mira
`res/gfx/characters/linus_shadow.png` como plantilla.

### 2.4 La cara para los diálogos

Un PNG de **64×64** con el retrato de hombros para arriba, en
`res/gfx/faces/<nombre>_face.png`. Sale dentro del marco del bocadillo, así que
deja un poco de aire alrededor.

### Dónde se guarda cada cosa

```
res/gfx/characters/<nombre>.png          la hoja de sprites
res/gfx/characters/<nombre>_shadow.png   la sombra
res/gfx/faces/<nombre>_face.png          la cara de los diálogos
```

---

## 3. Declarar el arte: `res/res_characters.res` y `res/res_faces.res`

Estos archivos son la **lista de la compra** del juego: le dicen qué imágenes
cargar y con qué nombre las conocerá el código. Añade tus líneas al final del
bloque que toque.

En `res/res_characters.res`:

```
# Aiden
SPRITE aiden_sprite "gfx/characters/aiden.png" 6 8 BEST 4
SPRITE aiden_shadow_sprite "gfx/characters/aiden_shadow.png" 6 1 BEST
```

Qué significa cada cosa, en orden:

| Trozo | Qué es |
|---|---|
| `SPRITE` | el tipo de recurso |
| `aiden_sprite` | **el nombre que usará el código** (acaba siempre en `_sprite`) |
| `"gfx/..."` | la ruta de tu PNG (desde la carpeta `res/`) |
| `6 8` | el tamaño de UN fotograma **en bloques de 8 píxeles**: 6×8 = 48×64 px |
| `BEST` | cómo comprimirlo (deja `BEST` siempre) |
| `4` | fotogramas por segundo de la animación (a mayor número, más rápida) |

> El tamaño se escribe en bloques de 8: si tu fotograma mide 48×64 píxeles,
> escribes `6 8` (48÷8 y 64÷8).

En `res/res_faces.res`:

```
SPRITE aiden_face_sprite "gfx/faces/aiden_face.png" 8 8 BEST
```

(Las caras son siempre `8 8`, o sea 64×64.)

---

## 4. Darlo de alta en el código

Son **tres apuntes pequeños**, todos en archivos que ya existen. Copia el patrón
del personaje más parecido al tuyo.

### 4.1 Reservarle un número — `src/actors/characters.h`

```c
#define MAX_CHR       5      // ← súbelo en uno
#define CHR_linus     0
#define CHR_clio      1
#define CHR_xander    2
#define CHR_swan      3
#define CHR_aiden     4      // ← tu personaje, con el siguiente número libre
```

### 4.2 Decirle qué sprite le toca — `src/actors/characters.c`

Dentro de `init_character`, en la lista de casos:

```c
case CHR_aiden:
    nsprite = &aiden_sprite;
    nsprite_shadow = &aiden_shadow_sprite;
    speed = FASTFIX32_FROM_INT(3) / 4;   // 0,75 píxeles por fotograma
    break;
```

- `speed` es lo rápido que anda. Referencias: Linus `3/2` (1,5 px), Clio y
  Xander `3/4` (0,75 px).
- `speed` marca la velocidad **horizontal**. El movimiento **vertical** usa la
  mitad de ese valor (`step >> 1` en `handle_character_movement`), porque la
  zona jugable en Y es corta y a igual velocidad se sentía demasiado rápido.
- Si **no lleva sombra**, pon `drops_shadow = false;` y no asignes
  `nsprite_shadow` (mira el caso del cisne).

### 4.3 Decirle qué cara le toca — `src/actors/characters.c`

Dentro de `init_face`, en su lista de casos:

```c
case CHR_aiden:
    nsprite = &aiden_face_sprite;
    break;
```

Y en el mismo `src/actors/characters.h`, junto a los `CHR_*`, hay una lista
paralela de caras: dale su `FACE_aiden` con el mismo número (mira cómo está
`FACE_xander`). Ese es el nombre que usarás en la tabla de textos.

---

## 5. Sacarlo en una escena

Ya está todo listo: ahora es **solo escribir el guion** de la escena
(`data/scenes/<acto>/<escena>.scene`). No hace falta tocar más código.

```
character CHR_aiden              # crearlo y prepararlo
move_instant CHR_aiden 200 154   # colocarlo (x, y del suelo donde pisa)
show CHR_aiden on                # hacerlo visible
look CHR_aiden left              # mirando a la izquierda (por defecto, derecha)

say ACT2_TALLER A2_AIDEN_HOLA sound   # que hable (con su cara)
move CHR_aiden 120 154                # que camine hasta ahí
anim CHR_aiden ANIM_MAGIC             # ponerle una animación concreta
```

Otras órdenes útiles:

- `active CHR_aiden` — pasa a ser **el personaje que controla el jugador**.
- `follow CHR_aiden on` — hace que **siga** al personaje activo (como Clio).
- `show CHR_aiden off` — lo oculta.

Para que **hable**, añade sus frases a `data/texts.csv` con `FACE_aiden` en la
columna de la cara (guía completa en `docs/texts.md`).

---

## 6. Comprobar que ha salido bien

1. Compila: `./build-theweave.sh build`.
2. Si algo del arte está mal declarado, el build **se para y te dice el archivo
   y la línea**.
3. Para probar la escena sin jugar el juego entero, abre `src/core/hack.h` y
   pon el nombre de tu escena en `HACK_START_SCENE`; al terminar, déjalo en `""`.

### Si algo va mal

| Síntoma | Casi siempre es |
|---|---|
| Anda **de espaldas** | La hoja de sprites mira a la izquierda: voltéala. |
| Colores raros | No usaste la paleta común `characters.pal`. |
| Fondo del sprite visible (un cuadrado) | El color transparente no es el primero de la paleta. |
| Se queda **congelado** en un fotograma | La fila de esa animación está incompleta: repite el último fotograma. |
| El juego se **cuelga** al animarlo | Pediste una animación que esa hoja no tiene (`ANIM_MAGIC` en un personaje de 2 filas). |
| Aparece **flotando** o hundido | El `y` de `move_instant` es la línea del **suelo**, no la cabeza. |

---

## 7. Resumen: la lista completa

1. Dibujar la hoja de sprites (**mirando a la derecha**, rejilla regular,
   múltiplos de 8, paleta `characters.pal`), la sombra y la cara.
2. Guardarlas en `res/gfx/characters/` y `res/gfx/faces/`.
3. Declararlas en `res/res_characters.res` y `res/res_faces.res`.
4. Reservar su `CHR_*` y subir `MAX_CHR` (`characters.h`).
5. Añadir su caso en `init_character` y en `init_face` (`characters.c`).
6. Añadir su `FACE_*` si habla, y sus frases en `data/texts.csv`.
7. Usarlo en el guion de la escena con `character` / `move` / `show` / `say`.
