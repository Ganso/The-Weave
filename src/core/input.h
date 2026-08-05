// input.h — costura con el hardware del mando
#ifndef _INPUT_H_
#define _INPUT_H_

#include <genesis.h>

/* -------------------------------------------------------------------------
 * EL EMBUDO DE INPUT
 *
 * Todo el juego lee el pad por pad_read(). Un único punto de lectura permite
 * SECUESTRAR la entrada: con el override activo, pad_read() devuelve la
 * máscara inyectada en vez del mando físico.
 *
 * Dos usuarios del override:
 *   - Los casos SIM de la smoke ROM, que se pilotan a sí mismos desde dentro.
 *   - El driver MCP de RetroArch, que escribe g_inputOverride/Active por RAM
 *     desde el host (docs/retroarch-mcp.md). Por eso los dos símbolos son
 *     globales no-static: tienen que aparecer en out/symbol.txt.
 *
 * Aquí NO hay lógica de flancos: cada llamador conserva la suya
 * (joy_state/prev_joy_state). Este módulo es solo la costura; la semántica de
 * gameplay sigue en controller.c.
 * ------------------------------------------------------------------------- */

extern volatile u16 g_inputOverride;        // Máscara BUTTON_* inyectada
extern volatile u16 g_inputOverrideActive;  // != 0 → pad_read ignora el mando físico

u16  pad_read(void);                 // Estado del pad: mando físico u override
void input_setOverride(u16 buttons); // Activa el override con esa máscara
void input_clearOverride(void);      // Vuelve a leer el mando físico

#endif // _INPUT_H_
