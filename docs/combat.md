# Cómo funciona el combate — guía completa

Este documento explica **todo el sistema de combate** de The Weave: el director
único, cómo se declara un encuentro, qué estados maneja y qué trampas hay que
conocer. Es la referencia canónica del dominio `src/combat/` (AGENTS.md §5
remite aquí).

---

## 1. La doctrina: un director, roles de enemigo, encuentros declarativos

Decisión de diseño (Javier, julio 2026):

- **Un combate significa "hay enemigos presentes"** — no un tipo de ataque
  concreto. Los puzzles de hechizos y los cast libres fuera de combate son otra
  cosa y no pasan por aquí.
- **El jugador** puede o no cantar patrones según la escena (flag `spells` del
  DSL) y puede o no tener disponible el **golpe físico** con A (configuración
  del encuentro).
- **Cada enemigo** es, por su CLASE (`actors/enemies.h`), de uno de dos ROLES:
  - `ENEMY_ROLE_CONTACT` — persigue y ataca de cerca (jabalí). Su ataque
    concreto (alcance, ritmo, daño) es el **ContactProfile** de su clase: el
    mordisco es del jabalí, otra clase atacará distinto con la misma FSM.
  - `ENEMY_ROLE_RANGED` — canta patrones desde lejos (espectro/WeaverGhost),
    con los `spell[]` de su clase.
- **La personalidad del encuentro se declara** (patrón SpellDef): datos + hooks
  opcionales en `CombatConfig`. La condición de victoria es del encuentro (uno
  futuro puede ganarse con un puzzle o un diálogo interno); reglas como "el
  trueno ahuyenta a la manada" son prefabs que el encuentro enchufa, no
  doctrina del motor.

El **director** (`combat/combat.c`) reparte cada enemigo activo a su
subsistema según el rol, y ambos roles pueden convivir en el mismo encuentro.

### La receta en escena (siempre la misma)

```
label pelea
call mi_hook_spawn        # el hook spawnea a los enemigos y deja la config
combat                    # el director ejecuta el encuentro completo
if_defeated goto derrota
say ... (victoria) / sigue la escena
label derrota
say ... (pista)
goto pelea
```

---

## 2. Declarar un encuentro: `CombatConfig`

El hook de escena spawnea (`init_enemy` + `move_enemy_instant` + `show_enemy`),
fija `player_max_hitpoints` si procede, y deja la configuración:

```c
combat_configure(&(CombatConfig){
    .weapon_strike = true,       // ¿el golpe con A está disponible?
    .hits_to_win = 4,            // impactos que ahuyentan a la manada (0 = n/a)
    .companion = CHR_clio,       // espera quieto durante el combate (CHR_NONE si no hay)
    .reposition_companion = true,// ¿se recoloca detrás del jugador al empezar?
    // Hooks opcionales del encuentro (como en SpellDef):
    // .onStart / .onTick / .isWon / .onEnd
});
```

- **OJO**: pon SIEMPRE `.companion` (`CHR_NONE` si no hay) — el 0 implícito de
  C es `CHR_linus`.
- **Cantar no es un campo**: lo decide la escena con `set spells on/off`.
- La config **se consume** al arrancar el encuentro y `end_level` descarta
  cualquier pendiente. Sin config previa, el op `combat` usa el encuentro por
  defecto: solo patrones, se gana matando a todos.

### Los hooks del encuentro

| Hook | Cuándo | Para qué |
|---|---|---|
| `onStart` | al arrancar (tras vida/acompañante/scroll) | preparación propia |
| `onTick` | cada frame | reglas del encuentro (p.ej. `combat_rule_thunder_scares`) |
| `isWon` | cada frame | victoria propia; NULL = "no quedan enemigos activos" |
| `onEnd` | al cerrar (victoria o derrota), antes de la limpieza común | epílogos propios |

### Reglas prefab (`weapons.h`)

- `combat_rule_thunder_scares` — al LANZARSE el TRUENO (el patrón suena entero,
  no al acabar su efecto), espanta a todos los de contacto y suma un impacto de
  manada. La usa la emboscada del regreso: `.hits_to_win = 1, .onTick =
  combat_rule_thunder_scares` → un trueno y huyen (guión 6.1).

Un encuentro futuro escribe las suyas en su hook de escena o reutiliza estas.

### Ejemplos reales del acto 1

| Encuentro | Config | Escena |
|---|---|---|
| Jabalíes del bosque (`act1_fday_boars`) | golpe A, 4 impactos, Clio recolocada | `spells off` |
| Jabalíes del regreso (`act1_return_boars`) | sin golpe, 1 impacto, regla del trueno, Clio donde esté | `spells on` |
| Espectros (`act1_return_ghosts`) | sin config (defecto: patrones, matar a todos) | `spells on` |

---

## 3. El director (`combat.c`): ciclo de vida

```c
combat_run();      // bloqueante: start + { next_frame(true); tick } + end
// — o por pasos (lo usa la instrumentación de la smoke ROM):
combat_start();    // consume la config; vida, acompañante, scroll, recargas
while (combat_running()) { next_frame(true); combat_tick(); }
combat_end();      // limpieza garantizada
```

- `combat_start` — consume la config pendiente; resetea `player_hitpoints`,
  el motor de hechizos y las recargas; `contact_reset` + `weapons_reset`;
  aparta al acompañante; guarda y desactiva `player_scroll_active`; muestra el
  HUD si hay patrones; deja `combat_state` en IDLE solo si hay enemigos a
  distancia.
- `combat_tick` — armas del jugador → `onTick` del encuentro → FSM de contacto
  → ¿fin? (derrota por `player_defeated`; victoria por `isWon` o, por defecto,
  sin enemigos activos).
- `combat_end` — **un único punto de salida** para victoria y derrota:
  `onEnd`, libera a los enemigos que queden, cancela el hechizo del jugador en
  vuelo y vacía la cola de notas, `combat_state = COMBAT_NO`, **restaura** el
  scroll (al valor previo, no a true) y el `follows_character` del acompañante.

La vida del jugador es común: `player_max_hitpoints` (5 por defecto) →
`player_hitpoints` se reinicia en cada `combat_start`; `hit_player()` la resta
y a 0 marca `player_defeated` (el tick cierra el encuentro y la escena decide
el reintento con `if_defeated`).

---

## 4. El lado de CONTACTO (`contact.c`)

FSM genérica, un tick por frame y por enemigo; el ataque se lee del
`ContactProfile` de la clase:

```
GONE ◄─────────────────────────────┐ (se libera al salir de pantalla)
  │ spawn                       LEAVE  ◄── (contact_hits == hits_to_win)
  ▼                                ▲
WAIT (entrada escalonada, 40f/enemigo)
  ▼                                │
CHASE ──a rango del perfil──► ATTACK│ (golpe en el frame hit_at, vuelve a CHASE)
  ▲  │ (impactado por un arma)     │
  │  ▼                             │
  │ HURT (flash ~1s) ──► RETREAT ──┘ (corre al borde más cercano...
  └──────────────────────┘          ...y desde fuera vuelve a la carga)
```

- Un impacto **no mata** (no usa `hit_enemy`): el enemigo escarmienta
  (`STATE_HIT`) y huye. A `hits_to_win` impactos acumulados (`contact_hits`,
  observable por RAM), todos huyen definitivamente y se liberan.
- Detalles de CHASE que dan vida y evitan atascos: desvío vertical aleatorio
  (±16 px re-sorteado, respetando los `limits` de la escena si están fijados),
  pausas aleatorias (~1/128, con cooldown) y deslizamiento por ejes si el paso
  choca; las huidas ignoran la colisión con personajes.
- Reparto con el motor: locomoción en `STATE_WALKING` (que
  `update_enemy_animations` no toca); el flash de daño sí es del motor
  (`STATE_HIT` + `modeTimer`).

## 5. El lado A DISTANCIA (`combat_state` + `spells/spell.c`)

El FSM de turnos del motor de patrones — **no es el estado del encuentro**:

```
COMBAT_NO ──combat_start (hay ranged)──► COMBAT_STATE_IDLE
                                   │ spell_enemy_try_launch()
                                   ▼
                     COMBAT_STATE_ENEMY_PLAYING   (el enemigo toca sus notas)
                                   ▼
                     COMBAT_STATE_ENEMY_EFFECT    (su hechizo está en el aire)
                          │                │
              counter del jugador       fin natural
              (patrón invertido)        (hit_player)
                          │                │
                     hit_enemy             ▼
                          ▼           COMBAT_STATE_IDLE ──► (siguiente ataque)
```

- Un enemigo a 0 HP reproduce su animación de muerte con timer determinista y
  lo libera `update_enemy_animations` (sin esperas bloqueantes). Sin enemigos
  activos, la condición de victoria por defecto cierra el encuentro.
- Los estados `PLAYER_*` también sirven a los **cast libres fuera de combate**
  (por eso `combat_state` vive aunque no haya encuentro).
- `set_idle()` pregunta al director: deja `IDLE` solo si el encuentro tiene
  enemigos a distancia vivos (`combat_ranged_present()`); si no, `COMBAT_NO`.
  Así un combate solo-contacto nunca hereda estados de patrones.
- Rechazos del TRUENO: invertido fuera de su ventana → con espectros, pista de
  "espera su momento"; sin espectros, **falla y ya está** (beep). Directo
  contra un espectro → pista de "tócalo al revés".

---

## 6. Recetas

### Encuentro nuevo (cualquier mezcla de roles)

1. Hook C: paleta (`PAL_setPalette(PAL_ENEMIES, ...)`), spawn (`init_enemy` +
   `move_enemy_instant` + `show_enemy`), `player_max_hitpoints` si procede, y
   `combat_configure(&(CombatConfig){...})`.
2. Escena: `set spells on/off` → `call hook` → `combat` → `if_defeated goto`.

### Clase de enemigo nueva

En `actors/enemies.h/c`: `ENEMY_CLS_*`, ficha en `init_enemy_classes` con su
`role` (+ `ContactProfile` propio si es de contacto, o sus `spell[]` si es a
distancia) y el sprite en el switch de `init_enemy`.

### Encuentro con condición de fin propia

Escribe el `isWon` (y/o `onTick`) en el mismo fichero del hook de escena y
enchúfalo en la config. El director sigue poniendo vida, derrota, acompañante
y limpieza; tú solo la regla.

---

## 7. Trampas conocidas (leer antes de tocar combate)

1. **`.companion` siempre explícito**: el 0 implícito de C es CHR_linus. El
   director ignora un companion igual al personaje activo, pero no te fíes.
2. **`combat_state` no es el estado del encuentro**: es el FSM del motor de
   patrones y también sirve a los cast libres. El estado del encuentro es
   `combat_running()`. No lo uses para saber "si hay pelea".
3. **Un impacto de contacto no usa `hit_enemy`**: mataría al enemigo (2 HP).
   Los de contacto escarmientan, no mueren.
4. **`SPR_update()` una sola vez por frame** (la de `next_frame`). Nada de
   llamarlo en helpers por-entidad (hundió el framerate a la mitad).
5. **Diálogos en combate**: `hit_player` no habla. El único diálogo ligado a
   recibir daño es el tutorial del forest DEMO, doblemente acotado
   (`combat_state != COMBAT_NO` **y** ESCONDERSE desbloqueado). Las pistas de
   reintento las da siempre la ESCENA tras `if_defeated`.
6. **Equilibrio del arma trueno**: cantar cuesta 4 notas y el efecto dura 4 s
   con el slot ocupado. Exigir varios truenos con una manada mordiendo hace el
   combate injugable (pasó con hits_to_win=2): con la regla del trueno,
   `hits_to_win = 1`.
7. **Config zombi**: la config se consume en `combat_start` y `end_level`
   descarta la pendiente. Si un hook configura y la escena olvida el op
   `combat`, la config esperaría al siguiente combate del MISMO nivel — no
   dejes hooks de spawn sin su op.

---

## 8. Mapa de archivos

| Archivo | Qué contiene |
|---|---|
| `src/combat/combat.h` | API pública: `CombatConfig`, ciclo de vida, vida del jugador, `CombatState`, `hit_*` |
| `src/combat/combat.c` | El director: config, start/tick/end/run, hooks, victoria/derrota, acompañante, scroll, `update_combat`, `set_idle` |
| `src/combat/contact.c/.h` | FSM genérica de los enemigos de contacto (perfil por clase) |
| `src/combat/weapons.c/.h` | El golpe con A + reglas prefab (`combat_rule_thunder_scares`) |
| `src/spells/spell.c` | Motor de hechizos de dos slots (counter incluido) |
| `src/spells/notes.c` | Cola de notas del jugador + `player_note_limit` |
| `src/actors/enemies.c/.h` | Clases (rol + perfil + spells), spawn, flash de daño y muerte diferida |
| `src/scenes/scene_vm.c` | Ops `combat` (→ `combat_run`) e `if_defeated` |

Verificación desatendida: `contact_hits`, `player_hitpoints`, `player_defeated`
y `combat_state` son observables por RAM (RetroArch NCI,
`docs/retroarch-mcp.md`); la suite AUTO de la smoke ROM cubre el combate de
patrones completo, y el de contacto se verificó conduciéndolo por RAM
(victoria por golpes, por trueno y ruta de derrota encadenadas).

---

## 9. Los corazones de vida del enemigo: QUITADOS

Los enemigos ya no muestran corazones sobre la cabeza (decisión de diseño). La
lógica sigue entera y funcional en `interface.c` (`update_life_counter`); lo
único que se hizo fue **comentar sus dos llamadas**:

- `actors/enemies.c`, al final de `update_enemy_animations` — la de cada frame.
- `combat/combat.c`, en `hit_enemy` — ocultarlo al morir el enemigo.

> **Las dos van juntas o ninguna.** La de `hit_enemy` no comprueba NULL y, con
> los corazones desactivados, el sprite nunca se crea: descomentar solo una
> provocaría un fallo al morir un enemigo. Está avisado en el propio código.

---

## 10. Lo que queda por hacer (combate)

- **Muerte de enemigo más rica**: la actual es flash de daño ~1 s + release;
  una animación propia y recompensas es diseño de juego que el motor ya
  soporta vía `onFinish`/`hit_enemy`.
