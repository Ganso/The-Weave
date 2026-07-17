# Cómo crear una escena (cutscene) nueva — guía completa

Esta guía te permite **crear una escena de principio a fin aunque no sepas
programar**. Una "escena" (o *cutscene*) es cada tramo del juego: un diálogo, una
decisión, un combate, una transición... Aquí se explica con palabras normales qué
es cada pieza, qué archivos hay que tocar, qué escribir en cada uno y **cuándo hacer
algo de una manera y cuándo de otra**. No necesitas leer ningún otro documento:
está todo aquí.

> La mayor parte de una escena se escribe en **archivos de texto sencillos** (el
> guion y unas tablas). Solo la lógica más complicada necesita tocar los archivos de
> programación (`.c` y `.h`), y para eso basta con copiar el patrón que se explica
> aquí.

---

## 1. ¿Qué es una escena?

Una escena es un tramo del juego con su propia secuencia de acontecimientos: los
personajes entran, hablan, el jugador elige una respuesta, quizá pelea, y al final
se pasa a la siguiente escena.

Datos importantes desde el principio:

- **Las escenas no tienen número, tienen nombre.** Se llaman `<acto>_<nombre>`, por
  ejemplo `act1_bedroom` (el dormitorio del acto 1) o `act1_forest` (el bosque). Se
  saltan de una a otra **por su nombre**, así que puedes meter una escena nueva
  entre dos existentes sin renombrar nada.
- Una escena vive principalmente en un **archivo de guion** con extensión `.scene`,
  guardado en `data/scenes/<acto>/<nombre>.scene`. El nombre de la escena sale de su
  ruta: el archivo `data/scenes/act1/bedroom.scene` es la escena `act1_bedroom`.

---

## 2. La idea clave: el guion y la lógica separados

El juego usa una filosofía **híbrida**, y entenderla te ahorrará muchas dudas:

- **El guion (`.scene`)** expresa **todo lo que se puede escribir como una lista de
  órdenes**: montar el escenario (fondo, personajes, objetos), la secuencia narrativa
  ("primero habla Clio, luego se mueve Xander, luego el jugador elige, luego hay un
  combate") y las transiciones. Son órdenes sencillas, una por línea, en un lenguaje
  muy fácil de leer. **Aquí escribes casi todo, incluida la preparación de la escena.**

- **La lógica (ganchos en C)** queda solo para lo que **no** se puede expresar como
  una lista de órdenes: bucles (por ejemplo, "reaccionar cada vez que el jugador toca
  un objeto, hasta que se cumpla una condición"), condiciones que dependen del estado
  del juego, efectos de color/paleta (fundidos, un cielo que pasa de la noche al día)
  y la aparición de enemigos. Eso se mete en una pequeña función de C (un **gancho**)
  y desde el guion se la invoca con la orden `call`.

### La regla para decidir dónde va cada cosa

**¿Es una orden suelta, aunque sea de preparación (poner un fondo, crear un
personaje, colocar un objeto, desbloquear un hechizo), o una secuencia de pasos
fija (habla, muévete, elige, espera, combate)?** → va en el **guion** `.scene`.

**¿Necesita repeticiones, condiciones que dependen del estado del juego,
manipulación de colores/paletas, o hacer aparecer enemigos?** → va en un **gancho de
C**, y lo llamas desde el guion con `call`.

En la práctica: casi toda escena empieza con un **bloque de montaje** escrito
directamente en el guion (unas cuantas órdenes que ponen el fondo, colocan a los
personajes y crean los objetos) y a partir de ahí el guion sigue con la secuencia
narrativa. Solo escribes un gancho de C cuando necesitas algo que una lista de
órdenes no puede expresar (como el bucle de objetos de una habitación o la aparición
de un enemigo). Hay escenas que **no tienen ningún gancho**: se escriben enteras en
el guion.

---

## 3. Vocabulario rápido

- **Escena / cutscene**: un tramo del juego, con su nombre `<acto>_<nombre>`.
- **Guion (`.scene`)**: el archivo de texto con la secuencia de órdenes de la escena.
- **Directiva / orden**: cada línea del guion (por ejemplo `say`, `move`, `combat`).
- **Gancho (hook) de C**: una pequeña función de programación para la lógica que el
  guion no puede expresar; se invoca con `call`.
- **Diálogo**: una frase que dice un personaje, definida en la tabla de textos.
- **Cluster**: un grupo de diálogos que se muestran seguidos.
- **Choice**: una pregunta con varias opciones que el jugador elige.
- **Set**: el "grupo" al que pertenece un diálogo o un choice (normalmente uno por
  escena).
- **Frame (fotograma)**: la unidad de tiempo del juego (50 por segundo en consolas
  europeas, 60 en las americanas/japonesas).
- **Flag**: un interruptor de "encendido/apagado" que activa o desactiva capacidades
  del jugador (moverse, ver el HUD, lanzar hechizos...).

---

## 4. Los textos de los diálogos: `data/texts.csv`

Todos los diálogos del juego viven en un único archivo de tabla,
`data/texts.csv`. Es un archivo de valores separados por comas (un CSV), que puedes
editar con cualquier hoja de cálculo o editor de texto. Cada línea es un diálogo (o
un separador). Las columnas, en orden, son:

```
set, id, face, side, time, es, en
```

- **set** — el grupo al que pertenece el diálogo, normalmente **una escena**. Se
  escribe en minúsculas: `act1_hall`, `act1_bedroom`, etc. (Hay un grupo especial,
  `system_dialog`, para mensajes del sistema como "no puedo usar ese patrón".)
- **id** — el nombre único de este diálogo dentro de su grupo. Se escribe en
  MAYÚSCULAS con el formato `A<acto>_<ESCENA>_<DE_QUÉ_VA>`, por ejemplo
  `A1_HALL_GUILD_YEAR`. Piensa en él como el "nombre de la frase" que usarás luego
  en el guion.
- **face** — qué cara (retrato) se muestra al hablar: `FACE_linus`, `FACE_clio`,
  `FACE_xander`, `FACE_swan` o `FACE_none` (sin cara, para textos ambientales).
- **side** — en qué lado de la pantalla aparece: `SIDE_LEFT` (izquierda),
  `SIDE_RIGHT` (derecha) o `SIDE_NONE` (que se comporta como izquierda).
- **time** — cuántos segundos, como máximo, se queda el texto antes de avanzar solo.
  Normalmente se pone `DEFAULT_TALK_TIME` (que son 10 segundos).
- **es** — el texto en **español**.
- **en** — el mismo texto en **inglés**.

### Cómo se escribe el texto

- Los caracteres españoles (`ñ`, `á`, `¿`, `¡`...) se escriben **normales**. El
  juego los convierte solo. No los sustituyas por letras sin acento.
- Para un **salto de línea** dentro de una frase, usa la barra vertical `|`. Por
  ejemplo `Es tarde, Linus|Y no debes llegar tarde` se verá en dos renglones.
- Para **resaltar** una palabra con color, enciérrala entre `@[` y `@]`. Por
  ejemplo `Un @[cisne@] venía a mi cuarto` resalta la palabra "cisne".
- Procura no pasar de unos **30 caracteres por renglón**, o el texto se saldrá de la
  pantalla.

### Los "clusters": varios diálogos seguidos

A menudo un personaje suelta varias frases de corrido. En vez de invocarlas una a
una, puedes agruparlas en un **cluster** y mostrarlas todas seguidas con una sola
orden en el guion.

Un cluster es, sencillamente, **todas las filas de la tabla desde un diálogo hasta
la siguiente fila separadora**. Una fila separadora es una fila cuyo **id es `NULL`**
(las demás columnas se dejan casi vacías). Ejemplo:

```
act1_claro,A1_CLARO_ARRIVE,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Un claro en el bosque|El aire está quieto,A forest clearing|The air is still
act1_claro,A1_CLARO_CLIO,FACE_clio,SIDE_RIGHT,DEFAULT_TALK_TIME,¿Exploramos|o seguimos?,Do we explore|or move on?
act1_claro,NULL,,,,,
```

Aquí, un cluster que empiece en `A1_CLARO_ARRIVE` mostrará esa frase y la de
`A1_CLARO_CLIO`, y se parará al llegar a la fila `NULL`. Es decir: **la fila `NULL`
marca el final del grupo.** Si quieres partir varios diálogos en grupos distintos,
mete una fila `NULL` entre medias.

### Después de editar los textos

No tienes que hacer nada especial: al construir el juego (ver la sección 12), la
tabla de textos se procesa automáticamente. Si prefieres procesarla tú antes, hay un
comando para ello, pero no es imprescindible.

---

## 5. Las preguntas al jugador: `data/choices.csv`

Cuando el jugador tiene que **elegir** entre varias opciones, esas opciones se
definen en otra tabla, `data/choices.csv`. Sus columnas son:

```
set, item, face, side, time, es_1, es_2, es_3, es_4, en_1, en_2, en_3, en_4
```

- **set** — el grupo, con el mismo criterio que los textos, pero con el sufijo
  `_choice`. Por ejemplo `act1_hall_choice`.
- **item** — el número de la pregunta dentro del grupo (0, 1, 2...), consecutivos.
  Un mismo grupo puede tener varias preguntas.
- **face / side / time** — igual que en los textos: quién pregunta, en qué lado y
  cuánto tiempo.
- **es_1 … es_4** — hasta **cuatro** opciones en español.
- **en_1 … en_4** — las mismas cuatro opciones en inglés.
- Si una escena solo tiene 2 ó 3 opciones, **deja vacías** las columnas sobrantes.
  El juego cuenta cuántas hay. Eso sí: el español y el inglés deben tener el **mismo
  número** de opciones.

Ejemplo (una pregunta con dos opciones):

```
act1_claro_choice,0,FACE_linus,SIDE_RIGHT,DEFAULT_CHOICE_TIME,Explorar el claro,Seguir de largo,,,Explore the clearing,Move on,,
```

Cuando el jugador elige, el juego **recuerda cuál opción marcó** (la primera opción
es la número 0, la segunda la 1, y así). En el guion podrás reaccionar a esa
elección (ver `branch` y `say_response` más abajo).

---

## 6. El guion `.scene`: reglas básicas de escritura

El guion es un archivo de texto con **una orden por línea**. Reglas:

- La **primera línea "de verdad"** debe declarar el nombre de la escena, y ese
  nombre **tiene que coincidir con la ruta del archivo**. Si el archivo es
  `data/scenes/act1/claro.scene`, la primera línea es `scene act1_claro`.
- Todo lo que va después de una almohadilla `#` es un **comentario**: el juego lo
  ignora. Sirve para dejarte notas a ti mismo.
- Las palabras en MAYÚSCULAS (nombres de diálogos, de personajes, de hechizos...)
  se escriben tal cual; el juego comprueba que existan.

---

## 7. Todas las órdenes del guion (referencia completa)

Aquí tienes **todas** las órdenes que puedes usar en un guion, agrupadas por tema.
De cada una se explica qué hace, cómo se escribe y cuándo usarla.

### 7.1. Montar la escena (fondo, límites, personajes, objetos)

Estas órdenes son el **bloque de montaje** con el que arranca casi toda escena:
ponen el escenario y colocan lo que hay en él. Antes vivían en un gancho de C; ahora
se escriben directamente en el guion, en orden, al principio.

- **`level <fondo_tras> <mapa_tras> <fondo_frente> <mapa_frente> <paleta> <ancho> <modo_scroll> <velocidad>`**
  — monta el nivel: los gráficos del fondo, su paleta y cómo se desplaza la pantalla.
  El fondo tiene **dos capas**: una trasera (detrás de los personajes) y una frontal
  (delante). Si tu escena no usa capa trasera, pon `none none` en sus dos primeros
  huecos. Ejemplo del bosque:

  ```
  level forest_bg_tile forest_bg_map forest_front_tile forest_front_map forest_dark_pal 1440 user_right 3
  ```

  - **paleta**: el juego de colores inicial del nivel (por ejemplo `forest_dark_pal`).
  - **ancho**: la anchura **real** del escenario en píxeles (por ejemplo `1440` para un
    bosque largo, o `SCREEN_WIDTH` para uno que ocupa justo una pantalla).
  - **modo_scroll**: cómo se desplaza la pantalla. `user_right`/`user_left` = se
    desplaza cuando el **jugador** camina (para escenarios anchos que se recorren);
    `auto_right`/`auto_left` = se desplaza **solo** (para fondos de una pantalla).
  - **velocidad**: la rapidez del desplazamiento (cada modo la usa a su manera; para
    `auto` es la velocidad del scroll automático).
  - **Cuándo:** es casi siempre la **primera** orden de la escena. Si dudas del ancho
    y el modo, cópialos de una escena que ya use el mismo fondo (poner mal esos dos
    valores puede colgar la consola al desplazar la pantalla).

- **`limits <x_min> <y_min> <x_max> <y_max>`** — fija el rectángulo por el que los
  personajes pueden caminar (los bordes del suelo transitable). Ejemplo:
  `limits 0 134 275 172`.

- **`palette <ranura> <paleta>`** — carga una paleta de colores en una de las cuatro
  ranuras (`PAL0`, `PAL1`, `PAL2`, `PAL3`). Ejemplo: `palette PAL1 swan_pal` (usa la
  paleta del cisne en la ranura 1 en lugar de la de personajes).
  - **Cuándo:** cuando un personaje u objeto de la escena necesita unos colores que no
    son los normales.

- **`character CHR_x`** — crea un personaje y lo prepara para la escena (por ejemplo
  `character CHR_linus`). Nace **oculto**: para que se vea, usa luego `show CHR_x on`
  o muévelo con `move`.
  - **Cuándo:** una vez por cada personaje que aparezca en la escena.

- **`active CHR_x`** — marca qué personaje es el que **controla el jugador** (el
  protagonista de la escena). Ejemplo: `active CHR_linus`.

- **`follow CHR_x on/off`** — hace que un personaje **siga** al personaje activo (o
  deje de seguirlo). Ejemplo: `follow CHR_clio on` (Clio acompaña a Linus).

- **`enable_spell SPELL_X`** — **desbloquea** un hechizo para que el jugador pueda
  lanzarlo, sin ninguna floritura (no muestra el mensaje de "has aprendido un patrón";
  para eso, cuando quieras el aviso, se usa otra vía dentro de un gancho). Ejemplo:
  `enable_spell SPELL_THUNDER`.
  - **Cuándo:** al montar la escena, para dejar disponibles los hechizos que el jugador
    usará (en un combate o en un puzzle).

- **`item <hueco> <sprite> <paleta> <x> <y> <ancho_col> <offx_col> <alto_col> <offy_col> <profundidad>`**
  — coloca un objeto del escenario (un decorado o algo con lo que interactuar). Los
  objetos se numeran por su `<hueco>` (0, 1, 2...). Ejemplo:

  ```
  item 0 item_bedroom_bed PAL0 31 139 93 0 23 0 FORCE_BACKGROUND
  ```

  - **sprite / paleta**: el gráfico del objeto y su juego de colores.
  - **x / y**: dónde se coloca (x en el escenario, y en la pantalla).
  - **ancho_col / offx_col / alto_col / offy_col**: la "caja" con la que el jugador
    puede chocar o interactuar (ancho, desplazamiento en x, alto, desplazamiento en y).
    Si no quieres afinarla, escribe `COLLISION_DEFAULT` en el hueco que no te importe.
  - **profundidad**: si el objeto se dibuja **detrás** de los personajes
    (`FORCE_BACKGROUND`), **delante** (`FORCE_FOREGROUND`) o se decide solo según su
    posición (`CALCULATE_DEPTH`).
  - **Cuándo:** una vez por cada objeto de la escena. El número de hueco es el que
    usará luego la lógica de un gancho para saber "con qué objeto ha interactuado el
    jugador".

> **Truco de la vara mágica.** Si en la escena Linus va a lanzar hechizos, necesitas
> que lleve su vara. Eso se enciende con **`set rod on`** (ver 7.6), y tiene que ir
> **antes** de `character CHR_linus`, porque decide qué versión del muñeco de Linus se
> carga. Si lo pones después, el juego se colgará al tocar la primera nota.

### 7.2. Hacer hablar a los personajes

- **`say SET ID [sound]`** — muestra **un** diálogo (la frase con ese `ID` del grupo
  `SET`). Ejemplo: `say ACT1_CLARO A1_CLARO_SKIP sound`.
  - La palabra `sound` al final hace que suene la voz mientras habla. Si la omites (o
    pones `silent`), el diálogo aparece en silencio. Por defecto es silencioso.
  - **Cuándo:** para una frase suelta.

- **`say_cluster SET ID [sound]`** — muestra **varios** diálogos seguidos: empieza
  en `ID` y sigue hasta la fila `NULL` de la tabla (ver clusters en la sección 4).
  - **Cuándo:** cuando un personaje (o varios) sueltan una parrafada de frases
    seguidas. Ahorra escribir muchos `say`.

- **`say_response SET BASE [sound]`** — muestra el diálogo que corresponde a **la
  opción que el jugador acaba de elegir**. En concreto, muestra la frase `BASE`
  desplazada por la elección: si eligió la opción 0, muestra `BASE`; si eligió la 1,
  la frase **siguiente** a `BASE` en la tabla; etc.
  - **Cuándo:** justo después de un `choice`, cuando cada opción tiene su propia
    frase de respuesta y esas frases están **una detrás de otra** en la tabla, en el
    mismo orden que las opciones. Es la forma más cómoda de responder a una
    pregunta. Si tus respuestas no están correlativas, usa `branch` (ver 7.3).

### 7.3. Preguntas y ramificaciones (decisiones)

- **`choice SET item`** — muestra una pregunta (la número `item` del grupo `SET`) y
  espera a que el jugador elija. La elección queda recordada para las órdenes
  siguientes. Ejemplo: `choice ACT1_CLARO_CHOICE 0`.

- **`branch <n> goto <etiqueta>`** — si el jugador eligió la opción número `<n>`,
  salta a la línea marcada con `label <etiqueta>`. Si eligió otra cosa, sigue de
  largo. Ejemplo: `branch 1 goto seguir` (si eligió la segunda opción, salta a
  `seguir`).
  - **Cuándo:** para que cada opción lleve a una parte distinta de la escena.

- **`label <etiqueta>`** — marca un punto del guion con un nombre, para poder saltar
  a él. Ejemplo: `label seguir`.

- **`goto <etiqueta>`** — salta directamente a una etiqueta, sin condición. Ejemplo:
  `goto confluye`.
  - **Cuándo:** para juntar de nuevo dos ramas que se habían separado, o para hacer
    bucles (por ejemplo, repetir una pregunta hasta que el jugador acierte).

> Combinando `choice`, `branch`, `label` y `goto` puedes montar cualquier estructura
> de decisiones: ramas que se separan y se vuelven a juntar, preguntas que se repiten
> si el jugador falla, menús, etc.

### 7.4. Mover y animar a los personajes

En estas órdenes, los personajes se nombran `CHR_linus`, `CHR_clio`, `CHR_xander`...

- **`move CHR x y`** — mueve al personaje **andando** hasta la posición `x y` (en
  píxeles). La escena **espera** a que llegue antes de seguir. Ejemplo:
  `move CHR_linus 200 174`.
- **`move_instant CHR x y`** — coloca al personaje **de golpe** en `x y`, sin
  caminar (un teletransporte). Útil para posicionarlo fuera de pantalla antes de que
  entre.
- **`show CHR on/off`** — muestra (`on`) u oculta (`off`) al personaje.
- **`look CHR left/right`** — hace que el personaje mire a la izquierda o a la
  derecha.
- **`anim CHR ANIM_*`** — fija una animación concreta del personaje, por ejemplo
  `anim CHR_linus ANIM_MAGIC` (pose de lanzar hechizo) o `anim CHR_linus ANIM_IDLE`
  (quieto). Se usa mucho combinado con una `wait` para que la pose dure un momento y
  luego volver a `ANIM_IDLE`.

### 7.5. Esperas y pausas

- **`wait <décimas>`** — espera ese número de **décimas de segundo**. Ejemplo:
  `wait 20` espera 2,0 segundos. (La espera se ajusta sola a la velocidad de la
  consola.)
  - **Cuándo:** para dejar un respiro entre acciones, o para que una animación dure
    un instante.
- **`wait_press`** — **pausa la escena hasta que el jugador pulse el botón A.**
  - **Cuándo:** al final de un momento importante, para que el jugador lo lea a su
    ritmo antes de continuar.
- **`wait_scroll <offset>`** — devuelve el control al jugador y le deja caminar hasta
  que el escenario se haya desplazado esa cantidad. Ejemplo: `wait_scroll 360`.
  - **Cuándo:** en tramos "andables", para que el jugador avance por el escenario
    hasta llegar a un punto (por ejemplo, la zona donde empieza un combate).
- **`wait_scroll_left <offset>`** — igual que `wait_scroll` pero para niveles que se
  recorren hacia la IZQUIERDA (`user_left`): deja jugar hasta que el scroll baja
  hasta ese offset. Ejemplo: el regreso del acto 1 (`return.scene`).
- **`wait_spell`** — espera a que **termine** el hechizo que se está lanzando.
  - **Cuándo:** después de un `cast` guionizado, para no seguir hasta que el efecto
    acabe.
- **`wait_puzzle <tag>`** — deja que el jugador lance hechizos libremente hasta
  completar un puzzle (ver la sección 8).

### 7.6. Dar y quitar control al jugador (`set`)

La orden **`set <flag> on/off`** enciende o apaga un interruptor. Hay cinco:

- **`set movement on/off`** — permite (o impide) que el jugador **camine**.
- **`set scroll on/off`** — permite (o impide) que el **escenario se desplace** al
  caminar.
- **`set interface on/off`** — muestra (o esconde) el **HUD** (los iconos de la
  interfaz en pantalla).
- **`set spells on/off`** — permite (o impide) que el jugador **lance hechizos**.
- **`set rod on/off`** — le da (o le quita) a Linus la **vara mágica**. Cambia el
  muñeco que se usa para Linus, así que **debe encenderse antes de crearlo** con
  `character CHR_linus` (ver el truco de la vara en 7.1).

**Cuándo:** durante los diálogos y cinemáticas normalmente todo está apagado (el
jugador solo mira). Cuando llega el momento de que juegue (caminar, pelear), enciende
lo que necesite. Ejemplo típico antes de un tramo jugable:

```
set movement on
set scroll on
set interface on
set spells on
```

> Detalle: `set interface off` solo **esconde** el HUD de la pantalla; no desactiva
> por dentro la capacidad de lanzar hechizos. Para eso está `set spells off`.

### 7.7. Combate

Hay **dos tipos de combate**, y cada uno se pone en el guion de una forma. En los
dos, el jugador empieza con **5 puntos de vida** (cada golpe enemigo resta uno) y,
si llega a cero, el combate termina en **derrota**.

**Tipo A — de patrones (enemigos a distancia, como los espectros).** Los enemigos
cantan hechizos y el jugador los contrarresta. Se pone en dos pasos: primero un
`call` a un gancho de C que hace **aparecer** a los enemigos (ver sección 9), y
luego la orden `combat`.

- **`combat`** — lanza el combate y **no sigue hasta que termina** (el jugador gana
  o cae derrotado). Toda la mecánica la gestiona el juego.

    ```
    call act1_return_ghosts    # gancho: hace aparecer a los espectros
    combat
    ```

**Tipo B — de contacto (enemigos que persiguen y muerden/embisten, como los
jabalíes).** Aquí NO se usa `combat`: **un solo gancho de C corre la pelea entera**
(hace aparecer a la manada y la gestiona hasta que huye o te derrota). En el guion
solo pones el `call` a ese gancho.

    ```
    set spells off             # el arma es el golpe con A (o "on" si es con hechizos)
    call act1_fday_boars       # gancho: aparece la manada Y corre el combate
    ```

*(Cómo se escribe ese gancho —incluidas las dos variantes de arma, golpe o
patrón— es trabajo de programación: el patrón exacto está en el código de los
ganchos de combate existentes.)*

- **`if_defeated goto <etiqueta>`** — vale para los dos tipos. Salta a una etiqueta
  **si el último combate terminó en derrota**. Ponla justo después del `combat`
  (tipo A) o del `call` al gancho (tipo B) para mostrar un mensaje de fallo y
  repetir. Si el jugador ganó, el guion sigue de largo.
  - Ejemplo de combate con reintento:

    ```
    label pelea
    call mi_escena_enemigos
    combat
    if_defeated goto pelea_fallida
    say MI_ESCENA VICTORIA sound
    goto continuar
    label pelea_fallida
    say MI_ESCENA DERROTA sound
    goto pelea
    ```

### 7.8. Lanzar hechizos desde el guion (cinemáticas)

- **`cast SPELL [direct/reversed]`** — lanza un hechizo de forma **guionizada** (lo
  ejecuta el guion, no el jugador). `direct` para la forma normal, `reversed` para la
  invertida. Ejemplo: `cast SPELL_OPEN direct`.
  - **Cuándo:** para cinemáticas donde un hechizo ocurre "por guion" (por ejemplo,
    una puerta que se abre sola). Un lanzamiento guionizado **no comprueba** las
    condiciones normales del hechizo: si lo pones, ocurre.
  - Suele ir seguido de `wait_spell` para esperar a que termine el efecto.
- **`zone ZONE_X`** — fija la "zona" narrativa actual del escenario (por ejemplo
  `zone ZONE_CAULDRON` = "estamos junto al caldero"). Algunos hechizos solo se pueden
  lanzar en ciertas zonas; esta orden es la que las activa. Para quitar la zona, usa
  `zone ZONE_NONE`.

### 7.9. Puzzles de secuencia de hechizos

(Explicados en detalle en la sección 8.)

- **`puzzle_sequence <tag> <hechizo:dirección> ...`** — define un puzzle: la
  secuencia de hechizos que el jugador debe lanzar, en orden.
- **`wait_puzzle <tag>`** — deja jugar hasta que el puzzle se complete.
- **`if_puzzle_solved <tag> goto <etiqueta>`** — salta si el puzzle ya está resuelto.

### 7.10. Terminar la escena y transiciones

- **`fade_out <frames>`** — funde la pantalla a negro en ese número de fotogramas.
  Ejemplo: `fade_out 60` (un fundido de algo más de un segundo).
- **`next_scene <escena>`** — termina esta escena y pasa a otra, **por su nombre**.
  Ejemplo: `next_scene act1_forest`.
  - **Cuándo:** al final de una escena, para encadenar con la siguiente.
- **`hard_reset`** — reinicia la consola. Se usa como final de la demo.
- **`end`** — termina la escena sin pasar a ninguna otra ni cerrar el nivel.

---

## 8. Puzzles de secuencia de hechizos (con detalle)

Un puzzle de secuencia es un reto en el que el jugador debe **lanzar varios hechizos
en el orden correcto**. Se monta con tres órdenes:

1. **`puzzle_sequence <tag> <hechizo:dirección> ...`** — define y activa el puzzle.
   El `<tag>` es un nombre que tú inventas para identificar este puzzle (por ejemplo
   `puerta`). Luego listas de 2 a 4 pasos, cada uno indicando el hechizo y si va
   directo o invertido. Ejemplo:

   ```
   puzzle_sequence puerta thunder:direct fire:direct hide:direct
   ```

   Esto pide: primero TRUENO directo, luego FUEGO directo, luego ESCONDER directo.

2. **`wait_puzzle <tag>`** — a partir de aquí, el jugador lanza hechizos libremente.
   Cada hechizo que **termina bien** avanza el puzzle si coincide con el paso
   esperado. **Si se equivoca, la secuencia se reinicia** (aunque, si el hechizo
   equivocado coincide con el primer paso, cuenta como que ha empezado de nuevo desde
   el principio). El juego no sigue hasta que la secuencia se completa.

3. **`if_puzzle_solved <tag> goto <etiqueta>`** — (opcional) salta a una etiqueta si
   el puzzle está resuelto. Útil para estructurar el después.

Cosas a tener en cuenta:

- Solo puede haber **un puzzle activo a la vez**. Al entrar en cualquier escena, el
  puzzle se reinicia.
- Para que el jugador pueda lanzar los hechizos del puzzle, tienes que **tenerlos
  desbloqueados**. Eso se hace en el bloque de montaje de la escena (ver 7.1), con una
  orden `enable_spell` por hechizo. Y si algún hechizo del puzzle exige estar en una
  zona concreta, acuérdate de fijarla con `zone`.

---

## 9. Los ganchos de C (para la lógica que el guion no puede expresar)

Cuando algo **no** se puede escribir como una lista de órdenes (un bucle, una
condición, un efecto de color, la aparición de un enemigo), se escribe en un **gancho
de C**: una pequeña función de programación. Desde el guion se la llama con
`call <nombre_del_gancho>`.

Ojo: el **montaje** de la escena (fondo, límites, personajes, objetos, hechizos) ya
**no** necesita un gancho — eso se escribe hoy directamente en el guion con las
órdenes de la sección 7.1. Los ganchos quedan solo para lo verdaderamente lógico.

Para crear ganchos hay que tocar dos sitios:

1. **El archivo del gancho**: `src/scenes/<acto>/<nombre>.c` y su `.h`. Ahí escribes
   las funciones.
2. **El registro de ganchos**: hay que apuntar cada gancho en dos archivos
   (`src/scenes/scene_hooks.h` y `src/scenes/scene_hooks.c`) para que el guion pueda
   encontrarlo por su nombre.

La receta exacta, con ejemplos completos, está en el paso a paso de la sección 11.

### ¿Cuándo hace falta un gancho?

- **Sí hace falta** para: hacer aparecer enemigos, cambios de paleta (fundidos de
  color, un cielo que pasa de la noche al día), bucles (por ejemplo, reaccionar cada
  vez que el jugador toca un objeto hasta que se cumpla una condición) y cualquier
  cosa que dependa del estado del juego.
- **No hace falta** para: montar el escenario, colocar personajes y objetos,
  desbloquear hechizos, ni para una escena de diálogos, decisiones y esperas. Todo eso
  se escribe entero en el guion. Hay escenas (como el hall) que **no tienen ningún
  gancho**.

---

## 10. Mapa de archivos: qué tocarás para una escena

| Archivo | Qué haces en él | ¿Siempre? |
|---|---|---|
| `data/scenes/<acto>/<nombre>.scene` | El guion de la escena. | Siempre. |
| `data/texts.csv` | Los diálogos de la escena. | Casi siempre. |
| `data/choices.csv` | Las preguntas al jugador. | Solo si hay decisiones. |
| `src/scenes/<acto>/<nombre>.c` y `.h` | Los ganchos de lógica (enemigos, cinemáticas de color, bucles). | Solo si la escena tiene lógica que el guion no puede expresar. |
| `src/scenes/scene_hooks.h` y `.c` | Registrar los ganchos para que el guion los encuentre. | Solo si creas ganchos. |
| `src/smoke/smoke_cases.h` | Un caso para probar la escena aislada. | Opcional (recomendado). |

Y para **enlazar** la escena, un `next_scene <nombre>` desde la escena anterior.

---

## 11. Receta completa, paso a paso: crear la escena `act1_claro`

Vamos a crear una escena de ejemplo que toca **todos los conceptos importantes**:
montaje del escenario en el propio guion, diálogos, una pregunta con dos ramas,
respuesta según la elección, movimientos y animación, una pausa, un combate y la
transición a la siguiente escena. Lo único que irá en un gancho de C es la aparición
del enemigo (que no se puede expresar como una simple orden).

Guion (el argumento): *"Linus y Clio llegan a un claro del bosque; el jugador decide
si explorar (lo que lleva a un combate) o seguir de largo; las dos opciones vuelven a
juntarse y la escena termina pasando al bosque"*.

### Paso 1 — Los textos: `data/texts.csv`

Añade las filas de diálogo del grupo `act1_claro`. Fíjate en la fila `NULL`, que
cierra el primer cluster:

```
act1_claro,A1_CLARO_ARRIVE,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Un claro en el bosque|El aire está quieto,A forest clearing|The air is still
act1_claro,A1_CLARO_CLIO,FACE_clio,SIDE_RIGHT,DEFAULT_TALK_TIME,¿Exploramos|o seguimos?,Do we explore|or move on?
act1_claro,NULL,,,,,
act1_claro,A1_CLARO_EXPLORE,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Hay algo entre|los árboles...,Something moves|in the trees...
act1_claro,A1_CLARO_SKIP,FACE_linus,SIDE_LEFT,DEFAULT_TALK_TIME,Mejor no perder|tiempo,Better not waste|time
act1_claro,A1_CLARO_AFTER,FACE_clio,SIDE_RIGHT,DEFAULT_TALK_TIME,Sigamos el camino,Let's follow the path
```

Un detalle que aprovecharemos luego: `A1_CLARO_EXPLORE` y, justo debajo,
`A1_CLARO_SKIP`, están **una detrás de otra**. Eso nos permitirá responder a la
pregunta con `say_response` (la opción 0 mostrará `EXPLORE`, la opción 1 mostrará la
siguiente, `SKIP`).

### Paso 2 — La pregunta: `data/choices.csv`

Añade la pregunta del grupo `act1_claro_choice`, con dos opciones:

```
act1_claro_choice,0,FACE_linus,SIDE_RIGHT,DEFAULT_CHOICE_TIME,Explorar el claro,Seguir de largo,,,Explore the clearing,Move on,,
```

### Paso 3 — El gancho de C: `src/scenes/act1/claro.c` y `.h`

En esta escena, lo único que **no** se puede escribir como una lista de órdenes es la
aparición del enemigo del combate opcional (prepara su paleta, para a los personajes
y lo hace entrar caminando). Eso va en un gancho. Primero la "tarjeta de presentación"
(`.h`):

```c
// claro.h
#ifndef _ACT1_CLARO_H_
#define _ACT1_CLARO_H_
void act1_claro_enemy(void); // Hace aparecer un enemigo para el combate opcional
#endif
```

Y el archivo `.c`. Empieza con el bloque de `#include` (cópialo tal cual; le dice al
archivo qué piezas necesita):

```c
// claro.c
#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "res_all.h"
#include "scenes/act1/claro.h"   // la tarjeta de presentación de arriba

void act1_claro_enemy(void)
{
    show_or_hide_interface(false);
    // Preparar y hacer aparecer un enemigo (un WeaverGhost) que entra caminando.
    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA);
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(140));
}
```

Fíjate en que **todo el montaje del escenario ya no está aquí**: el fondo, los
límites, los personajes y los hechizos se montan directamente en el guion (Paso 5).
Este gancho solo tiene lo que una lista de órdenes no puede expresar.

### Paso 4 — Registrar el gancho: `src/scenes/scene_hooks.h` y `.c`

Para que el guion pueda invocar `act1_claro_enemy` por su nombre, hay que apuntarlo
en dos sitios.

En `src/scenes/scene_hooks.h`, dentro de la lista de nombres de ganchos (antes de
`HOOK_COUNT`), añade:

```c
    HOOK_ACT1_CLARO_ENEMY,
```

En `src/scenes/scene_hooks.c`, añade el `#include` de tu archivo y la entrada en la
tabla:

```c
#include "scenes/act1/claro.h"      // arriba, junto a los otros includes

    // ... dentro de la tabla de ganchos:
    [HOOK_ACT1_CLARO_ENEMY] = act1_claro_enemy,
```

> **Verifica que está en los DOS sitios.** Si apuntas un gancho en la lista de la
> `.h` pero te olvidas de ponerlo en la tabla de la `.c`, cuando el guion intente
> llamarlo la consola se colgará. Cada gancho necesita las dos cosas.

### Paso 5 — El guion: `data/scenes/act1/claro.scene`

Y ahora, la secuencia completa. Empieza con el **bloque de montaje** (las órdenes de
7.1) y sigue con la narrativa. Fíjate cómo se combinan las órdenes de la sección 7:

```
scene act1_claro

# --- Montaje del escenario (antes esto vivía en un gancho de C) ---
set rod on                                       # vara: ANTES de crear a Linus
level forest_bg_tile forest_bg_map forest_front_tile forest_front_map forest_pal 1440 user_right 3
limits 0 134 275 172                             # bordes por los que se camina
character CHR_linus
character CHR_clio
active CHR_linus                                 # a Linus lo controla el jugador
move_instant CHR_linus 140 154
show CHR_linus on
move_instant CHR_clio 90 154
show CHR_clio on
enable_spell SPELL_THUNDER                       # hechizos para el combate opcional
enable_spell SPELL_HIDE

say_cluster ACT1_CLARO A1_CLARO_ARRIVE sound     # cluster: ARRIVE + CLIO, hasta el NULL

# Clio se acerca y mira a Linus, y Linus hace una pose
move CHR_clio 120 154
look CHR_clio right
anim CHR_linus ANIM_ACTION
wait 8
anim CHR_linus ANIM_IDLE

# A partir de aquí el jugador puede moverse y lanzar hechizos
set movement on
set spells on

# La decisión
choice ACT1_CLARO_CHOICE 0
say_response ACT1_CLARO A1_CLARO_EXPLORE sound   # opción 0 → EXPLORE; opción 1 → SKIP
branch 1 goto seguir                             # si eligió "Seguir de largo", salta el combate

# Rama "explorar": combate
call act1_claro_enemy                            # aparece el enemigo
combat                                           # a pelear (no sigue hasta ganar)
goto confluye                                    # al terminar, saltar al punto común

# Rama "seguir": una frase y ya
label seguir
say ACT1_CLARO A1_CLARO_SKIP sound

# Punto común: las dos ramas se juntan aquí
label confluye
set interface off
say ACT1_CLARO A1_CLARO_AFTER sound
wait_press                                        # espera a que el jugador pulse A
fade_out 60
next_scene act1_forest                            # pasa a la siguiente escena
```

Sobre el `say_response`: muestra la respuesta que corresponde a la opción elegida.
Como en la tabla de textos pusimos `A1_CLARO_EXPLORE` e, inmediatamente después,
`A1_CLARO_SKIP`, la opción 0 enseña `EXPLORE` y la opción 1 enseña `SKIP`. Si tus
respuestas no estuvieran una detrás de otra, usarías `branch` para elegir a mano qué
`say` mostrar en cada rama (como hicimos con el combate).

### Paso 6 — Enlazar la escena y probarla

Para que se llegue a esta escena, pon un `next_scene act1_claro` al final de la
escena anterior.

Para **probarla directamente**, sin jugar desde el principio, hay un archivo de
atajos de prueba, `src/core/hack.h`. Puedes decirle que arranque el juego
directamente en tu escena poniendo:

```c
HACK_START_SCENE "act1_claro"
```

(Acuérdate de quitarlo antes de la versión final.) Como alternativa, puedes añadir un
caso a la ROM de pruebas en `src/smoke/smoke_cases.h`:

```c
{"SCENE act1_claro", SMOKE_SCENE, 0, false, 0, false, "act1_claro"},
```

### Paso 7 — Compilar

Construye el juego desde una terminal, en la carpeta del proyecto:

```
./build-theweave.sh release
```

Al construir, el juego **revisa automáticamente** todas las referencias de tu escena
antes de nada. Si un diálogo, una pregunta, un gancho o una escena de destino están
mal escritos o no existen, la construcción se detiene y te dice **el archivo y la
línea** exactos del error. Esto atrapa la mayoría de las equivocaciones (un id de
diálogo mal tecleado, un gancho sin registrar, etc.) antes incluso de arrancar el
juego.

---

## 12. Validación automática y errores frecuentes

Cuando construyes el juego, se comprueba en serio que todo encaje. En concreto, se
verifica que existan de verdad:

- las **etiquetas** a las que saltas con `goto`/`branch`,
- los **grupos e ids de diálogo** que usas en `say`/`say_cluster`/`say_response`
  (contra la tabla de textos),
- las **preguntas** que usas en `choice` (contra la tabla de choices),
- los **ganchos** que invocas con `call` (contra el registro de ganchos),
- los **hechizos y zonas** que nombras,
- la **escena** de destino de cada `next_scene`.

Cualquier fallo detiene la construcción con el archivo y la línea, así que no tienes
que adivinar dónde está el problema.

Errores típicos y cómo se manifiestan:

- **"El nombre de la escena no coincide":** la línea `scene ...` del guion no cuadra
  con la ruta del archivo. `data/scenes/act1/claro.scene` debe empezar por
  `scene act1_claro`.
- **La consola se cuelga al llamar a un gancho:** apuntaste el gancho en la lista de
  la `.h` pero olvidaste ponerlo en la tabla de la `.c` (o al revés).
- **La consola se cuelga al tocar la primera nota:** pusiste `set rod on` **después**
  de `character CHR_linus`, en vez de antes.
- **La consola se cuelga al desplazar la pantalla:** el ancho o el modo de
  desplazamiento de la orden `level` no cuadran con el fondo. Copia esos valores de
  una escena que use el mismo fondo y funcione.
- **Un diálogo no aparece o sale otro:** el id está mal escrito, o en un cluster
  faltó (o sobró) una fila `NULL`, o en un `say_response` las respuestas no estaban
  correlativas en la tabla.
- **El jugador no puede moverse o lanzar hechizos cuando debería:** te falta el `set`
  correspondiente encendido (`set movement on`, `set spells on`), o los hechizos no
  se desbloquearon en el bloque de montaje (`enable_spell`).

Para depurar hay atajos en `src/core/hack.h` que puedes encender temporalmente:
`HACK_START_SCENE "..."` (arrancar directo en una escena), `HACK_ALL_SPELLS` (todos
los hechizos desbloqueados), `HACK_ENEMIES_ONE_HP` (enemigos frágiles, para ganar
rápido) o `HACK_PLAYER_INVULNERABLE` (el jugador no muere). Acuérdate de
desactivarlos antes de la versión final.

---

## 13. Una escena de referencia para copiar

En `data/scenes/act1/test.scene` hay una escena de pruebas que **usa todas las
órdenes del guion**: diálogos con caras a un lado y a otro, preguntas con ramas y
bucles, lanzamientos de hechizo guionizados, dos puzzles de secuencia, tramos de
scroll y combates. No forma parte del juego (se llega a ella con
`HACK_START_SCENE "act1_test"`), pero es la mejor "chuleta" para ver ejemplos reales
de cada orden en funcionamiento. Ábrela cuando dudes cómo se escribe algo.
