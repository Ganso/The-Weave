# Informe definitivo – eliminación de StateMachine de enemigos

## 0. Preparación
```bash
git checkout <commit-base>
git checkout -b refactor/no-sm
```

## 1. Estructura `Enemy`
**Archivo:** `src/enemies.h`

```diff
+ typedef enum {
+     ENEMY_MODE_IDLE,
+     ENEMY_MODE_PLAY_NOTE,
+     ENEMY_MODE_CASTING,
+     ENEMY_MODE_RECOVER
+ } EnemyMode;

  typedef struct {
      ...
+     EnemyMode mode;
+     u16       mode_timer;
+
+     u8  pattern_id;     /*255 = ninguno*/
+     u8  pattern_phase;
+     u16 pattern_timer;
  } Enemy;

+ void update_enemy_logic(u16 enemy_id);
```
**🛠 Compila / prueba** — 0 errores.

## 2. IA sin SM
### 2.1 `src/enemy_logic.h`
```c
#ifndef ENEMY_LOGIC_H
#define ENEMY_LOGIC_H
void update_enemy_logic(u16 enemy_id);
#endif
```

### 2.2 `src/enemy_logic.c`
*(copiar 92 líneas de decide_next/update_enemy_logic)*

### 2.3 Makefile
```diff
 OBJS += enemy_logic.o
```
**🛠 Compila / prueba**

## 3. Retirada de `enemy_state_machines`
### 3.1
```bash
rm src/enemies_patterns.c src/enemies_patterns.h
```

### 3.2 Makefile
```diff
- OBJS += enemies_patterns.o
```

### 3.3 `src/combat.c`
```diff
- #include "enemies_patterns.h"
+ #include "enemy_logic.h"

- extern StateMachine enemy_state_machines[MAX_ENEMIES];

- StateMachine_Init(&enemy_state_machines[i], ...);
- enemy_state_machines[i].owner_type = OWNER_ENEMY;

- StateMachine_Update(&enemy_state_machines[i], NULL);
+ update_enemy_logic(i);

- StateMachine_SendMessage(&enemy_state_machines[...]
- enemy_state_machines[...].current_state = ...
```

### 3.4 `src/counter_spell.c`
```diff
- execute_counter_spell(StateMachine* player_sm, StateMachine* enemy_sm, u16 pattern_id)
+ execute_counter_spell(StateMachine* player_sm, u16 enemy_id, u16 pattern_id)
```
Reemplazar `enemy_sm->entity_id` → `enemy_id + ENEMY_ENTITY_ID_BASE`, borrar resto.

**🛠 C-2** — aparecen símbolos de combate enemigo.

## 4. Estado global de combate enemigo
### 4.1 `src/enemy_combat_state.h`
(definiciones y `extern`)

### 4.2 `src/enemy_combat_state.c`
(inicializaciones)

### 4.3 Makefile
```diff
+ OBJS += enemy_combat_state.o
```

### 4.4 Includes
```c
#include "enemy_combat_state.h"
```

**🛠 Compila** — faltan metadatos HUD.

## 5. Metadatos y notas HUD
### 5.1 `src/enemy_pattern_data.h`
(EnemyPatternInfo, arrays, prototipos)

### 5.2 `src/enemy_pattern_data.c`
(obj_Pattern_Enemy[2], enemy_note_active[4], stubs HUD)

### 5.3 Makefile
```diff
+ OBJS += enemy_pattern_data.o
```

### 5.4 Includes
```c
#include "enemy_pattern_data.h"
```

### 5.5 Warning
```diff
- u16 current_pattern = enemy_attack_pattern;
```

**🛠 C-3** — compila.

## 6. Callbacks de patrones (`u16 enemy_id`)
Eliminar versiones `StateMachine*` en:
* `src/pattern_types/enemy_bite_pattern.c/.h`
* `src/pattern_types/enemy_electric_pattern.c/.h`

Actualizar tabla en `src/patterns_registry.c`:
```c
[PTRN_EN_ELECTIC] = { enemy_electric_pattern_launch, enemy_electric_pattern_do },
[PTRN_EN_BITE]    = { bite_pattern_launch,           bite_pattern_do }
```

**🛠 C-4** — sin warnings.

## 7. Alias opcional
```c
#define PTRN_EN_ELECTRIC PTRN_EN_ELECTIC
```

## 8. Limpieza final
```bash
grep -R "enemy_state_machines" src/   # → vacío
grep -R "StateMachine_" src/ | grep -v player
```

## 9. Makefile final
```make
OBJS = ...        enemy_logic.o        enemy_combat_state.o        enemy_pattern_data.o
```

## 10. Pruebas finales
1. Weaver Ghost → patrón eléctrico.
2. Three‑Head Monkey → patrón BITE.
3. Contra‑hechizo eléctrico refleja daño.
4. HUD de notas funciona.

---

### Check‑points de compilación

| ID | tras paso | estado |
|----|-----------|--------|
| C-0 | 1 | enlaza |
| C-1 | 2 | enlaza (enemigos quietos) |
| C-2 | 3 | errores de símbolos globales |
| C-3 | 5 | compila |
| C-4 | 6 | 0 warnings |
| Final | 9 | 0 errores / 0 warnings |

---

### Ficheros creados
```
src/enemy_logic.c/h
src/enemy_combat_state.c/h
src/enemy_pattern_data.c/h
```
### Ficheros eliminados
```
src/enemies_patterns.c/h
```

Fin del informe.
