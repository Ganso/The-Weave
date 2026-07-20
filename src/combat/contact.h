/*
 * src/combat/contact.h — Enemigos de CONTACTO (persiguen y atacan de cerca)
 * --------------------------------------------------------------------------
 * FSM genérica de los enemigos con role ENEMY_ROLE_CONTACT: entran escalonados,
 * persiguen al jugador (con desvío y pausas aleatorios), atacan a corta
 * distancia y, al ser impactados, huyen al borde de pantalla más cercano y
 * vuelven a la carga. El ataque CONCRETO (alcance, ritmo, daño) no vive aquí:
 * lo pone el ContactProfile de la clase del enemigo (el mordisco es del jabalí;
 * otra clase atacará distinto con esta misma FSM).
 *
 * Un impacto no mata: el enemigo escarmienta (flash de STATE_HIT) y huye. Al
 * llegar a hits_to_win impactos acumulados, todos huyen definitivamente y se
 * liberan. La vida/derrota del jugador es del director (combat.c), que llama a
 * contact_reset() al empezar y contact_tick() una vez por frame.
 */
#ifndef _CONTACT_H_
#define _CONTACT_H_

#include <genesis.h>

void contact_reset(u8 hits_to_win);   // toma el control de los enemigos de contacto activos
void contact_tick(void);              // un tick por frame: perseguir / atacar / huir
bool contact_all_gone(void);          // ya no queda ninguno (huidos o liberados)
void contact_scare_enemy(u16 nenemy); // un impacto: flash de daño + huida + vuelta
void contact_scare_all(void);         // espanta a todos los que persiguen o atacan
void contact_count_hit(void);         // suma un impacto de manada (lo llaman armas y reglas)
void contact_release_all(void);       // libera a los que queden (derrota / cierre)
u16  contact_find_in_front(s16 range_x, s16 range_y); // objetivo delante del jugador (MAX_ENEMIES si nada)

extern u16 contact_hits;              // impactos acumulados (observable por RAM en verificación)

#endif
