// thunder.c — TRUENO del jugador: datos + hooks.
//
// Efecto de rayo (guión del acto 1): el escenario del bosque usa siempre la
// paleta oscura (forest_dark). Al lanzar el trueno, el fondo PARPADEA con la
// paleta clara (forest_pal) y, además, el color del cielo con blanco. En
// escenas con otro fondo, el parpadeo del cielo sigue funcionando.
//
// Invertido: solo se permite dentro de la ventana de counter del trueno
// enemigo (lo gestiona el motor via canUse + spell_try_counter).

#include <genesis.h>
#include "core/core.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "narrative/narrative.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "spells/player/thunder.h"

#define COLOR_WHITE_VDP  RGB24_TO_VDPCOLOR(0xFFFFFF)
#define THUNDER_FLASH_PERIOD  4   // frames por medio ciclo del parpadeo

static u16  thunder_saved_pal[16];   // el fondo original de la escena
static bool thunder_bright;          // true = fotograma en "claro"
static bool thunder_in_forest;       // true = el fondo es forest_dark → parpadea entero

// Fotograma claro / oscuro. En el bosque (fondo forest_dark) el destello cambia
// TODA la paleta a la clara (forest_pal); en cualquier otro escenario solo
// parpadea el color del cielo y no se toca el resto del fondo.
static void thunder_set_bright(bool bright)
{
    thunder_bright = bright;
    if (bright) {
        if (thunder_in_forest) PAL_setPalette(PAL_BACKGROUND, forest_pal.data, DMA);
        PAL_setColor(PAL_BACKGROUND_COL4, COLOR_WHITE_VDP);                 // destello del cielo
    } else {
        if (thunder_in_forest) PAL_setPalette(PAL_BACKGROUND, thunder_saved_pal, DMA);
        else PAL_setColor(PAL_BACKGROUND_COL4, thunder_saved_pal[4]);      // solo el cielo
    }
}

static bool thunder_can_use(const SpellContext *ctx)
{
    // Invertido: solo permitido para contrarrestar el thunder enemigo en su ventana
    if (ctx->reversed)
        return (combat_state == COMBAT_STATE_ENEMY_EFFECT) &&
               (spell_active_id(SPELL_SLOT_ENEMY) == SPELL_EN_THUNDER);

    // Directo: rechazado si el enemigo actual es un WeaverGhost (el hint lo da onRejected)
    if (ctx->enemyId != ENEMY_NONE &&
        obj_enemy[ctx->enemyId].class_id == ENEMY_CLS_WEAVERGHOST)
        return false;

    return true;
}

static void thunder_on_rejected(SpellContext *ctx)
{
    // ¿Hay un espectro (WeaverGhost) en pantalla? Solo entonces damos pistas.
    bool ghost_present = false;
    for (u8 i = 0; i < MAX_ENEMIES; i++)
        if (obj_enemy[i].obj_character.active &&
            obj_enemy[i].class_id == ENEMY_CLS_WEAVERGHOST) { ghost_present = true; break; }

    if (ctx->reversed)
    {
        // Trueno invertido fuera de su ventana de counter: si hay espectros,
        // la pista de esperar el momento; si no, el hechizo falla y ya está
        // (el sonido de patrón inválido basta como feedback).
        if (ghost_present)
            talk_dialog(&dialogs[ACT1_FOREST][A1_FOREST_REVERSE_HINT], false); // pista: espera su momento
        else
            play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
        return;
    }

    // Trueno DIRECTO contra espectros: son inmunes. Linus solo constata que no
    // les hace nada; la pista de cómo vencerlos la da Bobbin al caer derrotado
    // (guión 6.3), para no regalar la solución al primer intento.
    if (ghost_present)
        talk_dialog(&dialogs[ACT1_FOREST][A1_FOREST_NO_EFFECT], false);
    else
        talk_dialog(&dialogs[SYSTEM_DIALOG][SYSMSG_CANT_USE_PATTERN], false);
}

static void thunder_on_launch(SpellContext *ctx)
{
    (void)ctx;
    PAL_getPalette(PAL_BACKGROUND, thunder_saved_pal);   // guardar el fondo de la escena
    // ¿Estamos en el bosque? (fondo == forest_dark) → parpadeo de fondo completo
    thunder_in_forest = true;
    for (u16 i = 0; i < 16; i++)
        if (thunder_saved_pal[i] != forest_dark_pal.data[i]) { thunder_in_forest = false; break; }
    thunder_set_bright(true);                  // primer frame: destello inmediato
    play_spell_jingle(SPELL_THUNDER);
}

static bool thunder_on_update(SpellContext *ctx)
{
    // Parpadeo entre el fondo claro y el oscuro cada THUNDER_FLASH_PERIOD frames
    if ((ctx->frameCounter % THUNDER_FLASH_PERIOD) == 0)
        thunder_set_bright(!thunder_bright);

    if (ctx->frameCounter >= spell_defs[SPELL_THUNDER].baseDuration)
    {
        thunder_set_bright(false);   // restaurar exactamente el fondo oscuro
        dprintf(2, "Thunder done: restoring palette");
        return true;
    }
    return false;
}

static void thunder_on_cancel(SpellContext *ctx)    // cancelado a mitad: restaurar el fondo
{
    (void)ctx;
    thunder_set_bright(false);
}

void thunder_init(void)
{
    spell_defs[SPELL_THUNDER] = (SpellDef){
        .id = SPELL_THUNDER,
        .notes = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_SOL }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 4,          // 4 segundos
        .enabled = false,
        .canUse = thunder_can_use, .onRejected = thunder_on_rejected,
        .onLaunch = thunder_on_launch, .onUpdate = thunder_on_update,
        .onCancel = thunder_on_cancel,
    };
}
