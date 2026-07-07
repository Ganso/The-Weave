/*
 * src/scenes/scene_vm.h — VM de cutscenes
 * -----------------------------------------
 * Una escena es un array de SceneStep en ROM (generado por tools/gen_scenes.py
 * desde data/scenes/ *.scene). scene_run() lo interpreta: un case por opcode.
 *
 * Filosofía HÍBRIDA (fase5_design.md): el DSL expresa la secuencia narrativa
 * (diálogos, choices/branch, movimientos, esperas, combate, transiciones);
 * la lógica (setup con punteros a recursos, bucles de items, cinemáticas de
 * paleta) vive en hooks C (scene_hooks.c) invocados con el op CALL.
 *
 * Reglas de la VM:
 *  - NUNCA escribe en los steps (ROM). Estado mutable (last_choice, pc) en RAM.
 *  - No hay next_frame entre steps: los ops instantáneos se encadenan y los
 *    bloqueantes (say, move, wait, call, combat) gestionan sus propios frames.
 *
 * Archivos:
 *   scene_vm.c     — intérprete
 *   scene_hooks.c  — hooks C por escena + scene_hook_table[]
 *   scene_data.c/h — GENERADO: steps + scenes[] + scene_lookup(act, escena)
 *
 * Ver docs/refactor/fase5_design.md y AGENTS.md §escenas.
 */
#ifndef _SCENE_VM_H_
#define _SCENE_VM_H_

#include <genesis.h>

// Opcodes de la VM (mantener sincronizado con tools/gen_scenes.py)
typedef enum {
    SCENE_OP_CALL         = 0,   // a=HOOK_* — ejecuta scene_hook_table[a]()
    SCENE_OP_SAY          = 1,   // a=set, b=id, c=sound_on — talk_dialog
    SCENE_OP_SAY_CLUSTER  = 2,   // a=set, b=startId, c=sound_on — talk_cluster
    SCENE_OP_SAY_RESPONSE = 3,   // a=set, b=baseId, c=sound_on — dialogs[a][b + last_choice]
    SCENE_OP_CHOICE       = 4,   // a=choiceSet, b=item — last_choice = choice_dialog
    SCENE_OP_BRANCH       = 5,   // a=valor, b=stepIdx — salta si last_choice == a
    SCENE_OP_GOTO         = 6,   // a=stepIdx
    SCENE_OP_MOVE         = 7,   // a=chr, b=x, c=y — move_character (bloquea, B5)
    SCENE_OP_MOVE_INSTANT = 8,   // a=chr, b=x, c=y
    SCENE_OP_SHOW         = 9,   // a=chr, b=visible
    SCENE_OP_LOOK         = 10,  // a=chr, b=direction_right (semántica de look_left)
    SCENE_OP_WAIT         = 11,  // a=décimas de segundo (frames no-interactivos)
    SCENE_OP_WAIT_SCROLL  = 12,  // a=offset — espera interactiva hasta offset_BGA >= a
    SCENE_OP_SET          = 13,  // a=SCENE_FLAG_*, b=on
    SCENE_OP_COMBAT       = 14,  // combat_init + espera interactiva hasta COMBAT_NO
    SCENE_OP_CAST         = 15,  // a=spell, b=reversed — cast scripted (origin NARRATIVE)
    SCENE_OP_WAIT_SPELL   = 16,  // espera a que el slot PLAYER quede libre
    SCENE_OP_ZONE         = 17,  // a=ZONE_* — fija spell_zone
    SCENE_OP_FADE_OUT     = 18,  // a=frames — PAL_fadeOutAll
    SCENE_OP_NEXT_SCENE   = 19,  // a=SceneId (SCENE_*) — end_level + transición + return
    SCENE_OP_HARD_RESET   = 20,  // SYS_hardReset (final de la demo)
    SCENE_OP_END          = 21,  // return (sin end_level)
    SCENE_OP_PUZZLE_SEQ   = 22,  // a=índice en puzzle_seqs — activa el puzzle (progreso a 0)
    SCENE_OP_WAIT_PUZZLE  = 23,  // a=índice — espera interactiva hasta resolverlo
    SCENE_OP_IF_PUZZLE_SOLVED = 24, // a=índice, b=stepIdx — salta si está resuelto
    SCENE_OP_ANIM         = 25,  // a=chr, b=ANIM_* — anim_character
    SCENE_OP_WAIT_PRESS   = 26   // pausa de cutscene hasta pulsar A (frames no-interactivos)
} SceneOp;

// Flags del op SET
typedef enum {
    SCENE_FLAG_MOVEMENT  = 0,   // movement_active
    SCENE_FLAG_SCROLL    = 1,   // player_scroll_active
    SCENE_FLAG_INTERFACE = 2,   // on: interface_active=true + mostrar HUD; off: SOLO ocultar HUD
                                // (asimetría deliberada: nada del juego desactiva interface_active
                                //  a mitad de escena, y los rechazos de hechizo dependen de él)
    SCENE_FLAG_SPELLS    = 3    // player_patterns_enabled
} SceneFlag;

// Secuencia esperada de un puzzle de hechizos (tabla lateral GENERADA por
// gen_scenes.py: los pasos no caben en los 4 args s16 de un SceneStep)
#define PUZZLE_SEQ_MAX 4
typedef struct {
    u8 len;                      // nº de pasos de la secuencia (2..PUZZLE_SEQ_MAX)
    u8 spell[PUZZLE_SEQ_MAX];    // SPELL_* esperado en cada paso
    u8 reversed[PUZZLE_SEQ_MAX]; // 0=directo, 1=invertido
} PuzzleSeq;

typedef struct {
    u8  op;         // SceneOp
    s16 a, b, c, d; // argumentos según op (los no usados, 0)
} SceneStep;

typedef struct {
    const char *name;          // "act1_bedroom" (transiciones por nombre, logs, smoke ROM)
    const SceneStep *steps;    // en ROM — solo lectura estricta
    u16 stepCount;
} SceneScript;

extern u8 last_choice;       // resultado del último CHOICE (RAM de la VM)
extern u8 current_scene_id;  // SceneId de la escena en curso (identificador INTERNO;
                             // el juego siempre habla de escenas por NOMBRE)

void scene_run(const SceneScript *s); // Ejecuta una escena completa (bloquea hasta END/NEXT_SCENE)
void scene_puzzle_notify(u8 spellId, bool reversed); // Lo llama el motor de hechizos al TERMINAR un cast (progreso del puzzle activo)

#endif
