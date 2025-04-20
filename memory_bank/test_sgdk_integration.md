# Integración de Tests con SGDK

## Cambios Realizados

### 1. Adaptación a SGDK
- Uso de tipos SGDK (TRUE/FALSE en lugar de true/false)
- Reemplazo de TEST_LOG por KLog para mejor integración
- Estructura de includes siguiendo patrón SGDK
- Macros de aserción compatibles con SGDK

### 2. Mejoras en Reportes de Error
```c
struct _test_result {
    const char* test_name;
    bool passed;
    const char* message;
    u16 line_number;         // Añadido para mejor diagnóstico
    const char* file_name;   // Añadido para mejor diagnóstico
};
```

### 3. Macros de Aserción
```c
#define TEST_ASSERT(condition) \
    if (!(condition)) { \
        result.passed = FALSE; \
        result.message = #condition; \
        result.line_number = __LINE__; \
        result.file_name = __FILE__; \
        return result; \
    }
```

## Errores de Inclusión Actuales

### Problema
Los archivos de test muestran errores relacionados con libres.h:
```
cannot open source file "libres.h" (dependency of "genesis.h")
```

### Solución Propuesta

1. **Estructura de Includes**
   ```c
   // Orden correcto de includes
   #include <genesis.h>      // SGDK core
   #include "globals.h"      // Project globals
   #include "test_config.h"  // Test configuration
   ```

2. **Actualización de Include Path**
   - Verificar SGDK_DIR está correctamente configurado
   - Añadir ruta a libres.h en includePath
   - Asegurar que res/ está en el path

3. **Compilación Condicional**
   ```c
   #ifdef TESTING_ENABLED
   // Test code here
   #endif
   ```

## Próximos Pasos

### 1. Configuración de Build
1. Verificar variable de entorno SGDK_DIR
2. Actualizar makefile para incluir:
   - Archivos de test
   - Rutas de include necesarias
   - Flags de compilación para tests

### 2. Estructura de Directorios
```
src/
  test/              # Directorio específico para tests
    test_config.h
    test_statemachine.h
    test_statemachine.c
    test_basic_sm.c
```

### 3. Makefile Updates
```makefile
# Test files
TESTS_DIR = $(SRC_DIR)/test
TEST_SRCS = $(wildcard $(TESTS_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:.c=.o)

# Include paths
INCLUDES += -I$(SGDK_DIR)/inc
INCLUDES += -I$(SRC_DIR)/test
```

### 4. Configuración de VSCode
```json
{
  "includePath": [
    "${SGDK_DIR}/inc",
    "${workspaceFolder}/src",
    "${workspaceFolder}/res"
  ]
}
```

## Consideraciones

1. **Compilación Condicional**
   - Tests solo compilados en modo debug
   - No afectan build de release
   - Control granular por tipo de test

2. **Rendimiento**
   - Minimizar overhead en runtime
   - Logging eficiente
   - Cleanup apropiado de recursos

3. **Mantenibilidad**
   - Estructura clara de archivos
   - Naming conventions consistente
   - Documentación inline

## Plan de Implementación

1. **Fase 1: Setup**
   - [ ] Crear estructura de directorios
   - [ ] Actualizar makefile
   - [ ] Configurar VSCode

2. **Fase 2: Correcciones**
   - [ ] Resolver errores de include
   - [ ] Verificar compilación limpia
   - [ ] Probar ejecución básica

3. **Fase 3: Validación**
   - [ ] Ejecutar suite completa
   - [ ] Verificar output
   - [ ] Documentar resultados

## Notas Técnicas

1. **SGDK Versión**
   - Verificar compatibilidad
   - Usar macros específicos de versión
   - Mantener compatibilidad futura

2. **Debug Output**
   - Usar KLog para consistencia
   - Formateo claro de mensajes
   - Niveles apropiados de detalle

3. **Memoria**
   - Gestión cuidadosa de recursos
   - Cleanup después de cada test
   - Monitoreo de uso de memoria

## Referencias

1. SGDK Documentation
2. Genesis Programming Guidelines
3. Test Best Practices