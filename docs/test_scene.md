# act1_test — banco de pruebas del motor (documento vivo)

> QUÉ ES: inventario de todo lo implementado en la escena de test y CÓMO probarlo.
> Se actualiza con cada mecánica nueva. Si una sesión se corta a medias, este
> fichero es la fuente de verdad de lo que hay y lo que falta.
>
> CÓMO ENTRAR: `HACK_START_SCENE "act1_test"` en `src/core/hack.h`, o smoke ROM
> (`./build-theweave.sh smoke` → caso "SCENE act1_test (motor)").

## Estado: EN AMPLIACIÓN (sesión 2026-07-07)

### Estructura objetivo: hub con secciones

La escena arranca con una intro corta y un MENÚ (choice de 4 opciones) que
permite probar cada sección por separado y volver al menú al terminar:

1. **Diálogos y quiz** — caras/lados distintos, animación por DSL, quiz de 2
   preguntas con reintentos y confirmación anidada (choices dentro de choices).
2. **Hechizos y puzzles** — casts scripted (sleep y el nuevo LUZ), y a elegir:
   puzzle básico (trueno→fuego→esconderse) o puzzle con INVERTIDO
   (trueno→luz invertida→esconderse).
3. **Combate** — oleada 1: WeaverGhost clásico (counter). Oleada 2: fantasma de
   TEST con DOS hechizos (trueno counterable + mordisco no-counterable).
4. **Terminar** — fundido y next_scene al dormitorio.

## Mecánicas implementadas y cómo probarlas

| Mecánica | Dónde | Qué verificar |
|---|---|---|
| (pendiente de rellenar según se implementen) | | |

## Cambios de motor de esta sesión

(pendiente de rellenar)

## Pendiente / ideas no implementadas aún

- Op `anim <chr> <ANIM_*>` (animación de personaje desde DSL)
- Op `wait_press` (pausa de cutscene hasta pulsar A)
- Hechizo nuevo SPELL_LIGHT por fases declarativas, casteable directo E invertido
- Puzzle 2 con paso invertido (ejercita `reversed` en PuzzleSeq de verdad)
- Quiz con choices anidados y bucles de reintento (branch/goto intensivo)
- Clase ENEMY_CLS_TESTGHOST con {EN_THUNDER, EN_BITE} — primer enemigo
  multi-hechizo (recargas alternas) y primer mordisco funcional
- Segunda oleada de combate en la misma escena (combat ×2)
- Casos nuevos de smoke ROM para LIGHT
