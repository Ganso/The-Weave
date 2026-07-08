#ifndef _SCENES_H_
#define _SCENES_H_

// Metalibrería del dominio SCENES: VM, hooks, datos generados, intro y logo.
// (los headers de hooks por escena scenes/act1/*.h son específicos: se incluyen
//  directamente donde hagan falta, no van aquí)
// Uso: incluye esta metalibrería desde los .c en vez de los headers sueltos.

#include "scenes/scene_vm.h"
#include "scenes/scene_hooks.h"
#include "scenes/scene_data.h"
#include "scenes/intro.h"
#include "scenes/geesebumps.h"

#endif // _SCENES_H_
