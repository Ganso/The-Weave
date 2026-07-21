# Cómo crear un enemigo nuevo — guía completa

Esta guía te lleva **desde el dibujo hasta verlo pelear**, aunque no sepas
programar. Un "enemigo" es alguien que ataca al jugador en un combate. Aquí se
explica cómo crear la **criatura**; cómo montar la **pelea** en la que aparece
está en `docs/combat.md`.

---

## 1. Lo primero: ¿de qué tipo es tu enemigo?

En The Weave todo enemigo es de **uno de estos dos tipos** (su *rol*). Elige
antes de dibujar nada, porque cambia lo que necesitas:

| | **De contacto** | **A distancia** |
|---|---|---|
| Cómo pelea | Persigue al jugador y le ataca de cerca | Canta melodías (hechizos) desde lejos |
| Ejemplo | El jabalí (muerde) | El espectro (lanza truenos) |
| Cómo se le vence | Ahuyentándolo a golpes o con un hechizo | Contrarrestando sus melodías |
| Qué tienes que definir | Un **perfil de ataque** (alcance, ritmo, daño) | Qué **hechizos** canta |

Un mismo combate puede mezclar los dos tipos sin problema.

---

## 2. El arte

### 2.1 La hoja de sprites

**Una sola imagen PNG** con los fotogramas en rejilla: cada **fila una
animación**, cada **columna un fotograma**. El orden de las filas:

| Fila | Animación | Cuándo se ve |
|---|---|---|
| 0 | `IDLE` | quieto |
| 1 | `WALK` | acercándose |
| 2 | `ACTION` | **atacando** (morder, embestir, cantar) |
| 3 | `MAGIC` | lanzando magia (o muerte, según la criatura) |
| 4 | `HURT` | recibiendo un golpe |
| 5 | *extra* | opcional (el jabalí lo usa para **galopar** al huir) |

Las mismas reglas de oro que con los personajes:

- Todos los fotogramas **del mismo tamaño**, rejilla regular.
- Ancho y alto **múltiplos de 8**.
- **Mirando a la DERECHA** (el juego voltea el dibujo solo). Si lo dibujas al
  revés, correrá de espaldas: amplía los colmillos o la cara para asegurarte.
- El color transparente debe ser **el primero de la paleta**.
- Fila incompleta → **repite el último fotograma**.

Ejemplo real: `res/gfx/enemies/boar.png`, fotogramas de 48×32, 6 columnas.

### 2.2 La paleta

Los enemigos **no comparten paleta** con los personajes: cada uno usa **su
propia paleta** (hasta 16 colores), que la escena carga en la ranura `PAL_ENEMIES`
justo antes de que aparezca. Así que dibuja con libertad; solo recuerda que son
16 colores contando el transparente.

### 2.3 La sombra (opcional)

Igual que la de los personajes: una tira bajita con una elipse oscura, del ancho
de un fotograma. El jabalí tiene (`boar_shadow.png`); el espectro no (flota).

### Dónde va

```
res/gfx/enemies/<nombre>.png
res/gfx/enemies/<nombre>_shadow.png   (si lleva sombra)
```

---

## 3. Declarar el arte: `res/res_enemies.res`

```
# Lobo de las nieves
SPRITE wolf_sprite "gfx/enemies/wolf.png" 6 4 BEST 8
SPRITE wolf_shadow_sprite "gfx/enemies/wolf_shadow.png" 6 1 BEST
```

| Trozo | Qué es |
|---|---|
| `wolf_sprite` | el nombre con el que lo conocerá el código |
| `"gfx/..."` | la ruta de tu PNG |
| `6 4` | tamaño de UN fotograma en bloques de 8 px → 48×32 píxeles |
| `BEST` | compresión (déjalo así) |
| `8` | velocidad de la animación (fotogramas por segundo) |

---

## 4. Darlo de alta en el código

Tres apuntes. Copia el patrón del enemigo más parecido.

### 4.1 Reservarle un número — `src/actors/enemies.h`

```c
#define MAX_ENEMY_CLASSES 10          // súbelo si te quedas sin sitio
#define ENEMY_CLS_WEAVERGHOST    0
#define ENEMY_CLS_TESTGHOST      1
#define ENEMY_CLS_BOAR           2
#define ENEMY_CLS_WOLF           3    // ← el tuyo
```

### 4.2 Escribir su ficha — `src/actors/enemies.c`

Aquí es donde dices **qué tipo de enemigo es y cómo pelea**, en
`init_enemy_classes`.

**Si es DE CONTACTO**, primero describe su ataque (esto es solo *datos*: el
motor de la persecución ya está hecho y es el mismo para todos):

```c
// El zarpazo del lobo: más alcance y más daño que el mordisco del jabalí
static const ContactProfile wolf_claw = {
    .range_x = 40, .range_y = 10,   // a qué distancia alcanza (píxeles)
    .attack_time = 40,              // cuánto dura el ciclo del ataque (fotogramas)
    .hit_at = 20,                   // en qué fotograma del ciclo hace daño
    .damage = 2,                    // cuánta vida quita
};
```

Y luego su ficha:

```c
obj_enemy_class[ENEMY_CLS_WOLF] =
    (Enemy_Class){ 2, ENEMY_ROLE_CONTACT, &wolf_claw, {SPELL_NONE, SPELL_NONE} };
//                 ↑ vida               ↑ su perfil    ↑ sin hechizos
```

**Si es A DISTANCIA**, no lleva perfil de ataque pero sí hechizos:

```c
obj_enemy_class[ENEMY_CLS_WRAITH] =
    (Enemy_Class){ 2, ENEMY_ROLE_RANGED, NULL, {SPELL_EN_THUNDER, SPELL_NONE} };
//                 ↑ vida              ↑ sin perfil  ↑ hasta dos hechizos
```

> Los hechizos de enemigo se crean aparte: guía en `docs/spells.md`.

### 4.3 Decirle qué sprite le toca — `src/actors/enemies.c`

Dentro de `init_enemy`, en la lista de casos:

```c
case ENEMY_CLS_WOLF:
    nsprite = &wolf_sprite;
    nsprite_shadow = &wolf_shadow_sprite;
    drops_shadow = true;      // false si flota (como el espectro)
    break;
```

---

## 5. Sacarlo en una pelea

Aquí sí hace falta **una función de C pequeñita** (un *gancho*), porque hacer
aparecer enemigos no se puede escribir como una lista de órdenes. Va en el
archivo de la escena, por ejemplo `src/scenes/act2/nieve.c`:

```c
void act2_nieve_wolves(void)
{
    // 1. Cargar SU paleta en la ranura de enemigos
    PAL_setPalette(PAL_ENEMIES, wolf_sprite.palette->data, DMA);

    // 2. Colocar dos lobos fuera de pantalla, a distintas alturas
    static const s16 spawn[2][2] = { { -60, 150 }, { SCREEN_WIDTH + 20, 165 } };
    for (u16 i = 0; i < 2; i++) {
        init_enemy(i, ENEMY_CLS_WOLF);
        move_enemy_instant(i, FASTFIX32_FROM_INT(spawn[i][0]),
                              FASTFIX32_FROM_INT(spawn[i][1]));
        show_enemy(i, true);
    }

    // 3. Cómo es esta pelea (detalle en docs/combat.md)
    player_max_hitpoints = 5;
    combat_configure(&(CombatConfig){
        .weapon_strike = true,     // el jugador pelea a golpes
        .hits_to_win = 5,          // 5 impactos y huyen
        .companion = CHR_NONE,     // ponlo SIEMPRE (aquí, nadie acompaña)
    });
}
```

Ese gancho hay que **registrarlo** para poder llamarlo desde el guion (dos
líneas en `src/scenes/scene_hooks.h` y `.c`; el proceso está en
`docs/scenes.md`).

Y en el guion de la escena, la receta de siempre:

```
set spells off               # ¿puede cantar el jugador en esta pelea?
label pelea
call act2_nieve_wolves       # aparecen los lobos + configuración
combat                       # ¡a pelear!
if_defeated goto pelea_fallida
say ACT2_NIEVE A2_VICTORIA sound
goto seguir

label pelea_fallida
say ACT2_NIEVE A2_DERROTA sound
goto pelea
```

---

## 6. Ajustar hasta que la pelea sea divertida

Todo lo que hace a un enemigo "justo o injusto" son **los números de su ficha**,
y se afinan jugando:

- **Vida** — cuántos impactos aguanta (los de contacto no mueren: escarmientan
  y huyen; ahí manda `hits_to_win` de la pelea, no la vida).
- **`damage`** — cuánta vida quita cada ataque. El jugador tiene 5 puntos.
- **`range_x` / `range_y`** — cuánto alcanza. Mucho alcance = agobiante.
- **`attack_time` / `hit_at`** — el ritmo: un ciclo largo con el golpe al final
  **da tiempo a esquivar**; uno corto es despiadado.

Prueba, cambia un número, vuelve a probar. La velocidad a la que persiguen y las
pausas que hacen son comunes a todos y ya están afinadas.

### Si algo va mal

| Síntoma | Casi siempre es |
|---|---|
| Corre **de espaldas** | La hoja mira a la izquierda: voltéala. |
| Colores raros | Falta el `PAL_setPalette(PAL_ENEMIES, ...)` en el gancho. |
| **No ataca nunca** | Es de contacto pero le falta el `ContactProfile` (o el rol está mal). |
| No hace nada de nada | Es "a distancia" pero no le pusiste hechizos. |
| El juego se **cuelga** | Pediste una animación que su hoja no tiene. |
| Ataca **demasiado pronto** | `hit_at` muy bajo: el golpe cae antes de que se vea venir. |

---

## 7. Resumen: la lista completa

1. Decidir el **rol**: de contacto o a distancia.
2. Dibujar la hoja (**mirando a la derecha**, rejilla regular, múltiplos de 8) y
   la sombra si lleva.
3. Guardarlas en `res/gfx/enemies/` y declararlas en `res/res_enemies.res`.
4. Reservar su `ENEMY_CLS_*` (`enemies.h`).
5. Escribir su ficha en `init_enemy_classes` (con `ContactProfile` si es de
   contacto, o con sus hechizos si es a distancia) y su caso en `init_enemy`.
6. Escribir el gancho que lo hace aparecer y configura la pelea, y registrarlo.
7. Llamarlo desde el guion: `call <gancho>` → `combat` → `if_defeated`.
