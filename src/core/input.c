// input.c — lectura del pad con override inyectable (ver input.h)
#include <genesis.h>
#include "core/core.h"

// Globales no-static a propósito: el driver MCP los localiza en out/symbol.txt
// y los escribe por RAM para pulsar botones desde el host.
volatile u16 g_inputOverride = 0;
volatile u16 g_inputOverrideActive = 0;

u16 pad_read(void)    // Estado del pad: mando físico u override
{
    if (g_inputOverrideActive) return g_inputOverride;
    return JOY_readJoypad(JOY_ALL);
}

void input_setOverride(u16 buttons)    // Activa el override con esa máscara
{
    g_inputOverride = buttons;
    g_inputOverrideActive = 1;
}

void input_clearOverride(void)    // Vuelve a leer el mando físico
{
    g_inputOverrideActive = 0;
    g_inputOverride = 0;
}
