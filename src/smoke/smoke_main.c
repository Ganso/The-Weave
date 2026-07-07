// smoke_main.c — main de la smoke ROM: menú de casos (hechizos + escenas).
// SOLO se compila con -DHACK_SMOKE_BUILD (el main del juego queda excluido).
// Ver docs/testing.md.
#ifdef HACK_SMOKE_BUILD

#include <genesis.h>
#include "core/config.h"
#include "core/init.h"
#include "smoke/smoke_cases.h"
#include "smoke/smoke_runner.h"

static u8 cursor = 0;

static void menu_draw(void)
{
    VDP_clearPlane(BG_A, true);
    VDP_clearPlane(BG_B, true);
    VDP_drawText("THE WEAVE - SMOKE TEST", 8, 1);
    VDP_drawText("UP-DOWN + A", 14, 3); // la fuente del juego reserva / < > ^ # $ % * para glifos ES

    for (u16 i = 0; i < SMOKE_CASE_COUNT; i++)
    {
        VDP_drawText(smoke_cases[i].name, 4, 5 + i);
        VDP_drawText((i == cursor) ? "-" : " ", 2, 5 + i); // cursor: '-' es seguro ('>' saldría como ¡ en la fuente ES)
    }
}

static u16 wait_joy_press(void)    // Espera una pulsación nueva (con release previo)
{
    u16 joy;
    do { SYS_doVBlankProcess(); joy = JOY_readJoypad(JOY_1); } while (joy);         // release
    do { SYS_doVBlankProcess(); joy = JOY_readJoypad(JOY_1); } while (joy == 0);    // press
    return joy;
}

int main(bool hard)
{
    (void)hard;
    initialize(true);

    while (true)
    {
        menu_draw();
        u16 joy = wait_joy_press();

        if (joy & BUTTON_UP)   cursor = (cursor == 0) ? (u8)(SMOKE_CASE_COUNT - 1) : cursor - 1;
        if (joy & BUTTON_DOWN) cursor = (cursor + 1) % SMOKE_CASE_COUNT;
        if (joy & BUTTON_A)
        {
            smoke_run_case(&smoke_cases[cursor]);
            // El runner deja el resultado en pantalla; A para volver al menú
            while (!(wait_joy_press() & BUTTON_A)) { }
        }
    }
}

#endif // HACK_SMOKE_BUILD
