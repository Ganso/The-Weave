// light.c — hechizo LUZ: showcase de fases declarativas.
//
// A diferencia de thunder (que alterna colores por frame en onUpdate), LUZ
// declara su efecto como DATOS: dos tramos de flash de paleta consecutivos.
// Los hooks solo guardan/restauran el color original. Es la demostración de
// que un hechizo visual sencillo no necesita lógica imperativa.
//
// Invertido: canUse lo permite siempre (a diferencia de thunder, cuyo invertido
// es solo el counter) — eso lo convierte en la pieza clave del puzzle con paso
// invertido de la escena de test.

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "spells/spell.h"
#include "spells/light.h"
#include "audio/sound.h"

#define COLOR_LIGHT_CYAN   RGB24_TO_VDPCOLOR(0x66DDFF)   // primer tramo: cian
#define COLOR_LIGHT_WHITE  RGB24_TO_VDPCOLOR(0xFFFFFF)   // segundo tramo: blanco

static SpellPhase light_phases[2];   // rellenadas en runtime (SCREEN_FPS)
static u16 light_saved_color;

static void light_on_launch(SpellContext *ctx)
{
    (void)ctx;
    light_saved_color = PAL_getColor(PAL0_COL4);
    play_spell_jingle(SPELL_LIGHT);
    // El efecto visual completo lo declaran las fases
}

static void light_on_finish(SpellContext *ctx)
{
    (void)ctx;
    PAL_setColor(PAL0_COL4, light_saved_color);   // restaurar el color original
}

void light_init(void)    // Registra LUZ (llamado desde init_spells)
{
    // Fase 1: cian la primera mitad; fase 2: blanco la segunda mitad
    light_phases[0] = (SpellPhase){ 0,                  SCREEN_FPS * 3 / 4,  PHASE_VISUAL_FLASH, PAL0_COL4, COLOR_LIGHT_CYAN };
    light_phases[1] = (SpellPhase){ SCREEN_FPS * 3 / 4, SCREEN_FPS * 3 / 2,  PHASE_VISUAL_FLASH, PAL0_COL4, COLOR_LIGHT_WHITE };

    spell_defs[SPELL_LIGHT] = (SpellDef){
        .id = SPELL_LIGHT,
        .notes = { NOTE_SOL, NOTE_LA, NOTE_SI, NOTE_DO }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 3 / 2,      // 1,5 segundos
        .enabled = false,                        // solo lo habilita la escena de test
        // sin canUse: usable siempre, directo E invertido
        .onLaunch = light_on_launch,
        .onFinish = light_on_finish,
        .phases = light_phases, .phaseCount = 2,
    };
}
