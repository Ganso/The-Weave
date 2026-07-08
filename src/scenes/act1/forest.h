#ifndef _ACT1_FOREST_H_
#define _ACT1_FOREST_H_

// Hooks C de act1_forest (bosque): tutorial de hechizos y combate.
// La secuencia narrativa vive en data/scenes/act1/forest.scene.

void act1_forest_pad_hint(void); // Aviso si el mando es de 3 botones
void act1_forest_day(void);      // Fade a la paleta de día
void act1_forest_enemies(void);  // Aparición de los dos WeaverGhosts (previo al op combat)

#endif
