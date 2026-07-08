# Cómo escribir los textos del juego — guía completa

Esta guía explica, **con palabras normales y sin necesidad de saber programar**, cómo
se escriben todas las frases que dicen los personajes, las preguntas que se le hacen
al jugador y las vocecillas que suenan cuando alguien habla. Todo lo que necesitas
está aquí: no hace falta leer ningún otro documento.

> Todos los textos del juego se escriben en **dos archivos de tabla muy sencillos**
> (los puedes abrir con cualquier hoja de cálculo o editor de texto). No se toca nada
> de programación.

---

## 1. Las dos tablas

- **`data/texts.csv`** — todas las **frases** que dicen los personajes (y los mensajes
  del sistema).
- **`data/choices.csv`** — todas las **preguntas con opciones** que el jugador elige.

Ambas son archivos "CSV": tablas de texto donde cada línea es una fila y las columnas
van separadas por **comas**. La primera línea de cada archivo es la cabecera (los
nombres de las columnas); a partir de ahí, cada línea es una entrada.

> Como las columnas se separan por comas, **evita escribir comas dentro de una frase**
> salvo que sepas lo que haces (romperían la fila). Si necesitas una pausa, casi
> siempre puedes reescribir la frase o usar un salto de línea (ver más abajo).

---

## 2. Los diálogos: `data/texts.csv`

Cada fila de esta tabla es **una frase** que dice un personaje. Las columnas, en
orden, son:

```
set, id, face, side, time, es, en
```

### 2.1. Qué significa cada columna

- **set** — el **grupo** al que pertenece la frase, normalmente **una escena**. Se
  escribe en minúsculas: `act1_hall`, `act1_bedroom`, `act1_forest`... Hay un grupo
  especial, `system_dialog`, para los mensajes del propio juego (por ejemplo, "no
  puedo usar ese patrón aquí").

- **id** — el **nombre único** de esta frase dentro de su grupo. Se escribe en
  MAYÚSCULAS, con el formato `A<acto>_<ESCENA>_<DE_QUÉ_VA>`. Por ejemplo,
  `A1_BEDROOM_SLEPT_BAD` ("dormí mal", en el dormitorio del acto 1). Los mensajes del
  sistema usan el prefijo `SYSMSG_` (por ejemplo `SYSMSG_DEMO_TITLE`). Piensa en el id
  como "la etiqueta de la frase": es el nombre con el que la escena la llamará.

- **face** — qué **cara** (retrato) se muestra al hablar:
  - `FACE_linus`, `FACE_clio`, `FACE_xander`, `FACE_swan` — el retrato de ese personaje.
  - `FACE_none` — sin retrato (para textos ambientales, del tipo "A la mañana
    siguiente...").

- **side** — en qué **lado** de la pantalla aparece el retrato y el texto:
  - `SIDE_LEFT` (izquierda), `SIDE_RIGHT` (derecha) o `SIDE_NONE` (que se comporta como
    la izquierda).
  - La costumbre: el personaje que "tiene la palabra" a la izquierda y el que le
    responde a la derecha, para que se note quién habla.

- **time** — cuántos **segundos, como máximo**, se queda el texto en pantalla antes de
  avanzar solo (el jugador también puede avanzar pulsando). Casi siempre se pone
  `DEFAULT_TALK_TIME`, que son 10 segundos.

- **es** — el texto en **español**.

- **en** — el mismo texto en **inglés**.

### 2.2. Cómo se escribe el texto (español e inglés)

- **Los caracteres españoles se escriben NORMALES.** La `ñ`, las vocales con tilde
  (`á`, `é`, `í`, `ó`, `ú`), la `ü`, y los signos `¿` y `¡` se teclean tal cual. El
  juego los convierte solo a su tipografía. **Nunca** los sustituyas por letras sin
  acento (no escribas "nino" en vez de "niño").

- **Salto de línea:** para partir una frase en dos renglones, usa la **barra vertical**
  `|`. Por ejemplo, `Es tarde, Linus|no debes retrasarte` se verá en dos líneas.

- **Resaltar una palabra:** enciérrala entre `@[` y `@]` para que salga con un color
  distinto. Por ejemplo, `Un @[cisne@] venía a mi cuarto` resalta "cisne".

- **Longitud:** procura no pasar de unos **30 caracteres por renglón**, o el texto se
  saldrá de la pantalla. Si una frase es larga, pártela con `|`.

- El **español y el inglés** son independientes: cada uno con sus propios saltos de
  línea y resaltados, según convenga a cada idioma.

Ejemplo de una fila completa:

```
act1_bedroom,A1_BEDROOM_SLEPT_BAD,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,He dormido|fatal,I slept|terribly
```

---

## 3. Los "clusters": varias frases seguidas

Muchas veces un personaje suelta **varias frases de corrido**. En lugar de invocarlas
una a una, se agrupan en un **cluster** y se muestran todas seguidas con una sola
orden desde la escena.

Un cluster es, sencillamente, **todas las filas desde una frase hasta la siguiente
fila separadora**. Una fila separadora es una fila cuyo **id es `NULL`** (las demás
columnas se dejan casi vacías). La fila `NULL` marca **el final del grupo**.

```
act1_claro,A1_CLARO_ARRIVE,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Un claro en el bosque|El aire está quieto,A forest clearing|The air is still
act1_claro,A1_CLARO_CLIO,FACE_clio,SIDE_RIGHT,DEFAULT_TALK_TIME,¿Exploramos|o seguimos?,Do we explore|or move on?
act1_claro,NULL,,,,,
```

Aquí, un cluster que empiece en `A1_CLARO_ARRIVE` mostrará esa frase **y** la de
`A1_CLARO_CLIO`, y se parará al llegar a la fila `NULL`. Si quieres separar frases en
grupos distintos (para mostrarlas en momentos diferentes de la escena), mete una fila
`NULL` entre medias.

> Regla práctica: **cierra siempre cada grupo de frases con una fila `NULL`**. Si te
> falta, un cluster seguirá "tragándose" las frases del siguiente grupo; si te sobra,
> partirás un grupo que querías junto.

---

## 4. Las preguntas al jugador: `data/choices.csv`

Cuando el jugador tiene que **elegir** entre varias respuestas, esas opciones se
definen en la segunda tabla, `data/choices.csv`. Sus columnas son:

```
set, item, face, side, time, es_1, es_2, es_3, es_4, en_1, en_2, en_3, en_4
```

- **set** — el grupo, con el mismo criterio que los diálogos pero con el sufijo
  `_choice`. Por ejemplo `act1_hall_choice`.
- **item** — el **número** de la pregunta dentro del grupo (`0`, `1`, `2`...),
  consecutivos. Un mismo grupo puede tener varias preguntas.
- **face / side / time** — igual que en los diálogos: quién pregunta, en qué lado y
  cuánto tiempo.
- **es_1 … es_4** — hasta **cuatro** opciones en español.
- **en_1 … en_4** — las mismas cuatro opciones en inglés.

Si una pregunta solo tiene 2 ó 3 opciones, **deja vacías** las columnas sobrantes. El
juego cuenta cuántas hay. Eso sí: el español y el inglés deben tener el **mismo
número** de opciones.

Ejemplo (una pregunta con dos opciones):

```
act1_claro_choice,0,FACE_linus,SIDE_RIGHT,DEFAULT_CHOICE_TIME,Explorar el claro,Seguir de largo,,,Explore the clearing,Move on,,
```

(Para las preguntas, el tiempo máximo suele ser `DEFAULT_CHOICE_TIME`.)

Cuando el jugador elige, el juego **recuerda qué opción marcó**: la primera opción es
la número 0, la segunda la 1, y así. La escena reacciona a esa elección (ver la
sección siguiente).

---

## 5. Cómo se usan los textos desde una escena

No necesitas programar nada para usar estos textos: se invocan desde el **guion** de
la escena (el archivo `.scene`) con órdenes muy sencillas. Un resumen de las que
tienen que ver con los textos:

- **`say SET ID [sound]`** — muestra **una** frase (la del `ID` en el grupo `SET`). Si
  añades `sound` al final, suena la voz mientras habla; si no, aparece en silencio.
  Ejemplo: `say ACT1_BEDROOM A1_BEDROOM_SLEPT_BAD sound`.

- **`say_cluster SET ID [sound]`** — muestra **varias** frases seguidas: empieza en
  `ID` y sigue hasta la fila `NULL` (el cluster de la sección 3).

- **`choice SET item`** — muestra una **pregunta** (la número `item` del grupo `SET`) y
  espera a que el jugador elija. La elección queda recordada.

- **`say_response SET BASE [sound]`** — muestra la **respuesta que corresponde a la
  opción elegida**: si el jugador eligió la opción 0, muestra la frase `BASE`; si
  eligió la 1, la frase **siguiente** a `BASE` en la tabla; etc. Por eso, para usar
  `say_response`, las frases de respuesta deben estar **una detrás de otra** en la
  tabla de textos, en el mismo orden que las opciones de la pregunta.

  Si tus respuestas no están correlativas, en lugar de `say_response` se usa `branch`
  (que salta a distintas partes del guion según la opción) y un `say` en cada rama.

---

## 6. Después de editar: nada especial

Al **construir el juego** (el comando de compilación), las dos tablas se procesan
automáticamente y sus textos quedan metidos en el juego. No tienes que hacer ningún
paso extra.

Al procesarlas, el juego **revisa que todo encaje**: si una escena nombra una frase o
una pregunta que no existe (un id mal escrito, por ejemplo), la construcción se
detiene y te dice **el archivo y la línea** del problema, para que no tengas que
adivinar dónde está el fallo.

Errores típicos y cómo se notan:

- **Una frase no aparece, o sale otra distinta:** el `id` está mal escrito, o en un
  cluster faltó (o sobró) una fila `NULL`, o en un `say_response` las respuestas no
  estaban una detrás de otra en la tabla.
- **Se ven letras raras en lugar de acentos:** alguien sustituyó los caracteres
  españoles por versiones sin acento, o el archivo no se guardó como texto normal
  (UTF-8). Escribe siempre `ñ`, `á`, `¿`... tal cual.
- **El texto se sale de la pantalla:** hay renglones de más de ~30 caracteres. Pártelos
  con `|`.
- **Una pregunta muestra un número de opciones distinto en español y en inglés:** hay
  que dejar el **mismo** número de opciones rellenas en los dos idiomas.

---

## 7. Las voces de los diálogos

Cuando un diálogo se muestra con `sound`, suena una **vocecilla** (un balbuceo estilo
"animalese", como en Animal Crossing) mientras aparece el texto. Esas voces son
archivos de sonido que ya vienen generados con el juego, guardados en
`res/sfx/dialogs/`.

Normalmente **no tienes que tocar nada**: al añadir o cambiar frases, las voces se
reutilizan solas. Solo si quieres **regenerarlas** (por ejemplo, tras cambiar mucho
los textos o para ajustar el tono de un personaje) hay una herramienta para ello,
`tools/voice/generate_animalese_voices.py`, que produce los archivos de sonido a
partir del texto. Ofrece varios **perfiles de voz** (por ejemplo, más aguda o más
grave) para dar a cada personaje un timbre distinto. Es un paso opcional y avanzado:
para escribir diálogos normales no hace falta.
