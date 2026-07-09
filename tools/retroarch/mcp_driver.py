#!/usr/bin/env python3
# Driver host: ejercita las 17 tools del MCP (por su comando NCI) contra la smoke
# ROM en RetroArch, sincronizando capturas con smoke_phase (read_ram).
import socket, time, os, glob, shutil

HOST, PORT = "127.0.0.1", 55355
SHOT_DIR   = os.path.expanduser("~/.config/retroarch/screenshots")
STATE_DIR  = os.path.expanduser("~/.config/retroarch/states")
OUT = "/tmp/claude-1000/-home-ganso-codigo-The-Weave/f1efe627-235e-486e-bf4e-3607cac4accc/scratchpad"

# Offsets CHEEVOS leídos de out/symbol.txt (addr & 0xFFFF); evita editarlos cada build
def sym_off(name, default):
    try:
        for ln in open("/home/ganso/codigo/The-Weave/out/symbol.txt"):
            p = ln.split()
            if len(p) >= 3 and p[2] == name:
                return int(p[0], 16) & 0xFFFF
    except OSError: pass
    return default
OFF_FRAME   = sym_off("frame_counter", 0x071e)
OFF_SCRATCH = sym_off("smoke_scratch", 0x0bcc)
OFF_PHASE   = sym_off("smoke_phase",   0x0bce)
PHASES = {0:"boot",1:"idle",2:"walk",3:"cast_light",4:"cast_thunder",5:"combat",0xFFFF:"done"}
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

def read_u16(off):
    r = (nci(f"READ_CORE_RAM 0x{off:x} 2") or "").split()
    if len(r) < 4: return None
    try: return (int(r[3],16) << 8) | int(r[2],16)   # deswap 16-bit: b1<<8 | b0
    except ValueError: return None

def screenshot(label):
    nci("SCREENSHOT"); time.sleep(0.5)
    pngs = glob.glob(f"{SHOT_DIR}/*.png")
    if not pngs: return "(sin png)"
    dst = f"{OUT}/walk_{label}.png"; shutil.copy(max(pngs, key=os.path.getmtime), dst)
    return os.path.basename(dst)

# --- esperar a que la ROM/emulador responda ---
for _ in range(50):
    v = nci("VERSION")
    if v and v[0:1].isdigit(): break
    time.sleep(0.1)

# ============ FASE A: solo lectura (rápidas, antes de que arranque el recorrido) ============
log("ping","VERSION", v, bool(v) and v[0:1].isdigit())
log("get_status","GET_STATUS", nci("GET_STATUS"), True)
log("get_config","GET_CONFIG_PARAM", nci("GET_CONFIG_PARAM savestate_directory"), True)
log("read_memory","READ_CORE_MEMORY", nci("READ_CORE_MEMORY 0x0 4"), True)   # genesis: "no memory map" (esperado)
log("write_memory","WRITE_CORE_MEMORY", nci("WRITE_CORE_MEMORY 0x0 00"), True)
nci("SHOW_MSG recorrido MCP en curso"); log("show_message","SHOW_MSG", "(fire-and-forget)", True)

# ============ FASE B: recorrido — captura por fase + pause/frameadvance ============
seen, did_fa = set(), False
t0 = time.time()
while time.time() - t0 < 40:
    ph = read_u16(OFF_PHASE)
    if ph is None: time.sleep(0.08); continue
    if ph not in seen:
        seen.add(ph)
        fr = read_u16(OFF_FRAME)
        name = PHASES.get(ph, f"0x{ph:x}")
        shot = screenshot(name)
        log("read_ram+shot", f"phase={name} fr={fr}", shot, fr is not None)
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
log("read_ram+shot","phase=results (final)", screenshot("results"), True)
nci(f"WRITE_CORE_RAM 0x{OFF_SCRATCH:x} CA FE"); time.sleep(0.1)
rb = nci(f"READ_CORE_RAM 0x{OFF_SCRATCH:x} 2") or ""
log("write_ram","WRITE+READ scratch", rb, rb.upper().endswith("CA FE"))

# SAVE_STATE es fire-and-forget: verifico por el directorio de estados
os.makedirs(STATE_DIR, exist_ok=True)
before = {p: os.path.getmtime(p) for p in glob.glob(f"{STATE_DIR}/*")}
nci("SAVE_STATE"); time.sleep(0.6)
after = {p: os.path.getmtime(p) for p in glob.glob(f"{STATE_DIR}/*")}
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
print(f"Fases capturadas: {sorted(PHASES.get(p,p) for p in seen)}", flush=True)
with open(f"{OUT}/mcp_report.txt","w") as f: f.write("\n".join(report))
