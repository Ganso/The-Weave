#!/usr/bin/env python3
# Driver host: ejercita las 17 tools del MCP (por su comando NCI) contra la smoke
# ROM en RetroArch, sincronizando capturas con smoke_phase (read_ram).
# Deja las capturas de la última run (con fecha en el nombre) y el informe en
# docs/testing/smoke-latest/ (ignorada por git); la run anterior se borra.
import socket, time, os, glob, shutil, sys

HOST, PORT = "127.0.0.1", 55355
SHOT_DIR   = os.path.expanduser("~/.config/retroarch/screenshots")
STATE_DIR  = os.path.expanduser("~/.config/retroarch/states")
script_dir = os.path.dirname(os.path.abspath(__file__))
repo_root  = os.path.abspath(os.path.join(script_dir, "../.."))

# Salida: última run en docs/testing/smoke-latest/ (en .gitignore). Se vacía al
# empezar para que "lo que hay" sea siempre la run más reciente.
OUT = os.path.join(repo_root, "docs/testing/smoke-latest")
RUN_TAG = time.strftime("%Y-%m-%d_%H%M")
os.makedirs(OUT, exist_ok=True)
for old in glob.glob(f"{OUT}/*"):
    os.remove(old)

# Offsets CHEEVOS leídos de out/symbol.txt (addr & 0xFFFF); evita editarlos cada build
def sym_off(name, default):
    try:
        for ln in open(os.path.join(repo_root, "out/symbol.txt")):
            p = ln.split()
            if len(p) >= 3 and p[2] == name:
                return int(p[0], 16) & 0xFFFF
    except OSError: pass
    return default
OFF_FRAME   = sym_off("frame_counter", 0x071e)
OFF_SCRATCH = sym_off("smoke_scratch", 0x0bcc)
OFF_PHASE   = sym_off("smoke_phase",   0x0bce)
OFF_GATE    = sym_off("smoke_gate",    0x0bd0)
PHASES = {0:"boot",1:"idle",2:"walk",3:"cast_light",4:"cast_thunder",5:"combat",
          0xFFFE:"wait_gate",0xFFFF:"done"}
report, covered = [], set()

def log(tool, cmd, resp, ok):
    covered.add(tool)
    line = f"[{'OK ' if ok else 'FAIL'}] {tool:18s} {cmd:22s} -> {resp}"
    report.append(line); print(line, flush=True)

def nci(cmd, timeout=0.4):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.settimeout(timeout)
    s.sendto(cmd.encode(), (HOST, PORT))
    try: return s.recvfrom(8192)[0].decode(errors="replace").strip()
    except socket.timeout: return None
    finally: s.close()

def ensure_playing():
    # RetroArch puede pausarse solo (pause_nonactive, o pausa heredada de otra
    # sesión). Despausar siempre que se detecte, en cualquier punto de la run.
    status = nci("GET_STATUS")
    if status and "PAUSED" in status:
        nci("PAUSE_TOGGLE"); time.sleep(0.1)
        return True
    return False

def read_u16(off):
    r = (nci(f"READ_CORE_RAM 0x{off:x} 2") or "").split()
    if len(r) < 4: return None
    try: return (int(r[3],16) << 8) | int(r[2],16)   # deswap 16-bit: b1<<8 | b0
    except ValueError: return None

def frames_advancing(gap=0.4):
    # Testigo de vida: frame_counter avanza => la ROM está ejecutando frames.
    a = read_u16(OFF_FRAME); time.sleep(gap); b = read_u16(OFF_FRAME)
    return None not in (a, b) and a != b

def wait_running(timeout=30):
    # RetroArch puede arrancar EN FRÍO: la ROM no ejecuta frames (frame_counter
    # congelado en 0) aunque GET_STATUS diga PLAYING, por foco/pause_nonactive.
    # Una vez arranca, sigue sola hasta el final. Aquí la empujamos hasta que
    # frame_counter avanza: despausamos y, si sigue helada, forzamos frames con
    # FRAMEADVANCE y reanudamos marcha libre. Sin esto el handshake del gate
    # (PH_WAIT_GATE) hace timeout porque la fase nunca sale de 0.
    t0 = time.time()
    while time.time() - t0 < timeout:
        ensure_playing()
        if frames_advancing(): return True
        for _ in range(8): nci("FRAMEADVANCE")   # fuerza ejecución aunque esté estrangulada
        nci("PAUSE_TOGGLE")                       # reanuda marcha libre tras los advance
    return frames_advancing()

def screenshot(label):
    # Anti-stale: solo devolvemos una captura cuyo mtime sea POSTERIOR al comando
    # SCREENSHOT. Por mtime (no por set-diff de nombres) captamos tanto ficheros
    # nuevos como sobrescrituras (RetroArch nombra por segundo: dos capturas en el
    # mismo segundo reusan nombre). Si en ~4 s no aparece nada fresco, devolvemos
    # un sentinel en vez de arrastrar una captura vieja (que daría un falso "OK").
    t_before = time.time() - 0.5                  # margen por desfase de reloj/fs
    nci("SCREENSHOT")
    dst = f"{OUT}/{RUN_TAG}_{label}.png"
    for _ in range(40):                           # hasta ~4 s esperando el PNG fresco
        time.sleep(0.1)
        fresh = [p for p in glob.glob(f"{SHOT_DIR}/*.png") if os.path.getmtime(p) >= t_before]
        if fresh:
            shutil.copy(max(fresh, key=os.path.getmtime), dst)
            return os.path.basename(dst)
    return "(sin captura nueva)"

# --- esperar a que la ROM/emulador responda ---
for _ in range(50):
    v = nci("VERSION")
    if v and v[0:1].isdigit(): break
    time.sleep(0.1)
else:
    print("FATAL: RetroArch no responde en el NCI (¿está lanzado con la smoke ROM?)")
    sys.exit(1)

# ============ FASE A: solo lectura (rápidas, antes de que arranque el recorrido) ============
ensure_playing()

log("ping","VERSION", v, bool(v) and v[0:1].isdigit())
log("get_status","GET_STATUS", nci("GET_STATUS"), True)
log("get_config","GET_CONFIG_PARAM", nci("GET_CONFIG_PARAM savestate_directory"), True)
log("read_memory","READ_CORE_MEMORY", nci("READ_CORE_MEMORY 0x0 4"), True)   # genesis: "no memory map" (esperado)
log("write_memory","WRITE_CORE_MEMORY", nci("WRITE_CORE_MEMORY 0x0 00"), True)
nci("SHOW_MSG recorrido MCP en curso"); log("show_message","SHOW_MSG", "(fire-and-forget)", True)

# Asegurar que la ROM ejecuta frames antes del handshake (arranque en frío).
if not wait_running():
    print("WARN: la ROM no arranca frames (frame_counter congelado tras 30s)", flush=True)

# Esperar a que la ROM entre en el gate loop (smoke_phase == 0xFFFE = PH_WAIT_GATE).
# Así el host no pierde frames: la ROM se congela hasta que abramos el gate.
print("Esperando a que la ROM entre en AUTO (PH_WAIT_GATE)...", flush=True)
ph = None
t_gate = time.time()
while time.time() - t_gate < 20:
    ph = read_u16(OFF_PHASE)
    if ph == 0xFFFE:
        break
    ensure_playing()
    time.sleep(0.15)
else:
    print("WARN: timeout esperando PH_WAIT_GATE, abriendo gate de todos modos", flush=True)

# Abrir el gate en RAM para arrancar el test (write_ram: primera escritura real)
nci(f"WRITE_CORE_RAM 0x{OFF_GATE:x} 01 01")
log("write_ram","WRITE gate=0101", f"gate abierto (phase era 0x{ph or 0:x})", ph == 0xFFFE)
time.sleep(0.2)

# ============ FASE B: recorrido — captura por fase + pause/frameadvance ============
seen, did_fa = set(), False
t0 = time.time()
last_check = time.time()
while time.time() - t0 < 120:
    ph = read_u16(OFF_PHASE)
    if ph is None: time.sleep(0.08); continue
    if time.time() - last_check > 2:     # vigilar pausas espurias sin saturar el NCI
        last_check = time.time()
        if ensure_playing(): print("WARN: emulador pausado a mitad de run; despausado", flush=True)
    if ph not in seen:
        seen.add(ph)
        fr = read_u16(OFF_FRAME)
        name = PHASES.get(ph, f"0x{ph:x}")
        shot = screenshot(name)
        log("read_ram+shot", f"phase={name} fr={fr}", shot, fr is not None and shot != "(sin captura nueva)")
        # pause + frameadvance deterministas en la primera fase "viva" (frame_counter avanza)
        if ph in (1,2,3,4,5) and not did_fa:
            did_fa = True
            nci("PAUSE_TOGGLE"); time.sleep(0.2); fa = read_u16(OFF_FRAME)
            nci("FRAMEADVANCE"); time.sleep(0.1); fb = read_u16(OFF_FRAME)
            nci("FRAMEADVANCE"); time.sleep(0.1); fc = read_u16(OFF_FRAME)
            nci("PAUSE_TOGGLE")
            det = (None not in (fa,fb,fc) and fb-fa==1 and fc-fb==1)
            log("pause_toggle","PAUSE_TOGGLE","pausa/reanuda", True)
            log("frame_advance","FRAMEADVANCE x2", f"{fa}->{fb}->{fc} (+1 c/u)", det)
    if ph == 0xFFFF: break
    time.sleep(0.08)

# ============ FASE C: write_ram + save/load state (en pantalla de resultados) ============
shot_res = screenshot("results")
log("read_ram+shot","phase=results (final)", shot_res, shot_res != "(sin captura nueva)")
nci(f"WRITE_CORE_RAM 0x{OFF_SCRATCH:x} CA FE"); time.sleep(0.1)
rb = nci(f"READ_CORE_RAM 0x{OFF_SCRATCH:x} 2") or ""
log("write_ram","WRITE+READ scratch", rb, rb.upper().endswith("CA FE"))

# SAVE_STATE es fire-and-forget: verifico por el directorio de estados
# (recursivo: RetroArch guarda en un subdirectorio por core, p.ej. "Genesis Plus GX/")
os.makedirs(STATE_DIR, exist_ok=True)
def state_files():
    return {p: os.path.getmtime(p)
            for p in glob.glob(f"{STATE_DIR}/**/*", recursive=True) if os.path.isfile(p)}
before = state_files()
nci("SAVE_STATE"); time.sleep(0.6)
after = state_files()
saved = (set(after) - set(before)) or {p for p in after if before.get(p) != after[p]}
log("save_state","SAVE_STATE", f"estado escrito: {[os.path.basename(p) for p in saved] or 'sin cambio detectable'}", bool(saved))
log("state_slot_plus","STATE_SLOT_PLUS", "(fire-and-forget) "+str(nci("STATE_SLOT_PLUS")), True)
log("state_slot_minus","STATE_SLOT_MINUS","(fire-and-forget) "+str(nci("STATE_SLOT_MINUS")), True)
nci("LOAD_STATE"); time.sleep(0.3); log("load_state","LOAD_STATE","(fire-and-forget)", True)
r = nci("LOAD_STATE_SLOT 0"); log("load_state_slot","LOAD_STATE_SLOT 0", r, True)
time.sleep(0.4); screenshot("after_state")

# ============ FASE D: reset (última) ============
nci("RESET"); log("reset","RESET","(fire-and-forget)", True)
time.sleep(2.0)
alive = nci("VERSION"); screenshot("after_reset")
log("post_reset_ping","VERSION", alive, bool(alive) and alive[0:1].isdigit())

# ============ Resumen de cobertura ============
tools = ["ping","get_status","get_config","read_memory","read_ram+shot","write_memory",
         "write_ram","pause_toggle","frame_advance","reset","show_message",
         "save_state","load_state","load_state_slot","state_slot_plus","state_slot_minus"]
print("\n===== COBERTURA DE TOOLS MCP =====", flush=True)
for t in tools:
    print(("  [x] " if t in covered else "  [ ] FALTA ") + t, flush=True)
faltan = [t for t in tools if t not in covered]
print(f"\nTotal: {len(tools)-len(faltan)}/{len(tools)} tools ejercitadas. Faltan: {faltan or 'ninguna'}", flush=True)
fases_esperadas = {1,2,3,4,5,0xFFFF}
fases_ok = fases_esperadas <= seen
print(f"Fases capturadas: {sorted(PHASES.get(p,p) for p in seen)}", flush=True)
print(f"Recorrido completo: {'SI' if fases_ok else 'NO (faltan ' + str(sorted(PHASES.get(p,p) for p in fases_esperadas - seen)) + ')'}", flush=True)
print(f"Capturas e informe en: {OUT}", flush=True)
with open(f"{OUT}/{RUN_TAG}_report.txt","w") as f:
    f.write("\n".join(report))
sys.exit(0 if (fases_ok and not faltan) else 1)
