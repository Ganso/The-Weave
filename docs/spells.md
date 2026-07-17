# Cómo crear un hechizo nuevo — guía completa

Esta guía está pensada para que **puedas crear un hechizo desde cero aunque no
sepas programar**. Explica qué es cada cosa con palabras normales, qué archivos
hay que tocar, qué escribir exactamente en cada uno y, sobre todo, **cuándo usar
cada opción y cuándo NO usarla**. No hace falta que leas ningún otro documento:
aquí está todo.

> Nota sobre el vocabulario: el juego está escrito en un lenguaje de programación
> llamado **C**. Vas a editar unos cuantos archivos de texto (los que terminan en
> `.c` y `.h`). No necesitas entender el lenguaje; solo copiar el patrón que se
> explica aquí y cambiar los nombres y números. Los archivos de texto se abren y
> editan con cualquier editor (el mismo que uses para escribir los diálogos).

---

## 1. ¿Qué es un hechizo en este juego?

En *The Weave*, lanzar un hechizo consiste en **tocar una melodía corta** con los
botones del mando. Cada botón es una nota musical:

| Botón del mando | Nota musical |
|---|---|
| A | MI |
| B | FA |
| C | SOL |
| X | LA |
| Y | SI |
| Z | DO |

Un hechizo es **una secuencia de 3 o 4 notas**. Por ejemplo, el hechizo TRUENO es
`MI FA SOL SOL`, que se toca pulsando `A B C C`. Cuando el jugador toca las notas
correctas en orden, el hechizo se lanza y ocurre su efecto (un destello, un daño,
abrir una puerta...).

### La forma directa y la forma invertida

Toda melodía se puede tocar **al derecho** (forma *directa*) o **al revés** (forma
*invertida*). Tocar `MI FA SOL SOL` al revés es `SOL SOL FA MI`. Muchos hechizos
usan esto: por ejemplo, el TRUENO tocado al revés sirve para **contrarrestar** el
trueno de un enemigo.

- Si las notas de un hechizo **se leen igual al derecho y al revés** (como
  `FA SOL SOL FA`), se dice que es un **palíndromo**. Un palíndromo no tiene forma
  invertida distinta, así que solo existe "una versión" de él.
- Si las notas son distintas al invertirlas, el hechizo tiene **dos versiones**: la
  directa y la invertida, y puedes darles comportamientos diferentes.

### ¿Por qué hay "dos carriles" a la vez?

Durante un combate, puede haber **dos hechizos vivos al mismo tiempo**: el del
enemigo y el del jugador. Esto es imprescindible para el *counter* (contraataque):
el enemigo lanza su trueno y, mientras ese trueno sigue "en el aire", el jugador
toca el trueno invertido para anularlo. El juego lleva internamente dos "carriles"
o **slots**: uno para el jugador y otro para el enemigo. **Tú no tienes que
gestionar esto**: el juego lo hace solo. Solo necesitas saber que existe, porque a
veces un hechizo quiere mirar "¿qué está haciendo el enemigo ahora mismo?".

### El "motor" y los "ganchos": quién hace qué

Piensa en el juego como una fábrica con una **cinta transportadora** (el *motor*).
El motor se encarga de TODO el trabajo pesado y repetitivo de cualquier hechizo:

- comprobar que el jugador tocó las notas bien,
- arrancar el efecto,
- contar cuántos fotogramas (frames) lleva el efecto en marcha,
- terminar el hechizo cuando se acaba su tiempo,
- limpiar todo al final.

**Tú nunca tocas nada de eso.** Lo único que tú aportas es la **personalidad** de
tu hechizo: "cuando empiece, guarda el color del cielo", "a mitad del efecto, haz
1 de daño al enemigo", "cuando termine, devuelve el color del cielo". Esos trocitos
de personalidad se llaman **ganchos** (en inglés, *hooks*). Son opcionales: pones
solo los que tu hechizo necesita y dejas el resto vacíos.

---

## 2. Vocabulario rápido

- **Nota**: una de las seis notas musicales (MI, FA, SOL, LA, SI, DO), cada una
  asociada a un botón.
- **Directa / invertida**: la melodía tocada al derecho o al revés.
- **Palíndromo**: melodía que se lee igual en los dos sentidos (no tiene invertida
  distinta).
- **Slot (carril)**: uno de los dos espacios donde puede haber un hechizo activo:
  el del jugador y el del enemigo.
- **Frame (fotograma)**: la unidad de tiempo del juego. El juego va a 50 fotogramas
  por segundo en Europa (PAL) o 60 en América/Japón (NTSC). **Nunca escribas un
  número fijo de fotogramas**: usa siempre `SCREEN_FPS` (que vale 50 ó 60 según la
  consola). Así, `SCREEN_FPS` = 1 segundo, `SCREEN_FPS * 2` = 2 segundos,
  `SCREEN_FPS / 2` = medio segundo. Esto hace que tu hechizo dure lo mismo en
  cualquier consola.
- **Motor**: la parte del juego que gestiona el ciclo de vida de cualquier hechizo.
- **Gancho (hook)**: un trocito de comportamiento propio de tu hechizo que el motor
  llama en un momento concreto.
- **Fase**: un efecto (un destello o un daño) descrito como un dato, no como código.
  El motor las ejecuta por ti.
- **Zona**: una etiqueta que la escena pone en el escenario (por ejemplo "estoy
  junto al caldero") para permitir o prohibir ciertos hechizos.

---

## 3. La ficha del hechizo (los "campos")

Cada hechizo se define rellenando una **ficha**. Cada línea de la ficha es un
**campo**. Aquí tienes todos los campos, qué significan y cuándo usarlos. No todos
son obligatorios.

| Campo | Qué significa | Cuándo y cómo se usa |
|---|---|---|
| `id` | El identificador del hechizo. | Pon aquí el nombre `SPELL_LOQUESEA` que reservaste (ver paso 1 de la receta). Siempre obligatorio. |
| `notes` | La melodía: la lista de notas en orden. | Escribe entre llaves las notas, p. ej. `{ NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }`. Recuerda la tabla de botones: A=MI, B=FA, C=SOL, X=LA, Y=SI, Z=DO. |
| `noteCount` | Cuántas notas tiene la melodía. | Casi siempre `4`. Puede ser `3` (el mordisco enemigo usa 3). Debe coincidir con el número de notas que pusiste en `notes`. |
| `isPalindrome` | ¿La melodía se lee igual al derecho y al revés? | `true` si es palíndromo (como `FA SOL SOL FA`); `false` si no. Si es palíndromo, el juego sabe que no existe una "forma invertida" distinta. |
| `counterable` | ¿El jugador puede contrarrestar este hechizo? | Solo tiene sentido en hechizos **de enemigo**. `true` = el jugador puede anularlo tocando su versión invertida. En hechizos de jugador, déjalo `false`. |
| `baseDuration` | Cuánto dura el efecto, en fotogramas. | Escríbelo SIEMPRE con `SCREEN_FPS`. Ejemplos: `SCREEN_FPS` (1 s), `SCREEN_FPS * 2` (2 s), `SCREEN_FPS * 3 / 2` (1,5 s), `SCREEN_FPS / 2` (medio segundo). Cuando pase ese tiempo, el hechizo termina solo. |
| `rechargeInit` | Cuánto tarda un enemigo en poder lanzar el hechizo por primera vez al empezar el combate. | Solo para hechizos **de enemigo**. En hechizos de jugador, no lo pongas. Se ajusta probando el juego hasta que el ritmo del combate se sienta justo. |
| `enabled` | ¿El jugador ya tiene desbloqueado este hechizo desde el principio? | Normalmente `false`: el hechizo empieza bloqueado y lo desbloquea una escena cuando toca (ver sección 9). Ponlo `true` solo si quieres que esté disponible siempre. |
| `phases` / `phaseCount` | Los efectos "automáticos" del hechizo (destellos, daños) descritos como datos. | Opcional. Ver la sección 5. Si no usas fases, no los pongas. |
| `icon` | El dibujito del hechizo en la interfaz. | No se rellena aquí directamente; el icono se conecta en el archivo de la interfaz (ver receta). Puedes ignorar este campo en la ficha. |

Y luego están los **ganchos**, que también son campos de la ficha pero merecen su
propia sección.

---

## 4. Los ganchos (comportamiento propio del hechizo)

Un gancho es una pequeña función que **tú escribes** y que **el motor llama** en un
momento concreto de la vida del hechizo. Todos son **opcionales**: si tu hechizo no
necesita uno, sencillamente no lo pongas.

Primero, entiende la **línea de tiempo** de un hechizo, de principio a fin:

```
   El jugador toca las notas
            │
            ▼
     ┌─────────────┐   ¿se puede lanzar ahora?  ── NO ──►  onRejected  (avisar al jugador)
     │   canUse    │
     └─────────────┘   ── SÍ ──►  empieza el efecto
                                        │
                                        ▼
                                    onLaunch   (justo al arrancar)
                                        │
                          cada fotograma del efecto:
                                    onUpdate   (+ las fases se ejecutan solas)
                                        │
              ┌─────────────────────────┼─────────────────────────┐
              ▼                          ▼                         ▼
    termina por su cuenta      lo contrarrestan          lo cortan (sin counter)
        onFinish                  onCounter                   onCancel
```

Aquí tienes cada gancho, explicado con detalle:

### `canUse` — "¿se puede lanzar ahora mismo?"

- **Cuándo se llama:** justo antes de lanzar, para decidir si se permite.
- **Para qué sirve:** poner condiciones. Por ejemplo: "solo se puede lanzar junto
  al caldero", "solo si hay un enemigo delante", "la versión invertida solo vale
  durante el trueno enemigo".
- **Qué devuelve:** `true` (sí se puede) o `false` (no se puede).
- **Cuándo NO lo pongas:** si tu hechizo se puede lanzar siempre, sin condiciones.
  Si no pones `canUse`, el motor entiende que siempre se puede.
- **Ojo:** los hechizos lanzados por una escena (de forma "guionizada") **no pasan
  por `canUse`**: el guion manda. `canUse` solo filtra lo que hace el jugador.

### `onRejected` — "avisar de que no se puede"

- **Cuándo se llama:** cuando `canUse` dijo `false` y el jugador aun así lo intentó.
- **Para qué sirve:** dar una pista al jugador con un diálogo. Por ejemplo: "No
  puedo usar ese patrón ahora mismo", o algo más específico como "prueba a tocarlo
  al revés".
- **Cuándo NO lo pongas:** si con el aviso genérico por defecto te basta, o si el
  hechizo no tiene condiciones (sin `canUse` no hay rechazo posible).

### `onLaunch` — "acabo de empezar"

- **Cuándo se llama:** en el instante en que el efecto arranca.
- **Para qué sirve:** preparar cosas al principio: sonar el "jingle" (efecto de
  sonido del hechizo), guardar el color original de algo que vas a cambiar, o
  reaccionar a lo que está haciendo el enemigo (por ejemplo, "comerse" el trueno
  enemigo).
- **Cuándo NO lo pongas:** si no hay nada que preparar al arrancar.

### `onUpdate` — "un fotograma más de efecto"

- **Cuándo se llama:** en **cada** fotograma mientras el efecto está en marcha.
- **Para qué sirve:** efectos que cambian a cada instante y que no se pueden
  describir como una simple fase (por ejemplo, un parpadeo que alterna colores a mano
  cada fotograma).
- **Detalle importante:** si tu `onUpdate` devuelve `true`, el hechizo termina ya
  mismo. Si devuelve `false`, sigue. Aunque nunca devuelvas `true`, el hechizo
  **siempre** termina solo cuando se cumple su `baseDuration`.
- **Cuándo NO lo pongas:** si tu efecto visual se puede describir con **fases** (ver
  sección 5), es mejor usar fases y dejar `onUpdate` vacío. Las fases son más
  sencillas y menos propensas a errores.

### `onFinish` — "he terminado bien"

- **Cuándo se llama:** SOLO cuando el hechizo termina de forma **natural** (se le
  acabó el tiempo). No se llama si lo contrarrestan ni si lo cortan.
- **Para qué sirve:** el efecto final "de verdad" y la limpieza tras un final normal.
  Por ejemplo: aplicar el daño diferido de un trueno enemigo al jugador, o devolver
  el color del cielo al valor que guardaste en `onLaunch`.
- **Cuándo NO lo pongas:** si no hay nada que hacer al terminar.

### `onCounter` — "me han contrarrestado"

- **Cuándo se llama:** cuando el jugador (o quien sea) contrarresta este hechizo.
- **Para qué sirve:** la consecuencia de ser anulado. Típicamente, hacer daño a
  quien lo lanzó y limpiar lo que hubiera cambiado.
- **Cuándo NO lo pongas:** si tu hechizo no es `counterable` (no se puede
  contrarrestar), este gancho nunca se llamaría; déjalo vacío.

### `onCancel` — "me han cortado"

- **Cuándo se llama:** cuando el hechizo se interrumpe **sin** ser un counter (por
  ejemplo, otro hechizo lo corta, o se cancela por una regla del juego).
- **Para qué sirve:** limpiar. Si en `onLaunch` cambiaste algo (un color, por
  ejemplo) y quieres deshacerlo también cuando te cortan, hazlo aquí.
- **Cuándo NO lo pongas:** si no hay nada que limpiar al ser cortado. **Ojo a este
  detalle frecuente:** `onFinish` solo se ejecuta en el final natural. Si guardaste
  un color en `onLaunch` y solo lo restauras en `onFinish`, quedará sin restaurar
  cuando te corten. Si eso te importa, añade también `onCancel` para restaurarlo.

### ¿Qué recibe cada gancho? (el "contexto")

Cuando el motor llama a un gancho, le pasa un paquete de información llamado `ctx`
(de *context*, contexto). Puedes consultar de él:

- `ctx->spellId`: qué hechizo es.
- `ctx->origin`: quién lo lanzó (el jugador, un enemigo, o el guion de una escena).
- `ctx->reversed`: si se lanzó en su forma invertida (`true`) o directa (`false`).
- `ctx->frameCounter`: cuántos fotogramas lleva el efecto (solo para leer; no lo
  cambies).
- `ctx->enemyId`: si lo lanzó un enemigo, cuál; si lo lanzó el jugador, el enemigo
  que tiene delante (o "ninguno").
- `ctx->zoneId`: en qué zona del escenario se lanzó (ver sección 6).

---

## 5. Las fases (efectos automáticos, sin programar apenas)

Muchos efectos son tan sencillos que no merece la pena escribir código fotograma a
fotograma. Para eso están las **fases**: describes el efecto como una **lista de
datos** y el motor lo ejecuta por ti. Hay **dos tipos** de fase:

- **Fase de destello visual** (`PHASE_VISUAL_FLASH`): pone un color concreto en una
  ranura de la paleta durante un tramo de tiempo. Sirve para tintar la pantalla (un
  fogonazo naranja de fuego, un destello blanco...).
- **Fase de daño** (`PHASE_LOGIC_DAMAGE`): aplica una cantidad de daño a un objetivo
  (el enemigo o el jugador) en un instante concreto.

Cada fase tiene cinco datos, en este orden:

```
{ fotograma_inicio, fotograma_fin, tipo_de_fase, parámetro1, parámetro2 }
```

- **fotograma_inicio** y **fotograma_fin**: el tramo de tiempo (contado desde que
  empieza el efecto) en el que la fase está activa. Recuerda usar `SCREEN_FPS`.
- **Para un daño**, que es algo puntual (ocurre una sola vez), pon el **mismo valor**
  en inicio y fin. Así el daño se aplica una única vez, en ese instante.
- **tipo_de_fase**: `PHASE_VISUAL_FLASH` o `PHASE_LOGIC_DAMAGE`.
- **parámetro1 / parámetro2**: dependen del tipo:
  - En un destello: parámetro1 es la ranura de paleta a tintar (usa `PAL0_COL4`, que
    es la que suele controlar el color del cielo/fondo) y parámetro2 es el color.
  - En un daño: parámetro1 es el objetivo (`PHASE_TARGET_ENEMY_ACTIVE` para el
    enemigo, `PHASE_TARGET_PLAYER` para el jugador) y parámetro2 es cuánto daño.

Ejemplo comentado:

```c
// Destello naranja durante los 2 primeros segundos:
fases[0] = (SpellPhase){ 0, SCREEN_FPS * 2, PHASE_VISUAL_FLASH, PAL0_COL4, COLOR_NARANJA };
// 2 puntos de daño al enemigo, UNA vez, al llegar al segundo 1 (inicio == fin):
fases[1] = (SpellPhase){ SCREEN_FPS, SCREEN_FPS, PHASE_LOGIC_DAMAGE, PHASE_TARGET_ENEMY_ACTIVE, 2 };
```

### ¿Fases o `onUpdate`?

- Usa **fases** siempre que puedas: destellos de un color, uno o varios tramos de
  color seguidos, un daño en un instante. Es lo más simple y lo menos propenso a
  fallos.
- Usa **`onUpdate`** solo cuando el efecto sea tan particular que no se puede
  describir con fases (un cálculo distinto cada fotograma, alternar colores a mano,
  etc.).

> Si usas una fase de destello para tintar un color, recuerda **guardar el color
> original en `onLaunch` y restaurarlo en `onFinish`** (y en `onCancel` si te
> importa el caso de que corten el hechizo). El destello cambia el color mientras
> dura, pero devolverlo a su sitio es cosa tuya.

---

## 6. Las zonas (para puzzles y hechizos de sitio concreto)

A veces quieres que un hechizo **solo funcione en un lugar del escenario**. Por
ejemplo, el FUEGO solo se puede lanzar junto al caldero. Para eso están las
**zonas**:

- La **escena** pone la zona actual con una orden en su guion (por ejemplo
  `zone ZONE_CAULDRON` fija "estamos junto al caldero").
- Tu gancho `canUse` recibe esa zona en `ctx->zoneId` y puede decidir: "si no
  estamos en la zona del caldero, devuelvo `false`".

Si tu hechizo no depende del sitio, no toques nada de esto.

---

## 7. Mapa de archivos: qué vas a tocar y para qué

Los hechizos están **separados por dueño y uno por fichero**: los del jugador en
`src/spells/player/` y los del enemigo en `src/spells/enemy/`. Un hechizo nuevo es
un fichero `.c`/`.h` en la carpeta que le toque (aquí usaremos VIENTO, un hechizo
del jugador → `src/spells/player/`).

Crear un hechizo toca varios archivos. Aquí tienes el mapa completo para que sepas
a dónde vas antes de empezar:

| Archivo | Qué haces en él |
|---|---|
| `src/spells/constants_spells.h` | Reservar el nombre/número del hechizo (`SPELL_LOQUESEA`). |
| `src/spells/player/loquesea.c` y `.h` | **El hechizo en sí**: su ficha y sus ganchos. Los creas nuevos. (Un hechizo de enemigo iría en `src/spells/enemy/`.) |
| `src/spells/player/player_spells.c` | Una línea para que el juego "registre" tu hechizo al arrancar. |
| `res/res_interface.res` | Declarar el dibujo (icono) del hechizo. |
| `src/interface/interface.c` | Dos líneas para que el icono se pinte en el HUD y en la pausa. |
| `src/audio/sound.c` | El efecto de sonido (jingle) que suena al lanzarlo. |
| `src/smoke/smoke_cases.h` | Un par de casos para probar el hechizo aislado. |

Además, para **desbloquearlo** en el juego, escribirás una orden en el guion de una
escena o en un gancho de escena (ver sección 9).

---

## 8. Receta completa, paso a paso: crear el hechizo VIENTO

Vamos a crear un hechizo de jugador de ejemplo que toca **todos los conceptos
importantes**. Diseño: *"una ráfaga de viento que solo se puede lanzar contra un
enemigo que tengas delante; destella verde y le hace 1 punto de daño a mitad del
efecto"*.

Este VIENTO usa: `canUse` con condición + su `onRejected`, `onLaunch`, dos fases
(una visual y una de daño), `onFinish`, y el cableado completo (nombre, registro,
icono, jingle, desbloqueo, pruebas).

### Paso 1 — Reservar el nombre del hechizo

Abre `src/spells/constants_spells.h`. Verás una lista de nombres con números:

```c
#define SPELL_THUNDER      0
#define SPELL_HIDE         1
#define SPELL_OPEN         2
#define SPELL_SLEEP        3
#define SPELL_FIRE         4
#define SPELL_LIGHT        5
#define SPELL_PLAYER_COUNT 6
// Enemy spells
#define SPELL_EN_THUNDER   6
#define SPELL_EN_BITE      7
#define SPELL_COUNT        8
```

Los hechizos **de jugador** van arriba, antes de `SPELL_PLAYER_COUNT`. Los de
enemigo van después. Como VIENTO es de jugador, lo insertamos justo antes de
`SPELL_PLAYER_COUNT` y **subimos en uno todos los números de abajo** (esos números
son solo cuentas internas; correrlos no rompe nada):

```c
#define SPELL_THUNDER      0
#define SPELL_HIDE         1
#define SPELL_OPEN         2
#define SPELL_SLEEP        3
#define SPELL_FIRE         4
#define SPELL_LIGHT        5
#define SPELL_WIND         6      // ← nuevo
#define SPELL_PLAYER_COUNT 7      // era 6
// Enemy spells
#define SPELL_EN_THUNDER   7      // era 6
#define SPELL_EN_BITE      8      // era 7
#define SPELL_COUNT        9      // era 8
```

Regla sencilla: **cada número debe ser único y la lista, consecutiva**. Si insertas
uno, todos los de debajo suben en uno.

### Paso 2 — Crear el hechizo: `src/spells/player/wind.h` y `.c`

Primero el archivo `.h`, que es una simple "tarjeta de presentación" del hechizo:

```c
// src/spells/player/wind.h
#ifndef _PLAYER_WIND_H_
#define _PLAYER_WIND_H_
void wind_init(void); // Registra el hechizo VIENTO (lo llamará el registrador)
#endif
```

Ahora el archivo `.c`, que es el hechizo de verdad. Empieza con una lista de
`#include` (le dice al juego qué piezas necesita este archivo). **Copia este bloque
tal cual**; sirve para casi cualquier hechizo:

```c
// src/spells/player/wind.c
#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "narrative/narrative.h"
#include "audio/audio.h"
#include "spells/player/wind.h"   // la tarjeta de presentación de arriba

#define COLOR_WIND_VDP  RGB24_TO_VDPCOLOR(0x66FF88)   // verde ráfaga (color en formato de la consola)

static SpellPhase wind_phases[2];   // aquí guardaremos las dos fases
static u16 wind_saved_color;        // aquí guardaremos el color original del cielo
```

> El truco `RGB24_TO_VDPCOLOR(0x66FF88)` convierte un color web normal (aquí un
> verde, `66FF88`) al formato que entiende la consola. Cambia esos seis dígitos por
> el color que quieras.

Ahora los ganchos. **`canUse`**: solo se permite si es la forma directa y hay un
enemigo delante:

```c
// ¿Se puede lanzar ahora? Solo directo y solo si hay un enemigo delante.
static bool wind_can_use(const SpellContext *ctx)
{
    if (ctx->reversed) return false;               // no tiene forma invertida útil
    if (ctx->enemyId == ENEMY_NONE) return false;  // necesita un enemigo al que golpear
    return true;                                   // en cualquier otro caso, adelante
}
```

**`onRejected`**: si el jugador lo intenta y no se puede, le mostramos un aviso:

```c
// Si canUse dijo que no, avisamos con un diálogo genérico.
static void wind_on_rejected(SpellContext *ctx)
{
    (void)ctx;   // (no usamos ctx aquí; esta línea evita un aviso del compilador)
    talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN], false);
}
```

**`onLaunch`**: al arrancar, guardamos el color del cielo (para poder restaurarlo
luego) y sonamos el jingle:

```c
static void wind_on_launch(SpellContext *ctx)
{
    (void)ctx;
    wind_saved_color = PAL_getColor(PAL0_COL4);  // guardar el color de antes
    play_spell_jingle(SPELL_WIND);               // sonar el efecto de sonido
    // el destello verde y el daño los hacen las fases, no hace falta código aquí
}
```

**`onFinish`**: al terminar de forma natural, devolvemos el color del cielo:

```c
static void wind_on_finish(SpellContext *ctx)
{
    (void)ctx;
    PAL_setColor(PAL0_COL4, wind_saved_color);   // restaurar el cielo
}
```

Y, por último, la función que **arma la ficha** del hechizo. Aquí se rellenan las
dos fases y todos los campos:

```c
void wind_init(void)    // el juego llama a esto al arrancar
{
    // Fase 1 (visual): destello verde durante el primer segundo.
    wind_phases[0] = (SpellPhase){ 0, SCREEN_FPS, PHASE_VISUAL_FLASH, PAL0_COL4, COLOR_WIND_VDP };
    // Fase 2 (daño): 1 punto de daño al enemigo, UNA vez, a mitad del efecto (inicio==fin).
    wind_phases[1] = (SpellPhase){ SCREEN_FPS/2, SCREEN_FPS/2, PHASE_LOGIC_DAMAGE, PHASE_TARGET_ENEMY_ACTIVE, 1 };

    spell_defs[SPELL_WIND] = (SpellDef){
        .id = SPELL_WIND,
        .notes = { NOTE_LA, NOTE_SOL, NOTE_FA, NOTE_MI }, .noteCount = 4,
        .isPalindrome = false,     // LA SOL FA MI al revés es distinto → no es palíndromo
        .counterable = false,      // es de jugador; el enemigo no lo contrarresta
        .baseDuration = SCREEN_FPS, // dura 1 segundo
        .enabled = false,          // empieza bloqueado; lo desbloqueará una escena
        .canUse = wind_can_use,
        .onRejected = wind_on_rejected,
        .onLaunch = wind_on_launch,
        .onFinish = wind_on_finish,
        .phases = wind_phases, .phaseCount = 2,
        // No ponemos onUpdate (el efecto es declarativo, con fases),
        // ni onCounter (no es counterable),
        // ni onCancel (no nos importa restaurar el color si nos cortan).
    };
}
```

Fíjate en **por qué faltan** algunos ganchos: es tan importante saber qué NO poner
como qué poner. No hay `onUpdate` porque el efecto se describe con fases; no hay
`onCounter` porque el hechizo no es `counterable`; no hay `onCancel` porque, si nos
cortan a mitad, no nos importa que el cielo se quede verde un instante (si te
importara, añadirías `onCancel` restaurando el color igual que en `onFinish`).

### Paso 3 — Registrar el hechizo: `src/spells/player/player_spells.c`

El juego necesita "conocer" tu hechizo al arrancar. Abre el **registrador de los
hechizos del jugador**, `src/spells/player/player_spells.c`, y añade dos cosas
(un hechizo de enemigo iría en `src/spells/enemy/enemy_spells.c`, igual):

```c
#include "spells/player/wind.h"    // arriba, junto a los otros includes

// ... dentro de init_player_spells(), junto a los demás *_init():
    wind_init();
```

Si te olvidas de este paso, tu hechizo sencillamente no existirá en el juego.

> **Hechizos "solo de guion"** (sin efecto propio, que solo se lanzan desde una
> escena con `cast`, como ABRIR/DORMIR/CURACIÓN): no necesitan `canUse` propio;
> reutiliza el del motor con `.canUse = spell_scripted_only_can_use` y omite el
> resto de ganchos. Su fichero es mínimo (mira `src/spells/player/sleep.c`).

### Paso 4 — El icono del HUD: `res/res_interface.res` + `src/interface/interface.c`

Primero, crea el dibujo. Necesitas una imagen `res/gfx/interface/pattern_wind.png`
de 32×32 píxeles, con la misma paleta de colores que los iconos ya existentes (los
de FUEGO y LUZ te sirven de referencia visual). Luego decláralo en el archivo de
recursos `res/res_interface.res`, añadiendo esta línea:

```
SPRITE int_pattern_wind "gfx/interface/pattern_wind.png" 4 4 BEST
```

Ahora conecta el icono en `src/interface/interface.c`. Hay **dos** funciones que
pintan iconos (una para el HUD del combate, otra para la lista de la pantalla de
pausa). Añade una línea en **cada una**:

```c
// En la función show_pattern_icon (junto a los otros if de SPELL_...):
if (npattern==SPELL_WIND) nsprite = &int_pattern_wind;

// Y también en la función show_icon_in_pause_list (mismo patrón):
if (npattern==SPELL_WIND) nsprite = &int_pattern_wind;
```

Si te olvidas de una de las dos, el icono no aparecerá en ese sitio.

### Paso 5 — El sonido (jingle): `src/audio/sound.c`

Abre `src/audio/sound.c`, busca la función `play_spell_jingle()` y añade un caso
para tu hechizo. Puedes **reutilizar un sonido existente** mientras no tengas uno
propio:

```c
case SPELL_WIND:
    play_sample(snd_pattern_open, sizeof(snd_pattern_open));
    break;
```

### Paso 6 — Desbloquear el hechizo y prepararlo para probar

El hechizo está `enabled = false`, así que empieza bloqueado. Para que el jugador
pueda usarlo, una escena debe **desbloquearlo** en el momento adecuado (ver la
sección 9 para la explicación completa). En el gancho de preparación de la escena
donde el jugador aprende el viento, pon:

```c
spell_enable(SPELL_WIND);    // desbloqueo silencioso
```

Para **probar el hechizo aislado**, sin tener que llegar hasta esa escena, añade un
par de casos a la ROM de pruebas (`src/smoke/smoke_cases.h`):

```c
{"CHK wind sin enemigo - NO", SMOKE_CHECK, SPELL_WIND, false, 0, false, NULL},
{"CAST wind (1s verde)",      SMOKE_CAST,  SPELL_WIND, false, 0, false, NULL},
```

- El primero (`SMOKE_CHECK`) comprueba una regla: "sin enemigo delante, el viento
  NO se puede usar" (por eso el `false` del final, el resultado esperado).
- El segundo (`SMOKE_CAST`) lanza el hechizo de forma guionizada para ver el
  destello y medir que dura lo que debe. Como es guionizado, no pasa por `canUse`,
  así que verás el destello aunque no haya enemigo; el daño de la fase no hará nada
  porque no hay a quién golpear.

### Paso 7 — Compilar y probar

Para construir el juego, ejecuta desde una terminal, en la carpeta del proyecto:

```
./build-theweave.sh release
```

O, para construir la ROM de pruebas con tus casos del paso anterior:

```
./build-theweave.sh smoke
```

El proceso de construcción hace por ti todas las comprobaciones. Si te olvidaste de
llamar a `wind_init()`, o de un `#include`, el proceso se detendrá y te dirá qué
falta. Cuando el juego arranque, las notas de tu hechizo son **LA SOL FA MI**, que
se tocan con los botones **X C B A**.

---

## 9. Desbloquear el hechizo dentro del juego

Un hechizo con `enabled = false` está bloqueado hasta que algo lo desbloquea. Hay
tres maneras de "activar" un hechizo, según lo que quieras:

- **`spell_enable(SPELL_WIND)`** — desbloqueo **silencioso**. El hechizo pasa a
  estar disponible, sin ningún aviso. Úsalo en la preparación de una escena para
  dejar hechizos listos "de fondo".
- **`activate_spell(SPELL_WIND)`** — desbloqueo **con fanfarria**: suena el jingle y
  se muestran sus notas en pantalla. Úsalo en una escena de "¡has aprendido un
  hechizo nuevo!" para que el jugador lo vea.
- **Lanzar el hechizo desde el guion de una escena** (un lanzamiento "guionizado"):
  esto no desbloquea nada, sino que ejecuta el efecto del hechizo aunque el jugador
  no lo toque. Sirve para cinemáticas. Recuerda que un lanzamiento guionizado **no
  pasa por `canUse`**.

---

## 10. Errores frecuentes y cómo se notan

- **El hechizo no existe / el juego no compila:** casi siempre falta llamar a
  `wind_init()` en `init_player_spells()`, o falta un `#include`. El proceso de compilación
  te dice el archivo y la línea.
- **Los números de `constants_spells.h` no cuadran:** si insertaste un nombre pero
  no subiste en uno los de abajo, habrá dos hechizos con el mismo número y el
  comportamiento será impredecible. Revisa que la lista sea consecutiva y sin
  repetidos.
- **El icono no aparece:** te falta la línea en una de las **dos** funciones de
  `interface.c`, o el nombre del sprite en el `.res` no coincide con el que usas en
  el `.c`.
- **El color del cielo se queda "pillado" en el color del destello:** guardaste el
  color en `onLaunch` pero no lo restauraste (falta `onFinish`, o te cortaron el
  hechizo y no tenías `onCancel`).
- **`noteCount` no coincide con las notas:** si dices que hay 4 notas pero escribes
  3, el hechizo no se reconocerá bien. Cuéntalas.
- **El daño no hace nada en la prueba:** en un `SMOKE_CAST` no hay enemigo, así que
  la fase de daño no tiene objetivo. Es normal; para probar el daño de verdad,
  necesitas un enemigo (en una escena de combate).

Para depurar más a fondo, existe un archivo `src/core/hack.h` con "atajos" de
pruebas que puedes activar temporalmente: por ejemplo `HACK_ALL_SPELLS` (todos los
hechizos desbloqueados), `HACK_ENEMIES_ONE_HP` (enemigos con 1 punto de vida, para
matarlos rápido) o `HACK_PLAYER_INVULNERABLE` (el jugador no recibe daño). Actívalos
mientras pruebas y acuérdate de desactivarlos antes de la versión final.

---

## 11. Los hechizos que ya existen (para inspirarte)

| Hechizo | Notas (botones) | Duración | Qué tiene de especial |
|---|---|---|---|
| TRUENO | MI FA SOL SOL (A B C C) | 4 s | Su forma invertida contrarresta el trueno enemigo; da pistas al jugador con `onRejected`. |
| ESCONDER | FA SOL SOL FA (B C C B) | 4 s | Es palíndromo; corta el hechizo enemigo que esté en curso. |
| ABRIR | FA SI SOL DO (B Y C Z) | 0,75 s | Solo se lanza de forma guionizada (`canUse` siempre dice que no). |
| DORMIR | FA MI DO LA (B A Z X) | 1,25 s | Solo guionizado. |
| FUEGO | MI FA SOL LA (A B C X) | 2 s | Ejemplo completo: zona (solo junto al caldero) + se come el trueno enemigo + fases. Empieza bloqueado. |
| LUZ | SOL LA SI DO (C X Y Z) | 1,5 s | Muestra fases de dos tramos de color (cian→blanco). Se puede lanzar directo E invertido. |
| CURACIÓN | LA SI DO SI (X Y Z Y) | 1,25 s | Solo guionizado. La canta Clio en el acto 1; usa la nota más alta (DO), fuera del límite de notas del jugador hasta el final del juego. |
| EN_THUNDER | (trueno enemigo) | 1 s | Es `counterable`; hace daño al jugador al terminar de forma natural. |
| EN_BITE | (mordisco enemigo) | 1 s | No es counterable. Solo lo usa la clase de TEST (el jabalí muerde por contacto en melee.c). |

Los dos mejores ejemplos para copiar y aprender son **FUEGO**
(`src/spells/player/fire.c`) y **LUZ** (`src/spells/player/light.c`): están muy comentados y cubren casi todo lo que
necesitarás.

### El límite de notas (progresión del jugador)

Aunque el mando tiene seis notas (MI FA SOL LA SI DO), el jugador **no siempre
puede tocarlas todas**. Una variable del juego (`player_note_limit`) marca la nota
más alta disponible; al intentar una por encima, suena un pitido de error, aparece
un aviso y se cancela el patrón que estuviera tocando. En el acto 1, al conseguir el
bastón el límite se fija en **SOL** (las tres primeras notas): por eso hechizos como
CURACIÓN, que usan notas más altas, quedan "vistos pero no tocables" hasta que una
escena posterior suba el límite. Esto se ajusta desde un gancho de C (por ejemplo al
recoger el bastón), no desde el guion.
