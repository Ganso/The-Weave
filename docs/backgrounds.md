# Cómo crear un fondo (escenario) nuevo — guía completa

El **fondo** es el escenario por el que anda el jugador: el dormitorio, el
bosque, la costa. Esta guía va del dibujo hasta verlo en pantalla con el
personaje paseando, sin saber programar.

---

## 1. Cómo está hecho un escenario en The Weave

Cada escenario son **dos dibujos superpuestos** (dos "capas") y una **paleta**:

```
        ┌──────────────────────────────────────┐
        │   CAPA DE ATRÁS (back)               │  cielo, montañas lejanas, luna
        │      ┌───────────────────────────┐   │
        │      │  CAPA DE DELANTE (front)  │   │  árboles, suelo, paredes
        │      │   ← aquí anda el jugador  │   │
        └──────┴───────────────────────────┴───┘
```

Las dos capas **se mueven a distinta velocidad** cuando el jugador camina: la de
atrás más despacio, y eso crea la sensación de profundidad (*parallax*). La capa
de delante es la que "toca" el jugador.

> **Puedes tener solo una capa.** Si tu escenario no necesita profundidad,
> repite el mismo dibujo en las dos o deja la de atrás en negro.

---

## 2. El arte

### 2.1 Tamaño y forma

- **Altura: siempre 240 píxeles.**
- **Anchura: la que quieras**, en múltiplos de 8. Si es más ancha que la pantalla
  (320 px), el escenario **se desplaza** cuando el jugador camina.
- Ejemplo real: el bosque son 1440×240 la capa de delante y 720×240 la de atrás
  (la mitad de ancha, porque se mueve a la mitad de velocidad).

### 2.2 **La regla más importante: el área jugable**

De los 240 píxeles de alto, **solo los 176 primeros (y = 0 a 175) son la zona de
juego**. Todo lo que quede **por debajo de y = 176** queda tapado por la caja de
diálogos, así que:

- En la capa **de delante**: de y=176 para abajo, **transparente**.
- En la capa **de atrás**: de y=176 para abajo, **negro**.

```
  y=0    ┌────────────────────────────────┐
         │                                │
         │      ZONA DE JUEGO             │  ← aquí ocurre todo
         │      (el suelo suele estar      │
         │       hacia y=150-175)         │
  y=175  ├────────────────────────────────┤
  y=176  │  transparente (front)          │  ← lo tapa el interfaz
  y=240  │  negro (back)                  │
         └────────────────────────────────┘
```

### 2.3 Los colores: la paleta

Un escenario usa **una paleta de 16 colores** (contando el transparente). Puede
ser:

- **La del propio PNG** — lo más fácil: el juego la saca del dibujo.
- **Un archivo `.pal` aparte** — útil cuando **el mismo escenario tiene varias
  iluminaciones**. El bosque tiene `forest.pal` (día) y `forest_dark.pal`
  (noche): el mismo dibujo, dos ambientes, cambiando solo la paleta. El
  dormitorio hace lo mismo con día y noche.

> Truco que ya usa el juego: si vas a querer un efecto de "relámpago" o de
> anochecer, **prepara dos paletas del mismo dibujo** desde el principio. Cambiar
> la paleta es instantáneo y no cuesta memoria.

### 2.4 **El aviso importante: los tiles**

La consola no dibuja el fondo píxel a píxel: lo trocea en **cuadraditos de 8×8**
("tiles") y **guarda solo los que son distintos**. Un fondo con mucho ruido o
degradados suaves genera miles de cuadraditos únicos y **no cabe en la memoria de
vídeo**.

Para que quepa:

- **Repite elementos**: si dibujas cuatro árboles iguales, ocupan lo que uno.
- **Evita degradados y texturas ruidosas**: usa zonas de color plano.
- Si necesitas suavizar un degradado, usa **tramas regulares** (el patrón de
  puntos ordenado tipo Bayer), no aleatorias.
- **Alinea los elementos a la rejilla de 8 píxeles** siempre que puedas.

Si te pasas, el build falla con un aviso de que no puede compilar el mapa.

**Cuánto margen tienes**: caben del orden de **950 cuadraditos** entre las dos
capas. Para hacerte una idea, estos son los fondos actuales (contando ya los
repetidos y los volteados, que no cuentan doble):

| Fondo | Únicos | Reutilización |
|---|---|---|
| bosque (delante + detrás) | 704 + 128 = **832** | 7,7× |
| costa | 189 + 80 = 269 | 12,7× |
| choza | 256 + 67 = 323 | 8,8× |
| dormitorio | 551 + 107 = 658 | 1,7× |

El bosque es el más caro y **aun así entra y funciona** (verificado recorriéndolo
entero). Si tu fondo se acerca a esa cifra, aplica los consejos de arriba.

### Dónde va

```
res/gfx/backgrounds/act2/taller_bg_front.png
res/gfx/backgrounds/act2/taller_bg_back.png
res/gfx/backgrounds/act2/taller.pal          (si usas paleta aparte)
```

---

## 3. Declarar el arte: `res/res_backgrounds.res`

Cada capa necesita **dos líneas** (el juego guarda por separado los cuadraditos y
el plano de cómo se colocan), más la paleta:

```
# Act 2 - Scene 1 - Taller del herrero
TILESET taller_front_tile "gfx/backgrounds/act2/taller_bg_front.png" BEST
MAP     taller_front_map  "gfx/backgrounds/act2/taller_bg_front.png" taller_front_tile BEST
TILESET taller_bg_tile    "gfx/backgrounds/act2/taller_bg_back.png"  BEST
MAP     taller_bg_map     "gfx/backgrounds/act2/taller_bg_back.png"  taller_bg_tile BEST
PALETTE taller_pal        "gfx/backgrounds/act2/taller.pal"
```

- `TILESET` = la colección de cuadraditos; `MAP` = el plano que dice dónde va
  cada uno. **Los dos apuntan al mismo PNG** y el `MAP` nombra a su `TILESET`.
- Si prefieres sacar la paleta del propio dibujo, apunta el `PALETTE` al PNG en
  vez de a un `.pal`.
- Los nombres (`taller_front_tile`, etc.) son los que usarás en el guion.

---

## 4. Usarlo en la escena

**No hace falta tocar código.** En la primera línea del bloque de montaje del
guion (`data/scenes/act2/taller.scene`):

```
level taller_bg_tile taller_bg_map taller_front_tile taller_front_map taller_pal 960 user_right 3
```

Los trozos, en orden:

| Trozo | Qué es |
|---|---|
| `taller_bg_tile taller_bg_map` | la capa **de atrás** (cuadraditos y plano) |
| `taller_front_tile taller_front_map` | la capa **de delante** |
| `taller_pal` | la paleta |
| `960` | **la anchura total** del escenario en píxeles |
| `user_right` | cómo se desplaza (ver abajo) |
| `3` | velocidad del desplazamiento |

### Los cuatro modos de desplazamiento

| Modo | Qué hace | Cuándo usarlo |
|---|---|---|
| `user_right` | El jugador avanza **hacia la derecha** | Lo normal |
| `user_left` | El jugador avanza **hacia la izquierda**, empezando por el extremo derecho | Volver por donde viniste (el regreso del acto 1) |
| `auto_right` | Se desplaza solo hacia la derecha | Escenas de viaje |
| `auto_left` | Se desplaza solo hacia la izquierda | Ídem |

> Con `user_left` el escenario **empieza mostrando el extremo derecho** y el
> número que veas en las esperas va **bajando** (usa `wait_scroll_left`).

### Poner el suelo y los bordes

Justo después conviene decir **por dónde puede caminar** el jugador:

```
limits 0 140 960 175
```

Son las cuatro esquinas de la franja andable: `x_min y_min x_max y_max`. En el
ejemplo, el jugador puede moverse por todo el ancho y entre las alturas 140 y 175
(la parte baja del escenario, como si fuera el suelo).

### Cambiar la iluminación a mitad de escena

Si preparaste dos paletas, cambiar de una a otra es una sola orden:

```
palette PAL_BACKGROUND taller_night_pal
```

El juego reparte las cuatro paletas de la consola así, y cada una se nombra por
lo que contiene (nunca por su número):

| Ranura | Para qué |
|---|---|
| `PAL_BACKGROUND` | el fondo del escenario (la carga `level`) |
| `PAL_CHARACTERS` | los personajes y sus caras |
| `PAL_INTERFACE` | el HUD y las cajas de diálogo |
| `PAL_ENEMIES` | los enemigos y los efectos con paleta propia |

---

## 5. Comprobar que ha salido bien

1. Compila: `./build-theweave.sh build`. Si el fondo tiene demasiados cuadraditos
   distintos, **el build falla aquí** con un error de mapa: vuelve al punto 2.4.
2. Arranca directo en tu escena con `HACK_START_SCENE` (`src/core/hack.h`).
3. Camina de un extremo a otro y comprueba el desplazamiento y los límites.

### Si algo va mal

| Síntoma | Casi siempre es |
|---|---|
| El build dice que **no puede compilar el MAP** | Demasiados cuadraditos únicos: simplifica el dibujo (punto 2.4). |
| **Basura de colores** en pantalla | Lo mismo: el fondo no cabe en memoria de vídeo. |
| Se ve el fondo **por debajo del interfaz** | No dejaste transparente/negro de y=176 para abajo. |
| Colores raros en **personajes** | Tu paleta se cargó donde no debía: el fondo va en `PAL_BACKGROUND`. |
| El escenario **no se desplaza** | La anchura del `level` es 320 o menor, o el modo de scroll no es el que crees. |
| El jugador **anda por el cielo** | Los `limits` están mal: baja `y_min` e `y_max`. |
| Con `user_left` **no pasa nada** al esperar | Estás usando `wait_scroll` en vez de `wait_scroll_left`. |

---

## 6. Resumen: la lista completa

1. Dibujar las dos capas a **240 px de alto**, dejando **de y=176 abajo**
   transparente (delante) y negro (detrás).
2. Vigilar la cantidad de cuadraditos distintos: repite elementos, colores
   planos, tramas regulares.
3. Guardar en `res/gfx/backgrounds/<acto>/` y, si quieres, una o varias `.pal`.
4. Declarar `TILESET` + `MAP` de cada capa y el `PALETTE` en
   `res/res_backgrounds.res`.
5. En el guion: `level ...` con la anchura y el modo de scroll, y `limits ...`
   para la franja andable.
6. Cambios de iluminación a mitad de escena con `palette PAL_BACKGROUND <otra_paleta>`.
