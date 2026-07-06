# Baseline de comportamiento — referencia del refactor (post-Fase-1)

> Este build (rama `refactor`, fin de Fase 1) es la **referencia de comportamiento**
> para todas las fases siguientes (decisión D10 de `refactorizar.md`). La comparación
> es funcional/visual, nunca binaria.
>
> PENDIENTE: playtest de validación del usuario (checklist abajo). Al completarlo,
> anotar aquí cualquier desviación observada y marcar la Fase 1 como cerrada.

## Cambios de comportamiento introducidos por los fixes de Fase 1

Todo lo demás debe comportarse exactamente como el build pre-refactor
(`docs/refactor/rom_pre_refactor.bin`).

| Fix | Cambio observable esperado |
|---|---|
| B4 (derrota sin bloqueo) | Al derrotar al WeaverGhost (counter con thunder invertido), la animación de muerte se reproduce **sin congelar el resto de la pantalla**: los sprites de fondo/HUD siguen animándose durante ~1s. Antes, todo se congelaba salvo el enemigo. El resultado final (enemigo desaparece, combate termina) es el mismo. |
| B12 (side de diálogo) | Ninguno esperado. Todos los diálogos y choices deben verse idénticos (posición de texto y cara). |
| B1, B2, B3 | Ninguno observable (sleep no es lanzable; bite sigue deshabilitado; solo hay un hechizo enemigo counterable). |
| B25 (underflow HP) | Ninguno con el balance actual (daño siempre 1). |

## Checklist de playtest (validación del baseline)

Ejecutar `out/rom.bin` en BlastEm y verificar:

- [ ] **Intro + logo**: GeeseBumps y la intro se ven como siempre.
- [ ] **Escena 1 (dormitorio)**: los 4 items responden en cualquier orden y son
      repetibles; el cabinet enseña el patrón sleep una sola vez; la escena termina
      ~3 s después de interactuar con el cabinet Y haber abierto la pausa.
- [ ] **Pausa**: el menú de pausa muestra el patrón aprendido; entrar/salir no rompe HUD.
- [ ] **Escena 2 (bosque)**: movimiento, followers e items como siempre.
- [ ] **Escena 3 (combate)**:
  - [ ] El WeaverGhost lanza thunder (notas + flash) con su cadencia habitual.
  - [ ] Thunder directo contra el ghost muestra el hint "pensar al revés".
  - [ ] El counter (thunder invertido durante el efecto enemigo) funciona.
  - [ ] Al morir el ghost: animación de muerte fluida (ver B4 arriba), el combate
        termina y la escena continúa.
  - [ ] Perder el combate (dejarse golpear) lleva a su rama correspondiente.
- [ ] **Escena 5 (finale)**: diálogos y cierre como siempre.
- [ ] **Diálogos ES y EN**: caracteres especiales españoles (ñ, tildes, ¿¡) se ven
      bien (B7 cambió el buffer de encoding); posiciones de texto idénticas (B12).

## Notas del playtest

(rellenar al validar)
