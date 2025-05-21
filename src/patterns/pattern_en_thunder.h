#ifndef PATTERN_EN_THUNDER_H
#define PATTERN_EN_THUNDER_H

#include "globals.h"


// Launch the spell
void enemyThunderLaunch(u8 enemyId);

// Update the ongoing spell
bool enemyThunderUpdate(u8 enemyId);

// Pattern has been countered
void enemyThunderOnCounter(u8 enemyId);     // opcional (solo si counterable)

#endif /* PATTERN_EN_THUNDER_H */
