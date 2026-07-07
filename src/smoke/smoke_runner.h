// smoke_runner.h — ejecución de un caso de la smoke ROM
#ifndef _SMOKE_RUNNER_H_
#define _SMOKE_RUNNER_H_

#include "smoke/smoke_cases.h"

bool smoke_run_case(const SmokeCase *c); // Ejecuta el caso; PASS/FAIL en pantalla y por KDebug (SMOKE: ...)

#endif
