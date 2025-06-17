#ifndef PATTERN_EN_THUNDER_H
#define PATTERN_EN_THUNDER_H

#include "globals.h"


// Launch the spell
void enemy_thunder_launch(u8 enemyId);

// Update the ongoing spell
bool enemy_thunder_update(u8 enemyId);

// Pattern has been countered
void enemy_thunder_on_counter(u8 enemyId);     // opcional (solo si counterable)

// Cancel thunder without damaging the enemy (used by HIDE)
void enemy_thunder_cancel(u8 enemyId);

#endif /* PATTERN_EN_THUNDER_H */
