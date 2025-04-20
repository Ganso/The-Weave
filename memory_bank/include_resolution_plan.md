# Plan de Resolución de Includes SGDK

## Análisis del Problema

### Síntomas Actuales
```
Error: cannot open source file "libres.h" (dependency of "genesis.h")
```

Este error indica que:
1. SGDK no está correctamente configurado en el path
2. La jerarquía de includes no sigue el patrón SGDK
3. Posibles conflictos en el orden de includes

## Estructura SGDK Esperada

```
$SGDK_DIR/
  ├── inc/
  │     ├── genesis.h
  │     ├── libres.h
  │     ├── types.h
  │     └── ...
  ├── lib/
  └── res/
```

## Jerarquía de Includes Actual

```
globals.h
  ├── genesis.h
  ├── Debug includes
  │     ├── test_config.h
  │     └── test_statemachine.h
  ├── Resource includes
  └── Project includes
```

## Plan de Resolución

### 1. Verificación de SGDK
- [ ] Confirmar instalación SGDK
- [ ] Verificar variable de entorno SGDK_DIR
- [ ] Validar estructura de directorios SGDK

### 2. Configuración del Proyecto
```json
// .vscode/c_cpp_properties.json
{
    "configurations": [
        {
            "name": "SGDK",
            "includePath": [
                "${env:SGDK_DIR}/inc",
                "${workspaceFolder}/src",
                "${workspaceFolder}/res"
            ],
            "defines": [
                "GENESIS_SDCC"
            ],
            "compilerPath": "${env:SGDK_DIR}/bin/gcc",
            "cStandard": "c11"
        }
    ],
    "version": 4
}
```

### 3. Reorganización de Includes
```c
// 1. SGDK Core (siempre primero)
#include <genesis.h>

// 2. SGDK Resources (generados)
#include "resources.h"

// 3. Project Headers
#include "globals.h"

// 4. Test Framework (solo en DEBUG_ON)
#ifdef DEBUG_ON
#include "test_config.h"
#endif
```

### 4. Pasos de Implementación

1. **Verificación SGDK**
   ```bash
   echo $SGDK_DIR
   ls $SGDK_DIR/inc
   ```

2. **Configuración VSCode**
   - Crear/actualizar c_cpp_properties.json
   - Verificar extensión C/C++
   - Recargar ventana VSCode

3. **Actualización de Headers**
   - Actualizar test.h
   - Actualizar test_config.h
   - Actualizar test_statemachine.h

4. **Verificación**
   - Compilar proyecto
   - Verificar resolución de includes
   - Validar sin warnings

## Orden de Actualización

### Fase 1: Setup
1. Verificar SGDK
2. Configurar VSCode
3. Validar paths

### Fase 2: Headers
1. globals.h
2. test.h
3. test_config.h
4. test_statemachine.h

### Fase 3: Verificación
1. Compilación limpia
2. No warnings
3. Includes resueltos

## Consideraciones

### 1. Compatibilidad
- Mantener compatibilidad con builds existentes
- No romper includes actuales
- Preservar DEBUG_ON logic

### 2. Mantenibilidad
- Documentar estructura de includes
- Mantener orden consistente
- Facilitar updates futuros

### 3. Performance
- Minimizar includes redundantes
- Optimizar header guards
- Reducir dependencias circulares

## Próximos Pasos

### Inmediato
1. Crear/actualizar c_cpp_properties.json
2. Verificar SGDK_DIR
3. Actualizar headers principales

### Corto Plazo
1. Validar compilación
2. Resolver warnings
3. Documentar cambios

### Medio Plazo
1. Optimizar includes
2. Mejorar estructura
3. Actualizar documentación

## Notas Técnicas

### 1. SGDK Specifics
```c
// Siempre incluir genesis.h primero
#include <genesis.h>

// Usar tipos SGDK
typedef struct {
    u16 value;    // No uint16_t
    bool flag;    // No _Bool
} Example;
```

### 2. Include Guards
```c
#ifndef _TEST_H_
#define _TEST_H_

// ... contenido ...

#endif // _TEST_H_
```

### 3. Forward Declarations
```c
// Evitar includes circulares
struct _test_result;
typedef struct _test_result TestResult;
```

## Referencias

1. SGDK Documentation
   - Include hierarchy
   - Build system
   - Type system

2. Project Structure
   - Current organization
   - Include patterns
   - Build process

3. C Best Practices
   - Header organization
   - Include guards
   - Forward declarations