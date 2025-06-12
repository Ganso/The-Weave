#!/usr/bin/env python3
# generate_texts.py – convierte texts.csv (+ clusters.csv) en texts_generated.{h,c}
import csv
import os
import re
from collections import OrderedDict, defaultdict

CSV_FILE      = 'texts.csv'
CLUSTERS_FILE = 'clusters.csv'
HEADER_FILE   = 'src/texts_generated.h'
SOURCE_FILE   = 'src/texts_generated.c'

# ------------------------------------------------------------
# Utilidades
# ------------------------------------------------------------
def enum_name(set_name: str) -> str:
    """Devuelve Act1Dialog1Id, SystemDialogId, …"""
    return ''.join(part.capitalize() for part in set_name.split('_')) + 'Id'

def prefix(id_name: str) -> str:
    """SYSTEM_DIALOG → SYSMSG, ACT1_DIALOG1 → A1D1, etc."""
    return id_name.split('_')[0]

# ------------------------------------------------------------
# 1. Leer texts.csv
# ------------------------------------------------------------
sets      = OrderedDict()           # {set_name: [rows]}
prefixes  = {}                      # {set_name: 'SYSMSG'}
indices   = defaultdict(int)        # índice incremental por set

with open(CSV_FILE, newline='', encoding='utf-8') as f:
    for row in csv.DictReader(f):
        set_name          = row['set']
        sets.setdefault(set_name, [])

        # Índice secuencial dentro del set
        row['index']      = indices[set_name]
        indices[set_name] += 1

        # Prefijo para IDs auto­generados
        cur_prefix = prefixes.get(set_name)
        if not cur_prefix:
            id_val = row['id'].strip()
            if id_val:
                cur_prefix = id_val.split('_')[0]
            else:
                # Ajustar prefijado automático (act1_dialog4 → A1D4_0…)
                m = re.match(r'act(\d+)_dialog(\d+)', set_name)
                cur_prefix = f"A{m.group(1)}D{m.group(2)}" if m else set_name.upper()
            prefixes[set_name] = cur_prefix

        # Auto-genera id si está vacío
        if not row['id'].strip():
            row['id'] = f"{cur_prefix}_{row['index']}"

        sets[set_name].append(row)

# ------------------------------------------------------------
# 2. Leer clusters.csv (opcional)
# ------------------------------------------------------------
clusters = []
if os.path.exists(CLUSTERS_FILE):
    with open(CLUSTERS_FILE, newline='', encoding='utf-8') as f:
        clusters = list(csv.DictReader(f))

# ------------------------------------------------------------
# 3. Generar texts_generated.h
# ------------------------------------------------------------
with open(HEADER_FILE, 'w', encoding='utf-8') as h:
    h.write('// Auto-generated from texts.csv and clusters.csv – DO NOT EDIT\n')
    h.write('#ifndef TEXTS_GENERATED_H\n#define TEXTS_GENERATED_H\n\n')

    # 3a. Enum de IDs por set
    for s, rows in sets.items():
        h.write(f'enum {enum_name(s)} {{\n')
        for i, r in enumerate(rows):
            h.write(f'    {r["id"]} = {i},\n')
        h.write(f'    {prefix(rows[0]["id"])}_COUNT = {len(rows)}\n')
        h.write('};\n\n')

    # 3b. Enum de índices de set (SYSTEM_DIALOG = 0, …)
    h.write('enum DialogSetId {\n')
    for i, s in enumerate(sets):
        h.write(f'    {s.upper()},  // {i}\n')
    h.write('    DIALOG_SET_COUNT\n};\n\n')

    # 3c. Enum de clusters (si existen)
    if clusters:
        h.write('enum DialogClusterId {\n')
        for cl in clusters:
            h.write(f'    {cl["cluster"]},\n')
        h.write('    CLUSTER_COUNT\n};\n\n')

    h.write('#endif // TEXTS_GENERATED_H\n')

# ------------------------------------------------------------
# 4. Generar texts_generated.c
# ------------------------------------------------------------
with open(SOURCE_FILE, 'w', encoding='utf-8') as c:
    c.write('// Auto-generated from texts.csv and clusters.csv – DO NOT EDIT\n')
    c.write('#include "globals.h"\n#include "texts_generated.h"\n\n')

    # 4a. Arrays de cada set ---------------------------------------------------
    for s, rows in sets.items():
        pref = prefix(rows[0]['id'])
        c.write(f'const DialogItem {s}[] = {{\n')
        for r in rows:
            es = r["es"].replace('"', '\\"')
            en = r["en"].replace('"', '\\"')
            c.write(f'    [{r["id"]}] = {{ {r["face"]}, {r["side"]}, {r["time"]},\n')
            c.write(f'        {{"{es}",\n         "{en}"}} }},\n')
        # Terminador
        c.write(f'    [{pref}_COUNT] = {{ 0, false, DEFAULT_TALK_TIME, {{ NULL, NULL }} }}\n')
        c.write('};\n\n')

    # 4b. Clusters -------------------------------------------------------------
    if clusters:
        # Mapeo id→índice por set
        id_index = {s: {r["id"]: i for i, r in enumerate(rows)}
                    for s, rows in sets.items()}

        # Agrupar y ordenar clusters por set para calcular rangos
        by_set = defaultdict(list)
        for cl in clusters:
            by_set[cl["set"]].append(cl)

        for s in by_set:
            by_set[s].sort(key=lambda x: id_index[s][x["start"]])
            for i, cl in enumerate(by_set[s]):
                start = id_index[s][cl["start"]]
                end   = id_index[s][by_set[s][i+1]["start"]] if i+1 < len(by_set[s]) else len(sets[s])
                cl["start_idx"], cl["end_idx"] = start, end

        # Arrays estáticos
        for cl in clusters:
            arr = cl["cluster"].lower()
            c.write(f'static const DialogItem {arr}[] = {{\n')
            for r in sets[cl["set"]][cl["start_idx"]:cl["end_idx"]]:
                es = r["es"].replace('"', '\\"')
                en = r["en"].replace('"', '\\"')
                c.write(f'    {{ {r["face"]}, {r["side"]}, {r["time"]},\n')
                c.write(f'        {{"{es}",\n         "{en}"}} }},\n')
            c.write('    { 0, false, DEFAULT_TALK_TIME, { NULL, NULL } }\n')
            c.write('};\n\n')

        # Tabla maestro de clusters
        c.write('const DialogCluster dialog_clusters[] = {\n')
        for cl in clusters:
            c.write(f'    [{cl["cluster"]}] = {{ {cl["cluster"].lower()} }},\n')
        c.write('};\n\n')

    # 4c. Tabla maestro de sets -------------------------------------
    c.write('const DialogItem *dialogs[] = {\n')
    for i, s in enumerate(sets):
        c.write(f'    {s},  // {i}\n')
    c.write('};\n')

print('✓ Archivos generados:')
print(f'  • {HEADER_FILE}')
print(f'  • {SOURCE_FILE}')
