#!/bin/bash
# Script de compilación para The Weave (Mega Drive / SGDK)
#
# Pipeline: codegen (python3 en el host) → make en contenedor SGDK → backup →
# MiSTer FPGA (vía FTP + Remote API) o BlastEm como fallback.
# Salida: out/rom.bin
#
# Uso:
#   ./build-theweave.sh [build|release|full|clean] [--no-run|-n]
#
#   --no-run / -n  →  Compila (y hace backup) pero NO lanza MiSTer ni BlastEm.
#                     Pensado para pruebas automatizadas / CI / verificación de build.
#
# Configuración personal (IP/pass de MiSTer, ruta de BlastEm, SGDK local...) en
# .build-theweave.local.sh (no versionado). Las constantes de abajo son valores
# por defecto sobreescribibles por entorno o por ese fichero local.

set -e

# --- LOCALIZACIÓN ---
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Carga configuración local si existe (NO se sube a GitHub)
if [ -f "$SCRIPT_DIR/.build-theweave.local.sh" ]; then
    . "$SCRIPT_DIR/.build-theweave.local.sh"
fi

# --- CONFIGURACIÓN (constantes, sobreescribibles) ---
PROJECT_DIR="${PROJECT_DIR:-$SCRIPT_DIR}"
SGDK_IMAGE="${SGDK_IMAGE:-ghcr.io/stephane-d/sgdk:latest}"

# SGDK: por defecto el de la imagen (/sgdk). Si SGDK_LOCAL apunta a una
# instalación completa, se monta readonly y se usa en su lugar.
SGDK_LOCAL="${SGDK_LOCAL:-}"
GDK_PATH="/sgdk"
SGDK_MOUNT=""
if [ -n "$SGDK_LOCAL" ]; then
    if [ -f "$SGDK_LOCAL/lib/libmd.a" ] && [ -f "$SGDK_LOCAL/makefile.gen" ]; then
        GDK_PATH="/sgdk_local"
        SGDK_MOUNT="-v $SGDK_LOCAL:/sgdk_local:ro"
    else
        echo "⚠ SGDK_LOCAL=$SGDK_LOCAL no parece una instalación completa; usando el SGDK de la imagen."
    fi
fi

DOCKER_RUN="docker run --rm -v $PROJECT_DIR:/src $SGDK_MOUNT -u $(id -u):$(id -g) -w /src --entrypoint make $SGDK_IMAGE GDK=$GDK_PATH -f /src/Makefile"

# MiSTer FPGA (MISTER_IP vacío = sin MiSTer, va directo a BlastEm)
MISTER_IP="${MISTER_IP:-}"
MISTER_USER="${MISTER_USER:-root}"
MISTER_PASS="${MISTER_PASS:-}"
MISTER_PATH="${MISTER_PATH:-/media/fat/games/MegaDrive/TheWeave.bin}"
MISTER_API_PORT="${MISTER_API_PORT:-8182}"

# BlastEm: ruta indicada; si no se encuentra ahí, se usa 'blastem' del PATH
BLASTEM_BIN="${BLASTEM_BIN:-blastem}"

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
RED='\033[0;31m'
NC='\033[0m'

cd "$PROJECT_DIR"

# --- Parseo de argumentos ---
MODE="build"
RUN_EMULATOR=true
for arg in "$@"; do
    case "$arg" in
        --no-run|-n|norun|no-run) RUN_EMULATOR=false ;;
        clean|release|full|build) MODE="$arg" ;;
    esac
done

# --- 1. DETECCIÓN DE HARDWARE (solo si se va a ejecutar) ---
MISTER_ONLINE=false
if [ "$RUN_EMULATOR" = true ]; then
    if [ -n "$MISTER_IP" ]; then
        echo -e "${BLUE}🔍 Comprobando MiSTer ($MISTER_IP)...${NC}"
        if ping -c 1 -W 1 "$MISTER_IP" > /dev/null 2>&1; then
            echo -e "${GREEN}✅ MiSTer online. Objetivo: Hardware Real.${NC}"
            MISTER_ONLINE=true
        else
            echo -e "${YELLOW}⚠ MiSTer offline. Objetivo: Emulador BlastEm.${NC}"
        fi
    else
        echo -e "${YELLOW}⚠ MiSTer no configurado. Objetivo: Emulador BlastEm.${NC}"
    fi
else
    echo -e "${CYAN}🤖 Modo --no-run: compilación sin ejecutar (pruebas automatizadas).${NC}"
fi

# --- 2. CODEGEN (host: la imagen SGDK no trae python3) ---
if [ "$MODE" != "clean" ]; then
    echo -e "${BLUE}⚙️  Generando datos (tools/gen_*.py)...${NC}"
    python3 tools/gen_texts.py
    # Fase 5 añadirá aquí: gen_choices.py y gen_scenes.py
fi

# --- 3. COMPILACIÓN ---
case "$MODE" in
    clean)
        echo -e "${YELLOW}🧹 Limpiando proyecto...${NC}"
        rm -rf out
        $DOCKER_RUN clean
        echo -e "${GREEN}✓ Limpieza completada${NC}"
        exit 0
        ;;
    release|full)
        echo -e "${YELLOW}🔨 Compilando (release)...${NC}"
        $DOCKER_RUN clean && $DOCKER_RUN release
        ;;
    build|*)
        echo -e "${YELLOW}🔨 Compilando (incremental)...${NC}"
        $DOCKER_RUN release
        ;;
esac

# --- 4. GESTIÓN DE BINARIOS LOCALES (Backup con fecha, mantiene 5) ---
if [ -f "out/rom.bin" ]; then
    ROM_SIZE=$(du -h out/rom.bin | cut -f1)
    echo -e "${GREEN}📦 ROM generada: out/rom.bin ($ROM_SIZE)${NC}"

    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    BACKUP_NAME="out/TheWeave_$TIMESTAMP.bin"
    cp "out/rom.bin" "$BACKUP_NAME"
    echo -e "${BLUE}💾 Backup local creado: $BACKUP_NAME${NC}"

    OLD_BINS=$(ls -t out/TheWeave_*.bin 2>/dev/null | tail -n +6)
    if [ -n "$OLD_BINS" ]; then
        echo -e "${YELLOW}♻️  Limpiando backups antiguos en out/...${NC}"
        echo "$OLD_BINS" | xargs rm -f
    fi
else
    echo -e "${RED}❌ No se encontró out/rom.bin. Revisa la compilación.${NC}"
    exit 1
fi

# --- 5. EJECUCIÓN ---
if [ "$RUN_EMULATOR" = false ]; then
    echo -e "${GREEN}✓ Build completado (--no-run). Sin ejecutar emulador/MiSTer.${NC}"
    exit 0
fi

if [ "$MISTER_ONLINE" = true ]; then
    # --- RUTA MISTER ---
    echo -e "${BLUE}🚀 Subiendo a MiSTer vía FTP...${NC}"
    if curl -s --ftp-create-dirs -T "out/rom.bin" "ftp://$MISTER_IP//$MISTER_PATH" --user "$MISTER_USER:$MISTER_PASS"; then
        echo -e "${GREEN}✅ Volcado correcto.${NC}"
        echo -e "${CYAN}🎮 Lanzando vía Remote API...${NC}"
        curl -s -X POST "http://$MISTER_IP:$MISTER_API_PORT/api/launch" \
             -H "Content-Type: application/json" \
             -d "{\"path\": \"$MISTER_PATH\"}"
        echo -e "\n${GREEN}✨ ¡The Weave en MiSTer!${NC}"
    else
        echo -e "${RED}❌ Falló el FTP. ¿Está el servidor FTP activo en MiSTer?${NC}"
        exit 1
    fi
else
    # --- RUTA BLASTEM ---
    BLASTEM_CMD=""
    if [ -n "$BLASTEM_BIN" ] && [ -x "$BLASTEM_BIN" ]; then
        BLASTEM_CMD="$BLASTEM_BIN"
    elif command -v blastem >/dev/null 2>&1; then
        BLASTEM_CMD="$(command -v blastem)"
    fi
    if [ -z "$BLASTEM_CMD" ]; then
        echo -e "${RED}❌ No se encontró BlastEm (ni '$BLASTEM_BIN' ni 'blastem' en PATH)${NC}"
        exit 1
    fi
    echo -e "${CYAN}🕹 Lanzando BlastEm ($BLASTEM_CMD)...${NC}"
    "$BLASTEM_CMD" "out/rom.bin"
    echo -e "${GREEN}✨ ¡The Weave en BlastEm!${NC}"
fi
