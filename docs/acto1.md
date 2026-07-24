# The Weave — Acto 1: estado y pendientes

> El **rediseño narrativo y jugable del Acto 1 está terminado**. Este documento
> recoge **cómo está hoy** (escena por escena) y **lo que queda por hacer**.
>
> Dónde vive cada cosa (fuente de verdad, no la dupliques aquí):
> - **Diálogos definitivos**: `data/texts.csv` (ES/EN, en clusters).
> - **Guiones de escena**: `data/scenes/act1/<escena>.scene` + lógica en
>   `src/scenes/act1/<escena>.c`.
> - **Mecánicas**: combate en `docs/combat.md`, hechizos en `docs/spells.md`,
>   cómo escribir escenas en `docs/scenes.md`.
> - **Motor y flujo global**: `AGENTS.md` (§1 tiene el mapa de escenas del acto).

Regla estructural del acto: **Linus no puede lanzar hechizos hasta encontrar el
bastón**. Antes de eso solo explora, dialoga y combate con la acción principal.

---

## Contexto narrativo

Han pasado cien años desde que Bobbin destruyó el Gran Telar para dividir el mundo
y sellar a Caos lejos de los vivos. Desde entonces la magia se ha vuelto tabú:
algunos la niegan, otros la recuerdan como la causa de la casi-destrucción del
mundo, y el Gremio de los Historiadores ha contribuido a enterrar parte de esa
verdad bajo capas de prudencia, miedo y silencio.

Linus Wordweaver cumple hoy diecisiete años. Vive con su madre, Clio, entre libros,
mapas y relatos incompletos. Su padre, Aiden, murió dejando una obsesión que nunca
culminó: encontrar la Isla de los Tejedores.

El Acto 1 es el **llamado a la aventura**: empieza con una visita onírica del cisne,
pasa por el hogar de los historiadores, y termina con la llegada a la Isla de Loom,
el hallazgo del bastón, la primera irrupción manifiesta de la magia y el regreso
hacia la nave mientras la sombra de Caos vuelve a moverse.

## Reparto

| Personaje | Rol |
|---|---|
| **Linus Wordweaver** | Protagonista, 17 años. Historiador por educación, heredero involuntario de una historia mágica. |
| **Clio Wordweaver** | Madre de Linus. Historiadora, prudente, secretamente conocedora de magia antigua. |
| **Xander Wordweaver** | Maestro historiador. Mentor severo pero lúcido. |
| **Bobbin / El Cisne** | Aparición profética. Guía remoto de Linus. |
| **Aiden Firmflanks** | Padre fallecido de Linus. Presencia a través de recuerdos, libros y legado. |
| **Jabalíes de la isla** | Primer enemigo físico (de contacto). |
| **Espectros del Caos** | Primera manifestación abierta del deterioro del tejido (enemigo de patrones). |

---

## Estado por escena

El acto está **completo mecánicamente** y con los **textos definitivos** ya
vertidos en `data/texts.csv`. Cada fila del flujo (AGENTS §1) está montada y jugable;
lo que queda es afinar posiciones/tiempos con playtest y el arte/audio definitivo.

### Esc. 1 — Dormitorio (`act1_bedroom`)
Cinemática del cisne (flash + diálogo profético) y despertar. Exploración de items:
cama, silla, ventana, **retrato** (sobre el cajón izquierdo), **baúl** y **armario /
partitura**. Al examinar la partitura se aprende el patrón **DORMIR** (teórico, aún
no lanzable); releerla da un texto propio ("Cuatro notas..."). Cierra con la llamada
de Clio.

### Esc. 2 — Pasillo del gremio (`act1_corridor`)
Linus aparece bajo la puerta de la derecha. Exploración lateral: dos **libros** (con
texto distinto en la relectura), tres puertas y los mapas. **Gating**: hay que leer
los dos libros para poder salir hacia el salón.

### Esc. 3 — Salón del gremio (`act1_hall`)
Escena narrativa con Clio y Xander. **Dos elecciones** (`choice`/`say_response`): la
leyenda de los Tejedores y el encargo de viajar a la isla. Convergen en "prepararé
lo necesario".

### Esc. 4 — Costa, lugar de paso (`act1_coast`)
Rótulo de llegada. La **gaviota** alza el vuelo al llegar a la altura del **árbol
rojo**, que es un objeto examinable. Sin combate: es un tramo de paso hacia el
interior.

### Esc. 4b — Bosque del atardecer (`act1_island`)
Linus lleva **antorcha**. Primer **combate de contacto**: los **jabalíes**
(persiguen y muerden; el arma es el golpe con A). Al empezar avisa *"Pulsa A para
actuar"* y Clio suelta la pista: *"A lo mejor les asusta el fuego"* — ambas se
repiten en cada reintento.

Ganado el combate, **un jabalí rezagado entra corriendo por la izquierda**, se
planta junto a Clio y la muerde: ella encaja el golpe y **queda herida en el
suelo** (pose sostenida) durante todo el diálogo, hasta que canta el patrón de
**CURACIÓN** y se levanta. Linus lo aprende y lo anota (jingle + notas + menú de
pausa, como la nana), aunque usa la nota más alta y no podrá cantarlo hasta mucho
más adelante. Al final del tramo aparece la choza a lo lejos.

### Esc. 5 — Choza / telar roto (`act1_hut`)
Sin antorcha dentro (sprite norod). Exploración libre con **inspecciones** opcionales
(bastidor, hilos). Interactuar con el **bastón** dispara la cutscene: Linus lo **coge** (anim GRAB)
y **entonces** cae el rayo, se inscribe el patrón **ELECTRICIDAD** y suena la
primera **voz de Bobbin**. Al coger
el bastón se apaga la antorcha y entra el **límite de notas** (solo las 3 primeras:
una superior avisa y cancela el patrón). Al salir, una **sombra** cruza entre los
restos.

### Esc. 6 — Regreso nocturno (`act1_return`)
Mismo bosque **de derecha a izquierda** (scroll `user_left`), ya sin antorcha.
Emboscada de **jabalíes** que solo se ahuyenta con el **TRUENO** (combate de contacto
en modo trueno; pista de Bobbin al caer). Después, los **espectros del Caos**
(combate de patrones: se vencen con el trueno invertido). Lanzarles el trueno
directo no les hace nada y Linus lo constata (*"No parece afectarles"*): la
solución la da Bobbin al caer derrotado, no se regala antes. Ambos combates con
reintento por derrota, y tras el consejo del cisne hay **fundido a negro y una
pausa** antes de volver a intentarlo.

### Esc. 7 — Cierre en la nave (`act1_coast_end`)
Tormenta. Diálogo final sobre la magia escondida y el rumbo al Gremio de los
Pastores. **Pausa interactiva** (mirar la isla por última vez, A para embarcar),
embarque, **fundido a negro** y, ya en la oscuridad, el susurro final de Bobbin
("Tranquilo, Linus. Nunca estarás solo"). **FIN DEL ACTO 1**.

---

## Estado de los assets

Todo lo listado existe en `res/` y compila. Lo marcado *(placeholder)* es arte
funcional generado por código, pensado para sustituirse por `.ase`/audio definitivos
sin tocar código (mismo archivo, mismas dimensiones y paleta).

- **Fondos**: dormitorio, pasillo, salón y bosque (definitivos); **costa**
  (`coast_bg_*`, paletas `coast_day`/`coast_storm`) y **choza** (`hut_bg_*`, con
  agujero del techo que deja ver la luna) *(placeholder)*.
- **Personajes**: Linus (vara/sin vara/antorcha), Clio (con su pose de herida en
  la fila extra), Xander, cisne.
- **Enemigos**: jabalí (con sombra) y WeaverGhost (hace de espectro del Caos).
- **Items**: retrato y baúl (dormitorio), bastón (con frame «resonando»), bastidor
  e hilos (choza), gaviota y árbol (costa) *(placeholder)*.
- **FX**: rayo `fx_lightning_sprite` (3 frames) *(placeholder)*. El resto de
  flashes/resplandores se hacen por paleta, sin asset.
- **Interfaz**: icono de patrón `pattern_heal` (Curación) *(placeholder)*.
- **Audio**: SFX ambientales `snd_ambient_waves/wind/seagull/thunder/steam`
  *(placeholder procedural)*.

---

## Lo que queda por hacer

**Arte definitivo** (sustituir placeholders):
- Fondos de costa y choza; paleta de atardecer propia para `act1_island` (hoy usa la
  diurna del bosque).
- Items de costa/choza, FX del rayo e icono de Curación.

**Audio**:
- Música de las escenas (composición VGM).
- Jingles propios de SLEEP, HEAL y EN_BITE (hoy silenciosos).

**Bugs / pulido conocidos**:
- Playtest de posiciones y tiempos de las cutscenes (paradas de personajes, X de los
  combates, duración de flashes).

**Diseño pendiente (no bloquea)**:
- Subir el límite de notas más adelante en el juego para que CURACIÓN sea jugable.
- Unificar los dos directores de combate en uno solo (ver `docs/combat.md` §8).

---

## Tono

El acto debe sonar a fábula oscura: íntimo, melancólico y extraño, con personajes
contenidos que rara vez dicen todo lo que piensan, pero lo dejan ver entre líneas.
