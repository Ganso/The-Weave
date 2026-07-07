#!/bin/bash
# smoke-test.sh — test desatendido de The Weave (nivel 1, sin GDB)
#
# Compila la smoke ROM en modo AUTORUN (-DHACK_SMOKE_BUILD -DSMOKE_AUTORUN),
# la ejecuta en BlastEm capturando su stdout (los dprintf del juego salen por
# KDebug), espera la línea "SMOKE RESULT: n/m PASS" y devuelve exit code 0/1.
#
# Uso:  ./tools/smoke-test.sh [--keep-rom]
#   --keep-rom  no borra out/smoke_autorun.bin al terminar
#
# Cubre los casos automáticos (CHECK de canUse + CAST con duración). Las
# escenas (SCENE) requieren jugador: usa ./build-theweave.sh smoke para ellas.
#
# EJECUTAR DESDE TU SESIÓN GRÁFICA (terminal normal): BlastEm abre su ventana
# ~15-20 s y el script la cierra solo al leer el resultado. No hay modo
# headless fiable en BlastEm 0.6.3 (probado: SDL dummy no da GL; con render
# software la emulación se para; bajo Xvfb falla el vsync y se cuelga).
#
# NOTA: deja out/ con objetos de smoke; el siguiente build normal debe ser
# './build-theweave.sh release' (hace clean).

set -e
cd "$(dirname "$0")/.."

# Config compartida con build-theweave.sh
if [ -f ".build-theweave.local.sh" ]; then . ".build-theweave.local.sh"; fi
SGDK_IMAGE="${SGDK_IMAGE:-ghcr.io/stephane-d/sgdk:latest}"
BLASTEM_BIN="${BLASTEM_BIN:-blastem}"
TIMEOUT="${SMOKE_TIMEOUT:-120}"   # segundos máximos de emulación

GREEN='\033[0;32m'; RED='\033[0;31m'; YELLOW='\033[1;33m'; NC='\033[0m'

KEEP_ROM=false
[ "$1" = "--keep-rom" ] && KEEP_ROM=true

# --- 1. Codegen + build con autorun ---
echo -e "${YELLOW}⚙️  Codegen...${NC}"
python3 tools/gen_texts.py >/dev/null
python3 tools/gen_choices.py >/dev/null
python3 tools/gen_scenes.py >/dev/null

echo -e "${YELLOW}🔨 Compilando smoke ROM (autorun)...${NC}"
DOCKER="docker run --rm -v $PWD:/src -u $(id -u):$(id -g) -w /src --entrypoint make $SGDK_IMAGE GDK=/sgdk -f /src/Makefile"
$DOCKER clean >/dev/null
$DOCKER release "EXTRA_FLAGS=-DHACK_SMOKE_BUILD -DSMOKE_AUTORUN" >/dev/null
mv out/rom.bin out/smoke_autorun.bin

# --- 2. Ejecutar BlastEm capturando KDebug ---
BLASTEM_CMD="$BLASTEM_BIN"
command -v "$BLASTEM_CMD" >/dev/null 2>&1 || { echo -e "${RED}❌ BlastEm no encontrado ($BLASTEM_CMD)${NC}"; exit 1; }

LOG="$(mktemp /tmp/theweave-smoke.XXXXXX.log)"
echo -e "${YELLOW}🕹  Ejecutando en BlastEm (máx ${TIMEOUT}s)...${NC}"
# stdbuf: sin tty, BlastEm bufferiza stdout y las líneas KDebug no llegarían al log
if command -v stdbuf >/dev/null 2>&1; then
    stdbuf -oL -eL "$BLASTEM_CMD" out/smoke_autorun.bin > "$LOG" 2>&1 &
else
    "$BLASTEM_CMD" out/smoke_autorun.bin > "$LOG" 2>&1 &
fi
EMU_PID=$!

RESULT_LINE=""
for ((i = 0; i < TIMEOUT; i++)); do
    if ! kill -0 "$EMU_PID" 2>/dev/null; then break; fi   # el emulador murió
    RESULT_LINE="$(grep -o 'SMOKE RESULT: [0-9]*/[0-9]* PASS' "$LOG" | head -1 || true)"
    [ -n "$RESULT_LINE" ] && break
    sleep 1
done
kill "$EMU_PID" 2>/dev/null || true
wait "$EMU_PID" 2>/dev/null || true

# --- 3. Informe ---
echo "---- casos ----"
grep -o 'SMOKE: .*' "$LOG" || true
echo "---------------"

$KEEP_ROM || rm -f out/smoke_autorun.bin

if [ -z "$RESULT_LINE" ]; then
    echo -e "${RED}❌ Sin resultado (timeout o cuelgue). Log conservado: $LOG${NC}"
    tail -5 "$LOG" 2>/dev/null
    exit 1
fi
rm -f "$LOG"

PASS="$(echo "$RESULT_LINE" | grep -o '[0-9]*/[0-9]*')"
if [ "${PASS%/*}" = "${PASS#*/}" ]; then
    echo -e "${GREEN}✅ SMOKE $PASS PASS${NC}"
    exit 0
else
    echo -e "${RED}❌ SMOKE $PASS PASS — hay fallos${NC}"
    exit 1
fi
