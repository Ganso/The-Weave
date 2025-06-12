import csv
import os
import re
from collections import OrderedDict, defaultdict

CSV_FILE = 'texts.csv'
CLUSTERS_FILE = 'clusters.csv'
HEADER_FILE = 'src/texts_generated.h'
SOURCE_FILE = 'src/texts_generated.c'

# Helper functions

def enum_name(set_name):
    return ''.join(part.capitalize() for part in set_name.split('_')) + 'Id'

def prefix(id_name):
    return id_name.split('_')[0]

sets = OrderedDict()
prefixes = {}
indices = defaultdict(int)

with open(CSV_FILE, newline='', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    for row in reader:
        set_name = row['set']
        sets.setdefault(set_name, [])

        # Assign sequential index per set
        row['index'] = indices[set_name]
        indices[set_name] += 1

        # Determine prefix for auto-generated ids
        cur_prefix = prefixes.get(set_name)
        if not cur_prefix:
            id_val = row['id'].strip()
            if id_val:
                cur_prefix = id_val.split('_')[0]
            else:
                m = re.match(r'act(\d+)_dialog(\d+)', set_name)
                if m:
                    cur_prefix = f"A{m.group(1)}D{m.group(2)}"
                else:
                    cur_prefix = set_name.upper()
            prefixes[set_name] = cur_prefix

        # Auto-generate id if missing
        if not row['id'].strip():
            row['id'] = f"{cur_prefix}_{row['index']}"

        sets[set_name].append(row)

# Optional clusters file
clusters = []
if os.path.exists(CLUSTERS_FILE):
    with open(CLUSTERS_FILE, newline='', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            clusters.append(row)

with open(HEADER_FILE, 'w', encoding='utf-8') as h:
    h.write('// Auto-generated from texts.csv and clusters.csv\n')
    h.write('#ifndef TEXTS_GENERATED_H\n#define TEXTS_GENERATED_H\n\n')
    for s, rows in sets.items():
        ename = enum_name(s)
        pref = prefix(rows[0]['id'])
        h.write(f'enum {ename} {{\n')
        for i, r in enumerate(rows):
            h.write(f'    {r["id"]} = {i},\n')
        h.write(f'    {pref}_COUNT = {len(rows)}\n');
        h.write('};\n\n')
    if clusters:
        h.write('enum DialogClusterId {\n')
        for cl in clusters:
            h.write(f'    {cl["cluster"]},\n')
        h.write('    CLUSTER_COUNT\n};\n\n')
    h.write('#endif\n')

with open(SOURCE_FILE, 'w', encoding='utf-8') as c:
    c.write('// Auto-generated from texts.csv and clusters.csv\n')
    c.write('#include "globals.h"\n\n')
    for s, rows in sets.items():
        pref = prefix(rows[0]['id'])
        c.write(f'const DialogItem {s}[] = {{\n')
        for r in rows:
            c.write(f'    [{r["id"]}] = {{ {r["face"]}, {r["side"]}, {r["time"]},\n')
            es = r['es'].replace('"','\\"')
            en = r['en'].replace('"','\\"')
            c.write(f'        {{"{es}",\n         "{en}"}}}},\n')
        c.write(f'    [{pref}_COUNT] = {{ 0, false, DEFAULT_TALK_TIME, {{ NULL, NULL }} }}\n')
        c.write('};\n\n')
    if clusters:
        # Build mapping from id to index for each set
        id_index = {s: {r['id']: i for i, r in enumerate(rows)} for s, rows in sets.items()}

        # Organize clusters by set to determine ranges
        by_set = defaultdict(list)
        for cl in clusters:
            by_set[cl['set']].append(cl)

        for s in by_set:
            by_set[s].sort(key=lambda x: id_index[s][x['start']])
            for i, cl in enumerate(by_set[s]):
                start = id_index[s][cl['start']]
                end = id_index[s][by_set[s][i+1]['start']] if i + 1 < len(by_set[s]) else len(sets[s])
                cl['start_idx'] = start
                cl['end_idx'] = end

        # Generate cluster arrays
        for cl in clusters:
            arrname = cl['cluster'].lower()
            c.write(f'static const DialogItem {arrname}[] = {{\n')
            for r in sets[cl['set']][cl['start_idx']:cl['end_idx']]:
                c.write(f'    {{ {r["face"]}, {r["side"]}, {r["time"]},\n')
                es = r['es'].replace('"','\\"')
                en = r['en'].replace('"','\\"')
                c.write(f'        {{"{es}",\n         "{en}"}}}},\n')
            c.write('    { 0, false, DEFAULT_TALK_TIME, { NULL, NULL } }\n')
            c.write('};\n\n')

        c.write('const DialogCluster dialog_clusters[] = {\n')
        for cl in clusters:
            arrname = cl['cluster'].lower()
            c.write(f'    [{cl["cluster"]}] = {{ {arrname} }},\n')
        c.write('};\n')

