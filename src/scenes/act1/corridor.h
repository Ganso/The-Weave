#ifndef _ACT1_CORRIDOR_H_
#define _ACT1_CORRIDOR_H_

// Hooks C de act1_corridor (pasillo de los historiadores): los libros y los recuerdos.
// La secuencia narrativa vive en data/scenes/act1/corridor.scene.

void act1_corridor_setup(void); // Nivel del pasillo + libros/lámparas/puertas/mapas + Linus
void act1_corridor_entry(void); // Entrada de Linus por la derecha
void act1_corridor_items(void); // Bucle de items + salida al leer ambos libros

#endif
