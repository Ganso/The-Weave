// fire.c — hechizo FIRE: el ejemplo canónico de autoría con el motor nuevo.
//
// Muestra el patrón completo:
//   - restricción de zona en canUse (solo en ZONE_CAULDRON)
//   - comportamiento distinto según el hechizo enemigo activo (come el thunder)
//   - efecto visual/daño DECLARADO con fases (el motor las ejecuta)
//   - hooks solo para lo que las fases no expresan (guardar/restaurar color)
//
// Para crear un hechizo nuevo: copiar este archivo, añadir su SPELL_* en
// constants_spells.h, llamar a su *_init() desde init_player_spells (player_spells.c) y
// añadir un caso a la smoke ROM (Fase 7). Ver también AGENTS.md.

#include <genesis.h>
#include "core/core.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "audio/audio.h"
#include "spells/player/fire.h"

#define COLOR_FIRE_VDP  RGB24_TO_VDPCOLOR(0xE08020)   // naranja fuego

static SpellPhase fire_phases[2];   // se rellenan en fire_init (SCREEN_FPS es runtime)
static u16 fire_saved_color;

static bool fire_can_use(const SpellContext *ctx)
{
    if (ctx->reversed) return false;
    if (ctx->zoneId != ZONE_CAULDRON)
        return false;                                       // solo en la zona del caldero

    u8 enemy_spell = spell_active_id(SPELL_SLOT_ENEMY);
    if (enemy_spell == SPELL_EN_THUNDER) return true;       // puede comerse el thunder enemigo
    if (enemy_spell != SPELL_NONE) return false;            // no interfiere con otros hechizos
    return true;                                            // uso normal
}

static void fire_on_launch(SpellContext *ctx)
{
    (void)ctx;
    fire_saved_color = PAL_getColor(PAL_BACKGROUND_COL4);

    // Comportamiento condicional: el fuego se come al thunder enemigo en curso
    if (spell_active_id(SPELL_SLOT_ENEMY) == SPELL_EN_THUNDER)
        spell_cancel(SPELL_SLOT_ENEMY);

    play_spell_jingle(SPELL_FIRE);
    // El resto del efecto (flash + daño) lo declaran las fases
}

static void fire_on_finish(SpellContext *ctx)
{
    (void)ctx;
    PAL_setColor(PAL_BACKGROUND_COL4, fire_saved_color);   // restaurar el color original
}

void fire_init(void)    // Registra FIRE (llamado desde init_spells)
{
    // Fases: [frames] — el motor las ejecuta; el daño puntual usa start==end
    fire_phases[0] = (SpellPhase){ 0,              SCREEN_FPS * 2,  PHASE_VISUAL_FLASH, PAL_BACKGROUND_COL4, COLOR_FIRE_VDP };
    fire_phases[1] = (SpellPhase){ SCREEN_FPS,     SCREEN_FPS,      PHASE_LOGIC_DAMAGE, PHASE_TARGET_ENEMY_ACTIVE, 2 };

    spell_defs[SPELL_FIRE] = (SpellDef){
        .id = SPELL_FIRE,
        .notes = { NOTE_MI, NOTE_FA, NOTE_SOL, NOTE_LA }, .noteCount = 4,
        .isPalindrome = false, .counterable = false,
        .baseDuration = SCREEN_FPS * 2,          // 2 segundos, PAL o NTSC
        .enabled = false,                        // no desbloqueado en la demo
        .canUse = fire_can_use,
        .onLaunch = fire_on_launch,
        .onFinish = fire_on_finish,
        .phases = fire_phases, .phaseCount = 2,
    };
}
