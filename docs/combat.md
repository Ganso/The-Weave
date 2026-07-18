# Cómo funciona el combate — guía completa

Este documento explica **todo el sistema de combate** de The Weave: sus dos
directores, cómo se reparten el trabajo, qué estados manejan, cómo se crea un
encuentro nuevo y qué trampas hay que conocer. Es la referencia canónica del
dominio `src/combat/` (AGENTS.md §5 remite aquí).

---

## 1. La doctrina: un modelo, dos directores

Decisión de diseño (Javier, julio 2026):

- **El jugador** puede o no cantar patrones según la escena (flag `spells` del
  DSL, que activa `player_patterns_enabled`). Su otra arma es el **golpe
  físico** con el botón A.
- **Cada enemigo** es de uno de estos dos tipos:
  - **De contacto** — persigue y muerde/embiste (jabalí). Sin patrones.
  - **A distancia** — canta patrones desde lejos (espectro/WeaverGhost).

Hoy cada tipo de encuentro tiene su **director** (el módulo que lleva la
batuta del combate):

| | Combate de CONTACTO | Combate de PATRONES |
|---|---|---|
| Director | `combat/melee.c` | `combat/combat.c` + `spells/spell.c` |
| Enemigos | jabalíes (persiguen y muerden) | espectros (cantan EN_THUNDER) |
| Arma del jugador | golpe con A **o** patrón de TRUENO (configurable) | counter: el patrón enemigo invertido |
| Se gana | ahuyentando a la manada (N impactos) | matando a los enemigos (counters) |
| Se invoca desde la escena | `call <hook>` (el hook spawnea y llama a `melee_combat_run`) | `call <hook de spawn>` + op `combat` |
| Estado global que gobierna | ninguno (normaliza `combat_state` a `COMBAT_NO`) | `combat_state` (el FSM de abajo) |

Los dos directores **no corren a la vez** y la frontera entre ambos está sellada
(ver §6, trampas). Unificarlos en un único director es trabajo futuro (§8).

**Común a ambos**: la vida del jugador (§4) y el reintento por derrota (op
`if_defeated` del DSL).

---

## 2. Combate de PATRONES (combat.c) — enemigos a distancia

### El FSM `combat_state`

```
COMBAT_NO ──combat_init()──► COMBAT_STATE_IDLE
                                   │ spell_enemy_try_launch()
                                   ▼
                     COMBAT_STATE_ENEMY_PLAYING   (el enemigo toca sus notas)
                                   │ termina de tocar
                                   ▼
                     COMBAT_STATE_ENEMY_EFFECT    (su hechizo está en el aire)
                          │                │
              counter del jugador       fin natural
              (patrón invertido)        (hit_player)
                          │                │
                     hit_enemy             ▼
                          ▼           COMBAT_STATE_IDLE ──► (siguiente ataque)
                     ¿quedan enemigos vivos?
                       no → COMBAT_NO (victoria)
```

- `combat_init()` — resetea la vida del jugador (§4), el motor de hechizos y
  las recargas de los enemigos activos. La invoca el **op `combat`** del DSL,
  que bloquea hasta victoria (`COMBAT_NO`) **o derrota** (`player_defeated`,
  en cuyo caso llama a `combat_abort()`: libera enemigos y cierra).
- Los estados `PLAYER_PLAYING/PLAYER_EFFECT` existen porque **el motor de
  hechizos usa `combat_state` también fuera del combate** (un cast libre del
  jugador pasa por ellos). Ver la trampa §6.1.
- El daño real lo aplican `hit_enemy()` / `hit_player()` (combat.c). Un
  enemigo a 0 HP reproduce su animación de muerte con un timer determinista y
  lo libera `update_enemy_animations` (nunca esperas bloqueantes).

### Qué hace cada archivo

- `combat/combat.c` — el FSM director + `hit_enemy`/`hit_player` +
  `combat_abort`. Vida del jugador (§4).
- `spells/spell.c` — el motor de hechizos con DOS SLOTS (jugador y enemigo)
  que pueden estar vivos a la vez (imprescindible para el counter). El
  director consulta y actualiza `combat_state`.
- `spells/notes.c` — la cola de notas del jugador. `player_note_limit` limita
  la nota más alta disponible (en el acto 1, tras coger el bastón: 3 notas;
  una superior → beep + aviso + patrón cancelado).

### Rechazos con y sin pista

Cuando un patrón válido no se puede usar (`canUse` == false), el motor llama a
`onRejected`. Para el TRUENO:

- **Invertido fuera de su ventana de counter**: si hay un espectro en pantalla,
  pista de "espera su momento"; si no, **falla y ya está** (suena el patrón
  inválido, sin diálogo).
- **Directo contra un espectro** (los espectros son inmunes al trueno directo):
  pista de "tócalo al revés".

---

## 3. Combate de CONTACTO (melee.c) — la manada

### Configuración

Un encuentro se define con `MeleeConfig` y una sola llamada bloqueante:

```c
melee_combat_run(&(MeleeConfig){
    .hits_to_win = 4,             // impactos que ahuyentan a la manada
    .companion = CHR_clio,        // espera quieta durante el combate (CHR_NONE si no hay)
    .reposition_companion = true, // ¿se recoloca detrás del jugador primero?
    .weapon_is_thunder = false,   // false: golpe con A | true: patrón de TRUENO
});
```

Antes de llamar, el hook spawnea los enemigos (`init_enemy` +
`move_enemy_instant` + `show_enemy`) y fija `player_max_hitpoints`. La escena
debe poner `spells off` para el arma de golpe y `spells on` para la de trueno.

### El arma del jugador

- **Golpe (A)**: animación de ataque (reutiliza `STATE_PLAYING_NOTE` →
  `ANIM_ACTION`, que además bloquea el movimiento mientras dura). En el frame
  de impacto se busca el enemigo más cercano **delante** del jugador
  (según su flip) y a su altura; si lo hay, cuenta un impacto y el enemigo
  huye. Cooldown entre golpes.
- **Trueno**: en cuanto el patrón suena entero (el cast ARRANCA, no cuando
  termina su efecto de 4 s), la descarga **espanta a toda la manada** y cuenta
  un impacto. Con `hits_to_win = 1`, un solo trueno gana el combate (guión
  6.1: «los enemigos huyen»).

Un impacto **no mata**: el enemigo hace el flash de daño (`STATE_HIT`), huye
corriendo al **borde de pantalla más cercano** y vuelve a la carga. Al llegar
a `hits_to_win`, todos huyen definitivamente y se liberan.

### FSM de cada enemigo (un tick por frame)

```
GONE ◄─────────────────────────────┐ (se libera al salir de pantalla)
  │ spawn                       LEAVE  ◄── (melee_hits == hits_to_win)
  ▼                                ▲
WAIT (entrada escalonada, 40f/enemigo)
  ▼                                │
CHASE ──a rango de mordisco──► BITE│  (daño a mitad del ciclo, vuelve a CHASE)
  ▲  │ (impactado por el arma)     │
  │  ▼                             │
  │ HURT (flash ~1s) ──► RETREAT ──┘ (corre al borde más cercano...
  └──────────────────────┘          ...y desde fuera vuelve a la carga)
```

Detalles de CHASE que dan vida y evitan atascos:

- **Desvío vertical aleatorio**: cada enemigo persigue `py + wander` (±16 px,
  re-sorteado cada ~1-2 s, o al instante si se bloquea).
- **Pausas aleatorias**: ~1/128 por frame se detiene ~1 s (cooldown de 2 s por
  enemigo).
- **Deslizamiento por ejes**: si el paso completo choca contra un personaje,
  prueba solo horizontal y luego solo vertical (ambos sentidos). Las huidas
  (`RETREAT`/`LEAVE`) ignoran la colisión con personajes para no encajonarse.

### Reparto de responsabilidades con el motor

- La locomoción es del melee: los enemigos van en `STATE_WALKING`, estado en
  el que `update_enemy_animations` **no toca** la animación.
- El flash de daño sí es del motor: `STATE_HIT` + `modeTimer` y
  `update_enemy_animations` lo cuenta y devuelve a `STATE_IDLE` (el melee
  detecta esa transición para pasar a `RETREAT`).
- El mordisco reutiliza `hit_player()` (vida, flash, stun). Sin diálogos: los
  golpes recibidos no hablan (§6.2).

### Limpieza garantizada al terminar (victoria o derrota)

1. Ningún hechizo del jugador a medias (`spell_cancel`) ni notas en cola
   (`reset_note_queue`) — un trueno en vuelo seguiría parpadeando el cielo.
2. `combat_state = COMBAT_NO` (sin residuos del motor de patrones).
3. El acompañante recupera su `follows_character`.
4. En derrota, los enemigos restantes se liberan; la escena decide el
   reintento con `if_defeated`.

---

## 4. Vida del jugador, derrota y reintento (común)

- `player_max_hitpoints` (5 por defecto) — se fija en el hook del encuentro.
- `player_hitpoints` — se reinicia al empezar CADA combate (`combat_init` o
  `melee_combat_run`). `hit_player()` la resta; a 0 marca `player_defeated`.
- Ambos directores salen inmediatamente con `player_defeated`; la escena
  muestra su mensaje y reintenta:

```
label pelea
call mi_hook_de_spawn      # spawnea y (si es de contacto) corre el melee
combat                     # solo en combates de patrones
if_defeated goto derrota
say ... (victoria) / sigue la escena
label derrota
say ... (pista de Bobbin)
goto pelea
```

---

## 5. Recetas

### Encuentro de contacto nuevo

1. Hook C: `PAL_setPalette(PAL3, boar_sprite.palette->data, DMA)`, spawnear
   con `init_enemy(i, ENEMY_CLS_BOAR)` + `move_enemy_instant` (y de pies) +
   `show_enemy`, fijar `player_max_hitpoints` y llamar a `melee_combat_run`
   con su `MeleeConfig`.
2. Escena: `set spells off` (arma golpe) u `on` (arma trueno) → `call hook` →
   `if_defeated goto ...`.

### Encuentro de patrones nuevo

1. Hook C de spawn (patrón de `act1_return_ghosts`): paleta + `init_enemy` +
   entrada andando con `move_enemy`.
2. Escena: `set spells on` + `enable_spell ...` → `call hook` → `combat` →
   `if_defeated goto ...`.

### Clase de enemigo nueva

En `actors/enemies.h/c`: `ENEMY_CLS_*`, ficha en `init_enemy_classes` (HP,
follow, y sus patrones si es a distancia — `{SPELL_NONE, SPELL_NONE}` si es de
contacto) y el sprite en el switch de `init_enemy`.

---

## 6. Trampas conocidas (leer antes de tocar combate)

1. **`combat_state` no es solo del combate**: el motor de hechizos lo usa para
   cualquier cast del jugador, y su `set_idle()` final lo deja en
   `COMBAT_STATE_IDLE` si ve enemigos activos — aunque el combate en curso sea
   de contacto. El melee lo normaliza a `COMBAT_NO` cada frame; si algún día
   otro sistema convive con enemigos activos fuera del op `combat`, necesitará
   la misma normalización.
2. **Diálogos en combate**: `hit_player` no habla. El único diálogo ligado a
   recibir daño es el tutorial del forest DEMO, doblemente acotado
   (`combat_state != COMBAT_NO` **y** ESCONDERSE desbloqueado). Las pistas de
   reintento las da siempre la ESCENA tras `if_defeated`.
3. **Un impacto de melee no usa `hit_enemy`**: mataría al enemigo (2 HP). Los
   jabalíes escarmientan, no mueren.
4. **`SPR_update()` una sola vez por frame** (la de `next_frame`). Nada de
   llamarlo en helpers por-entidad (hundió el framerate a la mitad).
5. **El acompañante** pierde `follows_character` durante el combate y lo
   recupera al salir; si se añade otro acompañante, pasa por `MeleeConfig`.
6. **Equilibrio del arma trueno**: cantar el patrón cuesta 4 notas + su efecto
   dura 4 s con el slot ocupado. Exigir varios truenos con 3 enemigos mordiendo
   hace el combate injugable (pasó con `hits_to_win = 2`): la manada muerde más
   rápido de lo que se canta. Con arma trueno, `hits_to_win = 1`.

---

## 7. Mapa de archivos

| Archivo | Qué contiene |
|---|---|
| `src/combat/combat.c/.h` | FSM de patrones, vida del jugador, `hit_*`, `combat_abort` |
| `src/combat/melee.c/.h` | director de contacto (`MeleeConfig` + FSM de la manada) |
| `src/spells/spell.c` | motor de hechizos de dos slots (counter incluido) |
| `src/spells/notes.c` | cola de notas del jugador + `player_note_limit` |
| `src/actors/enemies.c` | clases de enemigo, spawn, flash de daño y liberación |
| `src/scenes/scene_vm.c` | ops `combat` e `if_defeated` |

Verificación desatendida: el melee expone `melee_hits` y usa variables
observables por RAM (RetroArch NCI, `docs/retroarch-mcp.md`); la suite AUTO de
la smoke ROM cubre el combate de patrones completo.

---

## 8. Lo que queda por hacer (combate)

- **Unificar los dos directores** en uno solo, con el "tipo de ataque" (contacto vs
  patrones) como propiedad del enemigo en vez de dos módulos separados
  (`melee.c` y `combat.c`+`spell.c`) que no corren a la vez. Es un refactor aceptado
  pero no hecho; la doctrina y la frontera actual (§1, §6) son el punto de partida.
- **Muerte de enemigo más rica**: la actual es flash de daño ~1 s + release; una
  animación propia y recompensas es diseño de juego que el motor ya soporta vía
  `onFinish`/`hit_enemy`.
