#ifndef _ACT1_RET_H_
#define _ACT1_RET_H_

// Hooks C de act1_return (escena 6 del guión definitivo: regreso a la nave)

void act1_return_start(void);   // antorcha + viento
void act1_return_boars(void);   // segunda emboscada de jabalíes (melee)
void act1_return_ghosts(void);  // espectros del Caos (previo al op combat)
void act1_return_torch_out(void);     // la antorcha se apaga (inscribe el patrón)
void act1_return_wait_light(void);    // espera a que el jugador cante LUZ
void act1_return_torch_relight(void); // la Luz cantada la reenciende
void act1_return_end(void);           // apagar la antorcha antes de la escena final

#endif
