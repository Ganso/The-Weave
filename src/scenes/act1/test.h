#ifndef _ACT1_TEST_H_
#define _ACT1_TEST_H_

// Hooks C de act1_test: escena de PRUEBA del motor de escenas. No está enlazada
// desde el flujo del juego — solo se llega con HACK_START_SCENE "act1_test"
// (core/hack.h) o desde la smoke ROM. Reutiliza los recursos del pasillo.

void act1_test_setup(void); // Nivel del pasillo + items + Linus/Clio + vara y hechizos del test
void act1_test_ghost(void);  // Oleada 1: WeaverGhost clásico (counterable)
void act1_test_ghost2(void); // Oleada 2: TESTGHOST con dos hechizos (thunder + mordisco)

#endif
