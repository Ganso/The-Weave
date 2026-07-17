// smoke_main.c — main de la smoke ROM: menú de casos (hechizos + escenas).
// SOLO se compila con -DHACK_SMOKE_BUILD (el main del juego queda excluido).
// Ver docs/testing.md.
#ifdef HACK_SMOKE_BUILD

#include <genesis.h>
#include "core/core.h"
#include "smoke/smoke_cases.h"
#include "smoke/smoke_runner.h"

static u8 cursor = 0;

// Filas visibles del menú: de la 3 a la 3+MENU_ROWS-1, dentro de la zona
// segura NTSC. Ya hay más casos que filas: la lista es una VENTANA deslizante
// que sigue al cursor (los "..." indican que hay más arriba/abajo).
#define MENU_ROWS 17

static void menu_draw(void)
{
    VDP_clearPlane(BG_A, true);
    VDP_clearPlane(BG_B, true);
    VDP_drawText("THE WEAVE - SMOKE TEST", 8, 0);
    VDP_drawText("UP-DOWN + A", 14, 1); // la fuente del juego reserva / < > ^ # $ % * para glifos ES

    u16 first = 0;
    if (SMOKE_CASE_COUNT > MENU_ROWS && cursor >= MENU_ROWS - 1)
    {
        first = cursor - (MENU_ROWS - 1);
        u16 max_first = SMOKE_CASE_COUNT - MENU_ROWS;
        if (first > max_first) first = max_first;
    }

    if (first > 0) VDP_drawText("...", 4, 2);
    for (u16 row = 0; row < MENU_ROWS && first + row < SMOKE_CASE_COUNT; row++)
    {
        u16 i = first + row;
        VDP_drawText(smoke_cases[i].name, 4, 3 + row);
        VDP_drawText((i == cursor) ? "}" : " ", 2, 3 + row); // '}' = mismo glifo de selección que los diálogos
    }
    if (first + MENU_ROWS < SMOKE_CASE_COUNT) VDP_drawText("...", 4, 3 + MENU_ROWS);
}

static u16 wait_joy_press(void)    // Espera una pulsación nueva (con release previo)
{
    u16 joy;
    do { SYS_doVBlankProcess(); joy = JOY_readJoypad(JOY_1); } while (joy);         // release
    do { SYS_doVBlankProcess(); joy = JOY_readJoypad(JOY_1); } while (joy == 0);    // press
    return joy;
}

// Ventana de arranque: da ~3 s para saltar al menú con A. Si nadie pulsa,
// devuelve false y el main corre la suite AUTO (validación desatendida: el
// NCI de RetroArch no envia input, así que la ROM debe llegar sola a la
// pantalla de resultados; ver docs/retroarch-mcp.md).
static bool boot_wait_skip(void)
{
    VDP_clearPlane(BG_A, true);
    VDP_clearPlane(BG_B, true);
    VDP_drawText("THE WEAVE - SMOKE TEST", 8, 8);
    VDP_drawText("A = menu", 8, 11);
    VDP_drawText("espera = AUTO test", 8, 12);
    do { SYS_doVBlankProcess(); } while (JOY_readJoypad(JOY_1)); // release previo
    for (u16 f = 0; f < 180; f++)
    {
        SYS_doVBlankProcess();
        if (JOY_readJoypad(JOY_1) & BUTTON_A) return true;
    }
    return false;
}

int main(bool hard)
{
    (void)hard;
    dprintf(3, "SMOKE main: pre-initialize");
    initialize(true);
    dprintf(3, "SMOKE main: post-initialize, boot_wait_skip");

    if (!boot_wait_skip())
    {
        dprintf(3, "SMOKE main: arrancando AUTO (fila 0)");
        smoke_run_case(&smoke_cases[0]);              // fila 0 = SMOKE_AUTO
        while (!(wait_joy_press() & BUTTON_A)) { }     // resultados en pantalla hasta A
    }

    // Navegación con auto-repetición: al MANTENER arriba/abajo el cursor avanza
    // solo (retardo inicial y luego repetición rápida). A selecciona por flanco.
    #define MENU_REPEAT_DELAY 15   // frames que hay que mantener antes de repetir
    #define MENU_REPEAT_RATE   3   // frames entre repeticiones mientras se mantiene

    // prev arranca con los botones que YA estén pulsados al entrar (A viene
    // pulsado de llegar aquí): así un botón mantenido no cuenta como flanco
    // nuevo hasta soltarlo y volver a pulsarlo.
    u16 prev = JOY_readJoypad(JOY_1);
    u16 hold = 0;    // cuenta atrás para la próxima repetición de dirección
    menu_draw();
    while (true)
    {
        SYS_doVBlankProcess();
        u16 joy = JOY_readJoypad(JOY_1);

        // A: seleccionar (solo en el flanco de subida, no mientras se mantiene)
        if ((joy & BUTTON_A) && !(prev & BUTTON_A))
        {
            smoke_run_case(&smoke_cases[cursor]);
            while (!(wait_joy_press() & BUTTON_A)) { } // resultado en pantalla hasta A
            prev = JOY_readJoypad(JOY_1);   // A sigue pulsado: no lo cuentes como flanco
            hold = 0;
            menu_draw();
            continue;
        }

        // Arriba/abajo con auto-repetición
        u16 dir  = joy  & (BUTTON_UP | BUTTON_DOWN);
        u16 pdir = prev & (BUTTON_UP | BUTTON_DOWN);
        bool move = false;
        if (dir == 0)                     hold = 0;                              // soltado
        else if (dir != pdir)           { move = true; hold = MENU_REPEAT_DELAY; } // pulsación nueva
        else if (hold > 0 && --hold == 0) { move = true; hold = MENU_REPEAT_RATE; } // mantenido: repetir

        if (move)
        {
            if (dir & BUTTON_UP) cursor = (cursor == 0) ? (u8)(SMOKE_CASE_COUNT - 1) : cursor - 1;
            else                 cursor = (cursor + 1) % SMOKE_CASE_COUNT;
            menu_draw();
        }
        prev = joy;
    }
}

#endif // HACK_SMOKE_BUILD
