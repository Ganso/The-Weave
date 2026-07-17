# The Weave — Guión del Acto 1 (HISTÓRICO / OBSOLETO)

> ⚠️ **Este documento está superado.** El guión vigente del acto 1 es
> **`docs/acto1-definitivo.md`** (diálogos, acciones y estado real de las escenas).
> Este fichero se conserva solo como referencia histórica de una versión anterior
> (todavía menciona mecánicas descartadas, como el hechizo de Luz/Oscuridad). No lo
> uses para saber cómo debe quedar el juego.
>
> Fuentes de verdad actuales:
> - Secuencia de cada escena: `data/scenes/act1/*.scene` (DSL declarativo).
> - Textos de diálogo: `data/texts.csv` → `src/narrative/texts_data.c`.
> - Opciones de diálogo (choices): `data/choices.csv`.
> - Lógica no lineal (bucles de items, combate, cinemáticas): hooks C en `src/scenes/act1/*.c`.
>
> **Convenciones de transcripción**
> - Los diálogos se transcriben en **español** (el juego es bilingüe ES/EN; el inglés está en `data/texts.csv`).
> - El caracter `|` del CSV es un **salto de línea** dentro del bocadillo; aquí se representa con saltos de línea reales.
> - El marcado `@[palabra]@` resalta una palabra en pantalla (color distinto); aquí se representa en **negrita**.
> - Un **cluster** es una secuencia de bocadillos que se reproducen seguidos hasta encontrar un separador (fila `NULL` en el CSV).

---

## Contexto narrativo

*The Weave* es una secuela-fangame de **Loom** (LucasArts), ambientada **un siglo después** de aquella aventura. La magia se teje con **patrones de notas musicales**: cada hechizo es una melodía de cuatro notas que el jugador interpreta con los botones del mando (A=MI, B=FA, C=SOL, X=LA, Y=SI, Z=DO).

El Acto 1 nos presenta a **Linus**, un joven del **Gremio de los Historiadores**, en el día de su decimoséptimo cumpleaños. Arranca con un sueño premonitorio (el cisne), recorre su hogar rememorando a su padre fallecido, recibe de su madre y su maestro el encargo de retomar el viaje que su padre nunca completó —encontrar la isla del legendario **Gremio de los Tejedores**— y termina con un primer contacto con la magia y el combate en el bosque.

### Reparto

| Personaje | Sprite | Rol |
|-----------|--------|-----|
| **Linus** | `CHR_linus` | Protagonista. 17 años recién cumplidos. Historiador, hijo de Aiden. |
| **Clio** | `CHR_clio` | Madre de Linus. Historiadora pragmática ("documentar hechos, no perseguirlos"). |
| **Xander** | `CHR_xander` | Maestro/anciano del gremio. Empuja a Linus a viajar. |
| **El cisne** | `CHR_swan` | Aparición mágica onírica. Mensajero de una amenaza inminente. |
| **Aiden** | *(mencionado)* | Padre fallecido de Linus. Soñaba con hallar a los Tejedores. |
| **Los Tejedores** | *(leyenda)* | Gremio capaz de tejer hechizos, desaparecido sin dejar rastro. |
| **WeaverGhosts** | `weaver_ghost` | Enemigos espectrales del bosque. |

### Orden de las escenas

```
act1_bedroom  →  act1_corridor  →  act1_hall  →  act1_forest  →  (fin de la demo → hard reset)
```

> `act1_test` **no forma parte** del flujo narrativo: es un banco de pruebas del motor de escenas (ver sección final).

---

## Escena 1 — Dormitorio (`act1_bedroom`)

**La visita del cisne y el primer patrón.**

### Puesta en escena
- Nivel: dormitorio de Linus, de noche (paleta `bedroom_night_pal`).
- Items decorativos de fondo: **cama**, **silla**, **alféizar** de la ventana, **armario** (con la partitura), y **Linus dormido**.
- La paleta de personajes se sustituye por la del **cisne** durante la cinemática inicial.

### 1.1 Cinemática del cisne *(hook `act1_bedroom_swan`)*
Con Linus dormido, hay una espera; suena el efecto mágico de aparición y un **fundido a blanco**. Al volver la imagen, el **cisne** ha aparecido flotando sobre la cama. El cisne habla:

> **Cisne:**
> Han pasado ya cien años

> **Cisne:**
> No podremos pararles
> por mucho más tiempo

Nuevo **fundido a blanco** (efecto de desaparición) y el cisne se esfuma. Vuelve la penumbra del dormitorio.

### 1.2 Amanecer *(hook `act1_bedroom_wake`)*
La paleta funde a **día**. Rótulo:

> A la mañana siguiente...

Linus se levanta de la cama (deja de estar el sprite "Linus dormido", aparece Linus de pie). Se activa el **control del jugador** (movimiento y scroll).

### 1.3 Exploración del dormitorio *(hook `act1_bedroom_items`)*
El jugador puede examinar los objetos del cuarto. Cada uno dispara un comentario de Linus:

- **Cama:**
  > He dormido regular esta noche
  > Tuve horribles pesadillas

- **Silla:**
  > No tengo tiempo de sentarme
  > **Madre** me espera

- **Ventana (alféizar):**
  > No ha podido ser real
  > La ventana está cerrada

- **Armario (partitura):**
  > Esta es la **nana** que me
  > cantaban cada noche

  La **primera vez** que examina el armario, Linus **aprende el hechizo DORMIR** (SLEEP) y aparece el aviso de sistema:

  > Has aprendido
  > tu primer **patrón**

  > Entra en el menú de
  > pausa para verlo

  La escena no avanza hasta que el jugador **abre el menú de pausa** (para ver el patrón aprendido) y, tras ello, deja pasar unos segundos.

### 1.4 Cierre de la escena
Con el patrón aprendido y la pausa consultada, la voz de la madre llama desde fuera:

> **Clio:**
> **Linus**, ¿dónde estás?

> **Linus:**
> Se me ha hecho demasiado tarde
> tengo que ir al salón

→ Transición a **`act1_corridor`**.

---

## Escena 2 — Pasillo de los historiadores (`act1_corridor`)

**Los libros y los recuerdos del padre.**

### Puesta en escena
- Pasillo largo (mapa de 800 px de ancho) con **scroll controlado por el jugador**. Solo capa frontal.
- Decorado: **lámparas** en la pared, tres **puertas** (cuarto de los ancianos, biblioteca, cuarto de Linus), dos **mapas** colgados y dos **pedestales con libros**:
  - Libro 0: **historia del gremio**.
  - Libro 1: **mitos y leyendas**.

### 2.1 Aviso de demo técnica
Al entrar, aparece el rótulo de sistema (cluster `SYSMSG_DEMO_TITLE`):

> **The Weave**
> Demo técnica
> Junio de 2025

> Los gráficos, mecánicas o sonidos
> no son definitivos, ni
> representan el resultado final

### 2.2 Entrada de Linus
Linus entra por la **derecha** y camina hacia el centro del pasillo. Reflexiona (cluster `A1_CORRIDOR_OVERSLEPT`):

> **Linus:**
> Creo que he dormido demasiado
> Debo llegar rápido al salón

> **Linus:**
> Aunque siendo el día que es
> este pasillo me trae
> **demasiados recuerdos**

Se activan **scroll** y **movimiento**.

### 2.3 Exploración del pasillo *(hook `act1_corridor_items`)*
El jugador recorre el pasillo hacia la izquierda examinando objetos.

- **Libro de historia del gremio** *(primera lectura, cluster completo)*:
  > **Linus:**
  > Este tomo narra la historia
  > de nuestro gremio
  > desde la **Gran Separación**

  > **Linus:**
  > El último capítulo
  > termina con el fallecimiento
  > de mi padre

  > **Linus:**
  > Madre dice que seré yo
  > el que deba escribir
  > el siguiente

  *(Relecturas posteriores: solo la primera línea.)*

- **Libro de mitos y leyendas** *(primera lectura, cluster completo)*:
  > **Linus:**
  > Una colección de
  > mitos y leyendas
  > de los distintos gremios

  > **Linus:**
  > Gracias a mi padre
  > tenemos documentadas
  > las que cantaban los **Pastores**

  *(Relecturas posteriores: solo la primera línea.)*

- **Puerta izquierda (cuarto de los ancianos):**
  > **Linus:**
  > No puedo entrar en el cuarto
  > de los ancianos sin su permiso

- **Puerta central (biblioteca):**
  > **Linus:**
  > Luego iré a la biblioteca
  > Me paso allí media vida

- **Puerta derecha (su cuarto):**
  > **Linus:**
  > Luego volveré a mi cuarto
  > Ahora me esperan

- **Mapas:**
  > **Linus:**
  > Tenemos la mejor colección
  > de mapas de **todos los gremios**

### 2.4 Condición de salida
Si Linus intenta salir por la **izquierda** **sin haber leído los dos libros**, se detiene y dice:

> **Linus:**
> Antes de irme quiero
> repasar algunos recuerdos
> Se lo debo a papá

…y el juego le hace retroceder. Una vez **leídos ambos libros**, al llegar al borde izquierdo Linus sale de escena.

→ Transición a **`act1_hall`**.

---

## Escena 3 — El Salón (`act1_hall`)

**Clio y Xander hablan de los Tejedores.** Es la escena narrativa por excelencia, con las dos decisiones de diálogo del acto.

### Puesta en escena
- Salón del gremio (nivel `historians`).
- Al inicio: **Clio** a la izquierda mirando a la derecha; **Linus** entra por la derecha mirando a la izquierda.

### 3.1 Rótulo de situación
> **Gremio de los historiadores**
> Año 8121

> Lunes
> Primera hora de la mañana

### 3.2 Encuentro con la madre
Clio saluda; Linus se acerca (cluster `A1_HALL_CLIO_LATE`):

> **Clio:**
> Es tarde, Linus
> Y uno no debe llegar tarde
> a su cumpleaños

> **Linus:**
> He tenido un sueño
> de lo más extraño, Madre

> **Linus:**
> Un **cisne** venía a
> mi cuarto y...

> **Clio:**
> Luego me lo cuentas
> **Xander** nos espera

### 3.3 Entrada de Xander
Clio se desplaza y se gira; **Xander** entra por la izquierda y avanza (cluster `A1_HALL_XANDER_AWAKE`):

> **Xander:**
> Por fin
> estás despierto, Linus

> **Linus:**
> Perdóname, maestro
> Un extraño sueño me ha
> mantenido despierto

> **Xander:**
> Ciertamente eres el
> hijo de tu padre
> **Aiden** tenía grandes sueños

> **Xander:**
> Y estamos aquí para hablar
> sobre uno que
> nunca llegó a cumplir

> **Linus:**
> He leído sus historias
> mil veces
> ¿De cuál hablamos?

> **Xander:**
> Una que no encontrarás en
> un libro. La de la isla
> del **Gremio de los Tejedores**

### 3.4 Primera decisión *(choice `act1_hall_choice` #0)*
El jugador elige la réplica de Linus:

| # | Opción |
|---|--------|
| 1 | ¿Los Tejedores? |
| 2 | Era mi leyenda favorita |
| 3 | ¿Qué pasó con ellos? |

**Respuesta de Xander** (igual sea cual sea la opción; cluster `A1_HALL_XANDER_ABILITY`):

> **Xander:**
> Según la leyenda
> Fueron un gremio
> capaz de tejer hechizos

> **Xander:**
> Para él no era una leyenda
> Los **Pastores** la cantaban
> como cierta

> **Xander:**
> Desaparecieron sin dejar rastro
> Aunque muchos creen
> que era solo un cuento

Continúa (cluster `A1_HALL_XANDER_FATHER_WANTED`):

> **Xander:**
> Tu padre quería encontrar
> **la isla donde vivían**

> **Clio:**
> Nuestro destino es
> documentar hechos,
> no perseguirlos

> **Xander:**
> Linus tiene diecisiete años
> Esa era mi edad cuando
> viajé por el mundo

> **Xander:**
> Y la edad de su padre cuando
> llegó aquí

> **Xander:**
> Un año antes de que
> le acogiéramos
> como uno de los nuestros

> **Linus:**
> Madre...

### 3.5 Segunda decisión *(choice `act1_hall_choice` #1)*
El jugador elige de nuevo la réplica de Linus:

| # | Opción |
|---|--------|
| 1 | Tengo que ir a la isla |
| 2 | ¿Vendrías conmigo? |

Clio se gira hacia él. **Respuesta de Clio** (cluster `A1_HALL_CLIO_IF_XANDER_WANTS`):

> **Clio:**
> Si Xander lo quiere, así será
> Pero no irás solo

> **Clio:**
> Nunca me ha gustado viajar
> Pero no dejaré que vayas solo

→ Transición a **`act1_forest`**.

> **Nota de guion:** en el CSV existen también las líneas alternativas `A1_HALL_LINUS_MOTHER` ("Madre...") y `A1_HALL_CLIO_I_WONT_LET_YOU`, ya integradas en los clusters anteriores. Aún no hay ramificación real según la opción elegida: ambas decisiones son de sabor y desembocan en la misma respuesta.

---

## Escena 4 — El Bosque (`act1_forest`)

**Tutorial de hechizos y primer combate contra los WeaverGhosts.** Cierre de la demo técnica.

### Puesta en escena
- Bosque de scroll horizontal amplio (mapa de 1440 px), de noche al empezar (`forest_dark_pal`) y con **la vara equipada** (cambia el sprite de Linus).
- **Linus** (personaje activo) y **Clio** entran por la izquierda; **Clio sigue a Linus** automáticamente (`follow`).
- Hechizos habilitados desde el arranque: **TRUENO**, **ESCONDERSE**, **ABRIR**, **DORMIR**.

### 4.1 Entrada
Linus entra en escena y posa en reposo. Rótulo/cluster inicial (`A1_FOREST_SOME_TIME_LATER`):

> Algún tiempo después

> **Linus:**
> Se aproximan enemigos
> Tenemos que estar atentos
> Quédate cerca, madre

> **NOTA**: Puede que esta escena o estos
> hechizos no estén en el
> juego cuando esté terminado

> Se ha decidido incluirla
> en esta demo técnica
> como prueba de ciertas mecánicas

> Pulsa **START** para ver
> tu inventario de hechizos

### 4.2 Aviso de mando de 3 botones *(hook `act1_forest_pad_hint`)*
Si el mando conectado es de **3 botones**, aparece (cluster `A1_FOREST_3BUTTONS`):

> Vaya, parece que estás usando
> **un mando de tres botones**

> El juego completo necesitará
> **seis botones**, pero para esta demo
> solo necesitas tres

### 4.3 Amanece *(hook `act1_forest_day`)*
La paleta funde del bosque nocturno al **bosque de día**.

### 4.4 Paseo libre
Se activan **movimiento**, **scroll**, **interfaz** y **hechizos**. El jugador avanza (con la cámara siguiéndole) hasta llegar a la zona de combate (aprox. 360 px de scroll).

### 4.5 Combate contra los WeaverGhosts *(hook `act1_forest_enemies` + op `combat`)*
Se oculta la interfaz un instante; aparece la **paleta de enemigos**. Todo el mundo se detiene y surgen **dos WeaverGhosts** (uno por la derecha, otro por la izquierda), que avanzan hacia Linus. Comienza el **combate**.

**Mecánica del combate (tutorializada con diálogos contextuales de Linus):**
- Si un fantasma **golpea** a Linus:
  > **Linus:**
  > Eso ha dolido
  >
  > **Linus:**
  > Puedo probar a esconderme
  > o tratar de invocar
  > al trueno
- Al intentar **contraatacar el hechizo enemigo** en mal momento (pista de `onRejected` del trueno):
  > **Linus:**
  > Tengo que contraatacar
  > justo después de su hechizo
- Otras pistas disponibles del combate/inversión de patrones:
  > **Linus:**
  > Si reproduzco al revés
  > las notas, podré
  > contraatacar este hechizo
  >
  > **Linus:**
  > Quizá deba pensar
  > al revés
- Al **esconderse** con éxito:
  > **Linus:**
  > Buena idea. Así no me verán
- Si intenta lanzar un hechizo estando escondido:
  > **Linus:**
  > No puedo lanzar hechizos
  > si estoy escondido

> **Patrones de hechizo relevantes** (notas): **TRUENO** = MI FA SOL SOL · **ESCONDERSE** = FA SOL SOL FA (palíndromo, corta el hechizo enemigo en curso) · La **forma invertida** del trueno (SOL SOL FA MI) contrarresta el trueno enemigo.

### 4.6 Cierre de la demo
Superado el combate, se oculta la interfaz y aparece el mensaje final (cluster `A1_FOREST_THATS_ALL`):

> **¡Esto es todo!**
> (por ahora)

> Gracias por probar la demo técnica
> Síguenos por X o BlueSky
> **@GeeseBumpsGames**

> Apaga tu consola
> y haz algo constructivo
> como jugar un poco al frontón

> o preparar la cena,
> o organizar tu cajón de calcetines
> alfabéticamente.

**Fundido a negro** y **reinicio** de la ROM (`hard_reset`).

---

## Apéndice — Banco de pruebas (`act1_test`) · *no canónico*

`act1_test` **no pertenece al guion**: es una escena de QA que ejercita todos los opcodes del DSL (diálogos con caras/lados, animaciones, choices anidados con reintentos, casts guionizados, puzzles de secuencia —normal e invertido—, y dos oleadas de combate). Se accede con `HACK_START_SCENE "act1_test"` o desde la smoke ROM. Su intro y su HUB de 4 opciones (Diálogos y quiz · Hechizos y puzzles · Combate · Terminar test) están documentados en `docs/test_scene.md`. Se deja constancia aquí para que no se confunda con contenido del Acto 1.

---

## Pendiente / ideas para el guion definitivo

- [ ] Ramificación real de las decisiones del Salón (hoy son de sabor; ambas convergen).
- [ ] Consolidar qué mecánicas del bosque son definitivas y cuáles son solo de la demo técnica.
- [ ] Añadir acotaciones de dirección (música, planos, tiempos) cuando se cierren.
- [ ] Revisar el orden del cluster inicial del bosque (`A1_FOREST_SOME_TIME_LATER` mezcla el rótulo, la línea de "enemigos aproximándose" y las notas de demo).
- [ ] Definir la transición tras el bosque en la versión completa (hoy la demo hace `hard_reset`).
