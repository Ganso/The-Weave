#!/usr/bin/env python3
# gen_scenes.py – convierte data/scenes/*.scene en src/scenes/scene_data.{h,c}
# Ejecutar desde la raíz del repo: python3 tools/gen_scenes.py
#
# Validación FATAL (nunca warnings):
#  - sintaxis y aridad de cada op
#  - labels de goto/branch existen (dos pasadas)
#  - say*: set + id existen en data/texts.csv
#  - choice: set + item existen en data/choices.csv
#  - call: el hook existe en src/scenes/scene_hooks.h (enum HOOK_*)
#  - cast/zone: la constante existe en src/spells/constants_spells.h
# Las constantes C (CHR_*, ACT1_DIALOG*, SPELL_*, HOOK_*) se emiten VERBATIM:
# el compilador C es la segunda red de validación.
import csv
import glob
import os
import re
import sys
from collections import OrderedDict

SCENES_GLOB   = 'data/scenes/*/*.scene'
TEXTS_CSV     = 'data/texts.csv'
CHOICES_CSV   = 'data/choices.csv'
HOOKS_HEADER  = 'src/scenes/scene_hooks.h'
SPELLS_HEADER = 'src/spells/constants_spells.h'
HEADER_FILE   = 'src/scenes/scene_data.h'
SOURCE_FILE   = 'src/scenes/scene_data.c'

def die(msg):
    print(f"gen_scenes.py: ERROR: {msg}", file=sys.stderr)
    sys.exit(1)

# ---------------------------------------------------------------------------
# Cargar referencias válidas
# ---------------------------------------------------------------------------

# Diálogos: set (upper) -> set de ids válidos
dialog_ids = {}
with open(TEXTS_CSV, newline='', encoding='utf-8') as f:
    for row in csv.DictReader(f):
        dialog_ids.setdefault(row['set'].strip().upper(), set()).add(row['id'].strip())

# Choices: set (upper) -> número de items
choice_items = {}
with open(CHOICES_CSV, newline='', encoding='utf-8') as f:
    for row in csv.DictReader(f):
        name = row['set'].strip().upper()
        choice_items[name] = max(choice_items.get(name, 0), int(row['item']) + 1)

# Hooks: nombres del enum HOOK_* (sin el prefijo, en minúsculas)
hooks = set()
for m in re.finditer(r'\bHOOK_([A-Z0-9_]+)', open(HOOKS_HEADER, encoding='utf-8').read()):
    if m.group(1) != 'COUNT':
        hooks.add(m.group(1).lower())

# Spells y zonas válidos
spells_src = open(SPELLS_HEADER, encoding='utf-8').read()
valid_spells = set(re.findall(r'#define (SPELL_[A-Z0-9_]+)', spells_src))
valid_zones  = set(re.findall(r'#define (ZONE_[A-Z0-9_]+)', spells_src))

# ---------------------------------------------------------------------------
# Definición de ops: nombre DSL -> (opcode, [tipos de args])
# tipos: chr=verbatim, int, set+id (dialogo), choiceset, label, hook, spell,
#        zone, flag, onoff, sound
# ---------------------------------------------------------------------------
OPS = {
    'call':         ('SCENE_OP_CALL',         ['hook']),
    'say':          ('SCENE_OP_SAY',          ['dialogset', 'dialogid', 'sound?']),
    'say_cluster':  ('SCENE_OP_SAY_CLUSTER',  ['dialogset', 'dialogid', 'sound?']),
    'say_response': ('SCENE_OP_SAY_RESPONSE', ['dialogset', 'dialogid', 'sound?']),
    'choice':       ('SCENE_OP_CHOICE',       ['choiceset', 'int']),
    'branch':       ('SCENE_OP_BRANCH',       ['int', 'kwgoto', 'label']),
    'goto':         ('SCENE_OP_GOTO',         ['label']),
    'move':         ('SCENE_OP_MOVE',         ['verbatim', 'int', 'int']),
    'move_instant': ('SCENE_OP_MOVE_INSTANT', ['verbatim', 'int', 'int']),
    'show':         ('SCENE_OP_SHOW',         ['verbatim', 'onoff']),
    'look':         ('SCENE_OP_LOOK',         ['verbatim', 'leftright']),
    'wait':         ('SCENE_OP_WAIT',         ['int']),
    'wait_scroll':  ('SCENE_OP_WAIT_SCROLL',  ['int']),
    'wait_scroll_left': ('SCENE_OP_WAIT_SCROLL_L', ['int']),
    'set':          ('SCENE_OP_SET',          ['flag', 'onoff']),
    'combat':       ('SCENE_OP_COMBAT',       []),
    'cast':         ('SCENE_OP_CAST',         ['spell', 'direction?']),
    'wait_spell':   ('SCENE_OP_WAIT_SPELL',   []),
    'zone':         ('SCENE_OP_ZONE',         ['zone']),
    'fade_out':     ('SCENE_OP_FADE_OUT',     ['int']),
    'next_scene':   ('SCENE_OP_NEXT_SCENE',   ['scene']),
    'hard_reset':   ('SCENE_OP_HARD_RESET',   []),
    'end':          ('SCENE_OP_END',          []),
    'wait_puzzle':  ('SCENE_OP_WAIT_PUZZLE',  ['puzzletag']),
    'anim':         ('SCENE_OP_ANIM',         ['verbatim', 'verbatim']),
    'wait_press':   ('SCENE_OP_WAIT_PRESS',   []),
    'if_puzzle_solved': ('SCENE_OP_IF_PUZZLE_SOLVED', ['puzzletag', 'kwgoto', 'label']),
    'if_defeated':  ('SCENE_OP_IF_DEFEATED',  ['kwgoto', 'label']),
    # --- Setup declarativo (level/item/palette tienen handler propio abajo) ---
    'limits':       ('SCENE_OP_LIMITS',       ['int', 'int', 'int', 'int']),
    'character':    ('SCENE_OP_CHARACTER',    ['verbatim']),
    'active':       ('SCENE_OP_ACTIVE',       ['verbatim']),
    'follow':       ('SCENE_OP_FOLLOW',       ['verbatim', 'onoff']),
    'enable_spell': ('SCENE_OP_ENABLE_SPELL', ['spell']),
}
PUZZLE_SEQ_MAX = 4
FLAGS = {'movement': 'SCENE_FLAG_MOVEMENT', 'scroll': 'SCENE_FLAG_SCROLL',
         'interface': 'SCENE_FLAG_INTERFACE', 'spells': 'SCENE_FLAG_SPELLS',
         'rod': 'SCENE_FLAG_ROD'}
# Modos de scroll del op `level` (mapean a los BG_SCRL_* de world/background.h)
SCROLL_MODES = {'auto_right': 'BG_SCRL_AUTO_RIGHT', 'auto_left': 'BG_SCRL_AUTO_LEFT',
                'user_right': 'BG_SCRL_USER_RIGHT', 'user_left': 'BG_SCRL_USER_LEFT'}

# ---------------------------------------------------------------------------
# Parseo de una escena
# ---------------------------------------------------------------------------
def scene_name_of(path):
    # data/scenes/<acto>/<escena>.scene → "<acto>_<escena>" (p.ej. act1_bedroom)
    actdir = os.path.basename(os.path.dirname(path))
    base = os.path.splitext(os.path.basename(path))[0]
    return f"{actdir}_{base}"

puzzle_seqs = []   # global: cada entrada es {'spells': [(SPELL_X, 0|1), ...]}

# Tablas laterales de setup (globales a todas las escenas, como puzzle_seqs).
# Cada entrada guarda el fragmento C ya formateado que emitiremos en scene_data.c.
scene_levels    = []   # cada entrada: dict con los 8 campos de SceneLevel
scene_items     = []   # cada entrada: dict con los 9 campos de SceneItem
scene_palettes  = []   # cada entrada: nombre de recurso Palette (verbatim)

# Nombre de recurso rescomp: identificador C (letras, dígitos, _). Se emite
# VERBATIM (con & donde toque); el compilador C valida que exista de verdad.
RES_RE = re.compile(r'^[A-Za-z_][A-Za-z0-9_]*$')
def res_ref(val, where, kind, pointer=True):
    if not RES_RE.match(val):
        die(f"{where}: '{val}' no parece un nombre de recurso {kind} válido")
    return ('&' + val) if pointer else val

def parse_scene(path, all_scene_names):
    fname = scene_name_of(path)
    scene_name = None
    steps = []      # (opcode, [args C], línea, label_pendiente_en_b)
    labels = {}     # nombre -> índice de step
    puzzle_tags = {}  # nombre de tag (local a la escena) -> índice en puzzle_seqs

    for lineno, raw in enumerate(open(path, encoding='utf-8'), 1):
        line = raw.split('#', 1)[0].strip()
        if not line:
            continue
        tok = line.split()
        where = f"{path}:{lineno}"

        if tok[0] == 'scene':
            scene_name = tok[1]
            if scene_name != fname:
                die(f"{where}: el nombre de escena '{scene_name}' no coincide con la ruta ('{fname}')")
            continue

        if tok[0] == 'label':
            if len(tok) != 2: die(f"{where}: label necesita un nombre")
            if tok[1] in labels: die(f"{where}: label '{tok[1]}' duplicado")
            labels[tok[1]] = len(steps)   # apunta al siguiente step
            continue

        if tok[0] == 'puzzle_sequence':
            # puzzle_sequence <tag> <spell:dir> <spell:dir> ... (2..PUZZLE_SEQ_MAX pasos)
            if len(tok) < 4: die(f"{where}: puzzle_sequence necesita un tag y al menos 2 pasos spell:dir")
            tag = tok[1]
            pairs = tok[2:]
            if len(pairs) > PUZZLE_SEQ_MAX: die(f"{where}: máximo {PUZZLE_SEQ_MAX} pasos por puzzle")
            seq = []
            for pair in pairs:
                if ':' not in pair: die(f"{where}: '{pair}' debe ser spell:direct|reversed")
                sp, direc = pair.split(':', 1)
                spell_c = 'SPELL_' + sp.upper()
                if spell_c not in valid_spells: die(f"{where}: '{spell_c}' no existe en {SPELLS_HEADER}")
                if direc not in ('direct', 'reversed'): die(f"{where}: dirección '{direc}' inválida (direct/reversed)")
                seq.append((spell_c, 1 if direc == 'reversed' else 0))
            puzzle_tags[tag] = len(puzzle_seqs)
            puzzle_seqs.append({'spells': seq, 'where': where, 'tag': f"{fname}:{tag}"})
            steps.append({'op': 'SCENE_OP_PUZZLE_SEQ', 'args': [str(puzzle_tags[tag])], 'label': None, 'where': where})
            continue

        if tok[0] == 'level':
            # level <bg_tile> <bg_map> <front_tile> <front_map> <pal> <width> <scroll_mode> <speed>
            # bg_tile/bg_map pueden ser 'none' (NULL) si el nivel no usa capa de fondo.
            if len(tok) != 9:
                die(f"{where}: level espera 8 argumentos: bg_tile bg_map front_tile front_map pal width scroll_mode speed")
            def optres(v):  # recurso o 'none' → NULL
                return 'NULL' if v == 'none' else res_ref(v, where, 'de tileset/mapa')
            # anchura: un número (800, 1440) o una constante C (SCREEN_WIDTH); se emite verbatim
            if not tok[6].isdigit() and not RES_RE.match(tok[6]):
                die(f"{where}: la anchura '{tok[6]}' no es un número ni una constante válida")
            try: int(tok[8])
            except ValueError: die(f"{where}: la velocidad '{tok[8]}' no es un número")
            if tok[7] not in SCROLL_MODES:
                die(f"{where}: modo de scroll '{tok[7]}' inválido ({', '.join(SCROLL_MODES)})")
            idx = len(scene_levels)
            scene_levels.append({
                'bgTile':   optres(tok[1]), 'bgMap':    optres(tok[2]),
                'frontTile':optres(tok[3]), 'frontMap': optres(tok[4]),
                'pal':      res_ref(tok[5], where, 'de paleta'),
                'width':    tok[6], 'scrollMode': SCROLL_MODES[tok[7]], 'scrollSpeed': tok[8],
            })
            steps.append({'op': 'SCENE_OP_LEVEL', 'args': [str(idx)], 'label': None, 'where': where})
            continue

        if tok[0] == 'palette':
            # palette <PAL_slot> <pal_resource>
            if len(tok) != 3:
                die(f"{where}: palette espera 2 argumentos: ranura (PAL0..PAL3) y recurso de paleta")
            slot = tok[1]
            if slot not in ('PAL0', 'PAL1', 'PAL2', 'PAL3'):
                die(f"{where}: ranura de paleta '{slot}' inválida (PAL0..PAL3)")
            idx = len(scene_palettes)
            scene_palettes.append(res_ref(tok[2], where, 'de paleta'))
            steps.append({'op': 'SCENE_OP_PALETTE', 'args': [slot, str(idx)], 'label': None, 'where': where})
            continue

        if tok[0] == 'item':
            # item <slot> <sprite> <pal> <x> <y> <cw> <cxo> <ch> <cyo> <depth>
            # cw/cxo/ch/cyo admiten COLLISION_DEFAULT o un número (se emiten verbatim);
            # depth es FORCE_BACKGROUND / FORCE_FOREGROUND / CALCULATE_DEPTH.
            if len(tok) != 11:
                die(f"{where}: item espera 10 argumentos: slot sprite pal x y cw cxo ch cyo depth")
            for i in (1, 5, 9):  # slot, x, y son números
                pass
            try:
                int(tok[1]); int(tok[4]); int(tok[5])
            except ValueError:
                die(f"{where}: slot/x/y de item deben ser números")
            depth = tok[10]
            if depth not in ('FORCE_BACKGROUND', 'FORCE_FOREGROUND', 'CALCULATE_DEPTH'):
                die(f"{where}: depth '{depth}' inválido (FORCE_BACKGROUND/FORCE_FOREGROUND/CALCULATE_DEPTH)")
            idx = len(scene_items)
            scene_items.append({
                'sprite': res_ref(tok[2], where, 'de sprite'),
                'pal':  tok[3], 'x': tok[4], 'y': tok[5],
                'cw':   tok[6], 'cxo': tok[7], 'ch': tok[8], 'cyo': tok[9],
                'depth': depth,
            })
            steps.append({'op': 'SCENE_OP_ITEM', 'args': [tok[1], str(idx)], 'label': None, 'where': where})
            continue

        if tok[0] not in OPS:
            die(f"{where}: op desconocido '{tok[0]}'")

        opcode, argspec = OPS[tok[0]]
        args, argi = [], 1
        pending_label = None
        dialog_set = None

        for spec in argspec:
            optional = spec.endswith('?')
            spec_base = spec.rstrip('?')
            if argi >= len(tok):
                if optional:
                    # defaults de los args opcionales
                    if spec_base == 'sound': args.append('0')
                    elif spec_base == 'direction': args.append('0')
                    continue
                die(f"{where}: '{tok[0]}' espera más argumentos ({argspec})")
            val = tok[argi]; argi += 1

            if spec_base == 'int':
                try: int(val)
                except ValueError: die(f"{where}: '{val}' no es un número")
                args.append(val)
            elif spec_base == 'verbatim':
                args.append(val)
            elif spec_base == 'dialogset':
                if val not in dialog_ids: die(f"{where}: set de diálogo '{val}' no existe en {TEXTS_CSV}")
                dialog_set = val; args.append(val)
            elif spec_base == 'dialogid':
                if val not in dialog_ids[dialog_set]:
                    die(f"{where}: id '{val}' no existe en el set '{dialog_set}' de {TEXTS_CSV}")
                args.append(val)
            elif spec_base == 'choiceset':
                if val not in choice_items: die(f"{where}: choice set '{val}' no existe en {CHOICES_CSV}")
                args.append(val)
            elif spec_base == 'label':
                pending_label = val; args.append(None)  # se resuelve en la 2ª pasada
            elif spec_base == 'hook':
                if val not in hooks: die(f"{where}: hook '{val}' no existe en {HOOKS_HEADER}")
                args.append('HOOK_' + val.upper())
            elif spec_base == 'spell':
                if val not in valid_spells: die(f"{where}: '{val}' no existe en {SPELLS_HEADER}")
                args.append(val)
            elif spec_base == 'scene':
                if val not in all_scene_names:
                    die(f"{where}: la escena '{val}' no existe en data/scenes/ ({', '.join(sorted(all_scene_names))})")
                args.append('SCENE_' + val.upper())
            elif spec_base == 'kwgoto':
                if val != 'goto': die(f"{where}: se esperaba la palabra 'goto', no '{val}'")
                # palabra clave sintáctica: no emite argumento
            elif spec_base == 'puzzletag':
                if val not in puzzle_tags:
                    die(f"{where}: el tag de puzzle '{val}' no está definido antes con puzzle_sequence")
                args.append(str(puzzle_tags[val]))
            elif spec_base == 'zone':
                if val not in valid_zones: die(f"{where}: '{val}' no existe en {SPELLS_HEADER}")
                args.append(val)
            elif spec_base == 'flag':
                if val not in FLAGS: die(f"{where}: flag '{val}' desconocido ({', '.join(FLAGS)})")
                args.append(FLAGS[val])
            elif spec_base == 'onoff':
                if val not in ('on', 'off'): die(f"{where}: se esperaba on/off, no '{val}'")
                args.append('1' if val == 'on' else '0')
            elif spec_base == 'leftright':
                # look_left(chr, bool left): left=true — verificado en emulador
                # (el arte base de los personajes mira a la derecha; flipH=true lo espeja)
                if val not in ('left', 'right'): die(f"{where}: se esperaba left/right, no '{val}'")
                args.append('1' if val == 'left' else '0')
            elif spec_base == 'sound':
                if val not in ('sound', 'silent'): die(f"{where}: se esperaba sound/silent, no '{val}'")
                args.append('1' if val == 'sound' else '0')
            elif spec_base == 'direction':
                if val not in ('direct', 'reversed'): die(f"{where}: se esperaba direct/reversed, no '{val}'")
                args.append('1' if val == 'reversed' else '0')

        if argi < len(tok):
            die(f"{where}: argumentos de más en '{line}'")

        # choice item en rango
        if tok[0] == 'choice' and int(args[1]) >= choice_items[args[0]]:
            die(f"{where}: item {args[1]} fuera de rango en '{args[0]}'")

        steps.append({'op': opcode, 'args': args, 'label': pending_label, 'where': where})

    if scene_name is None:
        die(f"{path}: falta la directiva 'scene <nombre>'")

    # 2ª pasada: resolver labels
    for st in steps:
        if st['label'] is not None:
            if st['label'] not in labels:
                die(f"{st['where']}: label '{st['label']}' no existe en la escena")
            idx = labels[st['label']]
            st['args'] = [str(idx) if a is None else a for a in st['args']]

    return {'name': scene_name, 'steps': steps}

# ---------------------------------------------------------------------------
# Generación
# ---------------------------------------------------------------------------
scene_paths = sorted(glob.glob(SCENES_GLOB))
if not scene_paths:
    die(f"no hay escenas en {SCENES_GLOB}")
all_scene_names = {scene_name_of(p) for p in scene_paths}
scenes = [parse_scene(p, all_scene_names) for p in scene_paths]

with open(HEADER_FILE, 'w', encoding='utf-8') as h:
    h.write('// Generated by tools/gen_scenes.py from data/scenes/*.scene — DO NOT EDIT\n')
    h.write('#ifndef _SCENE_DATA_H_\n#define _SCENE_DATA_H_\n\n')
    h.write('#include "scenes/scene_vm.h"\n\n')
    h.write('typedef enum {\n')
    for i, sc in enumerate(scenes):
        h.write(f"    SCENE_{sc['name'].upper()} = {i},\n")
    h.write(f'    SCENE_COUNT_TOTAL = {len(scenes)}\n')
    h.write('} SceneId;\n\n')
    h.write('extern const SceneScript scenes[];\n')
    h.write('extern const u8 scene_count;\n\n')
    h.write('s16 scene_id_by_name(const char *name); // SceneId, o -1 si no existe (para hacks/smoke)\n\n')
    h.write('extern const PuzzleSeq puzzle_seqs[]; // secuencias de puzzle (indexadas por los ops PUZZLE_*)\n')
    h.write('extern const SceneLevel   scene_levels[];   // niveles (indexados por el op LEVEL)\n')
    h.write('extern const SceneItem    scene_items[];    // items del escenario (indexados por el op ITEM)\n')
    h.write('extern const ScenePalette scene_palettes[]; // paletas a cargar (indexadas por el op PALETTE)\n')
    h.write('\n#endif // _SCENE_DATA_H_\n')

with open(SOURCE_FILE, 'w', encoding='utf-8') as c:
    c.write('// Generated by tools/gen_scenes.py from data/scenes/*.scene — DO NOT EDIT\n')
    c.write('#include <genesis.h>\n')
    c.write('#include "core/core.h"  // SCREEN_WIDTH y otras constantes usadas en las tablas de setup\n')
    c.write('#include "scenes/scenes.h"\n')
    c.write('#include "narrative/narrative.h"\n')
    c.write('#include "actors/actors.h"\n')
    c.write('#include "spells/spells.h"\n')
    c.write('#include "world/world.h"\n')
    c.write('#include "res_all.h"      // recursos rescomp (tilesets, sprites, paletas) para las tablas de setup\n\n')

    for sc in scenes:
        c.write(f"static const SceneStep {sc['name']}_steps[] = {{\n")
        for i, st in enumerate(sc['steps']):
            a = (st['args'] + ['0', '0', '0', '0'])[:4]
            c.write(f"    {{ {st['op']}, {a[0]}, {a[1]}, {a[2]}, {a[3]} }}, // {i}\n")
        c.write('};\n\n')

    c.write('const SceneScript scenes[] = {\n')
    for sc in scenes:
        c.write(f'    {{ "{sc["name"]}", {sc["name"]}_steps, {len(sc["steps"])} }},\n')
    c.write('};\n\n')
    c.write(f'const u8 scene_count = {len(scenes)};\n\n')

    c.write('const PuzzleSeq puzzle_seqs[] = {\n')
    if puzzle_seqs:
        for i, pz in enumerate(puzzle_seqs):
            spells = ', '.join(sp for sp, _ in pz['spells'])
            revs   = ', '.join(str(r) for _, r in pz['spells'])
            c.write(f"    {{ {len(pz['spells'])}, {{{spells}}}, {{{revs}}} }}, // {i}: {pz['tag']}\n")
    else:
        c.write('    { 0, {0}, {0} } // (sin puzzles definidos)\n')
    c.write('};\n\n')

    # --- Tablas laterales de setup declarativo ---
    c.write('const SceneLevel scene_levels[] = {\n')
    if scene_levels:
        for i, L in enumerate(scene_levels):
            c.write(f"    {{ {L['bgTile']}, {L['bgMap']}, {L['frontTile']}, {L['frontMap']}, "
                    f"{L['pal']}, {L['width']}, {L['scrollMode']}, {L['scrollSpeed']} }}, // {i}\n")
    else:
        c.write('    { NULL, NULL, NULL, NULL, NULL, 0, 0, 0 } // (sin niveles definidos)\n')
    c.write('};\n\n')

    c.write('const SceneItem scene_items[] = {\n')
    if scene_items:
        for i, I in enumerate(scene_items):
            c.write(f"    {{ {I['sprite']}, {I['pal']}, {I['x']}, {I['y']}, "
                    f"{I['cw']}, {I['cxo']}, {I['ch']}, {I['cyo']}, {I['depth']} }}, // {i}\n")
    else:
        c.write('    { NULL, 0, 0, 0, 0, 0, 0, 0, 0 } // (sin items definidos)\n')
    c.write('};\n\n')

    c.write('const ScenePalette scene_palettes[] = {\n')
    if scene_palettes:
        for i, p in enumerate(scene_palettes):
            c.write(f"    {{ {p} }}, // {i}\n")
    else:
        c.write('    { {0} } // (sin paletas definidas)\n')
    c.write('};\n\n')

    c.write('s16 scene_id_by_name(const char *name)    // SceneId, o -1 si no existe\n{\n')
    c.write('    for (u8 i = 0; i < scene_count; i++)\n')
    c.write('        if (strcmp(scenes[i].name, name) == 0) return i;\n')
    c.write('    return -1;\n}\n')

print('✓ Archivos generados:')
print(f'  • {HEADER_FILE}')
print(f'  • {SOURCE_FILE}')
for sc in scenes:
    print(f"    - {sc['name']}: {len(sc['steps'])} steps")
