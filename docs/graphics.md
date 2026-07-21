# Cómo funciona el apartado gráfico — referencia técnica

Este documento explica **cómo reparte The Weave la memoria de vídeo (VRAM) y
quién dibuja qué**: los planos, el índice de tiles, las paletas, los sprites y
el scroll. Es la referencia del dominio gráfico; para *crear* arte nuevo, cada
elemento tiene su guía (`backgrounds.md`, `characters.md`, `enemies.md`,
`items.md`).

Casi todos los errores gráficos de este proyecto han sido **uno escribiendo
encima de otro en VRAM**, no arte mal hecho. Por eso este documento empieza por
el mapa de la memoria.

---

## 1. Lo que hay que saber de la Mega Drive

La consola no dibuja píxeles sueltos: todo son **tiles de 8×8**. Hay 64 KB de
VRAM y ahí dentro conviven cinco cosas:

- Los **tiles** (los dibujos), 32 bytes cada uno.
- Los **planos**: rejillas que dicen "en esta casilla va el tile N".
- La **tabla de sprites** y la **tabla de scroll horizontal**.

The Weave usa **tres planos**:

| Plano | Para qué | Se mueve |
|---|---|---|
| **BG_B** (fondo trasero) | cielo, montañas lejanas | scroll lento (parallax) |
| **BG_A** (fondo delantero) | el escenario donde anda el jugador | scroll normal |
| **WINDOW** | el interfaz de abajo (caja de diálogo, pentagrama, vara) | **fijo, no scrollea** |

El WINDOW se activa en `initialize` con `VDP_setWindowVPos(TRUE, 22)`: ocupa de
la **fila 22 hacia abajo**, es decir de y=176 a y=224. Eso es lo que hace que el
área jugable sean los **176 primeros píxeles** (la regla que repiten las guías
de arte).

---

## 2. El mapa de la VRAM (medido en ejecución)

```
0x0000 ┌──────────────────────────────────┐
       │  TILES (0x0000..0xC000)          │  48 KB = 1536 tiles
0xC000 ├──────────────────────────────────┤
       │  plano BG_B                      │  4 KB (64x32 casillas)
0xD000 ├──────────────────────────────────┤
       │  plano WINDOW                    │  4 KB
0xE000 ├──────────────────────────────────┤
       │  plano BG_A                      │  4 KB
0xF000 ├──────────────────────────────────┤
       │  tabla de scroll horizontal      │
0xF400 ├──────────────────────────────────┤
       │  tabla de sprites                │
       └──────────────────────────────────┘
```

Lo reparte SGDK; el juego no lo toca. **Comprobar que no se solapan** es una de
las primeras cosas que mirar ante corrupción gráfica: se leen con
`VDP_getPlaneAddress(BG_A,0,0)`, `VDP_getWindowAddress()`,
`VDP_getSpriteListAddress()` y `VDP_getHScrollTableAddress()`.

### Los 1536 tiles, por dentro

```
    0 ..   15   sistema (SGDK)
   16 ..  ???   ← FONDOS del nivel (los carga new_level)
  ??? ..  ???   ← INTERFAZ (reserva fija, ver §3)
  ??? .. 1019   hueco libre
 1020 .. 1439   SPRITES (pool de SGDK, 420 tiles)
 1440 .. 1535   fuente de texto (96 tiles)
```

Ejemplo real, el bosque (el escenario más caro del juego):

| Tramo | Tiles | Qué |
|---|---|---|
| 16..847 | **832** | fondo trasero (128) + delantero (704) |
| 848..953 | **106** | interfaz (pentagrama, vara, borde) |
| 954..1019 | 66 | **libre** |
| 1020..1439 | 420 | sprites |

Es decir: en el peor escenario quedan **66 tiles de margen**. No es mucho — por
eso conviene medir antes de meter un fondo nuevo muy detallado
(`backgrounds.md` §2.4 explica cómo, y trae la tabla de todos los escenarios).

---

## 3. `tile_ind`: el índice de tiles (LA trampa principal)

`tile_ind` (global, en `core/frame.c`) es **un puntero de asignación**: marca
"el primer tile libre". Funciona como una pila que solo crece durante el montaje
del nivel.

`new_level` (`core/init.c`) lo reparte así:

```c
tile_ind = TILE_USER_INDEX;                      // 16
VDP_loadTileSet(tile_bg, tile_ind, CPU);         // fondo trasero
tile_ind += tile_bg->numTile;
VDP_loadTileSet(tile_front, tile_ind, CPU);      // fondo delantero
tile_ind += tile_front->numTile;
tile_ind = interface_reserve_tiles(tile_ind);    // interfaz: sitio FIJO
```

### La regla

> **Quien necesite VRAM de tiles la reserva UNA VEZ y se guarda su índice.
> Jamás `tile_ind += N` al mostrar algo y `tile_ind -= N` al ocultarlo.**

Enseñar u ocultar algo es **dibujar o limpiar**, no reservar y liberar. Un
recurso que aparece y desaparece debe tener su sitio reservado todo el rato.

### Por qué: el bug que costó encontrar

El interfaz hacía exactamente lo prohibido: sumaba a `tile_ind` al mostrarse y
restaba al ocultarse. Eso solo funciona si cada `show` tiene su `hide` exacto —
y no lo tenían (`act1_return_ghosts` oculta sin mostrar; `combat_start` muestra
sin ocultar).

Medido por RAM mientras se reproducía el fallo, `tile_ind` oscilaba:

```
848  →  954  →  742   ← ¡dentro de los tiles del FONDO (16..847)!
              → 1166  ← ¡dentro de la región de SPRITES!
```

Cada vez que caía a 742, el interfaz escribía sus tiles **encima del escenario**:
en el suelo del bosque aparecían el pentagrama y letras del HUD. Parecía "el
fondo llena la VRAM" y no lo era en absoluto.

**Cómo se arregló**: `interface_reserve_tiles()` aparta su hueco una vez por
nivel; `show_or_hide_interface()` dibuja siempre en el mismo índice y ocultar
solo limpia el plano WINDOW. Llamarlo de más, en cualquier orden, es inofensivo.

### Excepción tolerada

`intro.c` y `geesebumps.c` sí hacen `tile_ind += ...` y luego `-= ...`, pero son
pantallas **autocontenidas** (cargan, muestran, descargan) que corren antes del
juego y no conviven con nada. Si tocas una, mantén el equilibrio o pásalas al
patrón de reserva.

---

## 4. Las paletas

Cuatro paletas de 16 colores. Se nombran por su contenido, **nunca por su
número** (definidas en `core/config.h`):

| Ranura | Contenido | Quién la carga |
|---|---|---|
| `PAL_BACKGROUND` | el escenario | `new_level` (y el op `palette` de la escena) |
| `PAL_CHARACTERS` | personajes y sus caras | `initialize` (permanente) |
| `PAL_INTERFACE` | HUD, diálogos y texto | `initialize` (permanente) |
| `PAL_ENEMIES` | enemigos y FX con paleta propia | cada encuentro, antes de spawnear |

Detalles útiles:

- El color 4 del fondo (`PAL_BACKGROUND_COL4`) es el "cielo/ambiente": es el que
  hacen parpadear el trueno, el fuego y la luz.
- **Cambiar de paleta es gratis y no consume VRAM.** Por eso el bosque tiene dos
  (`forest.pal` día, `forest_dark.pal` noche) sobre el MISMO dibujo, y el trueno
  alterna entre ellas para simular el relámpago.
- Usa siempre la API por número de paleta (`PAL_getPalette`/`PAL_setPalette`), no
  índices crudos de CRAM.
- **Un efecto que cambia un color GUARDA el original, no lo cablea.** El mismo
  escenario tiene paletas distintas (el bosque de día y el de noche no comparten
  el color del cielo), así que restaurar un valor fijo deja el color mal. Pasó de
  verdad: el trueno enemigo reponía un azul cableado que no era el de ninguna de
  las dos, y el cielo quedaba raro tras cada ataque. Guarda con `PAL_getColor()`
  al empezar el efecto y restaura ESE.

### Cuando algo necesita una paleta propia: el caso del cisne

`PAL_CHARACTERS` la comparten Linus, Clio, Xander, sus caras, sus sombras, los
marcos del bocadillo y **el color del texto resaltado** `@[...@]` (que es el
índice 15 de esa paleta). El **cisne** es la excepción: su arte usa `swan_pal`,
que no coincide con `characters.pal` en **ninguno** de los 16 colores.

La solución adoptada tiene dos mitades:

1. **En el dormitorio**, donde el cisne es lo único que se ve, la escena carga su
   paleta (`palette PAL_CHARACTERS swan_pal` en `bedroom.scene`) y la devuelve al
   despertar (`act1_bedroom_wake`). Para que eso no rompa nada, `swan_pal` está
   **ordenada a propósito** de forma que los índices con función compartida
   coincidan con `characters.pal` (ver el recuadro de abajo).
2. **En el resto de escenas** Bobbin habla sin que su paleta esté cargada, así que
   su cara existe **repintada con `characters.pal`**
   (`swan_face_charpal_sprite`). `init_face` elige una u otra **comparando la
   paleta que hay viva** con `swan_pal`: sin banderas que mantener, y si algún día
   otra escena carga `swan_pal`, funciona sola.

> Ojo: `release_face` no limpia el `sd` de la entidad, así que un sprite elegido
> una vez se quedaría cacheado para siempre. Por eso la cara del cisne se
> reevalúa en **cada** `init_face`, no solo la primera vez. Fue justo lo que hacía
> que Bobbin saliera con los colores mal en todas las escenas posteriores al
> dormitorio.

#### La técnica: reservar los índices de función compartida

Cuando cargas una paleta propia en `PAL_CHARACTERS`, **el motor sigue usando dos
índices concretos de esa ranura** aunque tú hayas cambiado el resto:

| Índice | Para qué lo usa el motor | `characters.pal` | `swan_pal` |
|---|---|---|---|
| **13** | el color de las **sombras** | negro `(0,0,0)` | casi negro `(16,4,24)` |
| **15** | el **texto resaltado** `@[...@]` | rojo `(205,80,65)` | naranja `(246,109,65)` |

Ordenando la paleta propia para que esos dos huecos lleven colores **del mismo
tipo**, todo lo del motor sigue funcionando y solo cambian los colores del
personaje. Fue lo que se hizo con el cisne: antes tenía el casi-negro en el 15,
y sus frases resaltadas salían ilegibles sobre la caja oscura; moviéndolo al 13
(donde hace falta oscuro, para las sombras) y poniendo el naranja en el 15, los
resaltes de Bobbin funcionan igual que los de cualquiera.

**Regla para el arte**: si un elemento necesita paleta propia, deja los índices
13 y 15 con un oscuro y un color de resalte legible.

---

## 5. Los sprites

Los gestiona el motor de sprites de SGDK (`SPR_*`), con su **propia región de
VRAM** (420 tiles por defecto, 1020..1439). Es un presupuesto **aparte** del de
los fondos: llenar uno no afecta al otro.

Lo que más ocupa:

| Sprite | Tiles por fotograma |
|---|---|
| cualquier **cara** de diálogo (y sus dos marcos) | 64 |
| Linus (cualquiera de sus tres formas) | 48 |
| Xander | 48 |
| Clio | 32 |
| jabalí | 24 |
| sombras | 4–6 |

Con caras + protagonistas + una manada se roza el límite, así que las caras se
**liberan al terminar de hablar** (`release_face`). Si un día aparece basura en
los sprites, comprueba primero si `SPR_addSpriteSafe` está devolviendo `NULL`
(eso indica que la región se agotó).

Reglas del motor de sprites que ya nos han mordido:

- **`SPR_update()` no va dentro de un helper por-entidad.** El latido normal es
  el de `next_frame`, uno por frame, y con eso basta: `update_enemy()` llegó a
  llamarlo por enemigo y el framerate se hundió de 50 a ~25 FPS con 5 jabalíes
  (por eso `enemies.c` lleva un comentario avisando). Sí son legítimas las
  llamadas **puntuales** fuera del bucle de frames — tras liberar un sprite o al
  montar una cutscene — porque ocurren una vez, no sesenta veces por segundo.
- **`SPR_setAnim` con un índice que la hoja no tiene cuelga el juego** dentro de
  `SPR_update`. Si un personaje solo tiene 5 filas, no le pidas `ANIM_MAGIC`.
- Un `Sprite*` liberado no se vuelve a usar: `release_*` pone el puntero a NULL.

---

## 6. El scroll y los fondos grandes

Los fondos son mucho más anchos que la pantalla (el bosque, 1440 px), pero el
plano solo mide 64 casillas y **da la vuelta**. SGDK resuelve esto con el sistema
`MAP`:

- `MAP_create(...)` prepara el mapa con un **índice base** (el `tile_ind` donde
  se cargaron sus tiles).
- `MAP_scrollTo(...)`, cada frame, **va inyectando las columnas nuevas** en el
  plano según avanzas.

Consecuencias prácticas:

- El ancho del fondo **no** tiene que ser múltiplo de 128 ni de nada especial
  (solo de 8). El 128 aparece porque el formato `MAP` agrupa el mapa en bloques
  de 128×128 px y deduplica los repetidos: es eficiencia de ROM, no un requisito.
- **No muevas la cámara escribiendo `offset_BGA` a mano.** Saltarse
  `scroll_background()` deja el plano con tiles obsoletos y produce corrupción
  falsa. Ya nos costó un diagnóstico entero equivocado (`retroarch-mcp.md` lo
  documenta como trampa de verificación).

---

## 7. Ante corrupción gráfica: por dónde empezar

Por orden, de lo más probable a lo menos:

1. **¿Alguien está escribiendo encima?** Mira `tile_ind` en ejecución (§3). Si
   se sale de su hueco, ya tienes al culpable. Es lo que ha pasado siempre.
2. **¿Qué se ve en la basura?** Es la mejor pista: si reconoces letras, es la
   fuente o el HUD; si reconoces un sprite, es la región de sprites.
3. **¿Se agotó la región de sprites?** Comprueba si `SPR_addSpriteSafe` devuelve
   `NULL`.
4. **¿Se solapan los planos?** Lee las direcciones (§2).
5. **¿No cabe el fondo?** Cuenta sus tiles únicos (`backgrounds.md` §2.4). Ojo:
   si no cupiera, el **build fallaría** al compilar el mapa; que compile no
   garantiza que quepa junto a todo lo demás, pero descarta el caso extremo.
6. **¿Es artefacto de tu método de prueba?** Si estás moviendo cosas por RAM
   desde el emulador, sospecha de ti antes que del juego (§6).

Herramienta clave: leer variables en vivo con RetroArch (`retroarch-mcp.md`).
Ojo con LTO — una variable de diagnóstico que no sea `volatile` desaparece del
binario y no la encontrarás en `out/symbol.txt`.

---

## 8. Cinemáticas dentro de un gancho: dejar quietos a los personajes

Un gancho que anima algo por su cuenta (el vuelo de la gaviota en la playa)
corre su propio bucle con `next_frame(false)`: no se lee el mando, así que el
jugador no puede moverse. Pero **quien viniera andando se queda con la
animación de andar congelada** todo lo que dure la escena, porque
`update_character_animations()` sí sigue corriendo y respeta el estado.

Por eso, antes de empezar la animación:

```c
idle_all_characters();   // todos quietos y en reposo
```

Hay dos funciones parecidas y conviene no confundirlas:

| Función | Qué hace | Cuándo |
|---|---|---|
| `idle_all_characters()` | pone a **todos** en `STATE_IDLE` **y** con `ANIM_IDLE` | cinemáticas de un gancho |
| `reset_character_animations()` | pone en reposo a todos **menos al personaje activo**, y solo el estado | diálogos y spawns de enemigos, donde el protagonista puede estar lanzando un hechizo y no se le debe tocar |

### Poses que tienen que AGUANTAR: `STATE_FIXED_ANIM`

Poner una animación con `anim_character()` no basta si debe mantenerse un rato,
porque el motor la deshace por dos vías:

1. `update_character_animations()` devuelve a `ANIM_IDLE` en cuanto la animación
   termina, si el personaje está en `STATE_IDLE`.
2. `talk_dialog()` llama a `reset_character_animations()`, que pone en reposo a
   todos menos al activo — o sea, **el propio diálogo deshace la pose**.

Para eso está `STATE_FIXED_ANIM`: en ese estado **la escena es dueña de la
animación** y el motor no la toca (mismo principio que el `STATE_WALKING` de los
enemigos durante el combate de contacto). Con dos ayudantes:

```c
set_character_anim(CHR_clio, ANIM_WOUNDED);  // fija la pose; aguanta los diálogos
...
release_character_anim(CHR_clio);            // la suelta: vuelve a idle
```

Caso real: Clio herida tras el mordisco del jabalí (`act1_fday_bite`), que debe
seguir en el suelo durante todo el diálogo hasta que canta Curación.

---

## 9. Mapa de archivos

| Archivo | Qué hace |
|---|---|
| `src/core/init.c` | `new_level`: carga fondos, reparte `tile_ind`, fija paletas |
| `src/core/frame.c` | `tile_ind` y el latido `next_frame` (un `SPR_update` por frame) |
| `src/world/background.c` | scroll: `update_bg`, `scroll_background`, límites |
| `src/interface/interface.c` | HUD en el plano WINDOW + `interface_reserve_tiles` |
| `src/actors/characters.c` · `enemies.c` · `items.c` | alta/baja de sprites y animaciones |
| `src/core/config.h` | `PAL_BACKGROUND` y compañía |
