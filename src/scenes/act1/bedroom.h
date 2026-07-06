#ifndef _ACT1_BEDROOM_H_
#define _ACT1_BEDROOM_H_

// Hooks C de act1_bedroom (dormitorio): la visita del cisne y el primer patrón.
// La secuencia narrativa vive en data/scenes/act1/bedroom.scene.

void act1_bedroom_setup(void); // Nivel nocturno, personajes e items del dormitorio
void act1_bedroom_swan(void);  // Cinemática del cisne (flash + diálogo + flash)
void act1_bedroom_wake(void);  // Amanece: paleta de día, Linus se levanta, control al jugador
void act1_bedroom_items(void); // Bucle de items: cama, silla, ventana, armario (aprende sleep)

#endif
