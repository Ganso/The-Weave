/*
 * src/combat/weapons.h — Armas del jugador y reglas prefab de encuentro
 * ----------------------------------------------------------------------
 * El GOLPE físico (botón A) es la única arma de motor: buscar objetivo
 * delante, animación de ataque y cooldown. Las demás "armas" son REGLAS del
 * encuentro: funciones onTick prefabricadas que la configuración del combate
 * enchufa si las quiere (el motor no las impone). La primera es
 * combat_rule_thunder_scares: el patrón de TRUENO ahuyenta a la manada.
 * Cada encuentro futuro puede escribir la suya en su hook de escena.
 */
#ifndef _WEAPONS_H_
#define _WEAPONS_H_

#include <genesis.h>

void weapons_reset(bool strike_enabled);  // al empezar el encuentro
void weapons_tick(void);                  // un tick por frame: el golpe con A (si está habilitado)

// Regla prefab: al LANZARSE el TRUENO (el patrón suena entero; no al acabar su
// efecto), la descarga espanta a todos los de contacto y cuenta un impacto de
// manada. Enchufar como onTick del encuentro (regreso del acto 1).
void combat_rule_thunder_scares(void);

#endif
