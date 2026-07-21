# Cómo añadir sonido y música — guía completa

Esta guía cubre los tres tipos de sonido del juego: **efectos** (un trueno, unas
olas), **música** de fondo y los **jingles** de los hechizos. De la grabación al
archivo hasta oírlo en la escena, sin saber programar.

---

## 1. Los dos formatos, y cuándo usar cada uno

| | **Efectos de sonido** | **Música** |
|---|---|---|
| Formato | `.wav` (un sonido grabado) | `.vgm` (una partitura para el chip) |
| Para | golpes, viento, olas, gaviotas, jingles | temas de fondo de las escenas |
| Cómo se hace | grabando o sintetizando | componiendo con un tracker de Mega Drive (Deflemask, Furnace) |
| Ocupa | **bastante** (son datos crudos) | muy poco |

Regla práctica: **todo lo corto es un `.wav`; lo que suena en bucle de fondo es
un `.vgm`.**

---

## 2. Efectos de sonido (`.wav`)

### 2.1 Preparar el archivo

Graba o exporta tu efecto en `.wav` **normal** (cualquier frecuencia, mono o
estéreo). No te preocupes por el formato exacto de la consola: **hay un script
que lo convierte solo**.

Consejos:

- **Cortos**: medio segundo, un segundo. Los efectos largos se comen la memoria
  del cartucho.
- **Sin silencio al principio** (se notaría como retardo) ni al final.
- Mono y sin mucha grave: el altavoz de la Mega Drive no lo reproduce bien.

Guárdalo por categorías:

```
res/sfx/ambient/ambient_rain.wav      ambientes
res/sfx/effects/effect_door.wav       efectos puntuales
res/sfx/player/player_jump.wav        cosas del jugador
res/sfx/patterns/pattern_heal.wav     jingles de hechizos
```

### 2.2 La conversión automática

La consola necesita los sonidos a una frecuencia concreta (**13300 Hz, 8 bits,
mono**, para el driver que usamos). De eso se encarga
`tools/resample_wavs.py`, que **se ejecuta solo al compilar**: por cada
`mi_sonido.wav` genera un `mi_sonido_resampled.wav`, y **ese** es el que usa el
juego.

Tú solo tienes que:

1. Dejar tu `.wav` original en su carpeta.
2. Compilar. El script hace el resto.

> Si cambias un `.wav` y el sonido no cambia en el juego, es que el script vio
> que el convertido ya existía. Fuerza la conversión con
> `python3 tools/resample_wavs.py --force`.

### 2.3 Declararlo: `res/res_sound.res`

Apunta siempre al archivo **`_resampled`**:

```
WAV snd_ambient_rain "sfx/ambient/ambient_rain_resampled.wav" XGM2
```

| Trozo | Qué es |
|---|---|
| `WAV` | tipo de recurso |
| `snd_ambient_rain` | el nombre con el que lo conocerá el código (empieza por `snd_`) |
| `"sfx/..."` | la ruta del archivo **convertido** |
| `XGM2` | el driver de sonido (déjalo así) |

### 2.4 Hacerlo sonar

Los efectos se disparan **desde un gancho de C**, con una línea:

```c
play_sample(snd_ambient_rain, sizeof(snd_ambient_rain));
```

Lo normal es tener un gancho de "ambiente" al principio de la escena, como los
que ya existen:

```c
void act2_taller_ambient(void)    // Ambiente del taller: el fuelle y el yunque
{
    play_sample(snd_ambient_bellows, sizeof(snd_ambient_bellows));
}
```

Y llamarlo desde el guion:

```
call act2_taller_ambient
```

---

## 3. Música (`.vgm`)

### 3.1 Componer

La música se compone con un **tracker de Mega Drive** (Deflemask, Furnace) y se
exporta a `.vgm`. Es una partitura para el chip de sonido, no un audio grabado:
por eso ocupa poquísimo.

Guárdala en una carpeta de música dentro de `res/` (hoy **todavía no existe**:
créala con la primera pieza):

```
res/music/act2_taller.vgm
```

### 3.2 Declararla: `res/res_sound.res`

```
XGM2 mus_taller "music/act2_taller.vgm"
```

### 3.3 Hacerla sonar

Desde un gancho de C:

```c
play_music(mus_taller);       // empezar (suena en bucle)
fade_music(SCREEN_FPS * 2);   // apagarla suavemente en 2 segundos
```

> **Estado actual**: el juego **todavía no tiene música compuesta** (está en la
> lista de pendientes de `docs/acto1.md`). El sistema está listo y probado; solo
> faltan las piezas.

---

## 4. Jingles de hechizos

Cada hechizo puede tener su **jingle**: el sonidito que suena al lanzarlo. Se
declara como cualquier otro efecto y se conecta en `src/audio/sound.c`, dentro de
`play_spell_jingle`, añadiendo el caso del hechizo:

```c
case SPELL_HEAL:
    play_sample(snd_pattern_heal, sizeof(snd_pattern_heal));
    break;
```

Si un hechizo no tiene jingle, sencillamente no suena (hoy les pasa a DORMIR,
CURACIÓN y el mordisco enemigo).

---

## 5. Comprobar que ha salido bien

1. Compila: `./build-theweave.sh build`. Si el `.wav` no existe o el nombre está
   mal, el build **se para y te lo dice**.
2. Arranca en tu escena con `HACK_START_SCENE` (`src/core/hack.h`).
3. Si necesitas silencio para trabajar, hay interruptores en el mismo archivo:
   `HACK_MUTE_MUSIC` y `HACK_MUTE_SFX`.

### Si algo va mal

| Síntoma | Casi siempre es |
|---|---|
| **No suena nada** | El `.res` apunta al `.wav` original en vez de al `_resampled`. |
| Suena **agudo o grave** | Lo mismo: se coló el archivo sin convertir. |
| Cambié el `.wav` y **suena el viejo** | Lanza `python3 tools/resample_wavs.py --force`. |
| Suena **con chasquidos** | El original tenía silencio o un corte brusco al principio/final. |
| **Se come la memoria** | Efectos demasiado largos: recórtalos. |
| El build **no encuentra** el sonido | La ruta del `.res` es relativa a `res/`, sin la carpeta `res/` delante. |

---

## 6. Resumen: la lista completa

**Un efecto nuevo:**
1. Dejar el `.wav` (corto, mono) en `res/sfx/<categoría>/`.
2. Compilar una vez (se genera el `_resampled`).
3. Declararlo en `res/res_sound.res` apuntando al `_resampled`.
4. Dispararlo con `play_sample(...)` desde un gancho y llamarlo con `call`.

**Música nueva:**
1. Componer y exportar a `.vgm` en `res/music/`.
2. Declararla con `XGM2` en `res/res_sound.res`.
3. `play_music(...)` al empezar y `fade_music(...)` al terminar.
